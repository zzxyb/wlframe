/**
 * @file swapchain.h
 * @brief CAMetalLayer-backed wlframe swapchain.
 */

#ifndef WLF_SWAPCHAIN_METAL_SWAPCHAIN_H
#define WLF_SWAPCHAIN_METAL_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

struct wlf_mtl_swapchain {
	struct wlf_swapchain base;
	struct wlf_buffer *back;
	void *layer; /**< Retained CAMetalLayer. */
};

struct wlf_mtl_swapchain *wlf_mtl_swapchain_create(
	struct wlf_window *window, int width, int height,
	const struct wlf_render_format *format);

bool wlf_swapchain_is_mtl(const struct wlf_swapchain *swapchain);

struct wlf_mtl_swapchain *wlf_mtl_swapchain_from_swapchain(
	struct wlf_swapchain *swapchain);

void *wlf_mtl_swapchain_get_layer(const struct wlf_mtl_swapchain *swapchain);

#endif /* WLF_SWAPCHAIN_METAL_SWAPCHAIN_H */
