#include "wlf/platform/windows/text.h"

#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static wchar_t *utf8_to_wide(const char *text) {
	int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		text, -1, NULL, 0);
	if (length == 0) {
		return NULL;
	}
	wchar_t *wide = calloc((size_t)length, sizeof(*wide));
	if (wide == NULL || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			text, -1, wide, length) == 0) {
		free(wide);
		return NULL;
	}
	return wide;
}

static const char *resolve_generic_family(const char *family) {
	if (family == NULL || strcmp(family, "sans-serif") == 0) {
		return "Segoe UI";
	}
	if (strcmp(family, "serif") == 0) {
		return "Times New Roman";
	}
	if (strcmp(family, "monospace") == 0) {
		return "Consolas";
	}
	return family;
}

static HFONT create_font(const struct wlf_text_options *options) {
	double pixel_size = options->font_size * options->raster_scale;
	if (!isfinite(pixel_size) || pixel_size <= 0 || pixel_size > INT_MAX) {
		return NULL;
	}
	wchar_t *family = utf8_to_wide(resolve_generic_family(
		options->font_family));
	if (family == NULL) {
		return NULL;
	}
	HFONT font = CreateFontW(-(int)lround(pixel_size), 0, 0, 0,
		options->weight == WLF_TEXT_FONT_WEIGHT_BOLD ? FW_BOLD : FW_NORMAL,
		options->slant != WLF_TEXT_FONT_SLANT_NORMAL, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, family);
	free(family);
	return font;
}

static uint8_t byte_channel(double channel) {
	if (channel <= 0) {
		return 0;
	}
	if (channel >= 1) {
		return UINT8_MAX;
	}
	return (uint8_t)lround(channel * UINT8_MAX);
}

static void windows_text_raster_destroy(struct wlf_text *text,
		struct wlf_text_raster *raster) {
	(void)text;
	free(raster->private_data);
	*raster = (struct wlf_text_raster){0};
}

static bool windows_text_rasterize(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	(void)text;
	wchar_t *wide = utf8_to_wide(options->text);
	HFONT font = create_font(options);
	HDC dc = CreateCompatibleDC(NULL);
	if (wide == NULL || font == NULL || dc == NULL) {
		free(wide);
		if (font != NULL) {
			DeleteObject(font);
		}
		if (dc != NULL) {
			DeleteDC(dc);
		}
		return false;
	}
	HGDIOBJ previous_font = SelectObject(dc, font);
	RECT measured = {0};
	UINT flags = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_CALCRECT;
	DrawTextW(dc, wide, -1, &measured, flags);
	TEXTMETRICW font_metrics = {0};
	GetTextMetricsW(dc, &font_metrics);
	int natural_width = measured.right - measured.left;
	int natural_height = measured.bottom - measured.top;
	raster->metrics = (struct wlf_text_metrics){
		.width = natural_width,
		.height = natural_height,
		.baseline = font_metrics.tmAscent,
	};
	if (options->text[0] == '\0' || natural_width <= 0 ||
			natural_height <= 0) {
		SelectObject(dc, previous_font);
		DeleteObject(font);
		DeleteDC(dc);
		free(wide);
		return true;
	}

	int width = natural_width;
	if (options->max_width > 0) {
		double maximum = options->max_width * options->raster_scale;
		if (!isfinite(maximum) || maximum < 0 || maximum > INT_MAX) {
			SelectObject(dc, previous_font);
			DeleteObject(font);
			DeleteDC(dc);
			free(wide);
			return false;
		}
		if (maximum < width) {
			width = (int)ceil(maximum);
		}
	}
	if (width <= 0 || (size_t)natural_height >
			SIZE_MAX / (size_t)width / sizeof(uint32_t)) {
		SelectObject(dc, previous_font);
		DeleteObject(font);
		DeleteDC(dc);
		free(wide);
		return width <= 0;
	}

	BITMAPINFO bitmap_info = {
		.bmiHeader = {
			.biSize = sizeof(BITMAPINFOHEADER),
			.biWidth = width,
			.biHeight = -natural_height,
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
		},
	};
	uint32_t *dib_pixels = NULL;
	HBITMAP bitmap = CreateDIBSection(dc, &bitmap_info, DIB_RGB_COLORS,
		(void **)&dib_pixels, NULL, 0);
	if (bitmap == NULL || dib_pixels == NULL) {
		SelectObject(dc, previous_font);
		DeleteObject(font);
		DeleteDC(dc);
		free(wide);
		return false;
	}
	HGDIOBJ previous_bitmap = SelectObject(dc, bitmap);
	PatBlt(dc, 0, 0, width, natural_height, BLACKNESS);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, RGB(255, 255, 255));
	RECT target = {0, 0, width, natural_height};
	DrawTextW(dc, wide, -1, &target, DT_LEFT | DT_TOP | DT_NOPREFIX |
		DT_NOCLIP);

	size_t pixel_count = (size_t)width * (size_t)natural_height;
	uint32_t *pixels = malloc(pixel_count * sizeof(*pixels));
	if (pixels != NULL) {
		struct wlf_color color = wlf_color_clamp(&options->color);
		for (size_t i = 0; i < pixel_count; ++i) {
			uint32_t sample = dib_pixels[i];
			uint8_t blue = sample & 0xff;
			uint8_t green = (sample >> 8) & 0xff;
			uint8_t red = (sample >> 16) & 0xff;
			double coverage = (double)(red > green ?
				(red > blue ? red : blue) : (green > blue ? green : blue)) /
				UINT8_MAX;
			double alpha = coverage * color.a;
			pixels[i] = ((uint32_t)byte_channel(alpha) << 24) |
				((uint32_t)byte_channel(color.r * alpha) << 16) |
				((uint32_t)byte_channel(color.g * alpha) << 8) |
				byte_channel(color.b * alpha);
		}
	}

	SelectObject(dc, previous_bitmap);
	SelectObject(dc, previous_font);
	DeleteObject(bitmap);
	DeleteObject(font);
	DeleteDC(dc);
	free(wide);
	if (pixels == NULL) {
		return false;
	}
	raster->width = (uint32_t)width;
	raster->height = (uint32_t)natural_height;
	raster->stride = (uint32_t)width * sizeof(uint32_t);
	raster->data = pixels;
	raster->private_data = pixels;
	return true;
}

static void windows_text_destroy(struct wlf_text *text) {
	free(text);
}

static const struct wlf_text_impl windows_text_impl = {
	.name = "windows-gdi",
	.rasterize = windows_text_rasterize,
	.destroy_raster = windows_text_raster_destroy,
	.destroy = windows_text_destroy,
};

struct wlf_windows_text *wlf_windows_text_create(void) {
	struct wlf_windows_text *text = calloc(1, sizeof(*text));
	if (text == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Windows text renderer");
		return NULL;
	}
	wlf_text_init(&text->base, &windows_text_impl);
	return text;
}
