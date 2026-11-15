/**
 * @file        allocator.h
 * @brief       XCB MIT-SHM buffer allocator.
 * @details     Declares an allocator that creates CPU-accessible MIT-SHM
 *              buffers using a borrowed XCB connection.
 * @author      YaoBing Xiao
 * @date        2026-11-15
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-11-15, initial version\n
 */

#ifndef ALLOCATOR_XCB_SHM_ALLOCATOR_H
#define ALLOCATOR_XCB_SHM_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"

#include <xcb/xcb.h>

/**
 * @brief Allocator for CPU-accessible XCB MIT-SHM buffers.
 * The borrowed XCB connection must outlive the allocator and its buffers.
 * Each buffer owns its shared memory segment and server attachment.
 */
struct wlf_xcb_shm_allocator {
	struct wlf_allocator base; /**< Generic allocator interface. */
	xcb_connection_t *xcb; /**< Borrowed XCB connection. */
};

/**
 * @brief Creates an allocator for XCB MIT-SHM buffers.
 * @param xcb Borrowed XCB connection supporting MIT-SHM.
 * @return New allocator, or NULL for an invalid connection or allocation failure.
 * Buffers support WLF_FORMAT_XRGB8888 with dimensions from 1 to INT16_MAX.
 * Format, dimensions and MIT-SHM support are checked when creating buffers.
 */
struct wlf_xcb_shm_allocator *wlf_xcb_shm_allocator_create(xcb_connection_t *xcb);

/**
 * @brief Checks whether an allocator uses the XCB MIT-SHM implementation.
 *
 * @param allocator Allocator to inspect or convert.
 * @return true for an XCB MIT-SHM allocator, false otherwise.
 */
bool wlf_allocator_is_xcb_shm(const struct wlf_allocator *allocator);

/**
 * @brief Converts a generic allocator to an XCB MIT-SHM allocator.
 * @note Asserts that the allocator uses the XCB MIT-SHM implementation.
 *
 * @param allocator Allocator to inspect or convert.
 * @return Enclosing XCB MIT-SHM allocator; asserts on an incompatible type.
 */
struct wlf_xcb_shm_allocator *wlf_xcb_shm_allocator_from_allocator(
	struct wlf_allocator *allocator);

#endif // ALLOCATOR_XCB_SHM_ALLOCATOR_H
