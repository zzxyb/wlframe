/**
 * @file        wlf_xdg_wm_base.h
 * @brief       Wayland xdg-shell protocol wrapper for wlframe.
 * @details     Implements the stable xdg-shell protocol (xdg_wm_base v7),
 *              covering the five tightly coupled interfaces:
 *
 *              - wlf_xdg_wm_base     – global manager; auto-replies to ping.
 *              - wlf_xdg_positioner  – positioning geometry builder.
 *              - wlf_xdg_surface     – role-agnostic shell surface wrapper.
 *              - wlf_xdg_toplevel    – application window (configure, close…).
 *              - wlf_xdg_popup       – transient popup window.
 *
 *              Typical window creation sequence:
 *                1. wlf_xdg_wm_base_create() on registry global.
 *                2. wlf_xdg_wm_base_get_xdg_surface(wm_base, wl_surface).
 *                3. wlf_xdg_surface_get_toplevel(surface).
 *                4. Configure the toplevel (set_title, set_app_id, etc.).
 *                5. Call wl_surface.commit() to trigger the initial configure.
 *                6. In the configure signal handler:
 *                     a. Resize/draw your buffer.
 *                     b. wlf_xdg_surface_ack_configure(serial).
 *                     c. wl_surface.commit() with the new buffer.
 *                7. Destroy in reverse creation order.
 *
 * @author      YaoBing Xiao
 * @date        2026-05-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-22, initial version\n
 */

#ifndef WAYLAND_WLF_XDG_WM_BASE_H
#define WAYLAND_WLF_XDG_WM_BASE_H

#include "wlf/utils/wlf_signal.h"

#include <stdint.h>
#include <stdbool.h>

struct wl_registry;
struct wl_surface;
struct wl_seat;
struct wl_output;
struct xdg_wm_base;
struct xdg_positioner;
struct xdg_surface;
struct xdg_toplevel;
struct xdg_popup;
struct wlf_xdg_positioner;
struct wlf_xdg_surface;

/** Anchor edge/corner for xdg_positioner (mirrors xdg_positioner.anchor). */
enum wlf_xdg_positioner_anchor {
	WLF_XDG_POSITIONER_ANCHOR_NONE         = 0,
	WLF_XDG_POSITIONER_ANCHOR_TOP          = 1,
	WLF_XDG_POSITIONER_ANCHOR_BOTTOM       = 2,
	WLF_XDG_POSITIONER_ANCHOR_LEFT         = 3,
	WLF_XDG_POSITIONER_ANCHOR_RIGHT        = 4,
	WLF_XDG_POSITIONER_ANCHOR_TOP_LEFT     = 5,
	WLF_XDG_POSITIONER_ANCHOR_BOTTOM_LEFT  = 6,
	WLF_XDG_POSITIONER_ANCHOR_TOP_RIGHT    = 7,
	WLF_XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT = 8,
};

/** Gravity direction for xdg_positioner (mirrors xdg_positioner.gravity). */
enum wlf_xdg_positioner_gravity {
	WLF_XDG_POSITIONER_GRAVITY_NONE         = 0,
	WLF_XDG_POSITIONER_GRAVITY_TOP          = 1,
	WLF_XDG_POSITIONER_GRAVITY_BOTTOM       = 2,
	WLF_XDG_POSITIONER_GRAVITY_LEFT         = 3,
	WLF_XDG_POSITIONER_GRAVITY_RIGHT        = 4,
	WLF_XDG_POSITIONER_GRAVITY_TOP_LEFT     = 5,
	WLF_XDG_POSITIONER_GRAVITY_BOTTOM_LEFT  = 6,
	WLF_XDG_POSITIONER_GRAVITY_TOP_RIGHT    = 7,
	WLF_XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT = 8,
};

/**
 * Constraint adjustment bits for xdg_positioner
 * (mirrors xdg_positioner.constraint_adjustment).
 */
enum wlf_xdg_positioner_constraint_adjustment {
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE     = 0,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X  = 1,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y  = 2,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X   = 4,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y   = 8,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X = 16,
	WLF_XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y = 32,
};

/**
 * xdg_toplevel window state bits.
 *
 * The states field in wlf_xdg_toplevel is a bitmask; use
 * (1u << WLF_XDG_TOPLEVEL_STATE_*) to test individual bits.
 */
