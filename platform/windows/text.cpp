extern "C" {
#include "wlf/platform/windows/text.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
}

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dwrite.h>

struct windows_text {
	struct wlf_windows_text base;
	IDWriteFactory *factory;
};

struct draw_context {
	IDWriteFactory *factory;
	uint32_t *pixels;
	uint32_t width, height;
	struct wlf_color color;
};

static wchar_t *utf8_to_wide(const char *text, UINT32 *length) {
	int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
		text, -1, NULL, 0);
	if (count == 0) return NULL;
	wchar_t *wide = static_cast<wchar_t *>(
		calloc((size_t)count, sizeof(*wide)));
	if (wide == NULL || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			text, -1, wide, count) == 0) {
		free(wide);
		return NULL;
	}
	if (length != NULL) *length = (UINT32)(count - 1);
	return wide;
}

static const char *resolve_generic_family(const char *family) {
	if (family == NULL || strcmp(family, "sans-serif") == 0) return "Segoe UI";
	if (strcmp(family, "serif") == 0) return "Times New Roman";
	if (strcmp(family, "monospace") == 0) return "Consolas";
	return family;
}

static uint8_t byte_channel(double channel) {
	if (channel <= 0) return 0;
	if (channel >= 1) return UINT8_MAX;
	return (uint8_t)lround(channel * UINT8_MAX);
}

