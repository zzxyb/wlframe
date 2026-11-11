/**
 * @file buffer.h
 * @brief CPU-renderable X11 MIT-SHM buffer.
 */
#ifndef BUFFER_WLF_XCB_SHM_BUFFER_H
#define BUFFER_WLF_XCB_SHM_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"

#include <xcb/shm.h>

/**
 * @brief Shared pixel storage attached read-only to an X server.
 * The borrowed connection must outlive the buffer. Access pixels through
 * wlf_buffer_begin_data_ptr_access(), which waits for pending server reads.
 */
struct wlf_xcb_shm_buffer {
	struct wlf_buffer base;
	xcb_connection_t *xcb;
	xcb_shm_seg_t segment;
	void *data;
	size_t size;
	size_t stride;
	uint32_t format;
	bool pending;
	xcb_get_input_focus_cookie_t fence;
};

/**
 * @brief Creates and attaches a SysV MIT-SHM buffer.
 * @param xcb Borrowed XCB connection supporting MIT-SHM.
 * @param width Width in pixels, from 1 to INT16_MAX.
 * @param height Height in pixels, from 1 to INT16_MAX.
 * @param format Must be WLF_FORMAT_XRGB8888 (32 bits per pixel).
 * @return Buffer, or NULL if allocation or server attachment fails.
 */
struct wlf_xcb_shm_buffer *wlf_xcb_shm_buffer_create(xcb_connection_t *xcb,
	int width, int height, uint32_t format);

bool wlf_buffer_is_xcb_shm(const struct wlf_buffer *buffer);

/** @brief Converts a buffer, asserting that it has the expected type. */
struct wlf_xcb_shm_buffer *wlf_xcb_shm_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Tracks server reads after submitting MIT-SHM image requests.
 * Call on the buffer's connection after the final request in a batch, before
 * accessing or dropping the buffer. Does not consume X11 events.
 */
void wlf_xcb_shm_buffer_mark_pending(struct wlf_xcb_shm_buffer *buffer);

/** @brief Waits for server reads; returns false if the connection fails. */
bool wlf_xcb_shm_buffer_wait(struct wlf_xcb_shm_buffer *buffer);

#endif // BUFFER_WLF_XCB_SHM_BUFFER_H
