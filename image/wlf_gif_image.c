#include "wlf/image/wlf_gif_image.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <gif_lib.h>

#ifndef GIFLIB_MAJOR
#define GIFLIB_MAJOR 5
#endif

#if GIFLIB_MAJOR >= 5
#define WLF_DGIF_OPEN(path, err) DGifOpenFileName((path), (err))
#define WLF_EGIF_OPEN(path, err) EGifOpenFileName((path), false, (err))
#define WLF_DGIF_CLOSE(gif, err) DGifCloseFile((gif), (err))
#define WLF_EGIF_CLOSE(gif, err) EGifCloseFile((gif), (err))
#else
#define WLF_DGIF_OPEN(path, err) DGifOpenFileName((path))
#define WLF_EGIF_OPEN(path, err) EGifOpenFileName((path), false)
#define WLF_DGIF_CLOSE(gif, err) DGifCloseFile((gif))
#define WLF_EGIF_CLOSE(gif, err) EGifCloseFile((gif))
#endif

static int gif_close_read(GifFileType *gif_file) {
	int gif_error = 0;
	return WLF_DGIF_CLOSE(gif_file, &gif_error);
}

static int gif_close_write(GifFileType *gif_file) {
	int gif_error = 0;
	return WLF_EGIF_CLOSE(gif_file, &gif_error);
}

