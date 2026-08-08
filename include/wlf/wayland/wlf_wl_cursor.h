/**
 * @file        wlf_wl_cursor.h
 * @brief       Wayland cursor-shape and themed-cursor implementation.
 * @details     Declares the Wayland cursor wrapper, including the cursor
 *              shape protocol path and the themed-cursor fallback.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */
#ifndef WAYLAND_WLF_WL_CURSOR_H
#define WAYLAND_WLF_WL_CURSOR_H

#include "wlf/types/wlf_cursor.h"

struct wl_compositor;
struct wl_pointer;
struct wl_shm;
struct wp_cursor_shape_manager_v1;

/**
 * @brief Creates a Wayland cursor controller.
 *
 * The controller prefers the cursor-shape protocol and falls back to themed
 * cursor images when the protocol is unavailable.
 *
 * @param pointer Pointer receiving cursor updates.
 * @param shape_manager Optional cursor-shape manager.
 * @param compositor Wayland compositor used by the themed fallback.
 * @param shm Shared-memory interface used by the themed fallback.
 * @return Newly allocated cursor controller, or NULL on failure.
 */
struct wlf_cursor *wlf_wl_cursor_create(struct wl_pointer *pointer,
	struct wp_cursor_shape_manager_v1 *shape_manager,
	struct wl_compositor *compositor, struct wl_shm *shm);

#endif
