#include "wlf/platform/linux/text.h"
#include "wlf/platform/x11/backend.h"
#include "wlf/platform/x11/xresource.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <cairo/cairo.h>
#include <limits.h>
#include <math.h>
#include <pango/pangocairo.h>
#include <pango/pangofc-fontmap.h>
#include <stdlib.h>

static PangoStyle to_pango_style(enum wlf_text_font_slant slant) {
	switch (slant) {
	case WLF_TEXT_FONT_SLANT_ITALIC:
		return PANGO_STYLE_ITALIC;
	case WLF_TEXT_FONT_SLANT_OBLIQUE:
		return PANGO_STYLE_OBLIQUE;
	case WLF_TEXT_FONT_SLANT_NORMAL:
	default:
		return PANGO_STYLE_NORMAL;
	}
}

static PangoWeight to_pango_weight(enum wlf_text_font_weight weight) {
	return weight == WLF_TEXT_FONT_WEIGHT_BOLD ?
		PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
}

static void substitute_xft(FcPattern *pattern, gpointer data) {
	const struct wlf_xft_settings *xft = data;
	static const int rgba[] = { FC_RGBA_NONE, FC_RGBA_RGB, FC_RGBA_BGR,
		FC_RGBA_VRGB, FC_RGBA_VBGR };
	if (xft->present & WLF_XFT_DPI) {
		FcPatternDel(pattern, FC_DPI);
		FcPatternAddDouble(pattern, FC_DPI, xft->dpi);
	}
	if (xft->present & WLF_XFT_ANTIALIAS) {
		FcPatternDel(pattern, FC_ANTIALIAS);
		FcPatternAddBool(pattern, FC_ANTIALIAS, xft->antialias);
	}
	if (xft->present & WLF_XFT_HINTING) {
		FcPatternDel(pattern, FC_HINTING);
		FcPatternAddBool(pattern, FC_HINTING, xft->hinting);
	}
	if (xft->present & WLF_XFT_HINTSTYLE) {
		FcPatternDel(pattern, FC_HINT_STYLE);
		FcPatternAddInteger(pattern, FC_HINT_STYLE, xft->hintstyle);
	}
	if (xft->present & WLF_XFT_RGBA) {
		FcPatternDel(pattern, FC_RGBA);
		FcPatternAddInteger(pattern, FC_RGBA, rgba[xft->rgba]);
	}
	if (xft->present & WLF_XFT_LCDFILTER) {
		FcPatternDel(pattern, FC_LCD_FILTER);
		FcPatternAddInteger(pattern, FC_LCD_FILTER, xft->lcdfilter);
	}
}

static void apply_xft(cairo_t *cr, const struct wlf_xft_settings *xft) {
	if (xft == NULL) {
		return;
	}
	cairo_font_options_t *font = cairo_font_options_create();
	cairo_get_font_options(cr, font);
	if (xft->present & WLF_XFT_ANTIALIAS) {
		cairo_font_options_set_antialias(font, xft->antialias ?
			CAIRO_ANTIALIAS_GRAY : CAIRO_ANTIALIAS_NONE);
	}
	if ((xft->present & WLF_XFT_RGBA) &&
			(!(xft->present & WLF_XFT_ANTIALIAS) || xft->antialias)) {
		static const cairo_subpixel_order_t orders[] = {
			CAIRO_SUBPIXEL_ORDER_DEFAULT, CAIRO_SUBPIXEL_ORDER_RGB,
			CAIRO_SUBPIXEL_ORDER_BGR, CAIRO_SUBPIXEL_ORDER_VRGB,
			CAIRO_SUBPIXEL_ORDER_VBGR,
		};
		cairo_font_options_set_subpixel_order(font, orders[xft->rgba]);
		cairo_font_options_set_antialias(font, xft->rgba == WLF_XFT_RGBA_NONE ?
			CAIRO_ANTIALIAS_GRAY : CAIRO_ANTIALIAS_SUBPIXEL);
	}
	if (xft->present & WLF_XFT_HINTSTYLE) {
		static const cairo_hint_style_t hints[] = { CAIRO_HINT_STYLE_NONE,
			CAIRO_HINT_STYLE_SLIGHT, CAIRO_HINT_STYLE_MEDIUM, CAIRO_HINT_STYLE_FULL };
		cairo_font_options_set_hint_style(font, hints[xft->hintstyle]);
	}
	if ((xft->present & WLF_XFT_HINTING) && !xft->hinting) {
		cairo_font_options_set_hint_style(font, CAIRO_HINT_STYLE_NONE);
	}
	cairo_set_font_options(cr, font);
	cairo_font_options_destroy(font);
}

