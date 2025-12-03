#include "wlf/image/wlf_jpeg_image.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <setjmp.h>

#include <jpeglib.h>
#include <jerror.h>

/**
 * @brief Error handling structure for JPEG operations.
 */
struct wlf_jpeg_error_mgr {
	struct jpeg_error_mgr pub;  /**< Public error manager */
	jmp_buf setjmp_buffer;      /**< Recovery point for errors */
};

/**
 * @brief Custom error exit function for JPEG library.
 * @param cinfo Pointer to the JPEG common info structure.
 * @note This function handles JPEG library errors by logging the error
 *       message and using longjmp to return control to the error handling
 *       code. This prevents the JPEG library from calling exit() directly.
 */
static void wlf_jpeg_error_exit(j_common_ptr cinfo) {
	struct wlf_jpeg_error_mgr *err = (struct wlf_jpeg_error_mgr *)cinfo->err;
	char jpeg_last_error_msg[JMSG_LENGTH_MAX];
	(*(cinfo->err->format_message))(cinfo, jpeg_last_error_msg);
	wlf_log(WLF_ERROR, "JPEG error: %s", jpeg_last_error_msg);

	longjmp(err->setjmp_buffer, 1);
}

/**
 * @brief Save a wlf_image as a JPEG file.
 * @param image Pointer to the wlf_image structure to save.
 * @param filename Path to the output JPEG file.
 * @return true on success, false on failure.
 * @note This function converts the image data to JPEG format using
 *       the current JPEG options (quality, subsampling, progressive, etc.).
 *       RGBA and GRAY_ALPHA images are automatically converted to RGB
 *       and GRAY respectively, as JPEG doesn't support transparency.
 */
