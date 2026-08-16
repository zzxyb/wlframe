/**
 * @file        wlf_window.h
 * @brief       Window abstraction and management for wlframe.
 * @details     This file provides a cross-platform window framework,
 *              offering a unified interface for creating, managing, and rendering
 *              windows. It supports various window types, events, and properties
 *              with platform-specific backends.
 *
 *              Typical usage:
 *                  - Initialize a window with wlf_window_init()
 *                  - Set window properties (size, title, etc.)
 *                  - Show the window and handle events
 *                  - Destroy when done
 *
 * @author      YaoBing Xiao
 * @date        2026-03-10
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-03-10, initial version\n
 */

#ifndef WINDOW_WLF_WINDOW_H
#define WINDOW_WLF_WINDOW_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/math/wlf_size.h"
#include "wlf/types/wlf_color.h"
#include "wlf/allocator/wlf_allocator.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/types/wlf_pointer.h"
#include "wlf/types/wlf_keyboard.h"
#include "wlf/types/wlf_touch.h"

#include <stdint.h>
#include <stdbool.h>
#include <pixman.h>

struct wlf_window;
struct wlf_scene;
struct wlf_scene_tree;
struct wlf_titlebar;
struct wlf_swapchain;
struct wlf_event_node;
struct wlf_window_touch_point;

/**
 * @brief Window states.
 *
 * This enumeration defines the various states a window can be in.
 * Each state represents a different window mode or visibility condition.
 */
enum wlf_window_state_flags {
	WLF_WINDOW_NORMAL = 0,         /**< Normal window state, visible and interactive */
	WLF_WINDOW_ACTIVE = 1 << 0,         /**< Window is active and has focus */
	WLF_WINDOW_SUSPENDED = 1 << 1,      /**< Window is suspended, not actively rendering */
	WLF_WINDOW_MINIMIZED = 1 << 2,      /**< Window is minimized */
	WLF_WINDOW_MAXIMIZED = 1 << 3,      /**< Window is maximized to fill the screen */
	WLF_WINDOW_FULLSCREEN = 1 << 4,     /**< Window is in fullscreen mode */
};

/**
 * @brief Window types.
 */
enum wlf_window_type {
	WLF_WINDOW_TYPE_TOPLEVEL,  /**< Top-level window */
	WLF_WINDOW_TYPE_POPUP,     /**< Popup window */
	WLF_WINDOW_TYPE_DIALOG,    /**< Dialog window */
	WLF_WINDOW_TYPE_TOOLTIP,   /**< Tooltip window */
	WLF_WINDOW_TYPE_LAYER,     /**< wlr-layer-shell window */
};

/**
 * @brief Window flags for behavior control.
 */
enum wlf_window_flags {
	WLF_WINDOW_FLAG_NONE         = 0,      /**< No special flags */
	WLF_WINDOW_FLAG_RESIZABLE    = 1 << 0, /**< Window can be resized */
	WLF_WINDOW_FLAG_DECORATED    = 1 << 1, /**< Window has decorations */
	WLF_WINDOW_FLAG_ALWAYS_ON_TOP = 1 << 2, /**< Window stays on top */
	WLF_WINDOW_FLAG_ALWAYS_ON_BOTTOM = 1 << 3, /**< Window stays on bottom */
	WLF_WINDOW_FLAG_MODAL        = 1 << 4, /**< Window is modal */
};

/**
 * @brief Edge or corner used for native interactive resize requests.
 *
 * Values may be combined to identify a corner, for example top-left.
 */
enum wlf_window_resize_edge {
	WLF_WINDOW_RESIZE_EDGE_NONE = 0,
	WLF_WINDOW_RESIZE_EDGE_TOP = 1,
	WLF_WINDOW_RESIZE_EDGE_BOTTOM = 2,
	WLF_WINDOW_RESIZE_EDGE_LEFT = 4,
	WLF_WINDOW_RESIZE_EDGE_TOP_LEFT = 5,
	WLF_WINDOW_RESIZE_EDGE_BOTTOM_LEFT = 6,
	WLF_WINDOW_RESIZE_EDGE_RIGHT = 8,
	WLF_WINDOW_RESIZE_EDGE_TOP_RIGHT = 9,
	WLF_WINDOW_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};

/**
 * @brief Window implementation interface for platform-specific operations.
 */
