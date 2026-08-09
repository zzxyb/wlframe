#include "wlf/swapchain/wlf_swapchain.h"
#include "wlf/swapchain/shm/swapchain.h"
#include "wlf/window/wlf_window.h"
#include "wlf/config.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/renderer/vulkan/renderer.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#include "wlf/swapchain/egl/swapchain.h"
#include "wlf/swapchain/vulkan/swapchain.h"
#endif

#include <assert.h>
#include <stdlib.h>

void wlf_swapchain_init(struct wlf_swapchain *swapchain, struct wlf_allocator *allocator,
		const struct wlf_swapchain_impl *impl, int width, int height) {
	assert(impl->destroy != NULL);
	*swapchain = (struct wlf_swapchain){
		.impl = impl,
		.allocator = allocator,
		.width = width,
		.height = height,
	};

	wlf_signal_init(&swapchain->events.destroy);
}

struct wlf_buffer *wlf_swapchain_get_back_buffer(
		struct wlf_swapchain *swapchain) {
	return swapchain != NULL ? swapchain->back : NULL;
}

struct wlf_swapchain *wlf_swapchain_auto_create(struct wlf_window *window, int width,
		int height, const struct wlf_render_format *format) {
	assert(width > 0 && height > 0);
	struct wlf_swapchain *swapchain = NULL;
#if WLF_HAS_LINUX_PLATFORM
	if (wlf_renderer_is_pixman(window->state.renderer)) {
		swapchain = wlf_shm_swapchain_create(window, width, height, format);
	} else if (wlf_renderer_is_gles(window->state.renderer)) {
		swapchain = wlf_egl_swapchain_create(window, width, height, format);
	} else if (wlf_renderer_is_vk(window->state.renderer)) {
		swapchain = wlf_vk_swapchain_create(window, width, height, format);
	}
#endif

	if (swapchain != NULL) {
		swapchain->window = window;
	}

	return swapchain;
}

void wlf_swapchain_destroy(struct wlf_swapchain *swapchain) {
	if (swapchain == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&swapchain->events.destroy, swapchain);

	if (swapchain->allocator != NULL) {
		wlf_allocator_destroy(swapchain->allocator);
		swapchain->allocator = NULL;
	}

	assert(wlf_linked_list_empty(&swapchain->events.destroy.listener_list));
	wlf_render_format_finish(&swapchain->format);
	if (swapchain->impl && swapchain->impl->destroy) {
		swapchain->impl->destroy(swapchain);
	} else {
		free(swapchain);
	}
}

bool wlf_swapchain_resize(struct wlf_swapchain *swapchain, int width, int height) {
	return swapchain->impl->resize(swapchain, width, height);
}

void wlf_swapchain_present(struct wlf_swapchain *swapchain,
		const pixman_region32_t *damage) {
	swapchain->impl->present(swapchain, damage);
}
