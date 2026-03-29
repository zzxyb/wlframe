#include "wlf/image/wlf_webp_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <webp/decode.h>
#include <webp/demux.h>
#include <webp/encode.h>
#include <webp/mux.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
	FILE *fp = fopen(path, "rb");
	if (!fp) {
		wlf_log_errno(WLF_ERROR, "Open %s failed!", path);
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	if (size <= 0) {
		fclose(fp);
		return NULL;
	}

	uint8_t *data = malloc((size_t)size);
	if (!data) {
		fclose(fp);
		return NULL;
	}

	size_t read_size = fread(data, 1, (size_t)size, fp);
	fclose(fp);
	if (read_size != (size_t)size) {
		free(data);
		return NULL;
	}

	*out_size = (size_t)size;
	return data;
}

static int write_file(const char *path, const uint8_t *data, size_t size) {
	FILE *fp = fopen(path, "wb");
	if (!fp) {
		wlf_log_errno(WLF_ERROR, "Cannot open %s for writing!", path);
		return -1;
	}
	size_t written = fwrite(data, 1, size, fp);
	fclose(fp);
	if (written != size) {
		wlf_log(WLF_ERROR, "Failed to write full WebP data to %s", path);
		return -1;
	}
	return 0;
}

static bool image_save(struct wlf_image *image, const char *filename) {
	if (image == NULL || image->data == NULL || filename == NULL) {
		wlf_log(WLF_ERROR, "Invalid WebP image or filename");
		return false;
	}
	if (image->bit_depth != 0 && image->bit_depth != WLF_IMAGE_BIT_DEPTH_8) {
		wlf_log(WLF_ERROR, "WebP only supports 8-bit channels, got %d", image->bit_depth);
		return false;
	}

	struct wlf_webp_image *webp_image = wlf_webp_image_from_image(image);
	if (webp_image->ani_info && webp_image->ani_info->frame_count > 1) {
		struct wlf_webp_animation_info *anim = webp_image->ani_info;
		WebPAnimEncoderOptions enc_opts;
		WebPAnimEncoder *enc = NULL;
		WebPConfig config;
		WebPData output = { 0 };
		int ret = false;

		if (!WebPAnimEncoderOptionsInit(&enc_opts)) {
			wlf_log(WLF_ERROR, "WebPAnimEncoderOptionsInit: version mismatch");
			return false;
		}
		enc_opts.anim_params.loop_count = anim->loop_count;
		enc_opts.anim_params.bgcolor = anim->bgcolor;

		enc = WebPAnimEncoderNew(anim->canvas_w, anim->canvas_h, &enc_opts);
		if (!enc) {
			wlf_log(WLF_ERROR, "WebPAnimEncoderNew failed");
			return false;
		}

		if (!WebPConfigInit(&config)) {
			wlf_log(WLF_ERROR, "WebPConfigInit: version mismatch");
			WebPAnimEncoderDelete(enc);
			return false;
		}
		config.lossless = 0;
		config.quality = 90.0f;
		config.method = 4;
		if (!WebPValidateConfig(&config)) {
			wlf_log(WLF_ERROR, "WebPValidateConfig failed");
			WebPAnimEncoderDelete(enc);
			return false;
		}

		for (int i = 0; i < anim->frame_count; i++) {
			struct wlf_webp_frame *f = &anim->frames[i];
			WebPPicture pic;

			if (!WebPPictureInit(&pic)) {
				wlf_log(WLF_ERROR, "WebPPictureInit: version mismatch");
				goto anim_out;
			}
			pic.width = f->width;
			pic.height = f->height;
			pic.use_argb = 1;

			if (!WebPPictureImportRGBA(&pic, f->pixels, f->width * 4)) {
				wlf_log(WLF_ERROR, "WebPPictureImportRGBA failed at frame %d", i);
				WebPPictureFree(&pic);
				goto anim_out;
			}

			if (!WebPAnimEncoderAdd(enc, &pic, f->timestamp, &config)) {
				wlf_log(WLF_ERROR, "WebPAnimEncoderAdd failed at frame %d: %s",
					i, WebPAnimEncoderGetError(enc));
				WebPPictureFree(&pic);
				goto anim_out;
			}
			WebPPictureFree(&pic);
		}

		{
			struct wlf_webp_frame *last = &anim->frames[anim->frame_count - 1];
			int final_ts = last->timestamp;
			if (anim->frame_count > 1) {
				int gap = last->timestamp - anim->frames[anim->frame_count - 2].timestamp;
				final_ts = last->timestamp + gap;
			} else {
				final_ts = 100;
			}
			if (!WebPAnimEncoderAdd(enc, NULL, final_ts, NULL)) {
				wlf_log(WLF_ERROR, "WebPAnimEncoderAdd(flush) failed: %s",
					WebPAnimEncoderGetError(enc));
				goto anim_out;
			}
		}

		if (!WebPAnimEncoderAssemble(enc, &output)) {
			wlf_log(WLF_ERROR, "WebPAnimEncoderAssemble failed: %s",
				WebPAnimEncoderGetError(enc));
			goto anim_out;
		}

		if (write_file(filename, output.bytes, output.size) == 0) {
			ret = true;
		}

anim_out:
		WebPDataClear(&output);
		WebPAnimEncoderDelete(enc);
		return ret;
	}

	const float quality = 90.0f;
	uint8_t *output = NULL;
	size_t output_size = 0;
	int channels = wlf_image_get_channels(image);
	int stride = image->stride ? (int)image->stride : (int)(image->width * channels);

	uint8_t *convert_buf = NULL;

	switch (image->format) {
		case WLF_COLOR_TYPE_RGBA:
			output_size = WebPEncodeRGBA(image->data, image->width, image->height,
				stride, quality, &output);
			break;
		case WLF_COLOR_TYPE_RGB:
			output_size = WebPEncodeRGB(image->data, image->width, image->height,
				stride, quality, &output);
			break;
		case WLF_COLOR_TYPE_GRAY: {
			size_t buf_size = (size_t)image->width * image->height * 3;
			convert_buf = malloc(buf_size);
			if (convert_buf == NULL) {
				wlf_log(WLF_ERROR, "Failed to allocate RGB conversion buffer");
				return false;
			}
			for (uint32_t i = 0; i < image->width * image->height; i++) {
				uint8_t v = image->data[i];
				convert_buf[i * 3 + 0] = v;
				convert_buf[i * 3 + 1] = v;
				convert_buf[i * 3 + 2] = v;
			}
			output_size = WebPEncodeRGB(convert_buf, image->width, image->height,
				(int)(image->width * 3), quality, &output);
			break;
		}
		case WLF_COLOR_TYPE_GRAY_ALPHA: {
			size_t buf_size = (size_t)image->width * image->height * 4;
			convert_buf = malloc(buf_size);
			if (convert_buf == NULL) {
				wlf_log(WLF_ERROR, "Failed to allocate RGBA conversion buffer");
				return false;
			}
			for (uint32_t i = 0; i < image->width * image->height; i++) {
				uint8_t g = image->data[i * 2 + 0];
				uint8_t a = image->data[i * 2 + 1];
				convert_buf[i * 4 + 0] = g;
				convert_buf[i * 4 + 1] = g;
				convert_buf[i * 4 + 2] = g;
				convert_buf[i * 4 + 3] = a;
			}
			output_size = WebPEncodeRGBA(convert_buf, image->width, image->height,
				(int)(image->width * 4), quality, &output);
			break;
		}
		default:
			wlf_log(WLF_ERROR, "Unsupported WebP format: %d", image->format);
			return false;
	}

	free(convert_buf);

	if (output_size == 0 || output == NULL) {
		wlf_log(WLF_ERROR, "WebP encoding failed");
		return false;
	}

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Cannot open %s for writing!", filename);
		WebPFree(output);
		return false;
	}

	size_t written = fwrite(output, 1, output_size, fp);
	fclose(fp);
	WebPFree(output);

	if (written != output_size) {
		wlf_log(WLF_ERROR, "Failed to write full WebP data to %s", filename);
		return false;
	}

	return true;
}

