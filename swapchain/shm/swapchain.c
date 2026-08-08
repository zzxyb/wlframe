#include "wlf/swapchain/shm/swapchain.h"
#include "wlf/allocator/wlf_allocator.h"
#include "wlf/buffer/shm/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/allocator/shm/allocator.h"
#include "wlf/window/wlf_window.h"
#include "wlf/platform/wayland/backend.h"

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <wayland-client-protocol.h>

static void swapchain_destroy(struct wlf_swapchain *swapchain) {
	struct wlf_shm_swapchain *shm_swapchain =
		wlf_shm_swapchain_from_swapchain(swapchain);
	wlf_buffer_drop(shm_swapchain->front);
	wlf_buffer_drop(shm_swapchain->back);
	free(shm_swapchain);
}

static void swapchain_present(struct wlf_swapchain *swapchain,
		const pixman_region32_t *damage) {
	struct wlf_shm_swapchain *shm_swapchain =
		wlf_shm_swapchain_from_swapchain(swapchain);

	struct wl_surface *surface =
		wlf_window_native_handle(shm_swapchain->base.window);
	if (surface == NULL) {
		wlf_log(WLF_ERROR, "Wayland shm swapchain requires wl_surface");
		return;
	}

	struct wlf_shm_buffer *buf =
		wlf_shm_buffer_from_buffer(shm_swapchain->back);

	wl_surface_attach(surface, buf->wl_buffer, 0, 0);
	int nrects = 0;
	pixman_box32_t *rects = damage != NULL ?
		pixman_region32_rectangles((pixman_region32_t *)damage, &nrects) : NULL;
	if (nrects > 0) {
		for (int i = 0; i < nrects; i++) {
			const pixman_box32_t *r = &rects[i];
			wl_surface_damage_buffer(surface,
				r->x1, r->y1, r->x2 - r->x1, r->y2 - r->y1);
		}
	} else {
		wl_surface_damage_buffer(surface, 0, 0, INT32_MAX, INT32_MAX);
	}

	wl_surface_commit(surface);

	struct wlf_buffer *tmp = shm_swapchain->front;
	shm_swapchain->front = shm_swapchain->back;
	shm_swapchain->back = tmp;
}

static bool swapchain_resize(struct wlf_swapchain *swapchain, int width,
		int height) {
	swapchain->width = width;
	swapchain->height = height;

	return true;
}

static const struct wlf_swapchain_impl swapchain_impl = {
	.destroy = swapchain_destroy,
	.resize = swapchain_resize,
	.present = swapchain_present,
};

struct wlf_swapchain *wlf_shm_swapchain_create(struct wlf_window *window,
		int width, int height, const struct wlf_render_format *format) {
	struct wlf_shm_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_shm_swapchain");
		return NULL;
	}

	struct wlf_wl_backend *wl_backend = wlf_wl_backend_from_backend(window->state.backend);
	struct wlf_allocator *allocator = wlf_shm_allocator_create(wl_backend->wl_shm.shm);
	if (allocator == NULL) {
		free(swapchain);
		return NULL;
	}

	wlf_swapchain_init(&swapchain->base, allocator, &swapchain_impl, width, height);
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	swapchain->front = wlf_allocator_create_buffer(allocator, width, height, format);
	if (swapchain->front == NULL) {
		wlf_log(WLF_ERROR, "failed to allocate front buffer");
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	swapchain->back = wlf_allocator_create_buffer(allocator, width, height, format);
	if (swapchain->back == NULL) {
		wlf_log(WLF_ERROR, "failed to allocate back buffer");
		wlf_buffer_drop(swapchain->front);
		swapchain->front = NULL;
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	return &swapchain->base;
}

bool wlf_swapchain_is_shm(const struct wlf_swapchain *swapchain) {
	return swapchain->impl == &swapchain_impl;
}

struct wlf_shm_swapchain *wlf_shm_swapchain_from_swapchain(
		struct wlf_swapchain *swapchain) {
	assert(swapchain->impl == &swapchain_impl);

	struct wlf_shm_swapchain *shm_swapchain =
		wlf_container_of(swapchain, shm_swapchain, base);

	return shm_swapchain;
}
