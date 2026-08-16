/**
 * @file swapchain.h
 * @brief Win32 software swapchain presented through GDI.
 */

#ifndef WLF_SWAPCHAIN_WINDOWS_SWAPCHAIN_H
#define WLF_SWAPCHAIN_WINDOWS_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#include <stdbool.h>

struct wlf_window;

struct wlf_swapchain *wlf_windows_swapchain_create(
	struct wlf_window *window, int width, int height,
	const struct wlf_render_format *format);

bool wlf_swapchain_is_windows(const struct wlf_swapchain *swapchain);

#endif /* WLF_SWAPCHAIN_WINDOWS_SWAPCHAIN_H */
