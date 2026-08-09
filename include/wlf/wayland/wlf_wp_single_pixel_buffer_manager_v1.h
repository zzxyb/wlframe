/**
 * @file        wlf_wp_single_pixel_buffer_manager_v1.h
 * @brief       Wayland wp_single_pixel_buffer_manager_v1 protocol wrapper for
 *              wlframe.
 * @details     Wraps the staging single-pixel-buffer-v1 protocol, which lets
 *              clients create a 1×1 wl_buffer whose RGBA colour is specified
 *              as four 32-bit integers rather than by allocating shared memory.
 *
 *              The returned wl_buffer is a plain Wayland buffer; the caller is
 *              responsible for attaching it to a surface and ultimately calling
 *              wl_buffer_destroy().  The colour components use pre-multiplied
 *              alpha and span [0, UINT32_MAX] (UINT32_MAX = 100 %).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_SINGLE_PIXEL_BUFFER_MANAGER_V1_H
#define WAYLAND_WLF_WP_SINGLE_PIXEL_BUFFER_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wl_registry;
struct wl_buffer;
struct wp_single_pixel_buffer_manager_v1;

/**
 * @brief Wrapper around a bound wp_single_pixel_buffer_manager_v1 global.
 */
struct wlf_wp_single_pixel_buffer_manager_v1 {
	struct wp_single_pixel_buffer_manager_v1 *base;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the wp_single_pixel_buffer_manager_v1 global from the
 *        registry.
 */
struct wlf_wp_single_pixel_buffer_manager_v1 *
wlf_wp_single_pixel_buffer_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_wp_single_pixel_buffer_manager_v1_destroy(
	struct wlf_wp_single_pixel_buffer_manager_v1 *manager);

/**
 * @brief Create a 1×1 wl_buffer from 32-bit pre-multiplied RGBA values.
 *
 * The colour components span [0, UINT32_MAX].  UINT32_MAX represents 100 %.
 * The returned wl_buffer is owned by the caller.
 *
 * @param manager  Bound manager.
 * @param r        Red channel (pre-multiplied).
 * @param g        Green channel (pre-multiplied).
 * @param b        Blue channel (pre-multiplied).
 * @param a        Alpha channel.
 * @return A new wl_buffer, or NULL on protocol error.
 */
struct wl_buffer *wlf_wp_single_pixel_buffer_manager_v1_create_u32_rgba_buffer(
	struct wlf_wp_single_pixel_buffer_manager_v1 *manager,
	uint32_t r, uint32_t g, uint32_t b, uint32_t a);

#endif /* WAYLAND_WLF_WP_SINGLE_PIXEL_BUFFER_MANAGER_V1_H */
