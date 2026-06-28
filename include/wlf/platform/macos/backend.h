/**
 * @file        backend.h
 * @brief       macOS backend implementation
 * @details     This backend allows wlframe to run natively on macOS,
 *              using Metal for rendering and native window management.
 * @author      YaoBing Xiao
 * @date        2026-02-11
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-11, initial version\n
 */

#ifndef PLATFORM_WLF_BACKEND_MACOS_H
#define PLATFORM_WLF_BACKEND_MACOS_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

/**
 * @brief macOS backend specific data.
 *
 * Extends the generic @ref wlf_backend with state used by the native
 * macOS backend implementation.
 */
struct wlf_backend_macos {
	struct wlf_backend base;              /**< Base backend structure */
};

/**
 * @brief Creates a macOS backend instance.
 *
 * Allocates and initializes a backend that integrates wlframe with the
 * AppKit event loop and exposes the native macOS application object via
 * @ref wlf_backend_impl.native_display.
 *
 * @return Pointer to the generic @ref wlf_backend, or NULL on failure.
 */
struct wlf_backend *macos_backend_create(void);

/**
 * @brief Register the macOS backend
 * @return true on success, false on failure
 */
bool wlf_backend_macos_register(void);

/**
 * @brief Checks whether a backend is implemented by the macOS backend.
 *
 * @param backend Pointer to the backend to check.
 * @return true if the backend is a macOS backend, false otherwise.
 */
bool wlf_backend_is_macos(struct wlf_backend *backend);

/**
 * @brief Casts a generic backend to a macOS backend.
 *
 * @param backend Pointer to the generic backend.
 * @return Pointer to the underlying @ref wlf_backend_macos.
 *
 * @note The caller must ensure @p backend is a macOS backend, for example by
 *       calling @ref wlf_backend_is_macos first.
 */
struct wlf_backend_macos *wlf_backend_macos_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_MACOS_H