struct wlf_window_impl {
	void (*destroy)(struct wlf_window *window);                                              /**< Destroy the window and free resources */
	void (*close)(struct wlf_window *window);                                                /**< Close the window */
	void (*show)(struct wlf_window *window);                                                 /**< Show the window */
	void (*hide)(struct wlf_window *window);                                                 /**< Hide the window */
	void (*set_title)(struct wlf_window *window, const char *title);                         /**< Set the window title */
	void (*set_geometry)(struct wlf_window *window, const struct wlf_rect *geometry);        /**< Set the window geometry */
	void (*set_size)(struct wlf_window *window, int width, int height);                      /**< Set the window size */
	void (*set_min_size)(struct wlf_window *window, int width, int height);                  /**< Set the minimum window size */
	void (*set_max_size)(struct wlf_window *window, int width, int height);                  /**< Set the maximum window size */
	void (*set_position)(struct wlf_window *window, int x, int y);                          /**< Set the window position */
	void (*begin_move)(struct wlf_window *window, struct wlf_pointer *pointer,
		uint32_t serial); /**< Begin compositor/native interactive move. */
	void (*begin_resize)(struct wlf_window *window, struct wlf_pointer *pointer,
		uint32_t serial, enum wlf_window_resize_edge edge);
	void (*set_state)(struct wlf_window *window, enum wlf_window_state_flags state);        /**< Set the window state */
	void (*set_flags)(struct wlf_window *window, uint32_t flags);                            /**< Set window behavior flags */
	void (*set_input_region)(struct wlf_window *window, const pixman_region32_t *region);    /**< Set the input region */
	void (*set_opaque_region)(struct wlf_window *window, const pixman_region32_t *region);   /**< Set the opaque region */
	void (*set_opacity)(struct wlf_window *window, float opacity);                           /**< Set the window opacity */
	void (*set_mask)(struct wlf_window *window, const pixman_region32_t *mask);              /**< Set the shape mask */
	void (*set_background_color)(struct wlf_window *window, const struct wlf_color *color);  /**< Set the background color */
	void *(*native_handle)(struct wlf_window *window);
	void (*arm_frame)(struct wlf_window *window);                                             /**< Request a frame callback without committing */
	void (*schedule_frame)(struct wlf_window *window);                                        /**< Schedule a window frame */
};

struct wlf_window_state {
	struct wlf_renderer *renderer;
	struct wlf_backend *backend;
	struct wlf_swapchain *swapchain;

	char *title;                        /**< Window title */

	struct wlf_render_format format;
	struct wlf_color background_color;  /**< Background color (RGBA doubles) */
	struct wlf_rect geometry;           /**< Window geometry (position and size) */
	struct wlf_size min_size;           /**< Minimum window size */
	struct wlf_size max_size;           /**< Maximum window size */

	uint32_t flags;                     /**< Window flags */
	enum wlf_window_type type;          /**< Window type */
	enum wlf_window_state_flags state; /**< Window state flags */
	float opacity;                      /**< Window opacity (0.0-1.0) */
	double scale;                       /**< Logical-to-buffer pixel scale. */

	bool visible;                       /**< Whether window is currently visible */
	bool focused;                       /**< Whether window has focus */
	bool server_side_decorated; /**< Whether this window is currently using server-side decorations. */
};

/**
 * @brief Main window structure.
 */
struct wlf_window {
	const struct wlf_window_impl *impl; /**< Platform-specific implementation */
	struct wlf_scene *scene;            /**< Scene attached to this window */
	struct wlf_scene_tree *tree;        /**< root node */
	void *data;                    /**< User data pointer */

	struct wlf_window_state state;
	struct wlf_listener theme_changed; /**< Internal system-theme subscription. */
	bool theme_listener_attached; /**< Whether the theme listener is registered. */
	bool uses_theme_background; /**< Whether the background follows the theme. */
	struct wlf_event_node *pointer_event_node; /**< Event node under the pointer. */
	struct wlf_event_node *pointer_grab_event_node; /**< Node captured by the current button press. */
	struct wlf_event_node *keyboard_event_node; /**< Event node receiving keyboard focus. */
	struct wlf_window_touch_point *touch_points; /**< Active touch-point state. */
	double pointer_x, pointer_y; /**< Latest pointer coordinates in window space. */
	struct {
		bool enable_set_position;          /**< Whether set_position is supported */
		bool enable_set_min_size;          /**< Whether set_min_size is supported */
		bool enable_set_max_size;          /**< Whether set_max_size is supported */
	} features;                   /**< Feature flags for window capabilities */

