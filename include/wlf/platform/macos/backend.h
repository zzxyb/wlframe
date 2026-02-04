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
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

/**
 * @brief macOS backend specific data
 */
struct wlf_backend_macos {
	struct wlf_backend base;              /**< Base backend structure */

	struct {
		struct wlf_listener output_manager_destroy; /**< Output manager destroy listener */
	} listeners;

	bool started;                         /**< Whether backend is started */
};

/**
 * @brief Register the macOS backend
 * @return true on success, false on failure
 */
bool wlf_backend_macos_register(void);

/**
 * @brief Check if a backend is a macOS backend
 * @param backend Pointer to the backend to check
 * @return true if the backend is a macOS backend, false otherwise
 */
bool wlf_backend_is_macos(struct wlf_backend *backend);

/**
 * @brief Get macOS backend from generic backend
 * @param backend Pointer to the generic backend
 * @return Pointer to macOS backend, or NULL if not a macOS backend
 */
struct wlf_backend_macos *wlf_backend_macos_from_backend(struct wlf_backend *backend);

#endif // PLATFORM_WLF_BACKEND_MACOS_H
