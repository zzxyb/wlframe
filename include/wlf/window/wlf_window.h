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
#include "wlf/math/wlf_region.h"

#include <stdint.h>
#include <stdbool.h>

struct wlf_window;

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
	void (*set_state)(struct wlf_window *window, enum wlf_window_state_flags state);        /**< Set the window state */
	void (*set_flags)(struct wlf_window *window, uint32_t flags);                            /**< Set window behavior flags */
	void (*set_input_region)(struct wlf_window *window, const struct wlf_region *region);    /**< Set the input region */
	void (*set_opaque_region)(struct wlf_window *window, const struct wlf_region *region);   /**< Set the opaque region */
	void (*set_opacity)(struct wlf_window *window, float opacity);                           /**< Set the window opacity */
	void (*set_mask)(struct wlf_window *window, const struct wlf_region *mask);              /**< Set the shape mask */
	void (*set_background_color)(struct wlf_window *window, const struct wlf_color *color);  /**< Set the background color */
};

struct wlf_window_state {
	char *title;                        /**< Window title */

	struct wlf_color background_color;  /**< Background color (RGBA doubles) */
	struct wlf_rect geometry;           /**< Window geometry (position and size) */
	struct wlf_size min_size;           /**< Minimum window size */
	struct wlf_size max_size;           /**< Maximum window size */

	uint32_t flags;                     /**< Window flags */
	enum wlf_window_type type;          /**< Window type */
	enum wlf_window_state_flags state; /**< Window state flags */
	float opacity;                      /**< Window opacity (0.0-1.0) */

	bool visible;                       /**< Whether window is currently visible */
	bool focused;                       /**< Whether window has focus */
};

/**
 * @brief Main window structure.
 */
struct wlf_window {
	const struct wlf_window_impl *impl; /**< Platform-specific implementation */
	void *data;                    /**< User data pointer */

	struct wlf_window_state state;
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
		struct wlf_signal show;         /**< Emitted when window is shown */
		struct wlf_signal hide;         /**< Emitted when window is hidden */
	} events;
};

/**
 * @brief Initialize a window object with backend implementation and defaults.
 * @param window Pointer to the window object to initialize.
 * @param type Window type.
 * @param impl Platform-specific window implementation callbacks.
 * @param width Initial window width.
 * @param height Initial window height.
 */
void wlf_window_init(struct wlf_window *window, enum wlf_window_type type,
	const struct wlf_window_impl *impl, uint32_t width, uint32_t height);

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
void wlf_window_set_input_region(struct wlf_window *window, const struct wlf_region *region);

/**
 * @brief Set the window opaque region for compositor optimization.
 * @param window Pointer to the window.
 * @param region New opaque region.
 */
void wlf_window_set_opaque_region(struct wlf_window *window, const struct wlf_region *region);

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
void wlf_window_set_mask(struct wlf_window *window, const struct wlf_region *mask);

/**
 * @brief Set the window background color.
 * @param window Pointer to the window.
 * @param color New background color.
 */
void wlf_window_set_background_color(struct wlf_window *window, const struct wlf_color *color);

#endif // WINDOW_WLF_WINDOW_H
