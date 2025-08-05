/**
 * @file        wlf_window.h
 * @brief       Window abstraction and management for wlframe.
 * @details     This file provides a window framework inspired by Qt's QWindow,
 *              offering a unified interface for creating, managing, and rendering
 *              windows. It supports various window types, events, and properties
 *              with platform-specific backends.
 *
 *              Typical usage:
 *                  - Create a window with wlf_window_create()
 *                  - Set window properties (size, title, etc.)
 *                  - Show the window and handle events
 *                  - Destroy when done
 *
 * @author      YaoBing Xiao
 * @date        2025-06-15
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-15, initial version\n
 */

#ifndef WINDOW_WLF_WINDOW_H
#define WINDOW_WLF_WINDOW_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_rect.h"

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
	WLF_WINDOW_MINIMIZED = 1 << 2,      /**< Window is minimized to taskbar/dock */
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
	WLF_WINDOW_FLAG_MODAL        = 1 << 3, /**< Window is modal */
};

/**
 * @brief Window event types.
 */
enum wlf_window_event_type {
	WLF_WINDOW_EVENT_EXPOSE,      /**< Window needs to be redrawn */
	WLF_WINDOW_EVENT_RESIZE,      /**< Window was resized */
	WLF_WINDOW_EVENT_MOVE,        /**< Window was moved */
	WLF_WINDOW_EVENT_CLOSE,       /**< Window close requested */
	WLF_WINDOW_EVENT_FOCUS_IN,    /**< Window gained focus */
	WLF_WINDOW_EVENT_FOCUS_OUT,   /**< Window lost focus */
	WLF_WINDOW_EVENT_SHOW,        /**< Window was shown */
	WLF_WINDOW_EVENT_HIDE,        /**< Window was hidden */
};

/**
 * @brief Window event data structure.
 */
struct wlf_window_event {
	enum wlf_window_event_type type; /**< Event type */
	struct wlf_window *window;       /**< Window that triggered the event */
	union {
		struct {
			struct wlf_rect area;    /**< Exposed area */
		} expose;
		struct {
			int width, height;       /**< New dimensions */
		} resize;
		struct {
			int x, y;                /**< New position */
		} move;
	} data;                          /**< Event-specific data */
};

/**
 * @brief Window implementation interface for platform-specific operations.
 */
struct wlf_window_impl {
	bool (*create)(struct wlf_window *window);
	void (*destroy)(struct wlf_window *window);
	void (*show)(struct wlf_window *window);
	void (*hide)(struct wlf_window *window);
	void (*set_title)(struct wlf_window *window, const char *title);
	void (*set_geometry)(struct wlf_window *window, const struct wlf_rect *geometry);
	void (*set_window_state)(struct wlf_window *window, enum wlf_window_state_flags visibility);
};

/**
 * @brief Main window structure.
 */
struct wlf_window {
	const struct wlf_window_impl *impl; /**< Platform-specific implementation */

	char *title;                        /**< Window title */
	struct wlf_rect geometry;           /**< Window geometry (position and size) */
	enum wlf_window_type type;          /**< Window type */
	enum wlf_window_state_flags visibility; /**< Window visibility state */
	uint32_t flags;                     /**< Window flags */

	bool visible;                       /**< Whether window is currently visible */
	bool focused;                       /**< Whether window has focus */

	struct {
		struct wlf_signal expose;       /**< Emitted when window needs redraw */
		struct wlf_signal resize;       /**< Emitted when window is resized */
		struct wlf_signal move;         /**< Emitted when window is moved */
		struct wlf_signal close;        /**< Emitted when close is requested */
		struct wlf_signal focus_in;     /**< Emitted when window gains focus */
		struct wlf_signal focus_out;    /**< Emitted when window loses focus */
		struct wlf_signal show;         /**< Emitted when window is shown */
		struct wlf_signal hide;         /**< Emitted when window is hidden */
	} events;

	void *user_data;
};

/**
 * @brief Create a new window.
 * @param type Window type.
 * @return Pointer to the newly created window, or NULL on failure.
 */
struct wlf_window *wlf_window_create(enum wlf_window_type type);

/**
 * @brief Destroy a window and free all associated resources.
 * @param window Pointer to the window to destroy.
 */
void wlf_window_destroy(struct wlf_window *window);

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
 * @brief Set the window position.
 * @param window Pointer to the window.
 * @param x New window x position.
 * @param y New window y position.
 */
void wlf_window_set_position(struct wlf_window *window, int x, int y);

/**
 * @brief Set the window visibility state.
 * @param window Pointer to the window.
 * @param visibility New visibility state.
 */
void wlf_window_set_visibility(struct wlf_window *window, enum wlf_window_state_flags visibility);

/**
 * @brief Set window flags.
 * @param window Pointer to the window.
 * @param flags Window flags to set.
 */
void wlf_window_set_flags(struct wlf_window *window, uint32_t flags);

/**
 * @brief Raise the window to the top.
 * @param window Pointer to the window.
 */
void wlf_window_raise(struct wlf_window *window);

/**
 * @brief Lower the window to the bottom.
 * @param window Pointer to the window.
 */
void wlf_window_lower(struct wlf_window *window);

/**
 * @brief Close the window.
 * @param window Pointer to the window.
 */
void wlf_window_close(struct wlf_window *window);

#endif // WINDOW_WLF_WINDOW_H
