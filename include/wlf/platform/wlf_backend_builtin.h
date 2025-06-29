/**
 * @file        wlf_backend_builtin.h
 * @brief       Built-in backend registration and initialization
 * @details     This file provides functions to register all built-in backends
 *              and initialize the backend subsystem with default backends.
 * @author      YaoBing Xiao
 * @date        2025-06-25
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-25, initial version\n
 */

#ifndef PLATFORM_WLF_BACKEND_BUILTIN_H
#define PLATFORM_WLF_BACKEND_BUILTIN_H

#include <stdbool.h>

/**
 * @brief Initialize the backend subsystem and register all built-in backends
 * This should be called once at program startup before using any backend functions
 * @return true on success, false on failure
 */
bool wlf_backend_builtin_init(void);

/**
 * @brief Cleanup the backend subsystem and unregister all backends
 * This should be called once at program shutdown
 */
void wlf_backend_builtin_cleanup(void);

#endif // PLATFORM_WLF_BACKEND_BUILTIN_H