static bool image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	(void)enable_16_bit;
	size_t file_size = 0;
	uint8_t *file_data = read_file(filename, &file_size);
	if (!file_data) {
		return false;
	}

	WebPBitstreamFeatures features;
	if (WebPGetFeatures(file_data, (size_t)file_size, &features) != VP8_STATUS_OK) {
		wlf_log(WLF_ERROR, "Invalid WebP bitstream: %s", filename);
		free(file_data);
		return false;
	}

	if (features.has_animation) {
		WebPData webp_data = { 0 };
		WebPAnimDecoderOptions dec_opts;
		WebPAnimDecoder *dec = NULL;
		WebPAnimInfo info;
		struct wlf_webp_animation_info *anim = NULL;
		int capacity = 0;
		int frame_stride = 0;
		int ret = false;

		webp_data.bytes = file_data;
		webp_data.size = (size_t)file_size;

		if (!WebPAnimDecoderOptionsInit(&dec_opts)) {
			wlf_log(WLF_ERROR, "WebPAnimDecoderOptionsInit: version mismatch");
			goto anim_out;
		}
		dec_opts.color_mode = MODE_RGBA;

		dec = WebPAnimDecoderNew(&webp_data, &dec_opts);
		if (!dec) {
			wlf_log(WLF_ERROR, "WebPAnimDecoderNew failed: %s", filename);
			goto anim_out;
		}

		if (!WebPAnimDecoderGetInfo(dec, &info)) {
			wlf_log(WLF_ERROR, "WebPAnimDecoderGetInfo failed: %s", filename);
			goto anim_out;
		}

		anim = calloc(1, sizeof(*anim));
		if (!anim) {
			goto anim_out;
		}
		anim->canvas_w = (int)info.canvas_width;
		anim->canvas_h = (int)info.canvas_height;
		anim->loop_count = (int)info.loop_count;
		anim->bgcolor = info.bgcolor;

		capacity = (int)info.frame_count ? (int)info.frame_count : 1;
		anim->frames = calloc((size_t)capacity, sizeof(*anim->frames));
		if (!anim->frames) {
			goto anim_out;
		}

		frame_stride = anim->canvas_w * 4;
		anim->_pixbuf = malloc((size_t)capacity * anim->canvas_h * frame_stride);
		if (!anim->_pixbuf) {
			goto anim_out;
		}

		while (WebPAnimDecoderHasMoreFrames(dec)) {
			uint8_t *dec_buf = NULL;
			int ts = 0;
			int idx = anim->frame_count;
			uint8_t *dst = NULL;

			if (!WebPAnimDecoderGetNext(dec, &dec_buf, &ts)) {
				wlf_log(WLF_ERROR, "WebPAnimDecoderGetNext failed at frame %d", idx);
				goto anim_out;
			}

			if (idx >= capacity) {
				int new_cap = capacity * 2;
				void *tmp_frames = realloc(anim->frames, (size_t)new_cap * sizeof(*anim->frames));
				void *tmp_buf = realloc(anim->_pixbuf,
					(size_t)new_cap * anim->canvas_h * frame_stride);
				if (!tmp_frames || !tmp_buf) {
					goto anim_out;
				}
				anim->frames = tmp_frames;
				anim->_pixbuf = tmp_buf;
				for (int i = 0; i < idx; i++) {
					anim->frames[i].pixels =
						(uint8_t *)anim->_pixbuf +
						(size_t)(i * anim->canvas_h * frame_stride);
				}
				capacity = new_cap;
			}

			dst = anim->_pixbuf + (size_t)(idx * anim->canvas_h * frame_stride);
			memcpy(dst, dec_buf, (size_t)(anim->canvas_h * frame_stride));
			anim->frames[idx].pixels = dst;
			anim->frames[idx].timestamp = ts;
			anim->frames[idx].width = anim->canvas_w;
			anim->frames[idx].height = anim->canvas_h;
			anim->frame_count++;
		}

		image->width = (uint32_t)anim->canvas_w;
		image->height = (uint32_t)anim->canvas_h;
		image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
		image->format = WLF_COLOR_TYPE_RGBA;
		image->has_alpha_channel = true;
		image->is_opaque = false;
		image->stride = (uint32_t)(anim->canvas_w * 4);
		image->data = anim->frames[0].pixels;

		wlf_webp_image_from_image(image)->ani_info = anim;
		ret = true;

anim_out:
		WebPAnimDecoderDelete(dec);
		free(file_data);
		if (!ret) {
			if (anim) {
				free(anim->frames);
				free(anim->_pixbuf);
				free(anim);
			}
		}
		return ret;
	}

	int width = features.width;
	int height = features.height;
	int channels = features.has_alpha ? 4 : 3;
	size_t stride = (size_t)width * (size_t)channels;
	size_t buf_size = stride * (size_t)height;

	image->data = malloc(buf_size);
	if (image->data == NULL) {
		free(file_data);
		return false;
	}

	uint8_t *decoded = NULL;
	if (features.has_alpha) {
		decoded = WebPDecodeRGBAInto(file_data, (size_t)file_size,
			image->data, buf_size, (int)stride);
		image->format = WLF_COLOR_TYPE_RGBA;
		image->has_alpha_channel = true;
		image->is_opaque = false;
	} else {
		decoded = WebPDecodeRGBInto(file_data, (size_t)file_size,
			image->data, buf_size, (int)stride);
		image->format = WLF_COLOR_TYPE_RGB;
		image->has_alpha_channel = false;
		image->is_opaque = true;
	}

	free(file_data);

	if (decoded == NULL) {
		free(image->data);
		image->data = NULL;
		return false;
	}

	image->width = (uint32_t)width;
	image->height = (uint32_t)height;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = (uint32_t)stride;

	return true;
}