static bool update_fontmap(struct wlf_linux_text *text,
		const struct wlf_text_options *options) {
	const struct wlf_xft_settings *settings = NULL;
	if (options->backend != NULL && wlf_backend_is_x11(options->backend)) {
		settings = &wlf_x11_backend_from_backend(options->backend)->resources->xft;
	}
	struct wlf_xft_settings empty = {0};
	if (settings == NULL) {
		settings = &empty;
	}
	struct wlf_xft_settings *previous = text->xft;
	if (previous != NULL && previous->present == settings->present &&
			previous->dpi == settings->dpi && previous->antialias == settings->antialias &&
			previous->hinting == settings->hinting && previous->hintstyle == settings->hintstyle &&
			previous->rgba == settings->rgba && previous->lcdfilter == settings->lcdfilter) {
		return true;
	}
	struct wlf_xft_settings *copy = malloc(sizeof(*copy));
	PangoFontMap *map = pango_cairo_font_map_new_for_font_type(CAIRO_FONT_TYPE_FT);
	if (copy == NULL || map == NULL) {
		free(copy);
		if (map != NULL) {
			g_object_unref(map);
		}
		return false;
	}
	*copy = *settings;
	pango_fc_font_map_set_default_substitute(PANGO_FC_FONT_MAP(map),
		substitute_xft, copy, free);
	if (text->fontmap != NULL) {
		g_object_unref(text->fontmap);
	}
	text->fontmap = map;
	text->xft = copy;
	return true;
}

static PangoLayout *create_layout(cairo_t *cr,
		const struct wlf_text_options *options, struct wlf_linux_text *text) {
	if (!update_fontmap(text, options)) {
		return NULL;
	}
	const struct wlf_xft_settings *xft = text->xft;
	apply_xft(cr, xft);
	PangoContext *context = pango_font_map_create_context(text->fontmap);
	pango_cairo_update_context(cr, context);
	if (xft->present & WLF_XFT_DPI) {
		pango_cairo_context_set_resolution(context, xft->dpi);
	}
	PangoLayout *layout = pango_layout_new(context);
	g_object_unref(context);
	if (layout == NULL) {
		return NULL;
	}

	PangoFontDescription *font = pango_font_description_new();
	if (font == NULL) {
		g_object_unref(layout);
		return NULL;
	}

	double pixel_size = options->font_size * options->raster_scale;
	if (xft->present & WLF_XFT_DPI) {
		pixel_size *= xft->dpi / 96;
	}
	if (!isfinite(pixel_size) || pixel_size <= 0 ||
			pixel_size > INT_MAX / (double)PANGO_SCALE) {
		pango_font_description_free(font);
		g_object_unref(layout);
		return NULL;
	}

	pango_font_description_set_family(font,
		options->font_family != NULL ? options->font_family : "sans-serif");
	pango_font_description_set_absolute_size(font,
		pixel_size * PANGO_SCALE);
	pango_font_description_set_style(font, to_pango_style(options->slant));
	pango_font_description_set_weight(font, to_pango_weight(options->weight));
	pango_layout_set_font_description(layout, font);
	pango_font_description_free(font);

	pango_layout_set_text(layout, options->text, -1);
	pango_layout_set_auto_dir(layout, true);
	return layout;
}

static void measure_layout(PangoLayout *layout,
		struct wlf_text_metrics *metrics) {
	PangoRectangle ink;
	PangoRectangle logical;
	pango_layout_get_pixel_extents(layout, &ink, &logical);

	int left = MIN(0, MIN(ink.x, logical.x));
	int top = MIN(0, MIN(ink.y, logical.y));
	int right = MAX(ink.x + ink.width, logical.x + logical.width);
	int bottom = MAX(ink.y + ink.height, logical.y + logical.height);
	*metrics = (struct wlf_text_metrics){
		.left = left,
		.top = top,
		.width = right - left,
		.height = bottom - top,
		.baseline = pango_layout_get_baseline(layout) /
			(double)PANGO_SCALE - top,
	};
}