static bool jpeg_image_save(struct wlf_image *image, const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Cannot open %s for writing!", filename);
		return false;
	}

	struct jpeg_compress_struct cinfo;
	struct wlf_jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = wlf_jpeg_error_exit;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_compress(&cinfo);
		fclose(fp);
		return false;
	}

	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_from_image(image);
	assert(jpeg_image != NULL);

	cinfo.image_width = image->width;
	cinfo.image_height = image->height;

	switch (image->format) {
		case WLF_COLOR_TYPE_RGB:
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			break;
		case WLF_COLOR_TYPE_RGBA:
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;
			wlf_log(WLF_INFO, "JPEG doesn't support alpha channel, converting RGBA to RGB");
			break;
		case WLF_COLOR_TYPE_GRAY:
			cinfo.input_components = 1;
			cinfo.in_color_space = JCS_GRAYSCALE;
			break;
		case WLF_COLOR_TYPE_GRAY_ALPHA:
			cinfo.input_components = 1;
			cinfo.in_color_space = JCS_GRAYSCALE;
			wlf_log(WLF_INFO, "JPEG doesn't support alpha channel, converting grayscale+alpha to grayscale");
			break;
		default:
			wlf_log(WLF_ERROR, "Unsupported image format for JPEG: %d", image->format);
			jpeg_destroy_compress(&cinfo);
			fclose(fp);
			return false;
	}

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, jpeg_image->options.quality, TRUE);

	if (jpeg_image->options.progressive) {
		jpeg_simple_progression(&cinfo);
	}

	if (jpeg_image->options.optimize) {
		cinfo.optimize_coding = TRUE;
	}

	if (jpeg_image->options.arithmetic) {
		cinfo.arith_code = TRUE;
	}

	switch (jpeg_image->options.subsampling) {
		case WLF_JPEG_SUBSAMPLING_444:
			for (int i = 0; i < cinfo.num_components; i++) {
				cinfo.comp_info[i].h_samp_factor = 1;
				cinfo.comp_info[i].v_samp_factor = 1;
			}
			break;
		case WLF_JPEG_SUBSAMPLING_422:
			if (cinfo.num_components >= 3) {
				cinfo.comp_info[0].h_samp_factor = 2; /* Y */
				cinfo.comp_info[0].v_samp_factor = 1;
				cinfo.comp_info[1].h_samp_factor = 1; /* Cb */
				cinfo.comp_info[1].v_samp_factor = 1;
				cinfo.comp_info[2].h_samp_factor = 1; /* Cr */
				cinfo.comp_info[2].v_samp_factor = 1;
			}
			break;
		case WLF_JPEG_SUBSAMPLING_420:
			if (cinfo.num_components >= 3) {
				cinfo.comp_info[0].h_samp_factor = 2; /* Y */
				cinfo.comp_info[0].v_samp_factor = 2;
				cinfo.comp_info[1].h_samp_factor = 1; /* Cb */
				cinfo.comp_info[1].v_samp_factor = 1;
				cinfo.comp_info[2].h_samp_factor = 1; /* Cr */
				cinfo.comp_info[2].v_samp_factor = 1;
			}
			break;
		case WLF_JPEG_SUBSAMPLING_411:
			if (cinfo.num_components >= 3) {
				cinfo.comp_info[0].h_samp_factor = 4; /* Y */
				cinfo.comp_info[0].v_samp_factor = 1;
				cinfo.comp_info[1].h_samp_factor = 1; /* Cb */
				cinfo.comp_info[1].v_samp_factor = 1;
				cinfo.comp_info[2].h_samp_factor = 1; /* Cr */
				cinfo.comp_info[2].v_samp_factor = 1;
			}
			break;
	}

	jpeg_start_compress(&cinfo, TRUE);
	JSAMPROW row_buffer = NULL;
	bool need_conversion = (image->format == WLF_COLOR_TYPE_RGBA || image->format == WLF_COLOR_TYPE_GRAY_ALPHA);
	if (need_conversion) {
		row_buffer = malloc(image->width * cinfo.input_components);
		if (row_buffer == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate row buffer");
			jpeg_destroy_compress(&cinfo);
			fclose(fp);
			return false;
		}
	}

	JSAMPROW row_pointer[1];
	int row_stride = image->width * wlf_image_get_channels(image);
	while (cinfo.next_scanline < cinfo.image_height) {
		if (need_conversion) {
			/* Convert RGBA to RGB or GRAY_ALPHA to GRAY */
			unsigned char *src = image->data + cinfo.next_scanline * row_stride;
			if (image->format == WLF_COLOR_TYPE_RGBA) {
				for (uint32_t x = 0; x < image->width; x++) {
					row_buffer[x * 3] = src[x * 4];     /* R */
					row_buffer[x * 3 + 1] = src[x * 4 + 1]; /* G */
					row_buffer[x * 3 + 2] = src[x * 4 + 2]; /* B */
				}
			} else if (image->format == WLF_COLOR_TYPE_GRAY_ALPHA) {
				for (uint32_t x = 0; x < image->width; x++) {
					row_buffer[x] = src[x * 2]; /* Gray, skip alpha */
				}
			}
			row_pointer[0] = row_buffer;
		} else {
			row_pointer[0] = image->data + cinfo.next_scanline * row_stride;
		}

		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	if (row_buffer) {
		free(row_buffer);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(fp);

	return true;
}

/**
 * @brief Load a JPEG file into a wlf_image structure.
 * @param image Pointer to the wlf_image structure to populate.
 * @param filename Path to the JPEG file to load.
 * @param enable_16_bit Whether to enable 16-bit mode (not supported by JPEG).
 * @return true on success, false on failure.
 * @note This function reads a JPEG file and populates the image structure
 *       with the decoded pixel data. It automatically handles different
 *       JPEG colorspaces and converts them to standard wlframe formats.
 *       The enable_16_bit parameter is ignored as JPEG only supports 8-bit.
 */
static bool jpeg_image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) {
		wlf_log_errno(WLF_ERROR, "Cannot open %s for reading!", filename);
		return false;
	}

	struct jpeg_decompress_struct cinfo;
	struct wlf_jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = wlf_jpeg_error_exit;

	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);

	struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_from_image(image);
	assert(jpeg_image != NULL);
	if (enable_16_bit) {
		wlf_log(WLF_INFO, "16-bit mode not supported by JPEG, using 8-bit");
	}

	switch (cinfo.jpeg_color_space) {
		case JCS_GRAYSCALE:
			cinfo.out_color_space = JCS_GRAYSCALE;
			image->format = WLF_COLOR_TYPE_GRAY;
			jpeg_image->colorspace = WLF_JPEG_COLORSPACE_GRAYSCALE;
			break;
		case JCS_RGB:
			cinfo.out_color_space = JCS_RGB;
			image->format = WLF_COLOR_TYPE_RGB;
			jpeg_image->colorspace = WLF_JPEG_COLORSPACE_RGB;
			break;
		case JCS_YCbCr:
			cinfo.out_color_space = JCS_RGB; /* Convert to RGB for consistency */
			image->format = WLF_COLOR_TYPE_RGB;
			jpeg_image->colorspace = WLF_JPEG_COLORSPACE_YCC;
			break;
		case JCS_CMYK:
			cinfo.out_color_space = JCS_CMYK;
			image->format = WLF_COLOR_TYPE_RGB; /* Will need conversion */
			jpeg_image->colorspace = WLF_JPEG_COLORSPACE_CMYK;
			wlf_log(WLF_INFO, "CMYK colorspace detected, conversion may be needed");
			break;
		case JCS_YCCK:
			cinfo.out_color_space = JCS_CMYK;
			image->format = WLF_COLOR_TYPE_RGB; /* Will need conversion */
			jpeg_image->colorspace = WLF_JPEG_COLORSPACE_YCCK;
			wlf_log(WLF_INFO, "YCCK colorspace detected, conversion may be needed");
			break;
		default:
			wlf_log(WLF_ERROR, "Unsupported JPEG colorspace: %d", cinfo.jpeg_color_space);
			jpeg_destroy_decompress(&cinfo);
			fclose(fp);
			return false;
	}

	jpeg_image->is_progressive = (cinfo.progressive_mode != 0);
	jpeg_start_decompress(&cinfo);
	image->width = cinfo.output_width;
	image->height = cinfo.output_height;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->has_alpha_channel = false;
	image->is_opaque = true;

	int row_stride = cinfo.output_width * cinfo.output_components;
	image->stride = row_stride;
	image->data = malloc(image->height * row_stride);
	if (image->data == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate image data");
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		return false;
	}

	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(image->data + (cinfo.output_scanline - 1) * row_stride,
		       buffer[0], row_stride);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(fp);

	wlf_log(WLF_DEBUG, "JPEG Info: %dx%d, Format: %d, Bit Depth: %d, Stride: %d, Progressive: %s",
		image->width, image->height, image->format, image->bit_depth, image->stride,
		jpeg_image->is_progressive ? "Yes" : "No");

	return true;
}