	struct {
		struct wlf_signal destroy;      /**< Emitted when window is destroyed */
		struct wlf_signal expose;       /**< Emitted when window needs redraw */
		struct wlf_signal resize;       /**< Emitted when window is resized */
		struct wlf_signal move;         /**< Emitted when window is moved */
		struct wlf_signal close;        /**< Emitted when close is requested */
		struct wlf_signal focus_in;     /**< Emitted when window gains focus */
		struct wlf_signal focus_out;    /**< Emitted when window loses focus */
		struct wlf_signal scale;        /**< Emitted after the buffer scale changes */
		struct wlf_signal show;         /**< Emitted when window is shown */
		struct wlf_signal hide;         /**< Emitted when window is hidden */
		struct wlf_signal pointer_enter;
		struct wlf_signal pointer_leave;
		struct wlf_signal pointer_motion;
		struct wlf_signal pointer_button;
		struct wlf_signal pointer_axis;
		struct wlf_signal pointer_frame;
		struct wlf_signal keyboard_enter;
		struct wlf_signal keyboard_leave;
		struct wlf_signal keyboard_keymap;
		struct wlf_signal keyboard_key;
		struct wlf_signal keyboard_modifiers;
		struct wlf_signal keyboard_repeat_info;
		struct wlf_signal tablet;
		struct wlf_signal touch_down;
		struct wlf_signal touch_up;
		struct wlf_signal touch_motion;
		struct wlf_signal touch_cancel;
		struct wlf_signal touch_frame;
		struct wlf_signal touch_shape;
		struct wlf_signal touch_orientation;
	} events;
};

/**
 * @brief Initialize a window object with backend implementation and defaults.
 * @param window Pointer to the window object to initialize.
 * @param type Window type.
 * @param impl Platform-specific window implementation callbacks.
 * @param backend Backend that owns the window.
 * @param width Initial window width.
 * @param height Initial window height.
 */
void wlf_window_init(struct wlf_window *window, enum wlf_window_type type,
	const struct wlf_window_impl *impl, struct wlf_backend *backend,
	uint32_t width, uint32_t height);

/**
 * Creates a native toplevel using the backend's host window system.
 * The requested width and height are logical (DPI-independent) units.
 */
struct wlf_window *wlf_window_create_toplevel(struct wlf_backend *backend,
	uint32_t width, uint32_t height);

/**
 * @brief Destroy a window and free all associated resources.
 * @param window Pointer to the window to destroy.
 */
void wlf_window_destroy(struct wlf_window *window);

/**
 * @brief Close the window.
 * @param window Pointer to the window.
 */
void wlf_window_close(struct wlf_window *window);

/**
 * @brief Show the window.
 * @param window Pointer to the window.
 */
void wlf_window_show(struct wlf_window *window);

/**
 * @brief Hide the window.
 * @param window Pointer to the window.
 */
void wlf_window_hide(struct wlf_window *window);

/**
 * @brief Set the window title.
 * @param window Pointer to the window.
 * @param title New window title.
 */
void wlf_window_set_title(struct wlf_window *window, const char *title);

/**
 * @brief Returns this window's active client-side titlebar, if any.
 * @param window Window to inspect.
 * @return Active titlebar, or NULL when client-side decoration is disabled.
 */
struct wlf_titlebar *wlf_window_get_titlebar(struct wlf_window *window);

/**
 * @brief Set the window geometry (position and size).
 * @param window Pointer to the window.
 * @param geometry New window geometry.
 */
void wlf_window_set_geometry(struct wlf_window *window, const struct wlf_rect *geometry);

/**
 * @brief Set the window size.
 * @param window Pointer to the window.
 * @param width New window width.
 * @param height New window height.
 */
void wlf_window_set_size(struct wlf_window *window, int width, int height);

/**
 * @brief Set the minimum window size.
 * @param window Pointer to the window.
 * @param width Minimum window width.
 * @param height Minimum window height.
 */
void wlf_window_set_min_size(struct wlf_window *window, int width, int height);

/**
 * @brief Set the maximum window size.
 * @param window Pointer to the window.
 * @param width Maximum window width.
 * @param height Maximum window height.
 */
void wlf_window_set_max_size(struct wlf_window *window, int width, int height);

