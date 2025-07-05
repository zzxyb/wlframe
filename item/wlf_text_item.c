#include "wlf/item/wlf_text_item.h"

#include <stdlib.h>
#include <string.h>

static void wlf_text_item_paint_hook(struct wlf_item *item, struct wlf_renderer *renderer,
									 struct wlf_rect *damage, struct wlf_render_context *context);
static void wlf_text_item_layout_hook(struct wlf_item *item, struct wlf_rect *available);
static void wlf_text_item_calculate_layout(struct wlf_text_item *item);
static void wlf_text_item_init_default_style(struct wlf_text_style *style);

struct wlf_text_item* wlf_text_item_create(struct wlf_window *window) {
	struct wlf_text_item *text_item = calloc(1, sizeof(struct wlf_text_item));
	if (!text_item) {
		return NULL;
	}

	struct wlf_item *base_item = wlf_item_create(window);
	if (!base_item) {
		free(text_item);
		return NULL;
	}

	memcpy(&text_item->base, base_item, sizeof(struct wlf_item));
	free(base_item);

	text_item->text = NULL;
	text_item->text_length = 0;
	text_item->max_width = 0;
	text_item->max_height = 0;
	text_item->layout_dirty = true;
	text_item->font_cache = NULL;

	wlf_text_item_init_default_style(&text_item->style);

	struct wlf_item_impl hooks = {0};
	hooks.on_paint = wlf_text_item_paint_hook;
	hooks.on_layout = wlf_text_item_layout_hook;
	wlf_item_set_hooks(&text_item->base, &hooks);

	return text_item;
}

void wlf_text_item_destroy(struct wlf_text_item *item) {
	if (!item) return;

	free(item->text);

	if (item->font_cache) {
		item->font_cache = NULL;
	}

	free(item->style.font_family);

	wlf_item_destroy(&item->base);

	free(item);
}

