#include "wlf/platform/linux/text.h"

#include "wlf/utils/wlf_log.h"

#include <cairo/cairo.h>
#include <limits.h>
#include <math.h>
#include <pango/pangocairo.h>
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

static PangoLayout *create_layout(cairo_t *cr,
		const struct wlf_text_options *options) {
	PangoLayout *layout = pango_cairo_create_layout(cr);
	if (layout == NULL) {
		return NULL;
	}

	PangoFontDescription *font = pango_font_description_new();
	if (font == NULL) {
		g_object_unref(layout);
		return NULL;
	}

	double pixel_size = options->font_size * options->raster_scale;
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
	(void)text;
	if (raster->private_data != NULL) {
		cairo_surface_destroy(raster->private_data);
	}
	*raster = (struct wlf_text_raster){0};
}

static bool linux_text_rasterize(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	(void)text;

	cairo_surface_t *measure_surface = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, 1, 1);
	if (cairo_surface_status(measure_surface) != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(measure_surface);
		return false;
	}
	cairo_t *measure = cairo_create(measure_surface);
	PangoLayout *layout = create_layout(measure, options);
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