/**
 * @brief Set the window position.
 * @param window Pointer to the window.
 * @param x New window x position.
 * @param y New window y position.
 */
void wlf_window_set_position(struct wlf_window *window, int x, int y);

/**
 * @brief Starts a native interactive move from a pointer press serial.
 * @param window Window to move.
 * @param pointer Pointer that generated the press.
 * @param serial Serial associated with the press.
 */
void wlf_window_begin_move(struct wlf_window *window,
	struct wlf_pointer *pointer, uint32_t serial);

/**
 * @brief Starts a native interactive resize from a pointer press serial.
 * @param window Window to resize.
 * @param pointer Pointer that generated the press.
 * @param serial Serial associated with the press.
 * @param edge Edge or corner being dragged.
 */
void wlf_window_begin_resize(struct wlf_window *window,
	struct wlf_pointer *pointer, uint32_t serial,
	enum wlf_window_resize_edge edge);

/**
 * @brief Changes the logical-to-buffer pixel scale and schedules a full repaint.
 * @param window Window whose scale is changed.
 * @param scale New positive scale factor.
 */
void wlf_window_set_scale(struct wlf_window *window, double scale);

/**
 * @brief Converts a logical size to a covering buffer-pixel size.
 * @param window Window supplying the scale factor.
 * @param logical_length Logical length to convert.
 * @return Corresponding buffer-pixel length rounded up.
 */
uint32_t wlf_window_scale_length(const struct wlf_window *window,
	uint32_t logical_length);

/**
 * @brief Set the window state (e.g. minimized, maximized).
 * @param window Pointer to the window.
 * @param state New window state.
 */
void wlf_window_set_state(struct wlf_window *window, enum wlf_window_state_flags state);

/**
 * @brief Set window flags.
 * @param window Pointer to the window.
 * @param flags Window flags to set.
 */
void wlf_window_set_flags(struct wlf_window *window, uint32_t flags);

/**
 * @brief Set the window input region.
 * @param window Pointer to the window.
 * @param region New input region.
 */
void wlf_window_set_input_region(struct wlf_window *window, const pixman_region32_t *region);

/**
 * @brief Set the window opaque region for compositor optimization.
 * @param window Pointer to the window.
 * @param region New opaque region.
 */
void wlf_window_set_opaque_region(struct wlf_window *window, const pixman_region32_t *region);

/**
 * @brief Set the window opacity.
 * @param window Pointer to the window.
 * @param opacity Opacity value in the range [0.0, 1.0].
 */
void wlf_window_set_opacity(struct wlf_window *window, float opacity);

/**
 * @brief Set the window shape mask for non-rectangular windows.
 * @param window Pointer to the window.
 * @param mask New shape mask region.
 */
void wlf_window_set_mask(struct wlf_window *window, const pixman_region32_t *mask);

/**
 * @brief Set the window background color.
 * @param window Pointer to the window.
 * @param color New background color.
 */
void wlf_window_set_background_color(struct wlf_window *window, const struct wlf_color *color);

/**
 * @brief Returns the platform-native handle for a window.
 * @param window Window whose handle is requested.
 * @return Native handle, or NULL when the backend does not expose one.
 */
void *wlf_window_native_handle(struct wlf_window *window);

/**
 * @brief Associates a renderer with a window.
 * @param window Window receiving the renderer.
 * @param renderer Renderer used for subsequent scene commits.
 */
void wlf_window_init_renderer(struct wlf_window *window, struct wlf_renderer *renderer);

/**
 * @brief Requests the backend to deliver a frame/expose event.
 * @param window Window for which a frame is requested.
 */
void wlf_window_schedule_frame(struct wlf_window *window);

/**
 * @brief Arms a frame/expose event for the next surface commit.
 * @param window Window for which a frame is armed.
 */
void wlf_window_arm_frame(struct wlf_window *window);

/**
 * @brief Forwards a pointer-enter event to the window event tree.
 * @param window Window receiving the event.
 * @param event Pointer-enter event payload.
 */
void wlf_window_pointer_enter(struct wlf_window *window,
	const struct wlf_pointer_enter_event *event);

/**
 * @brief Forwards a pointer-leave event to the window event tree.
 * @param window Window receiving the event.
 * @param event Pointer-leave event payload.
 */
void wlf_window_pointer_leave(struct wlf_window *window,
	const struct wlf_pointer_leave_event *event);

/**
 * @brief Forwards pointer motion to the window event tree.
 * @param window Window receiving the event.
 * @param event Pointer-motion event payload.
 */