void wlf_text_item_set_text(struct wlf_text_item *item, const char *text) {
	if (!item) return;

	free(item->text);

	if (text) {
		item->text_length = strlen(text);
		item->text = malloc(item->text_length + 1);
		if (item->text) {
			strcpy(item->text, text);
		} else {
			item->text_length = 0;
		}
	} else {
		item->text = NULL;
		item->text_length = 0;
	}

	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_font_family(struct wlf_text_item *item, const char *font_family) {
	if (!item) return;

	free(item->style.font_family);

	if (font_family) {
		item->style.font_family = malloc(strlen(font_family) + 1);
		if (item->style.font_family) {
			strcpy(item->style.font_family, font_family);
		}
	} else {
		item->style.font_family = NULL;
	}

	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_font_size(struct wlf_text_item *item, int size) {
	if (!item || size <= 0) return;

	item->style.font_size = size;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_color(struct wlf_text_item *item, uint32_t color) {
	if (!item) return;

	item->style.color = color;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_alignment(struct wlf_text_item *item, enum wlf_text_align alignment) {
	if (!item) return;

	item->style.alignment = alignment;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_valignment(struct wlf_text_item *item, enum wlf_text_valign valignment) {
	if (!item) return;

	item->style.valignment = valignment;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_wrap_mode(struct wlf_text_item *item, enum wlf_text_wrap wrap_mode) {
	if (!item) return;

	item->style.wrap_mode = wrap_mode;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_style_flags(struct wlf_text_item *item, bool bold, bool italic,
								   bool underline, bool strikethrough) {
	if (!item) return;

	item->style.bold = bold;
	item->style.italic = italic;
	item->style.underline = underline;
	item->style.strikethrough = strikethrough;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_shadow(struct wlf_text_item *item, uint32_t color,
							   float offset_x, float offset_y, float blur) {
	if (!item) return;

	item->style.shadow_color = color;
	item->style.shadow_offset_x = offset_x;
	item->style.shadow_offset_y = offset_y;
	item->style.shadow_blur = blur;
	item->style.has_shadow = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_outline(struct wlf_text_item *item, uint32_t color, float width) {
	if (!item) return;

	item->style.outline_color = color;
	item->style.outline_width = width;
	item->style.has_outline = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_shadow_enabled(struct wlf_text_item *item, bool enable) {
	if (!item) return;

	item->style.has_shadow = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_outline_enabled(struct wlf_text_item *item, bool enable) {
	if (!item) return;

	item->style.has_outline = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_text_item_set_max_size(struct wlf_text_item *item, int max_width, int max_height) {
	if (!item) return;

	item->max_width = max_width;
	item->max_height = max_height;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

struct wlf_item* wlf_text_item_get_base(struct wlf_text_item *item) {
	return item ? &item->base : NULL;
}

struct wlf_rect wlf_text_item_get_text_bounds(struct wlf_text_item *item) {
	if (!item) {
		return (struct wlf_rect){0, 0, 0, 0};
	}

	if (item->layout_dirty) {
		wlf_text_item_calculate_layout(item);
	}

	return item->text_bounds;
}

void wlf_text_item_measure_text(struct wlf_text_item *item, int *width, int *height) {
	if (!item || !width || !height) {
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}

	if (item->layout_dirty) {
		wlf_text_item_calculate_layout(item);
	}

	*width = item->text_bounds.width;
	*height = item->text_bounds.height;
}

static void wlf_text_item_init_default_style(struct wlf_text_style *style) {
	if (!style) return;

	style->font_family = NULL;
	style->font_size = 12;
	style->bold = false;
	style->italic = false;
	style->underline = false;
	style->strikethrough = false;

	style->color = 0x000000FF;
	style->background_color = 0x00000000;
	style->has_background = false;

	style->alignment = WLF_TEXT_ALIGN_LEFT;
	style->valignment = WLF_TEXT_VALIGN_TOP;
	style->wrap_mode = WLF_TEXT_WRAP_WORD;
	style->line_spacing = 1.0f;

	style->has_shadow = false;
	style->shadow_color = 0x000000AA;
	style->shadow_offset_x = 1.0f;
	style->shadow_offset_y = 1.0f;
	style->shadow_blur = 2.0f;

	style->has_outline = false;
	style->outline_color = 0x000000FF;
	style->outline_width = 1.0f;
}

static void wlf_text_item_paint_hook(struct wlf_item *item, struct wlf_renderer *renderer,
									 struct wlf_rect *damage, struct wlf_render_context *context) {
	if (!item || !renderer) return;

	struct wlf_text_item *text_item = (struct wlf_text_item*)item;

	if (!text_item->text || text_item->text_length == 0) return;

	if (text_item->layout_dirty) {
		wlf_text_item_calculate_layout(text_item);
	}

	float alpha = item->opacity * context->opacity_factor;

	uint32_t text_color = text_item->style.color;

	uint8_t r = (text_color >> 24) & 0xFF;
	uint8_t g = (text_color >> 16) & 0xFF;
	uint8_t b = (text_color >> 8) & 0xFF;
	uint8_t a = (uint8_t)((text_color & 0xFF) * alpha);
	uint32_t final_color = (r << 24) | (g << 16) | (b << 8) | a;

	if (text_item->style.has_background) {
		uint32_t bg_color = text_item->style.background_color;
		uint8_t bg_r = (bg_color >> 24) & 0xFF;
		uint8_t bg_g = (bg_color >> 16) & 0xFF;
		uint8_t bg_b = (bg_color >> 8) & 0xFF;
		uint8_t bg_a = (uint8_t)((bg_color & 0xFF) * alpha);
		uint32_t final_bg_color = (bg_r << 24) | (bg_g << 16) | (bg_b << 8) | bg_a;

		(void)final_bg_color;
	}

	if (text_item->style.has_shadow) {
		uint32_t shadow_color = text_item->style.shadow_color;
		uint8_t s_r = (shadow_color >> 24) & 0xFF;
		uint8_t s_g = (shadow_color >> 16) & 0xFF;
		uint8_t s_b = (shadow_color >> 8) & 0xFF;
		uint8_t s_a = (uint8_t)((shadow_color & 0xFF) * alpha);
		uint32_t final_shadow_color = (s_r << 24) | (s_g << 16) | (s_b << 8) | s_a;

		(void)final_shadow_color;
	}

	if (text_item->style.has_outline) {
		uint32_t outline_color = text_item->style.outline_color;
		uint8_t o_r = (outline_color >> 24) & 0xFF;
		uint8_t o_g = (outline_color >> 16) & 0xFF;
		uint8_t o_b = (outline_color >> 8) & 0xFF;
		uint8_t o_a = (uint8_t)((outline_color & 0xFF) * alpha);
		uint32_t final_outline_color = (o_r << 24) | (o_g << 16) | (o_b << 8) | o_a;

		(void)final_outline_color;
	}

	(void)damage;
	(void)final_color;
}

static void wlf_text_item_layout_hook(struct wlf_item *item, struct wlf_rect *available) {
	if (!item) return;

	struct wlf_text_item *text_item = (struct wlf_text_item*)item;
	text_item->layout_dirty = true;

	(void)available;
}

static void wlf_text_item_calculate_layout(struct wlf_text_item *item) {
	if (!item || !item->text) return;

	int char_width = item->style.font_size * 0.6f;
	int line_height = item->style.font_size * item->style.line_spacing;

	int text_width = 0;
	int text_height = line_height;

	if (item->style.wrap_mode == WLF_TEXT_WRAP_NONE) {
		text_width = (int)item->text_length * char_width;
	} else {
		int max_width = item->max_width > 0 ? item->max_width : item->base.geometry.width;
		if (max_width <= 0) max_width = 200;

		int chars_per_line = max_width / char_width;
		if (chars_per_line <= 0) chars_per_line = 1;

		int lines = ((int)item->text_length + chars_per_line - 1) / chars_per_line;
		if (lines <= 0) lines = 1;

		text_width = max_width;
		text_height = lines * line_height;
	}

	if (item->max_width > 0 && text_width > item->max_width) {
		text_width = item->max_width;
	}
	if (item->max_height > 0 && text_height > item->max_height) {
		text_height = item->max_height;
	}

	item->text_bounds.width = text_width;
	item->text_bounds.height = text_height;

	struct wlf_rect item_rect = item->base.geometry;

	switch (item->style.alignment) {
		case WLF_TEXT_ALIGN_LEFT:
			item->text_bounds.x = 0;
			break;
		case WLF_TEXT_ALIGN_CENTER:
			item->text_bounds.x = (item_rect.width - text_width) / 2;
			break;
		case WLF_TEXT_ALIGN_RIGHT:
			item->text_bounds.x = item_rect.width - text_width;
			break;
		case WLF_TEXT_ALIGN_JUSTIFY:
			item->text_bounds.x = 0;
			break;
	}

	switch (item->style.valignment) {
		case WLF_TEXT_VALIGN_TOP:
			item->text_bounds.y = 0;
			break;
		case WLF_TEXT_VALIGN_CENTER:
			item->text_bounds.y = (item_rect.height - text_height) / 2;
			break;
		case WLF_TEXT_VALIGN_BOTTOM:
			item->text_bounds.y = item_rect.height - text_height;
			break;
	}

	item->layout_dirty = false;
}