static GifByteType wlf_rgb332_index(uint8_t r, uint8_t g, uint8_t b) {
	return (GifByteType)(((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6));
}

static void wlf_fill_rgb332_palette(GifColorType *palette) {
	for (int r = 0; r < 8; r++) {
		for (int g = 0; g < 8; g++) {
			for (int b = 0; b < 4; b++) {
				int idx = (r << 5) | (g << 2) | b;
				palette[idx].Red = (GifByteType)((r * 255) / 7);
				palette[idx].Green = (GifByteType)((g * 255) / 7);
				palette[idx].Blue = (GifByteType)((b * 255) / 3);
			}
		}
	}
}

static void wlf_parse_loop_count(const ExtensionBlock *exts, int ext_count,
	uint32_t *loop_count, bool *found_loop) {
	for (int i = 0; i < ext_count; i++) {
		const ExtensionBlock *ext = &exts[i];
		if (ext->Function != APPLICATION_EXT_FUNC_CODE || ext->ByteCount < 11 || ext->Bytes == NULL) {
			continue;
		}

		bool is_netscape =
			(memcmp(ext->Bytes, "NETSCAPE2.0", 11) == 0) ||
			(memcmp(ext->Bytes, "ANIMEXTS1.0", 11) == 0);
		if (!is_netscape) {
			continue;
		}

		for (int j = i + 1; j < ext_count; j++) {
			const ExtensionBlock *sub = &exts[j];
			if (sub->ByteCount < 3 || sub->Bytes == NULL) {
				continue;
			}
			if (sub->Bytes[0] != 1) {
				continue;
			}

			*loop_count = (uint32_t)((unsigned char)sub->Bytes[1] |
				((unsigned char)sub->Bytes[2] << 8));
			*found_loop = true;
			return;
		}
	}
}

static bool wlf_get_frame_gcb(GifFileType *gif_file, int frame_idx, GraphicsControlBlock *gcb) {
	gcb->DisposalMode = DISPOSAL_UNSPECIFIED;
	gcb->UserInputFlag = false;
	gcb->DelayTime = 0;
	gcb->TransparentColor = NO_TRANSPARENT_COLOR;

	if (DGifSavedExtensionToGCB(gif_file, frame_idx, gcb) == GIF_OK) {
		return true;
	}

	SavedImage *frame = &gif_file->SavedImages[frame_idx];
	for (int i = 0; i < frame->ExtensionBlockCount; i++) {
		ExtensionBlock *ext = &frame->ExtensionBlocks[i];
		if (ext->Function != GRAPHICS_EXT_FUNC_CODE || ext->ByteCount < 4 || ext->Bytes == NULL) {
			continue;
		}
		if (DGifExtensionToGCB((size_t)ext->ByteCount, ext->Bytes, gcb) == GIF_OK) {
			return true;
		}
	}

	return true;
}

static uint8_t wlf_choose_transparent_index(const uint32_t hist[256]) {
	uint8_t best = 0;
	uint32_t min_count = hist[0];
	for (int i = 1; i < 256; i++) {
		if (hist[i] < min_count) {
			min_count = hist[i];
			best = (uint8_t)i;
		}
	}
	return best;
}

static uint8_t wlf_fallback_index(uint8_t idx, uint8_t transparent_index) {
	uint8_t alt = (idx == 255) ? 254 : 255;
	if (alt == transparent_index) {
		alt = (idx == 0) ? 1 : 0;
	}
	return alt;
}

static bool wlf_rgba_to_indices(const unsigned char *src, uint32_t width, uint32_t height,
	uint32_t stride, GifByteType *indices, bool *has_transparency,
	uint8_t *transparent_index_out) {
	uint32_t hist[256] = { 0 };
	*has_transparency = false;

	for (uint32_t y = 0; y < height; y++) {
		const unsigned char *row = src + (size_t)y * stride;
		GifByteType *out_row = indices + (size_t)y * width;
		for (uint32_t x = 0; x < width; x++) {
			uint8_t r = row[x * 4 + 0];
			uint8_t g = row[x * 4 + 1];
			uint8_t b = row[x * 4 + 2];
			uint8_t a = row[x * 4 + 3];

			if (a < 128) {
				out_row[x] = 0;
				*has_transparency = true;
			} else {
				GifByteType idx = wlf_rgb332_index(r, g, b);
				out_row[x] = idx;
				hist[idx]++;
			}
		}
	}

	if (!*has_transparency) {
		*transparent_index_out = 0;
		return true;
	}

	uint8_t transparent_index = wlf_choose_transparent_index(hist);
	*transparent_index_out = transparent_index;
	for (uint32_t y = 0; y < height; y++) {
		const unsigned char *row = src + (size_t)y * stride;
		GifByteType *out_row = indices + (size_t)y * width;
		for (uint32_t x = 0; x < width; x++) {
			uint8_t a = row[x * 4 + 3];
			if (a < 128) {
				out_row[x] = transparent_index;
			} else if (out_row[x] == transparent_index) {
				out_row[x] = wlf_fallback_index(out_row[x], transparent_index);
			}
		}
	}

	return true;
}

static bool wlf_base_to_indices(const struct wlf_image *image, GifByteType *indices,
	bool *has_transparency, uint8_t *transparent_index_out) {
	uint32_t hist[256] = { 0 };
	*has_transparency = false;

	for (uint32_t y = 0; y < image->height; y++) {
		const unsigned char *row = image->data + (size_t)y * image->stride;
		GifByteType *out_row = indices + (size_t)y * image->width;
		for (uint32_t x = 0; x < image->width; x++) {
			uint8_t r = 0;
			uint8_t g = 0;
			uint8_t b = 0;
			uint8_t a = 255;

			switch (image->format) {
				case WLF_COLOR_TYPE_RGB:
					r = row[x * 3 + 0];
					g = row[x * 3 + 1];
					b = row[x * 3 + 2];
					break;
				case WLF_COLOR_TYPE_RGBA:
					r = row[x * 4 + 0];
					g = row[x * 4 + 1];
					b = row[x * 4 + 2];
					a = row[x * 4 + 3];
					break;
				case WLF_COLOR_TYPE_GRAY:
					r = row[x];
					g = row[x];
					b = row[x];
					break;
				case WLF_COLOR_TYPE_GRAY_ALPHA:
					r = row[x * 2 + 0];
					g = row[x * 2 + 0];
					b = row[x * 2 + 0];
					a = row[x * 2 + 1];
					break;
				default:
					wlf_log(WLF_ERROR, "Unsupported image format for GIF: %d", image->format);
					return false;
			}

			if (a < 128) {
				out_row[x] = 0;
				*has_transparency = true;
			} else {
				GifByteType idx = wlf_rgb332_index(r, g, b);
				out_row[x] = idx;
				hist[idx]++;
			}
		}
	}

	if (!*has_transparency) {
		*transparent_index_out = 0;
		return true;
	}

	uint8_t transparent_index = wlf_choose_transparent_index(hist);
	*transparent_index_out = transparent_index;

	for (uint32_t y = 0; y < image->height; y++) {
		const unsigned char *row = image->data + (size_t)y * image->stride;
		GifByteType *out_row = indices + (size_t)y * image->width;
		for (uint32_t x = 0; x < image->width; x++) {
			uint8_t a = 255;
			if (image->format == WLF_COLOR_TYPE_RGBA) {
				a = row[x * 4 + 3];
			} else if (image->format == WLF_COLOR_TYPE_GRAY_ALPHA) {
				a = row[x * 2 + 1];
			}

			if (a < 128) {
				out_row[x] = transparent_index;
			} else if (out_row[x] == transparent_index) {
				out_row[x] = wlf_fallback_index(out_row[x], transparent_index);
			}
		}
	}

	return true;
}

static bool wlf_write_loop_extension(GifFileType *gif_file, uint32_t loop_count) {
	unsigned char app_id[11] = { 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0' };
	unsigned char sub_block[3] = {
		1,
		(unsigned char)(loop_count & 0xFF),
		(unsigned char)((loop_count >> 8) & 0xFF),
	};

	if (EGifPutExtensionLeader(gif_file, APPLICATION_EXT_FUNC_CODE) == GIF_ERROR) {
		return false;
	}
	if (EGifPutExtensionBlock(gif_file, 11, app_id) == GIF_ERROR) {
		return false;
	}
	if (EGifPutExtensionBlock(gif_file, 3, sub_block) == GIF_ERROR) {
		return false;
	}
	if (EGifPutExtensionTrailer(gif_file) == GIF_ERROR) {
		return false;
	}

	return true;
}

static bool image_save(struct wlf_image *image, const char *filename) {
	if (image == NULL || filename == NULL || image->data == NULL) {
		return false;
	}

	if (image->bit_depth != 0 && image->bit_depth != WLF_IMAGE_BIT_DEPTH_8) {
		wlf_log(WLF_ERROR, "GIF only supports 8-bit channels, got %d", image->bit_depth);
		return false;
	}

	struct wlf_gif_image *gif_image = wlf_gif_image_from_image(image);
	bool has_frames = (gif_image->frames != NULL && gif_image->frame_count > 0);
	uint32_t frame_count = has_frames ? gif_image->frame_count : 1;

	int gif_error = 0;
	GifFileType *gif_file = WLF_EGIF_OPEN(filename, &gif_error);
	if (gif_file == NULL) {
		wlf_log(WLF_ERROR, "Cannot open %s for GIF writing (err=%d)", filename, gif_error);
		return false;
	}

	GifColorType palette_colors[256];
	wlf_fill_rgb332_palette(palette_colors);
	ColorMapObject *color_map = GifMakeMapObject(256, palette_colors);
	if (color_map == NULL) {
		wlf_log(WLF_ERROR, "GifMakeMapObject failed");
		gif_close_write(gif_file);
		return false;
	}

	if (EGifPutScreenDesc(gif_file, (int)image->width, (int)image->height, 8, 0, color_map) == GIF_ERROR) {
		wlf_log(WLF_ERROR, "EGifPutScreenDesc failed");
		GifFreeMapObject(color_map);
		gif_close_write(gif_file);
		return false;
	}

	if (frame_count > 1 || gif_image->loop_count > 0) {
		if (!wlf_write_loop_extension(gif_file, gif_image->loop_count)) {
			wlf_log(WLF_ERROR, "Failed to write GIF loop extension");
			GifFreeMapObject(color_map);
			gif_close_write(gif_file);
			return false;
		}
	}

	size_t pixel_count = (size_t)image->width * image->height;
	GifByteType *indices = malloc(pixel_count);
	if (indices == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate GIF index buffer");
		GifFreeMapObject(color_map);
		gif_close_write(gif_file);
		return false;
	}

	bool ok = true;
	for (uint32_t i = 0; i < frame_count; i++) {
		bool has_transparency = false;
		uint8_t transparent_index = 0;
		uint32_t delay_ms = 0;
		uint8_t disposal = DISPOSAL_UNSPECIFIED;

		if (has_frames) {
			const struct wlf_gif_frame *f = &gif_image->frames[i];
			if (f->pixels == NULL || f->width != image->width || f->height != image->height) {
				wlf_log(WLF_ERROR, "Invalid GIF frame %u dimensions or data", i);
				ok = false;
				break;
			}
			if (!wlf_rgba_to_indices(f->pixels, f->width, f->height, f->stride,
					indices, &has_transparency, &transparent_index)) {
				ok = false;
				break;
			}
			delay_ms = f->delay_ms;
			disposal = f->disposal_method;
		} else {
			if (!wlf_base_to_indices(image, indices, &has_transparency, &transparent_index)) {
				ok = false;
				break;
			}
			delay_ms = gif_image->delay_ms;
			disposal = DISPOSAL_UNSPECIFIED;
		}

		GraphicsControlBlock gcb = {
			.DisposalMode = (int)disposal,
			.UserInputFlag = false,
			.DelayTime = (int)(delay_ms / 10),
			.TransparentColor = has_transparency ? (int)transparent_index : NO_TRANSPARENT_COLOR,
		};
		if (gcb.DelayTime < 0) {
			gcb.DelayTime = 0;
		}

		GifByteType gcb_ext[4] = { 0, 0, 0, 0 };
		size_t gcb_len = EGifGCBToExtension(&gcb, gcb_ext);
		if (EGifPutExtension(gif_file, GRAPHICS_EXT_FUNC_CODE, (int)gcb_len, gcb_ext) == GIF_ERROR) {
			wlf_log(WLF_ERROR, "EGifPutExtension(GCB) failed at frame %u", i);
			ok = false;
			break;
		}

		if (EGifPutImageDesc(gif_file, 0, 0, (int)image->width, (int)image->height, false, NULL) == GIF_ERROR) {
			wlf_log(WLF_ERROR, "EGifPutImageDesc failed at frame %u", i);
			ok = false;
			break;
		}

		for (uint32_t y = 0; y < image->height; y++) {
			GifByteType *row = indices + (size_t)y * image->width;
			if (EGifPutLine(gif_file, row, (int)image->width) == GIF_ERROR) {
				wlf_log(WLF_ERROR, "EGifPutLine failed at frame %u row %u", i, y);
				ok = false;
				break;
			}
		}
		if (!ok) {
			break;
		}
	}

	free(indices);
	GifFreeMapObject(color_map);

	if (gif_close_write(gif_file) == GIF_ERROR) {
		wlf_log(WLF_ERROR, "EGifCloseFile failed");
		ok = false;
	}

	return ok;
}

static bool wlf_build_initial_canvas(GifFileType *gif_file, unsigned char *canvas) {
	GifColorType bg = { 0, 0, 0 };
	if (gif_file->SColorMap && gif_file->SBackGroundColor >= 0 &&
		gif_file->SBackGroundColor < gif_file->SColorMap->ColorCount) {
		bg = gif_file->SColorMap->Colors[gif_file->SBackGroundColor];
	}

	for (int y = 0; y < gif_file->SHeight; y++) {
		for (int x = 0; x < gif_file->SWidth; x++) {
			size_t out_idx = ((size_t)y * gif_file->SWidth + x) * 4;
			canvas[out_idx + 0] = bg.Red;
			canvas[out_idx + 1] = bg.Green;
			canvas[out_idx + 2] = bg.Blue;
			canvas[out_idx + 3] = 255;
		}
	}

	return true;
}

static void wlf_clear_rect_to_background(unsigned char *canvas, GifFileType *gif_file,
	const GifImageDesc *desc) {
	GifColorType bg = { 0, 0, 0 };
	if (gif_file->SColorMap && gif_file->SBackGroundColor >= 0 &&
		gif_file->SBackGroundColor < gif_file->SColorMap->ColorCount) {
		bg = gif_file->SColorMap->Colors[gif_file->SBackGroundColor];
	}

	for (int y = 0; y < desc->Height; y++) {
		int dst_y = desc->Top + y;
		if (dst_y < 0 || dst_y >= gif_file->SHeight) {
			continue;
		}
		for (int x = 0; x < desc->Width; x++) {
			int dst_x = desc->Left + x;
			if (dst_x < 0 || dst_x >= gif_file->SWidth) {
				continue;
			}
			size_t idx = ((size_t)dst_y * gif_file->SWidth + dst_x) * 4;
			canvas[idx + 0] = bg.Red;
			canvas[idx + 1] = bg.Green;
			canvas[idx + 2] = bg.Blue;
			canvas[idx + 3] = 255;
		}
	}
}

static void wlf_blend_saved_image_to_canvas(unsigned char *canvas, GifFileType *gif_file,
	const SavedImage *frame, const GraphicsControlBlock *gcb) {
	ColorMapObject *color_map = frame->ImageDesc.ColorMap ? frame->ImageDesc.ColorMap : gif_file->SColorMap;
	if (color_map == NULL || frame->RasterBits == NULL) {
		return;
	}

	if (!frame->ImageDesc.Interlace) {
		for (int y = 0; y < frame->ImageDesc.Height; y++) {
			int dst_y = frame->ImageDesc.Top + y;
			if (dst_y < 0 || dst_y >= gif_file->SHeight) {
				continue;
			}
			for (int x = 0; x < frame->ImageDesc.Width; x++) {
				int dst_x = frame->ImageDesc.Left + x;
				if (dst_x < 0 || dst_x >= gif_file->SWidth) {
					continue;
				}

				GifByteType color_index = frame->RasterBits[(size_t)y * frame->ImageDesc.Width + x];
				if (color_index >= color_map->ColorCount) {
					continue;
				}
				if (gcb->TransparentColor != NO_TRANSPARENT_COLOR &&
					color_index == (GifByteType)gcb->TransparentColor) {
					continue;
				}

				GifColorType c = color_map->Colors[color_index];
				size_t out_idx = ((size_t)dst_y * gif_file->SWidth + dst_x) * 4;
				canvas[out_idx + 0] = c.Red;
				canvas[out_idx + 1] = c.Green;
				canvas[out_idx + 2] = c.Blue;
				canvas[out_idx + 3] = 255;
			}
		}
		return;
	}

	static const int starts[4] = { 0, 4, 2, 1 };
	static const int steps[4] = { 8, 8, 4, 2 };
	int src_row = 0;
	for (int pass = 0; pass < 4; pass++) {
		for (int y = starts[pass]; y < frame->ImageDesc.Height; y += steps[pass]) {
			int dst_y = frame->ImageDesc.Top + y;
			if (dst_y < 0 || dst_y >= gif_file->SHeight) {
				src_row++;
				continue;
			}
			for (int x = 0; x < frame->ImageDesc.Width; x++) {
				int dst_x = frame->ImageDesc.Left + x;
				if (dst_x < 0 || dst_x >= gif_file->SWidth) {
					continue;
				}

				GifByteType color_index = frame->RasterBits[(size_t)src_row * frame->ImageDesc.Width + x];
				if (color_index >= color_map->ColorCount) {
					continue;
				}
				if (gcb->TransparentColor != NO_TRANSPARENT_COLOR &&
					color_index == (GifByteType)gcb->TransparentColor) {
					continue;
				}

				GifColorType c = color_map->Colors[color_index];
				size_t out_idx = ((size_t)dst_y * gif_file->SWidth + dst_x) * 4;
				canvas[out_idx + 0] = c.Red;
				canvas[out_idx + 1] = c.Green;
				canvas[out_idx + 2] = c.Blue;
				canvas[out_idx + 3] = 255;
			}
			src_row++;
		}
	}
}

static bool image_load(struct wlf_image *image, const char *filename, bool enable_16_bit) {
	(void)enable_16_bit;
	if (image == NULL || filename == NULL) {
		return false;
	}

	int gif_error = 0;
	GifFileType *gif_file = WLF_DGIF_OPEN(filename, &gif_error);
	if (gif_file == NULL) {
		wlf_log(WLF_ERROR, "Cannot open %s for GIF reading (err=%d)", filename, gif_error);
		return false;
	}

	if (DGifSlurp(gif_file) == GIF_ERROR) {
		wlf_log(WLF_ERROR, "DGifSlurp failed for %s", filename);
		gif_close_read(gif_file);
		return false;
	}

	if (gif_file->ImageCount <= 0 || gif_file->SavedImages == NULL ||
		gif_file->SWidth <= 0 || gif_file->SHeight <= 0) {
		wlf_log(WLF_ERROR, "Invalid GIF content: %s", filename);
		gif_close_read(gif_file);
		return false;
	}

	struct wlf_gif_image *gif_image = wlf_gif_image_from_image(image);
	gif_image->frame_count = (uint32_t)gif_file->ImageCount;
	gif_image->frames = calloc(gif_image->frame_count, sizeof(*gif_image->frames));
	if (gif_image->frames == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate GIF frame array");
		gif_close_read(gif_file);
		return false;
	}

	size_t canvas_size = (size_t)gif_file->SWidth * gif_file->SHeight * 4;
	unsigned char *canvas = malloc(canvas_size);
	unsigned char *previous_canvas = malloc(canvas_size);
	if (canvas == NULL || previous_canvas == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate GIF composition buffers");
		free(canvas);
		free(previous_canvas);
		free(gif_image->frames);
		gif_image->frames = NULL;
		gif_image->frame_count = 0;
		gif_close_read(gif_file);
		return false;
	}

	wlf_build_initial_canvas(gif_file, canvas);

	bool found_loop = false;
	gif_image->loop_count = 0;
	wlf_parse_loop_count(gif_file->ExtensionBlocks, gif_file->ExtensionBlockCount,
		&gif_image->loop_count, &found_loop);
	if (!found_loop) {
		for (int i = 0; i < gif_file->ImageCount; i++) {
			SavedImage *s = &gif_file->SavedImages[i];
			wlf_parse_loop_count(s->ExtensionBlocks, s->ExtensionBlockCount,
				&gif_image->loop_count, &found_loop);
			if (found_loop) {
				break;
			}
		}
	}

	bool has_alpha = false;
	for (int i = 0; i < gif_file->ImageCount; i++) {
		SavedImage *frame = &gif_file->SavedImages[i];
		GraphicsControlBlock gcb;
		wlf_get_frame_gcb(gif_file, i, &gcb);

		if (gcb.DisposalMode == DISPOSE_PREVIOUS) {
			memcpy(previous_canvas, canvas, canvas_size);
		}

		wlf_blend_saved_image_to_canvas(canvas, gif_file, frame, &gcb);

		struct wlf_gif_frame *out = &gif_image->frames[i];
		out->width = (uint32_t)gif_file->SWidth;
		out->height = (uint32_t)gif_file->SHeight;
		out->stride = (uint32_t)gif_file->SWidth * 4;
		out->delay_ms = (uint32_t)gcb.DelayTime * 10;
		out->disposal_method = (uint8_t)gcb.DisposalMode;
		out->pixels = malloc(canvas_size);
		if (out->pixels == NULL) {
			wlf_log(WLF_ERROR, "Failed to allocate frame pixels for frame %d", i);
			for (int j = 0; j < i; j++) {
				free(gif_image->frames[j].pixels);
			}
			free(gif_image->frames);
			gif_image->frames = NULL;
			gif_image->frame_count = 0;
			free(canvas);
			free(previous_canvas);
			gif_close_read(gif_file);
			return false;
		}
		memcpy(out->pixels, canvas, canvas_size);

		if (gcb.TransparentColor != NO_TRANSPARENT_COLOR) {
			has_alpha = true;
		}

		if (gcb.DisposalMode == DISPOSE_BACKGROUND) {
			wlf_clear_rect_to_background(canvas, gif_file, &frame->ImageDesc);
		} else if (gcb.DisposalMode == DISPOSE_PREVIOUS) {
			memcpy(canvas, previous_canvas, canvas_size);
		}
	}

	free(canvas);
	free(previous_canvas);

	image->width = (uint32_t)gif_file->SWidth;
	image->height = (uint32_t)gif_file->SHeight;
	image->format = WLF_COLOR_TYPE_RGBA;
	image->bit_depth = WLF_IMAGE_BIT_DEPTH_8;
	image->stride = (uint32_t)gif_file->SWidth * 4;
	image->has_alpha_channel = has_alpha;
	image->is_opaque = !has_alpha;
	image->data = gif_image->frames[0].pixels;
	gif_image->delay_ms = gif_image->frames[0].delay_ms;

	if (gif_close_read(gif_file) == GIF_ERROR) {
		wlf_log(WLF_ERROR, "DGifCloseFile failed");
		return false;
	}

	return true;
}

static void image_destroy(struct wlf_image *wlf_image) {
	struct wlf_gif_image *image = wlf_gif_image_from_image(wlf_image);
	if (image->frames != NULL) {
		for (uint32_t i = 0; i < image->frame_count; i++) {
			free(image->frames[i].pixels);
			image->frames[i].pixels = NULL;
		}
		free(image->frames);
		image->frames = NULL;
		image->frame_count = 0;
		image->base.data = NULL;
		return;
	}
	free(image->base.data);
	free(image);
}

static const struct wlf_image_impl gif_image_impl = {
	.save = image_save,
	.load = image_load,
	.destroy = image_destroy,
};

struct wlf_gif_image *wlf_gif_image_create(void) {
	struct wlf_gif_image *image = malloc(sizeof(struct wlf_gif_image));
	if (image == NULL) {
		wlf_log(WLF_ERROR, "Failed to allocate wlf_gif_image");
		return NULL;
	}

	wlf_image_init(&image->base, &gif_image_impl, 0, 0, 0);
	image->loop_count = 0;
	image->delay_ms = 0;
	image->frame_count = 0;
	image->frames = NULL;

	return image;
}

bool wlf_image_is_gif(const struct wlf_image *image) {
	return image->impl == &gif_image_impl;
}

struct wlf_gif_image *wlf_gif_image_from_image(struct wlf_image *wlf_image) {
	assert(wlf_image->impl == &gif_image_impl);

	struct wlf_gif_image *image = wlf_container_of(wlf_image, image, base);

	return image;
}