void wlf_window_pointer_motion(struct wlf_window *window,
	const struct wlf_pointer_motion_absolute_event *event);

/**
 * @brief Forwards a pointer-button event to the window event tree.
 * @param window Window receiving the event.
 * @param event Pointer-button event payload.
 */
void wlf_window_pointer_button(struct wlf_window *window,
	const struct wlf_pointer_button_event *event);

/**
 * @brief Forwards a pointer-axis event to the window event tree.
 * @param window Window receiving the event.
 * @param event Pointer-axis event payload.
 */
void wlf_window_pointer_axis(struct wlf_window *window,
	const struct wlf_pointer_axis_event *event);

/**
 * @brief Forwards the end of a pointer event frame.
 * @param window Window receiving the event.
 * @param event Backend-specific frame payload, when available.
 */
void wlf_window_pointer_frame(struct wlf_window *window, void *event);

/**
 * @brief Forwards a keyboard-enter event to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-enter event payload.
 */
void wlf_window_keyboard_enter(struct wlf_window *window,
	const struct wlf_keyboard_enter_event *event);

/**
 * @brief Forwards a keyboard-leave event to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-leave event payload.
 */
void wlf_window_keyboard_leave(struct wlf_window *window,
	const struct wlf_keyboard_leave_event *event);

/**
 * @brief Forwards a keyboard keymap event to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-keymap event payload.
 */
void wlf_window_keyboard_keymap(struct wlf_window *window,
	const struct wlf_keyboard_keymap_event *event);

/**
 * @brief Forwards a keyboard-key event to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-key event payload.
 */
void wlf_window_keyboard_key(struct wlf_window *window,
	const struct wlf_keyboard_key_event *event);

/**
 * @brief Forwards keyboard modifier state to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-modifier event payload.
 */
void wlf_window_keyboard_modifiers(struct wlf_window *window,
	const struct wlf_keyboard_modifiers_event *event);

/**
 * @brief Forwards keyboard repeat information to the focused event node.
 * @param window Window receiving the event.
 * @param event Keyboard-repeat event payload.
 */
void wlf_window_keyboard_repeat_info(struct wlf_window *window,
	const struct wlf_keyboard_repeat_info_event *event);

/**
 * @brief Forwards a touch-down event to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-down event payload.
 */
void wlf_window_touch_down(struct wlf_window *window,
	const struct wlf_touch_down_event *event);

/**
 * @brief Forwards a touch-up event to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-up event payload.
 */
void wlf_window_touch_up(struct wlf_window *window,
	const struct wlf_touch_up_event *event);

/**
 * @brief Forwards touch motion to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-motion event payload.
 */
void wlf_window_touch_motion(struct wlf_window *window,
	const struct wlf_touch_motion_event *event);

/**
 * @brief Forwards a touch-cancel event to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-cancel event payload.
 */
void wlf_window_touch_cancel(struct wlf_window *window,
	const struct wlf_touch_cancel_event *event);

/**
 * @brief Forwards the end of a touch event frame.
 * @param window Window receiving the event.
 * @param event Backend-specific frame payload, when available.
 */
void wlf_window_touch_frame(struct wlf_window *window, void *event);

/**
 * @brief Forwards a touch-shape event to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-shape event payload.
 */
void wlf_window_touch_shape(struct wlf_window *window,
	const struct wlf_touch_shape_event *event);

/**
 * @brief Forwards a touch-orientation event to the window event tree.
 * @param window Window receiving the event.
 * @param event Touch-orientation event payload.
 */
void wlf_window_touch_orientation(struct wlf_window *window,
	const struct wlf_touch_orientation_event *event);

/**
 * @brief Drops active touch-point references to an event node being disabled or destroyed.
 * @param window Window whose touch-point state is updated.
 * @param node Event node no longer available for touch delivery.
 */
void wlf_window_forget_touch_event_node(struct wlf_window *window,
	struct wlf_event_node *node);

/**
 * @brief Forwards a protocol-specific tablet event to the current event node.
 * @param window Window receiving the event.
 * @param event Backend-specific tablet event payload.
 * @param x Tablet x coordinate in window space.
 * @param y Tablet y coordinate in window space.
 */
void wlf_window_tablet_event(struct wlf_window *window, void *event,
	double x, double y);

#endif // WINDOW_WLF_WINDOW_H
