/**
 * @file        wlf_wl_buffer.h
 * @brief       Wayland wl_buffer wrapper for wlframe.
 * @details     Wraps a wl_buffer, emitting a release signal when the compositor
 *              no longer uses the buffer so the client may reuse its backing store.
 * @author      YaoBing Xiao
 * @date        2026-05-23
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-23, initial version\n
 */

#ifndef WAYLAND_WLF_WL_BUFFER_H
#define WAYLAND_WLF_WL_BUFFER_H

#include "wlf/utils/wlf_signal.h"

struct wl_buffer;

/**
 * @brief Wayland buffer wrapper.
 */
struct wlf_wl_buffer {
	struct wl_buffer *wl_buffer; /**< Underlying Wayland object. */

	struct {
		struct wlf_signal destroy;  /**< Emitted before destruction. */
		struct wlf_signal release;  /**< Emitted when compositor releases the buffer. */
	} events;
};

/**
 * @brief Wraps an existing wl_buffer in a wlf_wl_buffer.
 *
 * Attaches a release listener and returns the wrapper. Ownership of
 * wl_buffer passes to the returned wlf_wl_buffer.
 *
 * @param wl_buffer Raw Wayland buffer object.
 * @return Newly allocated wlf_wl_buffer, or NULL on failure.
 */
struct wlf_wl_buffer *wlf_wl_buffer_wrap(struct wl_buffer *wl_buffer);

/**
 * @brief Destroys a wlf_wl_buffer. Passing NULL is a no-op.
 */
void wlf_wl_buffer_destroy(struct wlf_wl_buffer *buffer);

#endif /* WAYLAND_WLF_WL_BUFFER_H */
