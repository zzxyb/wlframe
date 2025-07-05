#include "wlf/item/wlf_image_item.h"
#include "wlf/image/wlf_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void wlf_image_item_calculate_layout(struct wlf_image_item *item) {
	if (!item || !item->image) return;

	int img_width = (int)item->image->width;
	int img_height = (int)item->image->height;

	if (img_width <= 0 || img_height <= 0) return;

	struct wlf_rect item_rect = item->base.geometry;

	item->source_rect.x = 0;
	item->source_rect.y = 0;
	item->source_rect.width = img_width;
	item->source_rect.height = img_height;

	switch (item->scale_mode) {
		case WLF_IMAGE_SCALE_NONE:
			item->dest_rect.width = img_width;
			item->dest_rect.height = img_height;
			break;

		case WLF_IMAGE_SCALE_FIT:
			{
				float scale_x = (float)item_rect.width / img_width;
				float scale_y = (float)item_rect.height / img_height;
				float scale = (scale_x < scale_y) ? scale_x : scale_y;

				item->dest_rect.width = (int)(img_width * scale);
				item->dest_rect.height = (int)(img_height * scale);
			}
			break;

		case WLF_IMAGE_SCALE_FILL:
			{
				float scale_x = (float)item_rect.width / img_width;
				float scale_y = (float)item_rect.height / img_height;
				float scale = (scale_x > scale_y) ? scale_x : scale_y;

				int scaled_width = (int)(img_width * scale);
				int scaled_height = (int)(img_height * scale);

				if (scaled_width > item_rect.width) {
					int crop_width = (int)(item_rect.width / scale);
					item->source_rect.x = (img_width - crop_width) / 2;
					item->source_rect.width = crop_width;
				}

				if (scaled_height > item_rect.height) {
					int crop_height = (int)(item_rect.height / scale);
					item->source_rect.y = (img_height - crop_height) / 2;
					item->source_rect.height = crop_height;
				}

				item->dest_rect.width = item_rect.width;
				item->dest_rect.height = item_rect.height;
			}
			break;

		case WLF_IMAGE_SCALE_STRETCH:
			item->dest_rect.width = item_rect.width;
			item->dest_rect.height = item_rect.height;
			break;
	}

	switch (item->alignment) {
		case WLF_IMAGE_ALIGN_TOP_LEFT:
			item->dest_rect.x = 0;
			item->dest_rect.y = 0;
			break;
		case WLF_IMAGE_ALIGN_TOP:
			item->dest_rect.x = (item_rect.width - item->dest_rect.width) / 2;
			item->dest_rect.y = 0;
			break;
		case WLF_IMAGE_ALIGN_TOP_RIGHT:
			item->dest_rect.x = item_rect.width - item->dest_rect.width;
			item->dest_rect.y = 0;
			break;
		case WLF_IMAGE_ALIGN_LEFT:
			item->dest_rect.x = 0;
			item->dest_rect.y = (item_rect.height - item->dest_rect.height) / 2;
			break;
		case WLF_IMAGE_ALIGN_CENTER:
			item->dest_rect.x = (item_rect.width - item->dest_rect.width) / 2;
			item->dest_rect.y = (item_rect.height - item->dest_rect.height) / 2;
			break;
		case WLF_IMAGE_ALIGN_RIGHT:
			item->dest_rect.x = item_rect.width - item->dest_rect.width;
			item->dest_rect.y = (item_rect.height - item->dest_rect.height) / 2;
			break;
		case WLF_IMAGE_ALIGN_BOTTOM_LEFT:
			item->dest_rect.x = 0;
			item->dest_rect.y = item_rect.height - item->dest_rect.height;
			break;
		case WLF_IMAGE_ALIGN_BOTTOM:
			item->dest_rect.x = (item_rect.width - item->dest_rect.width) / 2;
			item->dest_rect.y = item_rect.height - item->dest_rect.height;
			break;
		case WLF_IMAGE_ALIGN_BOTTOM_RIGHT:
			item->dest_rect.x = item_rect.width - item->dest_rect.width;
			item->dest_rect.y = item_rect.height - item->dest_rect.height;
			break;
	}

	item->layout_dirty = false;
}