static void linux_text_raster_destroy(struct wlf_text *text,
		struct wlf_text_raster *raster) {
	WLF_UNUSED(text);
	if (raster->private_data != NULL) {
		cairo_surface_destroy(raster->private_data);
	}
	*raster = (struct wlf_text_raster){0};
}

static bool linux_text_rasterize(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	WLF_UNUSED(text);

	cairo_surface_t *measure_surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, 1, 1);
	if (cairo_surface_status(measure_surface) != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(measure_surface);
		return false;
	}
	cairo_t *measure = cairo_create(measure_surface);
	struct wlf_linux_text *linux_text = wlf_container_of(text, linux_text, base);
	PangoLayout *layout = create_layout(measure, options, linux_text);
	struct wlf_text_metrics metrics = {0};
	if (layout != NULL) {
		measure_layout(layout, &metrics);
	}
	bool measured = layout != NULL &&
		cairo_status(measure) == CAIRO_STATUS_SUCCESS;
	cairo_destroy(measure);
	cairo_surface_destroy(measure_surface);
	if (!measured || metrics.width < 0 || metrics.height < 0 ||
			metrics.width > INT32_MAX || metrics.height > INT32_MAX) {
		if (layout != NULL) {
			g_object_unref(layout);
		}
		return false;
	}

	raster->metrics = metrics;
	if (options->text[0] == '\0' || metrics.width <= 0 ||
			metrics.height <= 0) {
		g_object_unref(layout);
		return true;
	}

	int width = (int)metrics.width;
	int height = (int)metrics.height;
	if (options->max_width > 0) {
		double max_pixel_width = options->max_width * options->raster_scale;
		if (!isfinite(max_pixel_width) || max_pixel_width < 0) {
			g_object_unref(layout);
			return false;
		}
		if (max_pixel_width < width) {
			width = (int)ceil(max_pixel_width);
		}
	}
	if (width <= 0 || height <= 0) {
		g_object_unref(layout);
		return true;
	}

	cairo_surface_t *surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, width, height);
	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "failed to create Cairo text surface: %s",
			cairo_status_to_string(cairo_surface_status(surface)));
		g_object_unref(layout);
		cairo_surface_destroy(surface);
		return false;
	}

	cairo_t *cr = cairo_create(surface);
	apply_xft(cr, linux_text->xft);
	cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	struct wlf_color color = wlf_color_clamp(&options->color);
	cairo_set_source_rgba(cr, color.r, color.g, color.b, color.a);
	cairo_move_to(cr, -metrics.left, -metrics.top);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);
	bool drawn = cairo_status(cr) == CAIRO_STATUS_SUCCESS;
	g_object_unref(layout);
	cairo_destroy(cr);
	if (!drawn) {
		wlf_log(WLF_ERROR, "failed to rasterize text node");
		cairo_surface_destroy(surface);
		return false;
	}

	cairo_surface_flush(surface);
	raster->width = (uint32_t)width;
	raster->height = (uint32_t)height;
	raster->stride = (uint32_t)cairo_image_surface_get_stride(surface);
	raster->data = cairo_image_surface_get_data(surface);
	raster->private_data = surface;
	return true;
}

static void linux_text_destroy(struct wlf_text *text) {
	struct wlf_linux_text *linux_text = wlf_container_of(text, linux_text, base);
	if (linux_text->fontmap != NULL) {
		g_object_unref(linux_text->fontmap);
	}
	free(text);
}

static const struct wlf_text_impl linux_text_impl = {
	.name = "linux-cairo-pango-harfbuzz",
	.rasterize = linux_text_rasterize,
	.destroy_raster = linux_text_raster_destroy,
	.destroy = linux_text_destroy,
};

struct wlf_linux_text *wlf_linux_text_create(void) {
	struct wlf_linux_text *text = calloc(1, sizeof(*text));
	if (text == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate Linux text implementation");
		return NULL;
	}

	wlf_text_init(&text->base, &linux_text_impl);
	return text;
}
