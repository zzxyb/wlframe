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
struct wlf_wl_seat;

/**
 * @brief Wayland backend specific data
 */
struct wlf_wl_backend {
	struct wlf_backend base;              /**< Base backend structure */

	struct wlf_linked_list interfaces;  /**< List of global interfaces */

	struct wl_display *display;         /**< Wayland display pointer */
	struct wl_registry *registry;       /**< Wayland registry pointer */
	struct {
		struct wlf_wl_seat *seat; /**< Bound wlf_wl_seat protocol object. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_seat; /**< Bound wl_seat global and registry metadata. */

	struct {
		struct wl_compositor *compositor;  /**< Wayland compositor interface */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_compositor; /**< Bound wl_compositor global and registry metadata. */

	struct {
		struct wl_subcompositor *subcompositor; /**< Bound wl_subcompositor protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_subcompositor; /**< Bound wl_subcompositor global and registry metadata. */

	struct {
		struct wl_fixes *fixes; /**< Bound wl_fixes protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_fixes; /**< Bound wl_fixes global and registry metadata. */

	struct {
		struct wl_shm *shm; /**< Bound wl_shm protocol object. */
		struct wlf_render_format_set shm_formats; /**< Render formats advertised by the Wayland shared memory global. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_shm; /**< Bound wl_shm global and registry metadata. */

	struct {
		struct wl_data_device_manager *data_device_manager; /**< Bound wl_data_device_manager protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wl_data_device_manager; /**< Bound wl_data_device_manager global and registry metadata. */

	struct {
		struct xdg_wm_base *wm_base; /**< Bound xdg_wm_base protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} xdg_wm_base; /**< Bound xdg_wm_base global and registry metadata. */

	struct {
		struct wp_viewporter *viewporter; /**< Bound wp_viewporter protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_viewporter; /**< Bound wp_viewporter global and registry metadata. */

	struct {
		struct zwlr_layer_shell_v1 *layer_shell_v1; /**< Bound zwlr_layer_shell_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zwlr_layer_shell_v1; /**< Bound zwlr_layer_shell_v1 global and registry metadata. */

	struct {
		struct xdg_activation_v1 *xdg_activation_v1; /**< Bound xdg_activation_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} xdg_activation_v1; /**< Bound xdg_activation_v1 global and registry metadata. */

	struct {
		struct zxdg_output_manager_v1 *output_manager_v1; /**< Bound zxdg_output_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zxdg_output_manager_v1; /**< Bound zxdg_output_manager_v1 global and registry metadata. */

	struct {
		struct wp_alpha_modifier_v1 *alpha_modifier_v1; /**< Bound wp_alpha_modifier_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_alpha_modifier_v1; /**< Bound wp_alpha_modifier_v1 global and registry metadata. */

	struct {
		struct wp_content_type_manager_v1 *content_type_manager_v1; /**< Bound wp_content_type_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_content_type_manager_v1; /**< Bound wp_content_type_manager_v1 global and registry metadata. */

	struct {
		struct zwp_pointer_gestures_v1 *pointer_gestures_v1; /**< Bound zwp_pointer_gestures_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zwp_pointer_gestures_v1; /**< Bound zwp_pointer_gestures_v1 global and registry metadata. */

	struct {
		struct zwp_text_input_manager_v3 *text_input_manager_v3; /**< Bound zwp_text_input_manager_v3 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zwp_text_input_manager_v3; /**< Bound zwp_text_input_manager_v3 global and registry metadata. */

	struct {
		struct wp_single_pixel_buffer_manager_v1 *single_pixel_buffer_manager_v1; /**< Bound wp_single_pixel_buffer_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_single_pixel_buffer_manager_v1; /**< Bound wp_single_pixel_buffer_manager_v1 global and registry metadata. */

	struct {
		struct zxdg_decoration_manager_v1 *decoration_manager; /**< Bound zxdg_decoration_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zxdg_decoration_manager_v1; /**< Bound zxdg_decoration_manager_v1 global and registry metadata. */

	struct {
		struct wp_cursor_shape_manager_v1 *cursor_shape_manager_v1; /**< Bound wp_cursor_shape_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_cursor_shape_manager_v1; /**< Bound wp_cursor_shape_manager_v1 global and registry metadata. */

	struct {
		struct xdg_toplevel_icon_manager_v1 *toplevel_icon_manager_v1; /**< Bound xdg_toplevel_icon_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} xdg_toplevel_icon_manager_v1; /**< Bound xdg_toplevel_icon_manager_v1 global and registry metadata. */

	struct {
		struct wp_fractional_scale_manager_v1 *fractional_scale_manager_v1; /**< Bound wp_fractional_scale_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} wp_fractional_scale_manager_v1; /**< Bound wp_fractional_scale_manager_v1 global and registry metadata. */

	struct {
		struct zwp_primary_selection_device_manager_v1 *primary_selection_device_manager_v1; /**< Bound zwp_primary_selection_device_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zwp_primary_selection_device_manager_v1; /**< Bound zwp_primary_selection_device_manager_v1 global and registry metadata. */

	struct {
		struct zwp_keyboard_shortcuts_inhibit_manager_v1 *keyboard_shortcuts_inhibit_manager_v1; /**< Bound zwp_keyboard_shortcuts_inhibit_manager_v1 protocol object. */
		uint32_t bind_version; /**< Version used to bind the Wayland global. */
		uint32_t name; /**< Registry name of the Wayland global. */
	} zwp_keyboard_shortcuts_inhibit_manager_v1; /**< Bound zwp_keyboard_shortcuts_inhibit_manager_v1 global and registry metadata. */

	struct {
		/** Emitted when a global is added. Payload: wlf_wl_interface. */
		struct wlf_signal global_add;
		/** Emitted when a global is removed. Payload: wlf_wl_interface. */
		struct wlf_signal global_remove;
	} events;
};

/**
 * @brief Creates a Wayland client backend.
 * @return New Wayland backend, or NULL if connection or initialization fails.
 */
struct wlf_wl_backend *wayland_backend_create(void);

/**
 * @brief Check if a backend is a Wayland backend
 * @param backend Pointer to the backend to check
 * @return true if the backend is a Wayland backend, false otherwise
 */
bool wlf_backend_is_wayland(const struct wlf_backend *backend);

/**
 * @brief Cast a generic backend to a Wayland backend
 * @param backend Pointer to the generic backend
 * @return Pointer to the enclosing implementation object.
 * @note Asserts that the object uses this implementation.
 */
struct wlf_wl_backend *wlf_wl_backend_from_backend(struct wlf_backend *backend);

#endif // WAYLAND_BACKEND_H
