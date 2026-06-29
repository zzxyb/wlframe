/**
 * @file        wlf_zwlr_layer_shell_v1.h
 * @brief       Wayland wlr-layer-shell-unstable-v1 protocol wrapper.
 * @details     Provides wrappers for creating and configuring layer surfaces
 *              such as desktop backgrounds, panels, notifications, and
 *              lock-screen components. Layer-surface state is double-buffered
 *              and takes effect on the next wl_surface commit.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WLF_ZWLR_LAYER_SHELL_V1_H
#define WLF_ZWLR_LAYER_SHELL_V1_H

#include <stdint.h>

#include "wlf/utils/wlf_signal.h"

struct wl_output;
struct wl_registry;
struct wl_surface;
struct xdg_popup;
struct zwlr_layer_shell_v1;
struct zwlr_layer_surface_v1;

/**
 * @brief Layer in which a layer surface is rendered.
 *
 * Values mirror the zwlr_layer_shell_v1.layer protocol enum. Layers are
 * ordered from bottom to top.
 */
enum wlf_zwlr_layer_v1 {
	WLF_ZWLR_LAYER_V1_BACKGROUND = 0, /**< Background layer. */
	WLF_ZWLR_LAYER_V1_BOTTOM = 1,	  /**< Bottom layer. */
	WLF_ZWLR_LAYER_V1_TOP = 2,	  /**< Top layer. */
	WLF_ZWLR_LAYER_V1_OVERLAY = 3,	  /**< Overlay layer. */
};

/**
 * @brief Edges to which a layer surface is anchored.
 *
 * Values may be combined with bitwise OR when passed to
 * wlf_zwlr_layer_surface_v1_set_anchor(). For set_exclusive_edge(), pass
 * either one edge or WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_NONE.
 */
enum wlf_zwlr_layer_surface_v1_anchor {
	WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_NONE = 0,   /**< No edge. */
	WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP = 1,    /**< Top edge. */
	WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM = 2, /**< Bottom edge. */
	WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT = 4,   /**< Left edge. */
	WLF_ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT = 8,  /**< Right edge. */
};

/**
 * @brief Keyboard focus behavior for a layer surface.
 */
enum wlf_zwlr_layer_surface_v1_keyboard_interactivity {
	/** The surface does not receive keyboard focus. */
	WLF_ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE = 0,
	/** The surface requests exclusive keyboard focus. */
	WLF_ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE = 1,
	/** The surface uses regular focus semantics. Requires version 4. */
	WLF_ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND = 2,
};

/**
 * @brief Wrapper around a zwlr_layer_surface_v1 protocol object.
 *
 * The configure fields contain the most recently received configure event.
 * The caller owns this object and must destroy it with
 * wlf_zwlr_layer_surface_v1_destroy().
 */
struct wlf_zwlr_layer_surface_v1 {
	struct zwlr_layer_surface_v1 *base; /**< Underlying protocol object. */
	uint32_t version;		    /**< Bound protocol version. */

	uint32_t
		configure_serial; /**< Serial from the latest configure event. */
	uint32_t width;		  /**< Suggested width from configure. */
	uint32_t height;	  /**< Suggested height from configure. */

	struct {
		/** Emitted with this object after configure fields are updated. */
		struct wlf_signal configure;
		/** Emitted with this object when the compositor closes it. */
		struct wlf_signal closed;
		/** Emitted with this object immediately before local destruction. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Wrapper around a bound zwlr_layer_shell_v1 global.
 */
struct wlf_zwlr_layer_shell_v1 {
	struct zwlr_layer_shell_v1 *base; /**< Underlying protocol object. */
	uint32_t version;		  /**< Bound protocol version. */