static void image_destroy(struct wlf_image *image) {
	struct wlf_webp_image *webp_image = wlf_webp_image_from_image(image);
	if (webp_image->ani_info) {
		free(webp_image->ani_info->frames);
		free(webp_image->ani_info->_pixbuf);
		free(webp_image->ani_info);
		webp_image->ani_info = NULL;
		webp_image->base.data = NULL;
		return;
	}
	free(webp_image->base.data);
	webp_image->base.data = NULL;
}

static const struct wlf_image_impl image_impl = {
	.save = image_save,
	.load = image_load,
	.destroy = image_destroy,
};

struct wlf_webp_image *wlf_webp_image_create(void) {
	struct wlf_webp_image *image = malloc(sizeof(struct wlf_webp_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_webp_image");
		return NULL;
	}

	wlf_image_init(&image->base, &image_impl, 0, 0, 0);
	image->base.image_type = WLF_IMAGE_TYPE_WEBP;
	image->ani_info = NULL;

	return image;
}

bool wlf_image_is_webp(const struct wlf_image *image) {
	return image->impl == &image_impl;
}

struct wlf_webp_image *wlf_webp_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &image_impl);
	struct wlf_webp_image *image = wlf_container_of(wlf_image, image, base);

	return image;
}

bool file_is_webp(const char *file_name) {
	FILE *fp = fopen(file_name, "rb");
	if (!fp) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	if (size <= 0) {
		fclose(fp);
		return false;
	}

	uint8_t *data = malloc(size);
	if (!data) {
		fclose(fp);
		return false;
	}

	fread(data, 1, size, fp);
	fclose(fp);
	
	int width, height;
	int ok = WebPGetInfo(data, size, &width, &height);

	free(data);

	return ok;
}
