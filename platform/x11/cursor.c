#include "wlf/platform/x11/cursor.h"
#include "wlf/platform/x11/backend.h"
#include "wlf/platform/x11/xresource.h"
#include "wlf/window/x11/window.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"

#include <X11/Xcursor/Xcursor.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <xcb/render.h>

struct wlf_x11_cursor {
	struct wlf_cursor base;
	struct wlf_x11_backend *backend;
	struct wlf_listener changed;
	xcb_font_t font;
	xcb_render_pictformat_t format;
	bool animated;
	xcb_window_t window;
	xcb_cursor_t cache[WLF_CURSOR_SHAPE_ALL_RESIZE + 1];
};

static const struct {
	const char *name, *alias;
	uint16_t glyph;
} shapes[] = {
	{ NULL, NULL, 0 },
	{ "default", "left_ptr", XC_left_ptr },
	{ "context-menu", "left_ptr", XC_left_ptr },
	{ "help", "question_arrow", XC_question_arrow },
	{ "pointer", "hand2", XC_hand2 },
	{ "progress", "left_ptr_watch", XC_watch },
	{ "wait", "watch", XC_watch },
	{ "cell", "plus", XC_plus },
	{ "crosshair", "cross", XC_crosshair },
	{ "text", "xterm", XC_xterm },
	{ "vertical-text", "xterm", XC_xterm },
	{ "alias", "dnd-link", XC_left_ptr },
	{ "copy", "dnd-copy", XC_left_ptr },
	{ "move", "fleur", XC_fleur },
	{ "no-drop", "dnd-none", XC_X_cursor },
	{ "not-allowed", "crossed_circle", XC_X_cursor },
	{ "grab", "openhand", XC_hand2 },
	{ "grabbing", "closedhand", XC_hand2 },
	{ "e-resize", "right_side", XC_right_side },
	{ "n-resize", "top_side", XC_top_side },
	{ "ne-resize", "top_right_corner", XC_top_right_corner },
	{ "nw-resize", "top_left_corner", XC_top_left_corner },
	{ "s-resize", "bottom_side", XC_bottom_side },
	{ "se-resize", "bottom_right_corner", XC_bottom_right_corner },
	{ "sw-resize", "bottom_left_corner", XC_bottom_left_corner },
	{ "w-resize", "left_side", XC_left_side },
	{ "ew-resize", "sb_h_double_arrow", XC_sb_h_double_arrow },
	{ "ns-resize", "sb_v_double_arrow", XC_sb_v_double_arrow },
	{ "nesw-resize", "fd_double_arrow", XC_sizing },
	{ "nwse-resize", "bd_double_arrow", XC_sizing },
	{ "col-resize", "sb_h_double_arrow", XC_sb_h_double_arrow },
	{ "row-resize", "sb_v_double_arrow", XC_sb_v_double_arrow },
	{ "all-scroll", "fleur", XC_fleur },
	{ "zoom-in", "left_ptr", XC_left_ptr },
	{ "zoom-out", "left_ptr", XC_left_ptr },
	{ "dnd-ask", "question_arrow", XC_question_arrow },
	{ "all-resize", "fleur", XC_fleur },
};

static bool request_ok(struct wlf_x11_cursor *cursor, xcb_void_cookie_t cookie) {
	xcb_generic_error_t *error = xcb_request_check(cursor->backend->xcb, cookie);
	bool ok = error == NULL;
	free(error);
	return ok;
}