class glyph_renderer final : public IDWriteTextRenderer {
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **object) override {
		if (object == nullptr) return E_POINTER;
		*object = nullptr;
		if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteTextRenderer)) {
			*object = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() override {
		return (ULONG)InterlockedIncrement(&references);
	}
	ULONG STDMETHODCALLTYPE Release() override {
		return (ULONG)InterlockedDecrement(&references);
	}
	HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(
			void *, BOOL *disabled) override {
		*disabled = FALSE;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetCurrentTransform(
			void *, DWRITE_MATRIX *transform) override {
		*transform = {1, 0, 0, 1, 0, 0};
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void *, FLOAT *value) override {
		*value = 1.0f;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE DrawGlyphRun(void *client_context,
			FLOAT baseline_x, FLOAT baseline_y,
			DWRITE_MEASURING_MODE measuring_mode,
			const DWRITE_GLYPH_RUN *glyph_run,
			const DWRITE_GLYPH_RUN_DESCRIPTION *, IUnknown *) override {
		auto *context = static_cast<draw_context *>(client_context);
		IDWriteGlyphRunAnalysis *analysis = nullptr;
		HRESULT hr = context->factory->CreateGlyphRunAnalysis(glyph_run, 1.0f,
			nullptr, DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC,
			measuring_mode, baseline_x, baseline_y, &analysis);
		if (FAILED(hr)) return hr;
		RECT bounds;
		hr = analysis->GetAlphaTextureBounds(
			DWRITE_TEXTURE_CLEARTYPE_3x1, &bounds);
		int width = bounds.right - bounds.left;
		int height = bounds.bottom - bounds.top;
		if (SUCCEEDED(hr) && width > 0 && height > 0 &&
				(size_t)height <= SIZE_MAX / (size_t)width / 3) {
			size_t size = (size_t)width * (size_t)height * 3;
			auto *alpha = static_cast<uint8_t *>(malloc(size));
			if (alpha == nullptr) hr = E_OUTOFMEMORY;
			else {
				hr = analysis->CreateAlphaTexture(DWRITE_TEXTURE_CLEARTYPE_3x1,
					&bounds, alpha, (UINT32)size);
				if (SUCCEEDED(hr)) composite(context, bounds, alpha);
				free(alpha);
			}
		}
		analysis->Release();
		return hr;
	}
	HRESULT STDMETHODCALLTYPE DrawUnderline(void *, FLOAT, FLOAT,
			const DWRITE_UNDERLINE *, IUnknown *) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE DrawStrikethrough(void *, FLOAT, FLOAT,
			const DWRITE_STRIKETHROUGH *, IUnknown *) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE DrawInlineObject(void *, FLOAT, FLOAT,
			IDWriteInlineObject *, BOOL, BOOL, IUnknown *) override {
		return E_NOTIMPL;
	}

private:
	LONG references = 1;
	static void composite(draw_context *context, const RECT &bounds,
			const uint8_t *alpha) {
		int width = bounds.right - bounds.left;
		int height = bounds.bottom - bounds.top;
		for (int y = 0; y < height; ++y) {
			int dst_y = bounds.top + y;
			if (dst_y < 0 || dst_y >= (int)context->height) continue;
			for (int x = 0; x < width; ++x) {
				int dst_x = bounds.left + x;
				if (dst_x < 0 || dst_x >= (int)context->width) continue;
				size_t index = ((size_t)y * (size_t)width + (size_t)x) * 3;
				uint8_t coverage = alpha[index];
				if (alpha[index + 1] > coverage) coverage = alpha[index + 1];
				if (alpha[index + 2] > coverage) coverage = alpha[index + 2];
				double opacity = (double)coverage / UINT8_MAX * context->color.a;
				context->pixels[(size_t)dst_y * context->width + (size_t)dst_x] =
					((uint32_t)byte_channel(opacity) << 24) |
					((uint32_t)byte_channel(context->color.r * opacity) << 16) |
					((uint32_t)byte_channel(context->color.g * opacity) << 8) |
					byte_channel(context->color.b * opacity);
			}
		}
	}
};

static void windows_text_raster_destroy(struct wlf_text *text,
		struct wlf_text_raster *raster) {
	(void)text;
	free(raster->private_data);
	*raster = {};
}

static bool windows_text_rasterize(struct wlf_text *base,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	auto *text = reinterpret_cast<windows_text *>(base);
	UINT32 text_length = 0;
	wchar_t *wide = utf8_to_wide(options->text, &text_length);
	wchar_t *family = utf8_to_wide(resolve_generic_family(options->font_family),
		NULL);
	double requested_size = options->font_size * options->raster_scale;
	if (wide == NULL || family == NULL || !isfinite(requested_size) ||
			requested_size <= 0 || requested_size > FLT_MAX) {
		free(wide); free(family); return false;
	}

	IDWriteTextFormat *format = NULL;
	IDWriteTextLayout *layout = NULL;
	DWRITE_TEXT_METRICS metrics = {};
	DWRITE_LINE_METRICS *lines = nullptr;
	UINT32 line_count = 0;
	int natural_width = 0, natural_height = 0, width = 0;
	uint32_t *pixels = nullptr;
	draw_context context = {};
	glyph_renderer renderer;
	DWRITE_FONT_WEIGHT weight = options->weight == WLF_TEXT_FONT_WEIGHT_BOLD ?
		DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE style = options->slant == WLF_TEXT_FONT_SLANT_NORMAL ?
		DWRITE_FONT_STYLE_NORMAL : DWRITE_FONT_STYLE_ITALIC;
	HRESULT hr = text->factory->CreateTextFormat(family, NULL,
		weight, style, DWRITE_FONT_STRETCH_NORMAL, (FLOAT)requested_size,
		L"en-us", &format);
	if (SUCCEEDED(hr))
		hr = format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	if (SUCCEEDED(hr))
		hr = text->factory->CreateTextLayout(wide, text_length,
			format, 1000000.0f, 1000000.0f, &layout);
	free(wide);
	free(family);
	if (FAILED(hr)) goto done;

	hr = layout->GetMetrics(&metrics);
	if (SUCCEEDED(hr)) {
		(void)layout->GetLineMetrics(nullptr, 0, &line_count);
		if (line_count > 0) {
			lines = static_cast<DWRITE_LINE_METRICS *>(
				calloc(line_count, sizeof(*lines)));
			hr = lines != nullptr ? layout->GetLineMetrics(
				lines, line_count, &line_count) : E_OUTOFMEMORY;
		}
	}
	if (FAILED(hr)) goto done;
	natural_width = (int)ceil(metrics.widthIncludingTrailingWhitespace);
	natural_height = (int)ceil(metrics.height);
	raster->metrics.width = natural_width;
	raster->metrics.height = natural_height;
	raster->metrics.baseline = line_count > 0 ? (int)ceil(lines[0].baseline) : 0;
	if (text_length == 0 || natural_width <= 0 || natural_height <= 0) {
		hr = S_OK;
		goto done;
	}

	width = natural_width;
	if (options->max_width > 0) {
		double maximum = options->max_width * options->raster_scale;
		if (!isfinite(maximum) || maximum < 0 || maximum > UINT32_MAX) {
			hr = E_INVALIDARG;
			goto done;
		}
		if (maximum < width) width = (int)ceil(maximum);
	}
	if (width <= 0 || (size_t)natural_height >
			SIZE_MAX / (size_t)width / sizeof(uint32_t)) {
		hr = width <= 0 ? S_OK : E_OUTOFMEMORY;
		goto done;
	}
	pixels = static_cast<uint32_t *>(calloc(
		(size_t)width * (size_t)natural_height, sizeof(*pixels)));
	if (pixels == NULL) { hr = E_OUTOFMEMORY; goto done; }
	context.factory = text->factory;
	context.pixels = pixels;
	context.width = (uint32_t)width;
	context.height = (uint32_t)natural_height;
	context.color = wlf_color_clamp(&options->color);
	hr = layout->Draw(&context, &renderer, 0, 0);
	if (FAILED(hr)) { free(pixels); goto done; }
	raster->width = (uint32_t)width;
	raster->height = (uint32_t)natural_height;
	raster->stride = (uint32_t)width * sizeof(uint32_t);
	raster->data = pixels;
	raster->private_data = pixels;

done:
	free(lines);
	if (layout != NULL) layout->Release();
	if (format != NULL) format->Release();
	return SUCCEEDED(hr);
}

static void windows_text_destroy(struct wlf_text *base) {
	auto *text = reinterpret_cast<windows_text *>(base);
	if (text->factory != NULL) text->factory->Release();
	free(text);
}

static const struct wlf_text_impl windows_text_impl = {
	"windows-directwrite",
	windows_text_rasterize,
	windows_text_raster_destroy,
	windows_text_destroy,
};

struct wlf_windows_text *wlf_windows_text_create(void) {
	auto *text = static_cast<windows_text *>(calloc(1, sizeof(windows_text)));
	if (text == NULL) return NULL;
	HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&text->factory));
	if (FAILED(hr)) {
		wlf_log(WLF_ERROR, "DWriteCreateFactory failed: 0x%08lx",
			(unsigned long)hr);
		free(text);
		return NULL;
	}
	wlf_text_init(&text->base.base, &windows_text_impl);
	return &text->base;
}