enum wlf_xdg_toplevel_state {
	WLF_XDG_TOPLEVEL_STATE_MAXIMIZED      = 1,
	WLF_XDG_TOPLEVEL_STATE_FULLSCREEN     = 2,
	WLF_XDG_TOPLEVEL_STATE_RESIZING       = 3,
	WLF_XDG_TOPLEVEL_STATE_ACTIVATED      = 4,
	WLF_XDG_TOPLEVEL_STATE_TILED_LEFT     = 5, /**< since v2 */
	WLF_XDG_TOPLEVEL_STATE_TILED_RIGHT    = 6, /**< since v2 */
	WLF_XDG_TOPLEVEL_STATE_TILED_TOP      = 7, /**< since v2 */
	WLF_XDG_TOPLEVEL_STATE_TILED_BOTTOM   = 8, /**< since v2 */
	WLF_XDG_TOPLEVEL_STATE_SUSPENDED      = 9, /**< since v6 */
};

/**
 * Compositor capability bits reported via wm_capabilities (since v5).
 * Use (1u << WLF_XDG_TOPLEVEL_WM_CAPABILITIES_*) to test.
 */
enum wlf_xdg_toplevel_wm_capabilities {
	WLF_XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU = 1,
	WLF_XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE    = 2,
	WLF_XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN  = 3,
	WLF_XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE    = 4,
};

/** Resize edge hint for wlf_xdg_toplevel_resize(). */
enum wlf_xdg_toplevel_resize_edge {
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_NONE         = 0,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_TOP          = 1,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM       = 2,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_LEFT         = 4,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT     = 5,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT  = 6,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_RIGHT        = 8,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT    = 9,
	WLF_XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT = 10,
};

/**
 * @brief Wrapper around the xdg_wm_base global.
 *
 * Automatically replies to ping events.
 */
struct wlf_xdg_wm_base {
	struct xdg_wm_base *base; /**< Underlying protocol object. */
	uint32_t version; /**< Negotiated bind version */

	struct {
		/**
		 * Emitted when a ping event arrives (after the automatic pong
		 * reply has already been sent).  Signal data is the serial.
		 */
		struct wlf_signal ping;
		/** Emitted before the manager is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around an xdg_positioner object.
 *
 * Created by wlf_xdg_wm_base_create_positioner().  The caller owns and must
 * destroy this before the associated surface is destroyed.
 */
struct wlf_xdg_positioner {
	struct xdg_positioner *base; /**< Underlying protocol object. */
	uint32_t version;            /**< Protocol object version. */
};

/**
 * @brief Wrapper around an xdg_surface object.
 *
 * Created by wlf_xdg_wm_base_get_xdg_surface().
 */
struct wlf_xdg_surface {
	struct xdg_surface *base; /**< Underlying protocol object. */
	uint32_t version;         /**< Protocol object version. */

	/**
	 * Latest configure serial received from the compositor.  Set to 0 before
	 * the first configure event.
	 */
	uint32_t configure_serial;
	/** Latest serial acknowledged by wlf_xdg_surface_ack_configure(). */
	uint32_t acked_configure_serial;
	/** True after at least one configure has been received and not yet acked. */
	bool has_pending_configure;

	struct {
		/**
		 * Emitted when the compositor sends a configure event.
		 * Signal data is the uint32_t serial.  The caller must call
		 * wlf_xdg_surface_ack_configure() and then commit.
		 */
		struct wlf_signal configure;
		/** Emitted before the xdg_surface wrapper is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around an xdg_toplevel.
 *
 * Created by wlf_xdg_surface_get_toplevel().
 *
 * The configure_width / configure_height / configure_states fields are updated
 * before the configure signal fires.  0×0 means the compositor does not
 * constrain the size.
 */
struct wlf_xdg_toplevel {
	struct xdg_toplevel *base; /**< Underlying protocol object. */
	uint32_t version;          /**< Protocol object version. */

	/* Latest configure values (valid inside configure listener) */
	int32_t  configure_width;
	int32_t  configure_height;
	uint32_t configure_states;   /**< Bitmask: (1u << wlf_xdg_toplevel_state) */

	/* configure_bounds (since v4): 0 = not set */
	int32_t bounds_width;
	int32_t bounds_height;

	/** Compositor capabilities bitmask (since v5): (1u << wlf_xdg_toplevel_wm_capabilities) */
	uint32_t wm_capabilities;