	struct {
		/** Emitted with this object immediately before destruction. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Bind the zwlr_layer_shell_v1 global from the Wayland registry.
 *
 * The advertised version is clamped to the version supported by the client
 * protocol definitions.
 *
 * @param registry Wayland registry containing the global.
 * @param name Registry name of the global.
 * @param version Version advertised by the compositor.
 * @return Newly allocated wrapper, or NULL on failure.
 */
struct wlf_zwlr_layer_shell_v1 *wlf_zwlr_layer_shell_v1_create(
	struct wl_registry *registry, uint32_t name, uint32_t version);

/**
 * @brief Destroy a layer-shell wrapper.
 *
 * Existing layer surfaces created from this object are not affected.
 * Accepts NULL.
 *
 * @param shell Layer-shell wrapper to destroy.
 */
void wlf_zwlr_layer_shell_v1_destroy(struct wlf_zwlr_layer_shell_v1 *shell);

/**
 * @brief Assign the layer-surface role to a wl_surface.
 *
 * The wl_surface must not already have a role or have a buffer committed.
 * The output may be NULL, in which case the compositor chooses an output.
 *
 * @param shell Bound layer-shell wrapper.
 * @param surface Surface to which the layer-surface role is assigned.
 * @param output Target output, or NULL for compositor selection.
 * @param layer Initial layer for the surface.
 * @param namespace Namespace identifying the surface's purpose.
 * @return Newly allocated layer-surface wrapper, or NULL on failure.
 */
struct wlf_zwlr_layer_surface_v1 *wlf_zwlr_layer_shell_v1_get_layer_surface(
	struct wlf_zwlr_layer_shell_v1 *shell, struct wl_surface *surface,
	struct wl_output *output, enum wlf_zwlr_layer_v1 layer,
	const char *namespace);

/**
 * @brief Set the desired dimensions of a layer surface.
 *
 * A dimension of zero asks the compositor to determine that dimension. The
 * corresponding axis must then be anchored to both opposite edges.
 *
 * @param surface Layer surface to configure.
 * @param width Desired width in surface-local coordinates, or zero.
 * @param height Desired height in surface-local coordinates, or zero.
 */
void wlf_zwlr_layer_surface_v1_set_size(
	struct wlf_zwlr_layer_surface_v1 *surface, uint32_t width,
	uint32_t height);

/**
 * @brief Set the edges to which a layer surface is anchored.
 *
 * @param surface Layer surface to configure.
 * @param anchor Bitwise combination of wlf_zwlr_layer_surface_v1_anchor.
 */
void wlf_zwlr_layer_surface_v1_set_anchor(
	struct wlf_zwlr_layer_surface_v1 *surface,
	enum wlf_zwlr_layer_surface_v1_anchor anchor);

/**
 * @brief Set the exclusive zone reserved by a layer surface.
 *
 * A positive value reserves that many surface-local coordinates. Zero does
 * not reserve space. A value of -1 requests placement without regard to other
 * layer surfaces' exclusive zones.
 *
 * @param surface Layer surface to configure.
 * @param zone Exclusive-zone size or special value described above.
 */
void wlf_zwlr_layer_surface_v1_set_exclusive_zone(
	struct wlf_zwlr_layer_surface_v1 *surface, int32_t zone);

/**
 * @brief Set margins from the layer surface's anchored edges.
 *
 * @param surface Layer surface to configure.
 * @param top Top margin in surface-local coordinates.
 * @param right Right margin in surface-local coordinates.
 * @param bottom Bottom margin in surface-local coordinates.
 * @param left Left margin in surface-local coordinates.
 */
void wlf_zwlr_layer_surface_v1_set_margin(
	struct wlf_zwlr_layer_surface_v1 *surface, int32_t top, int32_t right,
	int32_t bottom, int32_t left);

/**
 * @brief Set keyboard focus behavior for a layer surface.
 *
 * ON_DEMAND requires protocol version 4. On earlier versions the wrapper logs
 * an error and does not send the request.
 *
 * @param surface Layer surface to configure.
 * @param interactivity Requested keyboard focus behavior.
 */
void wlf_zwlr_layer_surface_v1_set_keyboard_interactivity(
	struct wlf_zwlr_layer_surface_v1 *surface,
	enum wlf_zwlr_layer_surface_v1_keyboard_interactivity interactivity);

/**
 * @brief Assign an xdg_popup as a child of a layer surface.
 *
 * This request must be made before the popup's initial commit.
 *
 * @param surface Parent layer surface.
 * @param popup Popup whose parent is set to the layer surface.
 */
void wlf_zwlr_layer_surface_v1_get_popup(
	struct wlf_zwlr_layer_surface_v1 *surface, struct xdg_popup *popup);

/**
 * @brief Acknowledge a configure event.
 *
 * @param surface Configured layer surface.
 * @param serial Serial received through events.configure.
 */
void wlf_zwlr_layer_surface_v1_ack_configure(
	struct wlf_zwlr_layer_surface_v1 *surface, uint32_t serial);

/**
 * @brief Change the layer of a layer surface.
 *
 * Requires protocol version 2. On version 1 the wrapper logs an error and
 * does not send the request.
 *
 * @param surface Layer surface to configure.
 * @param layer New layer for the surface.
 */
void wlf_zwlr_layer_surface_v1_set_layer(
	struct wlf_zwlr_layer_surface_v1 *surface,
	enum wlf_zwlr_layer_v1 layer);

/**
 * @brief Select the edge to which the exclusive zone applies.
 *
 * Requires protocol version 5. Use ANCHOR_NONE to restore automatic edge
 * selection. Otherwise edge must be a single edge included in the current
 * anchor state.
 *
 * @param surface Layer surface to configure.
 * @param edge Exclusive edge, or ANCHOR_NONE for automatic selection.
 */
void wlf_zwlr_layer_surface_v1_set_exclusive_edge(
	struct wlf_zwlr_layer_surface_v1 *surface,
	enum wlf_zwlr_layer_surface_v1_anchor edge);

/**
 * @brief Destroy a layer-surface wrapper and its protocol object.
 *
 * Emits events.destroy before releasing resources. Accepts NULL.
 *
 * @param surface Layer-surface wrapper to destroy.
 */
void wlf_zwlr_layer_surface_v1_destroy(
	struct wlf_zwlr_layer_surface_v1 *surface);

#endif /* WLF_ZWLR_LAYER_SHELL_V1_H */
