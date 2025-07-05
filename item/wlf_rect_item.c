#include "wlf/item/wlf_rect_item.h"
#include "wlf/render/wlf_render.h"

#include <stdlib.h>
#include <string.h>

static void wlf_rect_item_paint_hook(struct wlf_item *item, struct wlf_renderer *renderer,
									 struct wlf_rect *damage, struct wlf_render_context *context);

struct wlf_rect_item* wlf_rect_item_create(struct wlf_window *window) {
	struct wlf_rect_item *rect_item = calloc(1, sizeof(struct wlf_rect_item));
	if (!rect_item) {
		return NULL;
	}

	struct wlf_item *base_item = wlf_item_create(window);
	if (!base_item) {
		free(rect_item);
		return NULL;
	}

	memcpy(&rect_item->base, base_item, sizeof(struct wlf_item));
	free(base_item);

	rect_item->style.has_fill = false;
	rect_item->style.fill_color = 0x000000FF;
	rect_item->style.has_stroke = false;
	rect_item->style.stroke_color = 0x000000FF;
	rect_item->style.stroke_width = 1.0f;
	rect_item->style.corner_radius = 0.0f;

	struct wlf_item_impl hooks = {0};
	hooks.on_paint = wlf_rect_item_paint_hook;
	wlf_item_set_hooks(&rect_item->base, &hooks);

	return rect_item;
}

void wlf_rect_item_destroy(struct wlf_rect_item *item) {
	if (!item) return;

	wlf_item_destroy(&item->base);

	free(item);
}

void wlf_rect_item_set_fill_color(struct wlf_rect_item *item, uint32_t color) {
	if (!item) return;

	item->style.fill_color = color;
	item->style.has_fill = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_rect_item_set_stroke(struct wlf_rect_item *item, uint32_t color, float width) {
	if (!item) return;

	item->style.stroke_color = color;
	item->style.stroke_width = width;
	item->style.has_stroke = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_rect_item_set_corner_radius(struct wlf_rect_item *item, float radius) {
	if (!item) return;

	item->style.corner_radius = radius;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_rect_item_set_fill_enabled(struct wlf_rect_item *item, bool enable) {
	if (!item) return;

	item->style.has_fill = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_rect_item_set_stroke_enabled(struct wlf_rect_item *item, bool enable) {
	if (!item) return;

	item->style.has_stroke = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

struct wlf_item* wlf_rect_item_get_base(struct wlf_rect_item *item) {
	return item ? &item->base : NULL;
}

static void wlf_rect_item_paint_hook(struct wlf_item *item, struct wlf_renderer *renderer,
									 struct wlf_rect *damage, struct wlf_render_context *context) {
	if (!item || !renderer) return;

	struct wlf_rect_item *rect_item = (struct wlf_rect_item*)item;

	struct wlf_rect rect = item->geometry;

	float alpha = item->opacity * context->opacity_factor;

	if (rect_item->style.has_fill) {
		uint32_t fill_color = rect_item->style.fill_color;

		uint8_t r = (fill_color >> 24) & 0xFF;
		uint8_t g = (fill_color >> 16) & 0xFF;
		uint8_t b = (fill_color >> 8) & 0xFF;
		uint8_t a = (uint8_t)((fill_color & 0xFF) * alpha);
		uint32_t final_color = (r << 24) | (g << 16) | (b << 8) | a;

		if (rect_item->style.corner_radius > 0.0f) {
		} else {
		}

		(void)final_color;
	}

	if (rect_item->style.has_stroke && rect_item->style.stroke_width > 0.0f) {
		uint32_t stroke_color = rect_item->style.stroke_color;

		uint8_t r = (stroke_color >> 24) & 0xFF;
		uint8_t g = (stroke_color >> 16) & 0xFF;
		uint8_t b = (stroke_color >> 8) & 0xFF;
		uint8_t a = (uint8_t)((stroke_color & 0xFF) * alpha);
		uint32_t final_color = (r << 24) | (g << 16) | (b << 8) | a;

		if (rect_item->style.corner_radius > 0.0f) {
		} else {
		}

		(void)final_color;
	}

	(void)damage;
	(void)rect;
}