static xcb_cursor_t load_image(struct wlf_x11_cursor *cursor, XcursorImage *image) {
	if (image->width == 0 || image->height == 0 || image->width > 4096 ||
			image->height > 4096 || image->xhot >= image->width || image->yhot >= image->height) {
		return XCB_CURSOR_NONE;
	}
	xcb_connection_t *xcb = cursor->backend->xcb;
	xcb_pixmap_t pixmap = xcb_generate_id(xcb);
	if (!request_ok(cursor, xcb_create_pixmap_checked(xcb, 32, pixmap,
			cursor->backend->screen->root, image->width, image->height))) {
		return XCB_CURSOR_NONE;
	}
	xcb_gcontext_t gc = xcb_generate_id(xcb);
	if (!request_ok(cursor, xcb_create_gc_checked(xcb, gc, pixmap, 0, NULL))) {
		xcb_free_pixmap(xcb, pixmap);
		return XCB_CURSOR_NONE;
	}
	uint32_t max_pixels = xcb_get_maximum_request_length(xcb) - 7;
	bool swap = xcb_get_setup(xcb)->image_byte_order !=
		(WLF_LITTLE_ENDIAN ? XCB_IMAGE_ORDER_LSB_FIRST : XCB_IMAGE_ORDER_MSB_FIRST);
	uint32_t *row = swap ? malloc(image->width * sizeof(*row)) : NULL;
	if (swap && row == NULL) {
		xcb_free_gc(xcb, gc);
		xcb_free_pixmap(xcb, pixmap);
		return XCB_CURSOR_NONE;
	}
	for (uint32_t y = 0; y < image->height; y++) {
		const uint32_t *pixels = image->pixels + y * image->width;
		if (swap) {
			for (uint32_t x = 0; x < image->width; x++) {
				uint32_t p = pixels[x];
				row[x] = (p >> 24) | ((p >> 8) & 0xff00) |
					((p << 8) & 0xff0000) | (p << 24);
			}
			pixels = row;
		}
		for (uint32_t x = 0; x < image->width;) {
			uint32_t width = image->width - x;
			if (width > max_pixels) {
				width = max_pixels;
			}
			xcb_put_image(xcb, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap, gc,
				width, 1, x, y, 0, 32, width * 4,
				(const uint8_t *)(pixels + x));
			x += width;
		}
	}
	free(row);
	xcb_free_gc(xcb, gc);
	xcb_render_picture_t picture = xcb_generate_id(xcb);
	xcb_cursor_t id = XCB_CURSOR_NONE;
	if (request_ok(cursor, xcb_render_create_picture_checked(xcb, picture,
			pixmap, cursor->format, 0, NULL))) {
		id = xcb_generate_id(xcb);
		if (!request_ok(cursor, xcb_render_create_cursor_checked(xcb, id,
				picture, image->xhot, image->yhot))) {
			id = XCB_CURSOR_NONE;
		}
		xcb_render_free_picture(xcb, picture);
	}
	xcb_free_pixmap(xcb, pixmap);
	return id;
}

static xcb_cursor_t load_theme(struct wlf_x11_cursor *cursor, unsigned int shape) {
	const struct wlf_xcursor_settings *settings = &cursor->backend->resources->xcursor;
	if (!settings->theme_core || cursor->format == XCB_NONE) {
		return XCB_CURSOR_NONE;
	}
	XcursorImages *images = XcursorLibraryLoadImages(shapes[shape].name,
		settings->theme, settings->size);
	if (images == NULL) {
		images = XcursorLibraryLoadImages(shapes[shape].alias, settings->theme, settings->size);
	}
	if (images == NULL) {
		return XCB_CURSOR_NONE;
	}
	int count = cursor->animated ? images->nimage : (images->nimage > 0 ? 1 : 0);
	xcb_render_animcursorelt_t *frames = calloc(count > 0 ? count : 1, sizeof(*frames));
	xcb_cursor_t id = XCB_CURSOR_NONE;
	int loaded = 0;
	if (frames != NULL) {
		for (; loaded < count; loaded++) {
			frames[loaded].cursor = load_image(cursor, images->images[loaded]);
			frames[loaded].delay = images->images[loaded]->delay;
			if (frames[loaded].delay == 0) {
				frames[loaded].delay = 1;
			}
			if (frames[loaded].cursor == XCB_CURSOR_NONE) {
				break;
			}
		}
		if (loaded == count && count == 1) {
			id = frames[0].cursor;
			loaded = 0;
		} else if (loaded == count && count > 1) {
			id = xcb_generate_id(cursor->backend->xcb);
			if (!request_ok(cursor, xcb_render_create_anim_cursor_checked(
					cursor->backend->xcb, id, count, frames))) {
				id = XCB_CURSOR_NONE;
			}
		}
		for (int i = 0; i < loaded; i++) {
			xcb_free_cursor(cursor->backend->xcb, frames[i].cursor);
		}
	}
	free(frames);
	XcursorImagesDestroy(images);
	return id;
}

static xcb_cursor_t get_shape(struct wlf_x11_cursor *cursor, unsigned int shape) {
	if (cursor->cache[shape] != XCB_CURSOR_NONE) {
		return cursor->cache[shape];
	}
	xcb_cursor_t id = load_theme(cursor, shape);
	if (id == XCB_CURSOR_NONE) {
		id = xcb_generate_id(cursor->backend->xcb);
		uint16_t glyph = shapes[shape].glyph;
		if (!request_ok(cursor, xcb_create_glyph_cursor_checked(cursor->backend->xcb,
				id, cursor->font, cursor->font, glyph, glyph + 1,
				0, 0, 0, UINT16_MAX, UINT16_MAX, UINT16_MAX))) {
			return XCB_CURSOR_NONE;
		}
	}
	cursor->cache[shape] = id;
	return id;
}