static void jpeg_image_destroy(struct wlf_image *wlf_image) {
	struct wlf_jpeg_image *image = wlf_jpeg_image_from_image(wlf_image);
	free(image->base.data);
}

static const struct wlf_image_impl jpeg_image_impl = {
	.save = jpeg_image_save,
	.load = jpeg_image_load,
	.destroy = jpeg_image_destroy,
};

struct wlf_jpeg_image *wlf_jpeg_image_create(void) {
	struct wlf_jpeg_image *image = malloc(sizeof(struct wlf_jpeg_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_jpeg_image");
		return NULL;
	}

	wlf_image_init(&image->base, &jpeg_image_impl, 0, 0, 0);
	image->options = wlf_jpeg_get_default_options();
	image->colorspace = WLF_JPEG_COLORSPACE_UNKNOWN;
	image->is_progressive = false;

	return image;
}

struct wlf_jpeg_image *wlf_jpeg_image_create_with_options(const struct wlf_jpeg_options *options) {
	struct wlf_jpeg_image *image = wlf_jpeg_image_create();
	if (image && options) {
		image->options = *options;
	}
	return image;
}

struct wlf_jpeg_image *wlf_jpeg_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &jpeg_image_impl);
	struct wlf_jpeg_image *image = wlf_container_of(wlf_image, image, base);
	return image;
}

bool wlf_image_is_jpeg(struct wlf_image *image) {
	if (image == NULL) {
		return false;
	}

	return (image->impl == &jpeg_image_impl &&
			image->image_type == WLF_IMAGE_TYPE_JPEG);
}

enum wlf_jpeg_colorspace wlf_color_type_to_jpeg_colorspace(struct wlf_image *image) {
	if (image == NULL) {
		return WLF_JPEG_COLORSPACE_UNKNOWN;
	}

	switch (image->format) {
		case WLF_COLOR_TYPE_RGB:
		case WLF_COLOR_TYPE_RGBA: /* RGBA will be converted to RGB */
			return WLF_JPEG_COLORSPACE_RGB;
		case WLF_COLOR_TYPE_GRAY:
		case WLF_COLOR_TYPE_GRAY_ALPHA: /* GRAY_ALPHA will be converted to GRAY */
			return WLF_JPEG_COLORSPACE_GRAYSCALE;
		default:
			return WLF_JPEG_COLORSPACE_UNKNOWN;
	}
}

bool wlf_jpeg_image_set_quality(struct wlf_jpeg_image *jpeg_image, int quality) {
	if (jpeg_image == NULL || quality < 0 || quality > 100) {
		return false;
	}

	jpeg_image->options.quality = quality;
	return true;
}

bool wlf_jpeg_image_set_subsampling(struct wlf_jpeg_image *jpeg_image, enum wlf_jpeg_subsampling subsampling) {
	if (jpeg_image == NULL) {
		return false;
	}

	jpeg_image->options.subsampling = subsampling;
	return true;
}

bool wlf_jpeg_image_set_progressive(struct wlf_jpeg_image *jpeg_image, bool progressive) {
	if (jpeg_image == NULL) {
		return false;
	}

	jpeg_image->options.progressive = progressive;
	return true;
}

struct wlf_jpeg_options wlf_jpeg_get_default_options(void) {
	struct wlf_jpeg_options options = {
		.quality = 85,                           /* Good quality/size balance */
		.subsampling = WLF_JPEG_SUBSAMPLING_420, /* Standard subsampling */
		.progressive = false,                    /* Non-progressive by default */
		.optimize = true,                        /* Enable optimization */
		.arithmetic = false,                     /* Use Huffman coding */
	};
	return options;
}
