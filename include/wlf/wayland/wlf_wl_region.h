/**
 * @file        wlf_wl_region.h
 * @brief       Wayland wl_region wrapper for wlframe.
 * @details     Wraps a wl_region, providing add/subtract rectangle operations
 *              for compositor input and opaque region hints.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_REGION_H
#define WAYLAND_WLF_WL_REGION_H

#include <stdint.h>

struct wl_region;
struct wlf_wl_compositor;

/**
 * @brief Wayland compositor region.
 */
struct wlf_wl_region {
	struct wl_region *wl_region; /**< Underlying Wayland object. */
};

/**
 * @brief Creates a wl_region from a compositor.
 *
 * @param compositor Compositor to create the region from.
 * @return Newly allocated wlf_wl_region, or NULL on failure.
 */
struct wlf_wl_region *wlf_wl_region_create(struct wlf_wl_compositor *compositor);

/**
 * @brief Destroys a wlf_wl_region. Passing NULL is a no-op.
 */
void wlf_wl_region_destroy(struct wlf_wl_region *region);

/**
 * @brief Adds a rectangle to the region.
 */
void wlf_wl_region_add(struct wlf_wl_region *region,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Subtracts a rectangle from the region.
 */
void wlf_wl_region_subtract(struct wlf_wl_region *region,
	int32_t x, int32_t y, int32_t width, int32_t height);

#endif /* WAYLAND_WLF_WL_REGION_H */