static void wlf_image_item_paint_hook(struct wlf_item *item, struct wlf_renderer *renderer,
		struct wlf_rect *damage, struct wlf_render_context *context) {
	if (item == NULL || renderer == NULL) {
		return;
	}

	struct wlf_image_item *image_item = (struct wlf_image_item*)item;

	if (image_item->image == NULL) {
		return;
	}

	if (image_item->layout_dirty) {
		wlf_image_item_calculate_layout(image_item);
	}

	float alpha = item->opacity * context->opacity_factor;

	uint32_t tint_color = image_item->tint_color;
	if (image_item->has_tint) {
		uint8_t r = (tint_color >> 24) & 0xFF;
		uint8_t g = (tint_color >> 16) & 0xFF;
		uint8_t b = (tint_color >> 8) & 0xFF;
		uint8_t a = (uint8_t)((tint_color & 0xFF) * alpha);
		tint_color = (r << 24) | (g << 16) | (b << 8) | a;
	} else {
		tint_color = (0xFF << 24) | (0xFF << 16) | (0xFF << 8) | (uint8_t)(0xFF * alpha);
	}
}

static void wlf_image_item_layout_hook(struct wlf_item *item, struct wlf_rect *available);

struct wlf_image_item* wlf_image_item_create(struct wlf_window *window) {
	struct wlf_image_item *image_item = calloc(1, sizeof(struct wlf_image_item));
	if (!image_item) {
		return NULL;
	}

	struct wlf_item *base_item = wlf_item_create(window);
	if (!base_item) {
		free(image_item);
		return NULL;
	}

	memcpy(&image_item->base, base_item, sizeof(struct wlf_item));
	free(base_item);

	image_item->image = NULL;
	image_item->owns_image = false;
	image_item->scale_mode = WLF_IMAGE_SCALE_FIT;
	image_item->alignment = WLF_IMAGE_ALIGN_CENTER;
	image_item->smooth_scaling = true;
	image_item->tint_color = 0xFFFFFFFF;
	image_item->has_tint = false;
	image_item->layout_dirty = true;

	struct wlf_item_impl hooks = {0};
	hooks.on_paint = wlf_image_item_paint_hook;
	hooks.on_layout = wlf_image_item_layout_hook;
	wlf_item_set_hooks(&image_item->base, &hooks);

	return image_item;
}

void wlf_image_item_destroy(struct wlf_image_item *item) {
	if (!item) return;

	if (item->owns_image && item->image) {
		wlf_image_finish(item->image);
		free(item->image);
	}

	wlf_item_destroy(&item->base);

	free(item);
}

void wlf_image_item_set_image(struct wlf_image_item *item, struct wlf_image *image, bool take_ownership) {
	if (!item) return;

	if (item->owns_image && item->image) {
		wlf_image_finish(item->image);
		free(item->image);
	}

	item->image = image;
	item->owns_image = take_ownership;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

bool wlf_image_item_load_from_file(struct wlf_image_item *item, const char *path) {
	if (!item || !path) return false;

	struct wlf_image *image = wlf_image_load(path);
	if (!image) {
		return false;
	}

	wlf_image_item_set_image(item, image, true);
	return true;
}

void wlf_image_item_set_scale_mode(struct wlf_image_item *item, enum wlf_image_scale_mode mode) {
	if (!item) return;

	item->scale_mode = mode;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_image_item_set_alignment(struct wlf_image_item *item, enum wlf_image_align alignment) {
	if (!item) return;

	item->alignment = alignment;
	item->layout_dirty = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_image_item_set_tint(struct wlf_image_item *item, uint32_t color) {
	if (!item) return;

	item->tint_color = color;
	item->has_tint = true;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_image_item_set_tint_enabled(struct wlf_image_item *item, bool enable) {
	if (!item) return;

	item->has_tint = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

void wlf_image_item_set_smooth_scaling(struct wlf_image_item *item, bool enable) {
	if (!item) return;

	item->smooth_scaling = enable;

	wlf_item_mark_dirty(&item->base, NULL);
}

struct wlf_item* wlf_image_item_get_base(struct wlf_image_item *item) {
	return item ? &item->base : NULL;
}

void wlf_image_item_get_natural_size(struct wlf_image_item *item, int *width, int *height) {
	if (!item || !width || !height) {
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}

	if (item->image) {
		*width = (int)item->image->width;
		*height = (int)item->image->height;
	} else {
		*width = 0;
		*height = 0;
	}
}


static void wlf_image_item_layout_hook(struct wlf_item *item, struct wlf_rect *available) {
	if (!item) return;

	struct wlf_image_item *image_item = (struct wlf_image_item*)item;
	image_item->layout_dirty = true;

	(void)available;
}
