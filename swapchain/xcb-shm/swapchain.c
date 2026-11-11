#include "wlf/swapchain/xcb-shm/swapchain.h"
#include "wlf/window/x11/window.h"
#include "wlf/buffer/wlf_buffer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

struct x11_buffer {
	struct wlf_buffer base;
	void *pixels;
};

static void buffer_destroy(struct wlf_buffer *base) {
	struct x11_buffer *buffer = wlf_container_of(base, buffer, base);
	wlf_buffer_finish(base);
	free(buffer->pixels);
	free(buffer);
}

static bool buffer_begin_access(struct wlf_buffer *base, uint32_t flags,
		void **data, uint32_t *format, size_t *stride) {
	struct x11_buffer *buffer = wlf_container_of(base, buffer, base);
	*data = buffer->pixels;
	*format = WLF_FORMAT_XRGB8888;
	*stride = base->width * 4;
	return true;
}

static void buffer_end_access(struct wlf_buffer *base) {
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.begin_data_ptr_access = buffer_begin_access,
	.end_data_ptr_access = buffer_end_access,
};

static struct wlf_buffer *create_buffer(int width, int height) {
	if (width <= 0 || height <= 0 || width > INT16_MAX || height > INT16_MAX ||
			(size_t)width > SIZE_MAX / 4 / (size_t)height) {
		return NULL;
	}
	struct x11_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	buffer->pixels = calloc(height, (size_t)width * 4);
	if (buffer->pixels == NULL) {
		free(buffer);
		return NULL;
	}
	wlf_buffer_init(&buffer->base, &buffer_impl, width, height);
	return &buffer->base;
}

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
	struct wlf_buffer *buffer = create_buffer(width, height);
	if (buffer == NULL) {
		return false;
	}
	wlf_buffer_drop(base->back);
	base->back = buffer;
	base->width = width;
	base->height = height;
	return true;
}

static void swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	struct wlf_xcb_shm_swapchain *swapchain =
		wlf_xcb_shm_swapchain_from_swapchain(base);
	struct wlf_x11_window *window = wlf_x11_window_from_window(base->window);
	struct x11_buffer *buffer = wlf_container_of(base->back, buffer, base);
	pixman_region32_t clipped;
	pixman_region32_init_rect(&clipped, 0, 0, base->width, base->height);
	if (damage != NULL) {
		pixman_region32_intersect(&clipped, &clipped, (pixman_region32_t *)damage);
	}
	int count;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &count);
	/* Reserve request header and BIG-REQUESTS length word. Split rows to
	 * respect the server's maximum request size even on remote X servers. */
	uint32_t max_pixels = (xcb_get_maximum_request_length(window->backend->xcb) - 7);
	for (int i = 0; i < count; i++) {
		for (int y = rects[i].y1; y < rects[i].y2; y++) {
			for (int x = rects[i].x1; x < rects[i].x2;) {
				uint32_t width = rects[i].x2 - x;
				if (width > max_pixels) {
					width = max_pixels;
				}
				const char *pixels = (const char *)buffer->pixels +
					((size_t)y * base->width + x) * 4;
				xcb_put_image(window->backend->xcb, XCB_IMAGE_FORMAT_Z_PIXMAP,
					window->window, swapchain->gc, width, 1, x, y, 0,
					window->depth, width * 4, (const uint8_t *)pixels);
				x += width;
			}
		}
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