	struct {
		/** Fired after configure_width/height/states are updated. */
		struct wlf_signal configure;
		/** Compositor requests the window be closed. */
		struct wlf_signal close;
		/** Compositor reports suggested maximum size (since v4). */
		struct wlf_signal configure_bounds;
		/** Compositor reports available capabilities (since v5). */
		struct wlf_signal wm_capabilities;
		/** Emitted before the xdg_toplevel wrapper is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around an xdg_popup.
 *
 * Created by wlf_xdg_surface_get_popup().
 */
struct wlf_xdg_popup {
	struct xdg_popup *base; /**< Underlying protocol object. */
	uint32_t version;       /**< Protocol object version. */

	/* Latest configure values */
	int32_t configure_x;
	int32_t configure_y;
	int32_t configure_width;
	int32_t configure_height;
	/** Latest token received from a repositioned event (since v3). */
	uint32_t repositioned_token;

	struct {
		/** Fired after configure_x/y/width/height are updated. */
		struct wlf_signal configure;
		/** Popup has been dismissed by the compositor. */
		struct wlf_signal popup_done;
		/** Reposition reply received (since v3). Signal data is the token. */
		struct wlf_signal repositioned;
		/** Emitted before the xdg_popup wrapper is destroyed and freed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind to the xdg_wm_base global from the registry.
 *
 * The wrapper automatically replies to compositor ping events with pong.
 *
 * @param wl_registry Wayland registry that announced the global.
 * @param name Global name from the registry event.
 * @param version Advertised global version.
 * @return A new manager wrapper, or NULL on allocation/bind failure.
 */
struct wlf_xdg_wm_base *wlf_xdg_wm_base_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy the wm_base wrapper.
 *
 * @param wm_base Manager to destroy; ignored when NULL.
 */
void wlf_xdg_wm_base_destroy(struct wlf_xdg_wm_base *wm_base);

/**
 * @brief Create a positioner for popup placement.
 *
 * @param wm_base Bound xdg_wm_base wrapper.
 * @return A new positioner wrapper, or NULL on failure.
 */
struct wlf_xdg_positioner *wlf_xdg_wm_base_create_positioner(
	struct wlf_xdg_wm_base *wm_base);

/**
 * @brief Create an xdg_surface for the given wl_surface.
 *
 * The wl_surface must not already have an assigned role.
 *
 * @param wm_base Bound xdg_wm_base wrapper.
 * @param surface Wayland surface to assign the xdg_surface role wrapper to.
 * @return A new xdg_surface wrapper, or NULL on failure.
 */
struct wlf_xdg_surface *wlf_xdg_wm_base_get_xdg_surface(
	struct wlf_xdg_wm_base *wm_base, struct wl_surface *surface);

/**
 * @brief Set the popup size in surface-local coordinates.
 *
 * @param positioner Positioner to update.
 * @param width Desired popup width.
 * @param height Desired popup height.
 */
void wlf_xdg_positioner_set_size(struct wlf_xdg_positioner *positioner,
	int32_t width, int32_t height);

/**
 * @brief Set the anchor rectangle in parent surface coordinates.
 *
 * @param positioner Positioner to update.
 * @param x Anchor rectangle x coordinate.
 * @param y Anchor rectangle y coordinate.
 * @param width Anchor rectangle width.
 * @param height Anchor rectangle height.
 */
void wlf_xdg_positioner_set_anchor_rect(struct wlf_xdg_positioner *positioner,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Set which edge or corner of the anchor rectangle is used.
 *
 * @param positioner Positioner to update.
 * @param anchor Anchor value mirroring xdg_positioner.anchor.
 */
void wlf_xdg_positioner_set_anchor(struct wlf_xdg_positioner *positioner,
	enum wlf_xdg_positioner_anchor anchor);

/**
 * @brief Set popup gravity relative to the anchor point.
 *
 * @param positioner Positioner to update.
 * @param gravity Gravity value mirroring xdg_positioner.gravity.
 */
void wlf_xdg_positioner_set_gravity(struct wlf_xdg_positioner *positioner,
	enum wlf_xdg_positioner_gravity gravity);

/**
 * @brief Set compositor constraint adjustment behavior.
 *
 * @param positioner Positioner to update.
 * @param constraint_adjustment Bitmask of wlf_xdg_positioner_constraint_adjustment.
 */
void wlf_xdg_positioner_set_constraint_adjustment(
	struct wlf_xdg_positioner *positioner, uint32_t constraint_adjustment);

/**
 * @brief Set popup offset from the computed anchor position.
 *
 * @param positioner Positioner to update.
 * @param x Horizontal offset.
 * @param y Vertical offset.
 */
void wlf_xdg_positioner_set_offset(struct wlf_xdg_positioner *positioner,
	int32_t x, int32_t y);

/**
 * @brief Mark the positioner as reactive.
 *
 * @param positioner Positioner to update.
 * @since xdg-shell v3
 */
void wlf_xdg_positioner_set_reactive(struct wlf_xdg_positioner *positioner);

/**
 * @brief Set the parent surface size used for constraint adjustment.
 *
 * @param positioner Positioner to update.
 * @param parent_width Parent surface width.
 * @param parent_height Parent surface height.
 * @since xdg-shell v3
 */
void wlf_xdg_positioner_set_parent_size(struct wlf_xdg_positioner *positioner,
	int32_t parent_width, int32_t parent_height);

/**
 * @brief Set the parent configure serial this positioner responds to.
 *
 * @param positioner Positioner to update.
 * @param serial Parent configure serial.
 * @since xdg-shell v3
 */
void wlf_xdg_positioner_set_parent_configure(
	struct wlf_xdg_positioner *positioner, uint32_t serial);

/**
 * @brief Destroy the positioner wrapper.
 *
 * @param positioner Positioner to destroy; ignored when NULL.
 */
void wlf_xdg_positioner_destroy(struct wlf_xdg_positioner *positioner);

/**
 * @brief Promote the xdg_surface to a toplevel window.
 *
 * @param surface xdg_surface wrapper to promote.
 * @return A new toplevel wrapper, or NULL on failure.
 */
struct wlf_xdg_toplevel *wlf_xdg_surface_get_toplevel(
	struct wlf_xdg_surface *surface);

/**
 * @brief Promote the xdg_surface to a popup.
 *
 * @param surface     The xdg_surface to promote.
 * @param parent      Parent xdg_surface (may be NULL for root popups).
 * @param positioner  Placement positioner.
 * @return A new popup wrapper, or NULL on failure.
 */
struct wlf_xdg_popup *wlf_xdg_surface_get_popup(
	struct wlf_xdg_surface *surface,
	struct wlf_xdg_surface *parent,
	struct wlf_xdg_positioner *positioner);

/**
 * @brief Set the visible window geometry for the xdg_surface.
 *
 * @param surface xdg_surface wrapper to update.
 * @param x Geometry x coordinate.
 * @param y Geometry y coordinate.
 * @param width Geometry width.
 * @param height Geometry height.
 */
void wlf_xdg_surface_set_window_geometry(struct wlf_xdg_surface *surface,
	int32_t x, int32_t y, int32_t width, int32_t height);

/**
 * @brief Acknowledge a configure event.
 *
 * Must be called (with the serial from the configure signal) before the
 * next wl_surface.commit.
 *
 * @param surface xdg_surface wrapper to acknowledge on.
 * @param serial Configure serial received from the compositor.
 */
void wlf_xdg_surface_ack_configure(struct wlf_xdg_surface *surface,
	uint32_t serial);

/**
 * @brief Destroy the xdg_surface wrapper.
 *
 * @param surface Surface wrapper to destroy; ignored when NULL.
 */
void wlf_xdg_surface_destroy(struct wlf_xdg_surface *surface);

/**
 * @brief Return true when @p state is present in @p toplevel's configure state.
 *
 * @param toplevel Toplevel wrapper to query.
 * @param state Toplevel state to test.
 * @return true when the state bit is set, otherwise false.
 */
bool wlf_xdg_toplevel_has_state(const struct wlf_xdg_toplevel *toplevel,
	enum wlf_xdg_toplevel_state state);

/**
 * @brief Return true when @p capability is present in the compositor capabilities.
 *
 * @param toplevel Toplevel wrapper to query.
 * @param capability Capability to test.
 * @return true when the capability bit is set, otherwise false.
 */
bool wlf_xdg_toplevel_has_wm_capability(
	const struct wlf_xdg_toplevel *toplevel,
	enum wlf_xdg_toplevel_wm_capabilities capability);

/**
 * @brief Set the parent toplevel for transient window behavior.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param parent Parent toplevel, or NULL to clear.
 */
void wlf_xdg_toplevel_set_parent(struct wlf_xdg_toplevel *toplevel,
	struct wlf_xdg_toplevel *parent);

/**
 * @brief Set the user-visible window title.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param title UTF-8 title string.
 */
void wlf_xdg_toplevel_set_title(struct wlf_xdg_toplevel *toplevel,
	const char *title);

/**
 * @brief Set the application identifier for the toplevel.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param app_id Application identifier string.
 */
void wlf_xdg_toplevel_set_app_id(struct wlf_xdg_toplevel *toplevel,
	const char *app_id);

/**
 * @brief Request the compositor window menu at a surface-local position.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param seat Seat associated with the triggering input event.
 * @param serial Serial from the triggering input event.
 * @param x Surface-local x coordinate.
 * @param y Surface-local y coordinate.
 */
void wlf_xdg_toplevel_show_window_menu(struct wlf_xdg_toplevel *toplevel,
	struct wl_seat *seat, uint32_t serial, int32_t x, int32_t y);

/**
 * @brief Start an interactive compositor-managed move.
 *
 * @param toplevel Toplevel wrapper to move.
 * @param seat Seat associated with the triggering input event.
 * @param serial Serial from the triggering input event.
 */
void wlf_xdg_toplevel_move(struct wlf_xdg_toplevel *toplevel,
	struct wl_seat *seat, uint32_t serial);

/**
 * @brief Start an interactive compositor-managed resize.
 *
 * @param toplevel Toplevel wrapper to resize.
 * @param seat Seat associated with the triggering input event.
 * @param serial Serial from the triggering input event.
 * @param edge Resize edge/corner hint.
 */
void wlf_xdg_toplevel_resize(struct wlf_xdg_toplevel *toplevel,
	struct wl_seat *seat, uint32_t serial,
	enum wlf_xdg_toplevel_resize_edge edge);

/**
 * @brief Set the maximum window size.
 *
 * A width or height of 0 means unconstrained for that dimension.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param width Maximum width.
 * @param height Maximum height.
 */
void wlf_xdg_toplevel_set_max_size(struct wlf_xdg_toplevel *toplevel,
	int32_t width, int32_t height);

/**
 * @brief Set the minimum window size.
 *
 * A width or height of 0 means unconstrained for that dimension.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param width Minimum width.
 * @param height Minimum height.
 */
void wlf_xdg_toplevel_set_min_size(struct wlf_xdg_toplevel *toplevel,
	int32_t width, int32_t height);

/**
 * @brief Request maximized state.
 *
 * @param toplevel Toplevel wrapper to update.
 */
void wlf_xdg_toplevel_set_maximized(struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Request leaving maximized state.
 *
 * @param toplevel Toplevel wrapper to update.
 */
void wlf_xdg_toplevel_unset_maximized(struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Request fullscreen state.
 *
 * @param toplevel Toplevel wrapper to update.
 * @param output Preferred output, or NULL to let the compositor choose.
 */
void wlf_xdg_toplevel_set_fullscreen(struct wlf_xdg_toplevel *toplevel,
	struct wl_output *output);

/**
 * @brief Request leaving fullscreen state.
 *
 * @param toplevel Toplevel wrapper to update.
 */
void wlf_xdg_toplevel_unset_fullscreen(struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Request minimized state.
 *
 * @param toplevel Toplevel wrapper to update.
 */
void wlf_xdg_toplevel_set_minimized(struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Destroy the toplevel wrapper.
 *
 * @param toplevel Toplevel wrapper to destroy; ignored when NULL.
 */
void wlf_xdg_toplevel_destroy(struct wlf_xdg_toplevel *toplevel);

/**
 * @brief Grab input for a popup.
 *
 * @param popup Popup wrapper to grab.
 * @param seat Seat associated with the triggering input event.
 * @param serial Serial from the triggering input event.
 */
void wlf_xdg_popup_grab(struct wlf_xdg_popup *popup,
	struct wl_seat *seat, uint32_t serial);

/**
 * @brief Reposition an existing popup.
 *
 * @param popup Popup wrapper to reposition.
 * @param positioner New positioner describing popup placement.
 * @param token Client-chosen token returned in the repositioned event.
 * @since xdg-shell v3
 */
void wlf_xdg_popup_reposition(struct wlf_xdg_popup *popup,
	struct wlf_xdg_positioner *positioner, uint32_t token);

/**
 * @brief Destroy the popup wrapper.
 *
 * @param popup Popup wrapper to destroy; ignored when NULL.
 */
void wlf_xdg_popup_destroy(struct wlf_xdg_popup *popup);

#endif /* WAYLAND_WLF_XDG_WM_BASE_H */
