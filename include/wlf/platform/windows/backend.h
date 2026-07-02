/**
 * @file        backend.h
 * @brief       Windows backend implementation for wlframe.
 * @details     This file defines the native Windows backend used by wlframe.
 *              It extends the generic `wlf_backend` interface, stores the
 *              application instance handle, and runs the Win32 message loop
 *              used by Windows platform objects.
 * @author      YaoBing Xiao
 * @date        2026-07-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-02, initial version\n
 */

#ifndef WINDOWS_BACKEND_H
#define WINDOWS_BACKEND_H

#include "wlf/platform/wlf_backend.h"

#include <stdbool.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/**
 * @brief Windows backend specific data.
 *
 * Extends the generic @ref wlf_backend with state used by the Win32 backend.
 * The backend owns the event loop, exposes the process module instance through
 * @ref wlf_backend_impl.native_display, and records the thread running the
 * Win32 message loop.
 *
 * @note `instance` is obtained with GetModuleHandleW(NULL). It is borrowed
 *       from the process and must not be released with FreeLibrary().
 * @note `thread_id` is a numeric thread identifier, not a HANDLE. It must not
 *       be closed with CloseHandle().
 */
struct wlf_windows_backend {
	struct wlf_backend base;  /**< Base backend structure. */
	HINSTANCE instance;       /**< Current process module instance handle. */
	DWORD thread_id;          /**< ID of the thread that created the backend. */
};

/**
 * @brief Creates a Windows backend instance.
 *
 * Allocates and initializes a backend that integrates wlframe with the Win32
 * message loop.
 *
 * @return Pointer to the generic @ref wlf_backend, or NULL on failure.
 */
struct wlf_backend *windows_backend_create(void);

/**
 * @brief Checks whether a backend is implemented by the Windows backend.
 *
 * @param backend Pointer to the backend to check.
 * @return true if the backend is a Windows backend, false otherwise.
 */
bool wlf_backend_is_windows(const struct wlf_backend *backend);

/**
 * @brief Casts a generic backend to a Windows backend.
 *
 * @param backend Pointer to the generic backend.
 * @return Pointer to the underlying @ref wlf_windows_backend.
 *
 * @note The caller must ensure @p backend is a Windows backend, for example by
 *       calling @ref wlf_backend_is_windows first.
 */
struct wlf_windows_backend *wlf_windows_backend_from_backend(
	struct wlf_backend *backend);

#endif // WINDOWS_BACKEND_H
