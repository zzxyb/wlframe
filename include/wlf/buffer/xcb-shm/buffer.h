/**
 * @file        buffer.h
 * @brief       CPU-renderable XCB MIT-SHM buffer.
 * @details     Declares shared pixel storage attached to an X server and the
 *              synchronization helpers used before CPU access and destruction.
 * @author      YaoBing Xiao
 * @date        2026-11-11
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-11, initial version\n
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
	struct wlf_buffer base; /**< Generic wlf_buffer interface. */
	xcb_connection_t *xcb; /**< Borrowed XCB connection. */
	xcb_shm_seg_t segment; /**< MIT-SHM segment attached to the X server. */
	void *data; /**< Mapped pixel storage. */
	size_t size; /**< Size of the mapped storage in bytes. */
	size_t stride; /**< Bytes between consecutive pixel rows. */
	uint32_t format; /**< Pixel format used by the buffer. */
	bool pending; /**< Whether server reads must be completed before CPU access or destruction. */
	xcb_get_input_focus_cookie_t fence; /**< Reply cookie fencing pending server reads. */
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

/**
 * @brief Checks whether a buffer uses the XCB MIT-SHM implementation.
 *
 * @param buffer Buffer to inspect or render into.
 * @return true for an XCB MIT-SHM buffer, false otherwise.
 */
bool wlf_buffer_is_xcb_shm(const struct wlf_buffer *buffer);

/**
 * @brief Converts a buffer, asserting that it has the expected type.
 *
 * @param buffer Buffer to inspect or render into.
 * @return Enclosing XCB MIT-SHM buffer; asserts on an incompatible type.
 */
struct wlf_xcb_shm_buffer *wlf_xcb_shm_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Tracks server reads after submitting MIT-SHM image requests.
 * Call on the buffer's connection after the final request in a batch, before
 * accessing or dropping the buffer. Does not consume X11 events.
 *
 * @param buffer Buffer to inspect or render into.
 */
void wlf_xcb_shm_buffer_mark_pending(struct wlf_xcb_shm_buffer *buffer);

/**
 * @brief Waits for server reads; returns false if the connection fails.
 *
 * @param buffer Buffer to inspect or render into.
 * @return true after server reads complete, false if the connection fails.
 */
bool wlf_xcb_shm_buffer_wait(struct wlf_xcb_shm_buffer *buffer);

#endif // BUFFER_WLF_XCB_SHM_BUFFER_H
