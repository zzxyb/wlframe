/**
 * @file        backend.h
 * @brief       Wayland client backend implementation
 * @details     This backend allows wlframe to run as a Wayland client,
 *              creating windows on an existing Wayland compositor.
 * @author      YaoBing Xiao
 * @date        2025-06-25
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-25, initial version\n
 */

#ifndef WAYLAND_BACKEND_H
#define WAYLAND_BACKEND_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/types/wlf_format_set.h"

#include <stdbool.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

struct wl_data_device_manager;
struct xdg_wm_base;
struct wp_viewporter;
struct wp_cursor_shape_manager_v1;
struct wp_content_type_manager_v1;
struct wp_content_type_v1;
struct xdg_activation_v1;
struct xdg_activation_token_v1;
struct wp_alpha_modifier_v1;
struct zxdg_decoration_manager_v1;
struct xdg_toplevel_icon_manager_v1;
struct xdg_toplevel_icon_v1;
struct wp_fractional_scale_manager_v1;
struct zxdg_output_manager_v1;
struct zwp_text_input_manager_v3;
struct wp_single_pixel_buffer_manager_v1;
struct zwlr_layer_shell_v1;
struct zwp_primary_selection_device_manager_v1;
struct zwp_pointer_gestures_v1;
struct zwp_keyboard_shortcuts_inhibit_manager_v1;

/**
 * @brief Wayland backend specific data
 */
struct wlf_wl_backend {
	struct wlf_backend base;              /**< Base backend structure */

	struct wlf_linked_list interfaces;  /**< List of global interfaces */

	struct wl_display *display;         /**< Wayland display pointer */
	struct wl_registry *registry;       /**< Wayland registry pointer */

	struct {
		struct wl_compositor *compositor;  /**< Wayland compositor interface */
		uint32_t bind_version;
		uint32_t name;
	} wl_compositor;

	struct {
		struct wl_subcompositor *subcompositor;
		uint32_t bind_version;
		uint32_t name;
	} wl_subcompositor;

	struct {
		struct wl_fixes *fixes;
		uint32_t bind_version;
		uint32_t name;
	} wl_fixes;

	struct {
		struct wl_shm *shm;
		struct wlf_render_format_set shm_formats;
		uint32_t bind_version;
		uint32_t name;
	} wl_shm;

	struct {
		struct wl_data_device_manager *data_device_manager;
		uint32_t bind_version;
		uint32_t name;
	} wl_data_device_manager;

	struct {
		struct xdg_wm_base *wm_base;
		uint32_t bind_version;
		uint32_t name;
	} xdg_wm_base;

	struct {
		struct wp_viewporter *viewporter;
		uint32_t bind_version;
		uint32_t name;
	} wp_viewporter;

	struct {
		struct zwlr_layer_shell_v1 *layer_shell_v1;
		uint32_t bind_version;
		uint32_t name;
	} zwlr_layer_shell_v1;

	struct {
		struct xdg_activation_v1 *xdg_activation_v1;
		uint32_t bind_version;
		uint32_t name;
	} xdg_activation_v1;

	struct {
		struct zxdg_output_manager_v1 *output_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} zxdg_output_manager_v1;

	struct {
		struct wp_alpha_modifier_v1 *alpha_modifier_v1;
		uint32_t bind_version;
		uint32_t name;
	} wp_alpha_modifier_v1;

	struct {
		struct wp_content_type_manager_v1 *content_type_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} wp_content_type_manager_v1;

	struct {
		struct zwp_pointer_gestures_v1 *pointer_gestures_v1;
		uint32_t bind_version;
		uint32_t name;
	} zwp_pointer_gestures_v1;

	struct {
		struct zwp_text_input_manager_v3 *text_input_manager_v3;
		uint32_t bind_version;
		uint32_t name;
	} zwp_text_input_manager_v3;

	struct {
		struct wp_single_pixel_buffer_manager_v1 *single_pixel_buffer_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} wp_single_pixel_buffer_manager_v1;

	struct {
		struct zxdg_decoration_manager_v1 *decoration_manager;
		uint32_t bind_version;
		uint32_t name;
	} zxdg_decoration_manager_v1;

	struct {
		struct wp_cursor_shape_manager_v1 *cursor_shape_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} wp_cursor_shape_manager_v1;

	struct {
		struct xdg_toplevel_icon_manager_v1 *toplevel_icon_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} xdg_toplevel_icon_manager_v1;

	struct {
		struct wp_fractional_scale_manager_v1 *fractional_scale_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} wp_fractional_scale_manager_v1;

	struct {
		struct zwp_primary_selection_device_manager_v1 *primary_selection_device_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} zwp_primary_selection_device_manager_v1;

	struct {
		struct zwp_keyboard_shortcuts_inhibit_manager_v1 *keyboard_shortcuts_inhibit_manager_v1;
		uint32_t bind_version;
		uint32_t name;
	} zwp_keyboard_shortcuts_inhibit_manager_v1;

	struct {
		struct wlf_signal global_add;   /**< Signal emitted when a global is added */
		struct wlf_signal global_remove;/**< Signal emitted when a global is removed */
	} events;
};

struct wlf_backend *wayland_backend_create(void);

/**
 * @brief Check if a backend is a Wayland backend
 * @param backend Pointer to the backend to check
 * @return true if the backend is a Wayland backend, false otherwise
 */
bool wlf_backend_is_wayland(const struct wlf_backend *backend);

/**
 * @brief Cast a generic backend to a Wayland backend
 * @param backend Pointer to the generic backend
 * @return Pointer to the Wayland backend, or NULL if the backend is not a Wayland backend
 */
struct wlf_wl_backend *wlf_wl_backend_from_backend(struct wlf_backend *backend);

#endif // WAYLAND_BACKEND_H
