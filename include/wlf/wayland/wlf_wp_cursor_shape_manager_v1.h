/**
 * @file        wlf_wp_cursor_shape_manager_v1.h
 * @brief       Wayland wp_cursor_shape_manager_v1 protocol wrapper for wlframe.
 * @details     Implements the staging cursor-shape-v1 protocol, which provides
 *              an enumerated alternative to wl_pointer.set_cursor / tablet
 *              set_cursor for setting the cursor image.  The compositor
 *              renders a standard cursor shape instead of a client-supplied
 *              surface.
 *
 *              Usage:
 *                1. Bind wlf_wp_cursor_shape_manager_v1 from the registry.
 *                2. For each wl_pointer, call
 *                   wlf_wp_cursor_shape_manager_v1_get_pointer() to obtain a
 *                   per-pointer wlf_wp_cursor_shape_device_v1.
 *                   (Optionally, call get_tablet_tool_v2 for tablet tools.)
 *                3. Inside a pointer enter/motion handler, call
 *                   wlf_wp_cursor_shape_device_v1_set_shape() with the enter
 *                   serial and the desired shape.
 *                4. Destroy the device object when done (or when the seat
 *                   loses the pointer capability).
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_WP_CURSOR_SHAPE_MANAGER_V1_H
#define WAYLAND_WLF_WP_CURSOR_SHAPE_MANAGER_V1_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/types/wlf_cursor.h"

#include <stdint.h>

struct wl_pointer;
struct wl_registry;
struct wp_cursor_shape_manager_v1;
struct wp_cursor_shape_device_v1;
struct zwp_tablet_tool_v2;

/**
 * @brief Cursor shape identifiers (mirrors wp_cursor_shape_device_v1.shape).
 *
 * Values match the protocol enum directly.  Names follow the CSS cursor spec
 * with the addition of wlframe-style WLF_ prefix.
 */
/**
 * @brief Wrapper around a bound wp_cursor_shape_manager_v1 global.
 */
struct wlf_wp_cursor_shape_manager_v1 {
	struct wp_cursor_shape_manager_v1 *base;
	uint32_t version;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Per-device cursor shape controller.
 *
 * Created by wlf_wp_cursor_shape_manager_v1_get_pointer() or
 * wlf_wp_cursor_shape_manager_v1_get_tablet_tool_v2().
 * The caller owns and must destroy this object.
 */
struct wlf_wp_cursor_shape_device_v1 {
	struct wp_cursor_shape_device_v1 *base;
	uint32_t version;

	struct {
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the wp_cursor_shape_manager_v1 global from the registry.
 */
struct wlf_wp_cursor_shape_manager_v1 *wlf_wp_cursor_shape_manager_v1_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the manager and free its resources.
 */
void wlf_wp_cursor_shape_manager_v1_destroy(
	struct wlf_wp_cursor_shape_manager_v1 *manager);

/**
 * @brief Create a cursor shape controller for a wl_pointer.
 *
 * @param manager  Bound manager.
 * @param pointer  wl_pointer to control.
 * @return Newly allocated device, or NULL on failure.
 */
struct wlf_wp_cursor_shape_device_v1 *
wlf_wp_cursor_shape_manager_v1_get_pointer(
	struct wlf_wp_cursor_shape_manager_v1 *manager,
	struct wl_pointer *pointer);

/**
 * @brief Create a cursor shape controller for a zwp_tablet_tool_v2.
 *
 * @param manager      Bound manager.
 * @param tablet_tool  zwp_tablet_tool_v2 to control.
 * @return Newly allocated device, or NULL on failure.
 */
struct wlf_wp_cursor_shape_device_v1 *
wlf_wp_cursor_shape_manager_v1_get_tablet_tool_v2(
	struct wlf_wp_cursor_shape_manager_v1 *manager,
	struct zwp_tablet_tool_v2 *tablet_tool);

/**
 * @brief Set the cursor shape for the device.
 *
 * Must be called with the serial from the most recent wl_pointer.enter or
 * zwp_tablet_tool_v2.proximity_in event.  Requests with a stale serial are
 * silently ignored by the compositor.
 *
 * @param device  Cursor shape device.
 * @param serial  Enter/proximity serial.
 * @param shape   One of the wlf_cursor_shape enum values.
 */
void wlf_wp_cursor_shape_device_v1_set_shape(
	struct wlf_wp_cursor_shape_device_v1 *device, uint32_t serial,
	enum wlf_cursor_shape shape);

/**
 * @brief Destroy the per-device cursor shape controller.
 */
void wlf_wp_cursor_shape_device_v1_destroy(
	struct wlf_wp_cursor_shape_device_v1 *device);

#endif /* WAYLAND_WLF_WP_CURSOR_SHAPE_MANAGER_V1_H */