static bool cursor_set_shape(struct wlf_cursor *base, uint32_t serial,
		enum wlf_cursor_shape shape) {
	struct wlf_x11_cursor *cursor = wlf_container_of(base, cursor, base);
	xcb_cursor_t id = get_shape(cursor, shape);
	if (id == XCB_CURSOR_NONE || cursor->window == XCB_WINDOW_NONE) {
		return false;
	}
	xcb_change_window_attributes(cursor->backend->xcb, cursor->window, XCB_CW_CURSOR, &id);
	xcb_flush(cursor->backend->xcb);
	return true;
}

static void clear_cache(struct wlf_x11_cursor *cursor) {
	for (size_t i = 1; i < sizeof(cursor->cache) / sizeof(cursor->cache[0]); i++) {
		if (cursor->cache[i] != XCB_CURSOR_NONE) {
			xcb_free_cursor(cursor->backend->xcb, cursor->cache[i]);
			cursor->cache[i] = XCB_CURSOR_NONE;
		}
	}
}

static void cursor_destroy(struct wlf_cursor *base) {
	struct wlf_x11_cursor *cursor = wlf_container_of(base, cursor, base);
	wlf_linked_list_remove(&cursor->changed.link);
	clear_cache(cursor);
	xcb_close_font(cursor->backend->xcb, cursor->font);
	free(cursor);
}

static void handle_resources_changed(struct wlf_listener *listener, void *data) {
	struct wlf_x11_cursor *cursor = wlf_container_of(listener, cursor, changed);
	clear_cache(cursor);
	struct wlf_x11_window *window;
	wlf_linked_list_for_each(window, &cursor->backend->windows, link) {
		xcb_cursor_t id = get_shape(cursor, window->window == cursor->window ?
			cursor->base.shape : WLF_CURSOR_SHAPE_DEFAULT);
		if (id != XCB_CURSOR_NONE) {
			xcb_change_window_attributes(cursor->backend->xcb, window->window, XCB_CW_CURSOR, &id);
		}
	}
	xcb_flush(cursor->backend->xcb);
}

static const struct wlf_cursor_impl cursor_impl = {
	.destroy = cursor_destroy,
	.set_shape = cursor_set_shape,
};

struct wlf_cursor *wlf_x11_cursor_create(struct wlf_x11_backend *backend) {
	struct wlf_x11_cursor *cursor = calloc(1, sizeof(*cursor));
	if (cursor == NULL) {
		return NULL;
	}
	wlf_cursor_init(&cursor->base, &cursor_impl);
	cursor->backend = backend;
	cursor->font = xcb_generate_id(backend->xcb);
	if (!request_ok(cursor, xcb_open_font_checked(backend->xcb, cursor->font, 6, "cursor"))) {
		free(cursor);
		return NULL;
	}
	const xcb_query_extension_reply_t *render = xcb_get_extension_data(backend->xcb, &xcb_render_id);
	if (render != NULL && render->present) {
		xcb_render_query_version_reply_t *version = xcb_render_query_version_reply(backend->xcb,
			xcb_render_query_version(backend->xcb, 0, 11), NULL);
		cursor->animated = version != NULL && (version->major_version > 0 || version->minor_version >= 8);
		free(version);
		xcb_render_query_pict_formats_reply_t *formats = xcb_render_query_pict_formats_reply(
			backend->xcb, xcb_render_query_pict_formats(backend->xcb), NULL);
		if (formats != NULL) {
			for (xcb_render_pictforminfo_iterator_t it = xcb_render_query_pict_formats_formats_iterator(formats);
					it.rem; xcb_render_pictforminfo_next(&it)) {
				xcb_render_directformat_t *d = &it.data->direct;
				if (it.data->type == XCB_RENDER_PICT_TYPE_DIRECT && it.data->depth == 32 &&
						d->red_shift == 16 && d->green_shift == 8 && d->blue_shift == 0 &&
						d->alpha_shift == 24 && d->red_mask == 255 && d->green_mask == 255 &&
						d->blue_mask == 255 && d->alpha_mask == 255) {
					cursor->format = it.data->id;
					break;
				}
			}
			free(formats);
		}
	}
	cursor->changed.notify = handle_resources_changed;
	wlf_signal_add(&backend->resources->events.changed, &cursor->changed);
	return &cursor->base;
}

void wlf_x11_cursor_set_window(struct wlf_cursor *base, xcb_window_t window) {
	if (base == NULL) {
		return;
	}
	struct wlf_x11_cursor *cursor = wlf_container_of(base, cursor, base);
	cursor->window = window;
	if (window != XCB_WINDOW_NONE) {
		cursor_set_shape(base, 1, base->shape);
	}
}

void wlf_x11_cursor_forget_window(struct wlf_cursor *base, xcb_window_t window) {
	if (base == NULL) {
		return;
	}
	struct wlf_x11_cursor *cursor = wlf_container_of(base, cursor, base);
	if (cursor->window == window) {
		cursor->window = XCB_WINDOW_NONE;
	}
}
