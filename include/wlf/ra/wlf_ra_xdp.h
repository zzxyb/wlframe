/**
 * @file        wlf_ra_xdp.h
 * @brief       XDG Desktop Portal screen capture for Wayland.
 * @details     Provides PipeWire-based screen capture functionality using
 *              XDG Desktop Portal for Wayland compositors.
 * @author      YaoBing Xiao
 * @date        2026-02-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-09, initial version\n
 */

#ifndef RA_XDP_WLF_RA_XDP_H
#define RA_XDP_WLF_RA_XDP_H

#include "../common/wlf_ra_types.h"
#include "../common/wlf_ra_framebuffer.h"
#include "wlf/utils/wlf_signal.h"

/**
 * @brief XDP context for screen capture.
 */
struct wlf_ra_xdp_context {
	struct {
		struct wlf_signal frame_captured;    /**< Signal emitted when frame is captured */
		struct wlf_signal cursor_moved;      /**< Signal emitted when cursor position changes */
		struct wlf_signal capture_started;   /**< Signal emitted when capture starts */
		struct wlf_signal capture_stopped;   /**< Signal emitted when capture stops */
		struct wlf_signal error;             /**< Signal emitted on error */
	} events;

	void *internal;  /**< Internal implementation data */
};

/**
 * @brief Creates XDP context for screen capture.
 * @return XDP context handle or NULL on error.
 */
struct wlf_ra_xdp_context *wlf_ra_xdp_create(void);

/**
 * @brief Destroys XDP context.
 * @param ctx XDP context to destroy.
 */
void wlf_ra_xdp_destroy(struct wlf_ra_xdp_context *ctx);

/**
 * @brief Starts screen capture session.
 * @param ctx XDP context.
 * @param fb Framebuffer to update with captured frames.
 * @return 0 on success, negative on error.
 */
int wlf_ra_xdp_start_capture(struct wlf_ra_xdp_context *ctx,
		struct wlf_ra_framebuffer *fb);

/**
 * @brief Stops screen capture session.
 * @param ctx XDP context.
 */
void wlf_ra_xdp_stop_capture(struct wlf_ra_xdp_context *ctx);

/**
 * @brief Gets current cursor position.
 * @param ctx XDP context.
 * @return Cursor position.
 */
struct wlf_ra_point wlf_ra_xdp_get_cursor_position(struct wlf_ra_xdp_context *ctx);

/**
 * @brief Sends keyboard event (requires RemoteDesktop portal).
 * @param ctx XDP context.
 * @param keysym X11 keysym.
 * @param pressed True for press, false for release.
 */
void wlf_ra_xdp_send_keyboard(struct wlf_ra_xdp_context *ctx,
		uint32_t keysym, bool pressed);

/**
 * @brief Sends pointer event (requires RemoteDesktop portal).
 * @param ctx XDP context.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param button_mask Button mask.
 */
void wlf_ra_xdp_send_pointer(struct wlf_ra_xdp_context *ctx,
		int x, int y, uint32_t button_mask);

/**
 * @brief Checks if XDP is available.
 * @return True if available, false otherwise.
 */
bool wlf_ra_xdp_is_available(void);

#endif /* RA_XDP_WLF_RA_XDP_H */
