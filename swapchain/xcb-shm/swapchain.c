#include "wlf/swapchain/xcb-shm/swapchain.h"
#include "wlf/window/x11/window.h"
#include "wlf/buffer/xcb-shm/buffer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static void swapchain_destroy(struct wlf_swapchain *base) {
	struct wlf_xcb_shm_swapchain *swapchain =
		wlf_xcb_shm_swapchain_from_swapchain(base);
	struct wlf_x11_window *window = wlf_x11_window_from_window(base->window);
	if (swapchain->gc != XCB_NONE) {
		xcb_free_gc(window->backend->xcb, swapchain->gc);
	}
	wlf_buffer_drop(base->back);
	free(swapchain);
}

static bool swapchain_resize(struct wlf_swapchain *base, int width, int height) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base->window);
	struct wlf_xcb_shm_buffer *buffer = wlf_xcb_shm_buffer_create(
		window->backend->xcb, width, height, base->format.format);
	if (buffer == NULL) {
		return false;
	}
	wlf_buffer_drop(base->back);
	base->back = &buffer->base;
	base->width = width;
	base->height = height;
	return true;
}

static void swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	struct wlf_xcb_shm_swapchain *swapchain =
		wlf_xcb_shm_swapchain_from_swapchain(base);
	struct wlf_x11_window *window = wlf_x11_window_from_window(base->window);
	struct wlf_xcb_shm_buffer *buffer = wlf_xcb_shm_buffer_from_buffer(base->back);
	assert(!base->back->accessing_data_ptr);
	pixman_region32_t clipped;
	pixman_region32_init_rect(&clipped, 0, 0, base->width, base->height);
	if (damage != NULL) {
		pixman_region32_intersect(&clipped, &clipped, damage);
	}
	int count;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &count);
	for (int i = 0; i < count; i++) {
		const pixman_box32_t *r = &rects[i];
		xcb_shm_put_image(window->backend->xcb, window->window, swapchain->gc,
			base->width, base->height, r->x1, r->y1,
			r->x2 - r->x1, r->y2 - r->y1, r->x1, r->y1,
			window->depth, XCB_IMAGE_FORMAT_Z_PIXMAP, false, buffer->segment, 0);
	}
	if (count > 0) {
		wlf_xcb_shm_buffer_mark_pending(buffer);
	}
	pixman_region32_fini(&clipped);
	xcb_flush(window->backend->xcb);
}

static const struct wlf_swapchain_impl swapchain_impl = {
	.destroy = swapchain_destroy,
	.resize = swapchain_resize,
	.present = swapchain_present,
};

struct wlf_swapchain *wlf_xcb_shm_swapchain_create(struct wlf_window *base,
		int width, int height, const struct wlf_render_format *format) {
	struct wlf_x11_window *window = wlf_x11_window_from_window(base);
	const xcb_setup_t *setup = xcb_get_setup(window->backend->xcb);
	/* XRGB8888 is supported only with a matching native TrueColor visual. */
	if (format->format != WLF_FORMAT_XRGB8888 || window->depth != 24 ||
			setup->image_byte_order != (WLF_LITTLE_ENDIAN ? XCB_IMAGE_ORDER_LSB_FIRST :
				XCB_IMAGE_ORDER_MSB_FIRST)) {
		return NULL;
	}
	bool visual_supported = false;
	for (xcb_depth_iterator_t d = xcb_screen_allowed_depths_iterator(window->backend->screen);
			d.rem; xcb_depth_next(&d)) {
		for (xcb_visualtype_iterator_t v = xcb_depth_visuals_iterator(d.data);
				v.rem; xcb_visualtype_next(&v)) {
			if (v.data->visual_id == window->backend->screen->root_visual &&
					v.data->_class == XCB_VISUAL_CLASS_TRUE_COLOR &&
					v.data->red_mask == 0xff0000 && v.data->green_mask == 0xff00 &&
					v.data->blue_mask == 0xff) {
				visual_supported = true;
			}
		}
	}
	if (!visual_supported) {
		return NULL;
	}
	bool supported = false;
	for (xcb_format_iterator_t it = xcb_setup_pixmap_formats_iterator(setup);
			it.rem; xcb_format_next(&it)) {
		if (it.data->depth == 24 && it.data->bits_per_pixel == 32) {
			supported = true;
		}
	}
	if (!supported) {
		return NULL;
	}
	struct wlf_xcb_shm_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	wlf_swapchain_init(&swapchain->base, NULL, &swapchain_impl, width, height);
	swapchain->base.window = base;
	if (!wlf_render_format_copy(&swapchain->base.format, format) ||
			!swapchain_resize(&swapchain->base, width, height)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	swapchain->gc = xcb_generate_id(window->backend->xcb);
	xcb_generic_error_t *error = xcb_request_check(window->backend->xcb,
		xcb_create_gc_checked(window->backend->xcb, swapchain->gc, window->window, 0, NULL));
	if (error != NULL) {
		free(error);
		swapchain->gc = XCB_NONE;
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	return &swapchain->base;
}

bool wlf_swapchain_is_xcb_shm(const struct wlf_swapchain *swapchain) {
	return swapchain != NULL && swapchain->impl == &swapchain_impl;
}

struct wlf_xcb_shm_swapchain *wlf_xcb_shm_swapchain_from_swapchain(
		struct wlf_swapchain *swapchain) {
	assert(swapchain->impl == &swapchain_impl);

	struct wlf_xcb_shm_swapchain *xcb_shm_swapchain =
		wlf_container_of(swapchain, xcb_shm_swapchain, base);

	return xcb_shm_swapchain;
}
