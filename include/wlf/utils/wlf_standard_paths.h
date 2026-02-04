/**
 * @file        wlf_standard_paths.h
 * @brief       Cross-platform standard paths utility for wlframe.
 * @details     This file provides a cross-platform API for accessing standard system directories
 *              such as documents, cache, config, data, and temporary directories. Similar to
 *              Qt's QStandardPaths, it abstracts platform-specific path conventions on Linux
 *              (following XDG Base Directory specification) and macOS (using NSSearchPathForDirectoriesInDomains).
 *
 * @author      YaoBing Xiao
 * @date        2026-02-04
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-02-04, initial version.
 */

#ifndef UTILS_WLF_STANDARD_PATHS_H
#define UTILS_WLF_STANDARD_PATHS_H

#include <stdbool.h>

struct wlf_standard_paths;

/**
 * @enum wlf_standard_location
 * @brief Standard directory locations.
 *
 * This enumeration defines standard system directories that can be accessed
 * in a platform-independent manner.
 */
enum wlf_standard_location {
	WLF_LOCATION_HOME,           /**< User's home directory */
	WLF_LOCATION_DESKTOP,        /**< User's desktop directory */
	WLF_LOCATION_DOCUMENTS,      /**< User's documents directory */
	WLF_LOCATION_DOWNLOADS,      /**< User's downloads directory */
	WLF_LOCATION_MUSIC,          /**< User's music directory */
	WLF_LOCATION_PICTURES,       /**< User's pictures directory */
	WLF_LOCATION_VIDEOS,         /**< User's videos directory */
	WLF_LOCATION_CONFIG,         /**< User's configuration directory */
	WLF_LOCATION_DATA,           /**< User's application data directory */
	WLF_LOCATION_CACHE,          /**< User's cache directory */
	WLF_LOCATION_RUNTIME,        /**< Runtime directory (for sockets, pipes, etc.) */
	WLF_LOCATION_TEMP,           /**< Temporary directory */
	WLF_LOCATION_APPLICATIONS,   /**< Applications directory */
};

/**
 * @struct wlf_standard_paths_impl
 * @brief Standard paths implementation interface.
 *
 * This structure defines function pointers used by platform-specific implementations.
 */
struct wlf_standard_paths_impl {
	char *(*get_path)(struct wlf_standard_paths *paths, enum wlf_standard_location location);
	void (*destroy)(struct wlf_standard_paths *paths);
};

/**
 * @struct wlf_standard_paths
 * @brief Core standard paths object.
 *
 * This structure represents a standard paths accessor with platform-specific
 * implementation data.
 */
struct wlf_standard_paths {
	const struct wlf_standard_paths_impl *impl;
	void *data;  /**< Platform-specific data (opaque pointer). */
};

/**
 * @brief Create a standard paths instance for the current platform.
 *
 * Automatically detects the platform and creates the appropriate implementation.
 *
 * @return Pointer to the created standard paths instance, or NULL on error.
 */
struct wlf_standard_paths *wlf_standard_paths_auto_create(void);

/**
 * @brief Destroy a standard paths instance.
 *
 * Frees all resources associated with the standard paths instance.
 *
 * @param paths Pointer to the standard paths instance to destroy.
 */
void wlf_standard_paths_destroy(struct wlf_standard_paths *paths);

/**
 * @brief Get the path for a standard location.
 *
 * Returns the path to a standard system directory. The returned string
 * must be freed by the caller using free().
 *
 * Platform-specific behavior:
 * - Linux: Follows XDG Base Directory Specification
 * - macOS: Uses NSSearchPathForDirectoriesInDomains and standard paths
 *
 * @param paths Pointer to the standard paths instance (can be NULL for singleton).
 * @param location The standard location to query.
 * @return A newly allocated string containing the path, or NULL on error.
 *         The caller must free the returned string.
 *
 * @example
 * @code
 * char *config_path = wlf_standard_path_get(NULL, WLF_LOCATION_CONFIG);
 * if (config_path) {
 *     printf("Config directory: %s\n", config_path);
 *     free(config_path);
 * }
 * @endcode
 */
char *wlf_standard_path_get(struct wlf_standard_paths *paths, 
	enum wlf_standard_location location);

/**
 * @brief Get the writable path for a standard location with an application name.
 *
 * Returns the path to a standard system directory, appending the application
 * name to make it specific to your application. This is useful for creating
 * application-specific config, data, and cache directories.
 *
 * The returned string must be freed by the caller using free().
 *
 * @param paths Pointer to the standard paths instance (can be NULL for singleton).
 * @param location The standard location to query.
 * @param app_name The application name to append (can be NULL for no suffix).
 * @return A newly allocated string containing the path, or NULL on error.
 *         The caller must free the returned string.
 *
 * @example
 * @code
 * char *config_path = wlf_standard_path_writable(NULL, WLF_LOCATION_CONFIG, "myapp");
 * // Linux: ~/.config/myapp
 * // macOS: ~/Library/Application Support/myapp
 * if (config_path) {
 *     printf("App config directory: %s\n", config_path);
 *     free(config_path);
 * }
 * @endcode
 */
char *wlf_standard_path_writable(struct wlf_standard_paths *paths,
	enum wlf_standard_location location, const char *app_name);

/**
 * @brief Ensure a directory exists, creating it if necessary.
 *
 * Creates the specified directory and any necessary parent directories.
 * Similar to `mkdir -p`.
 *
 * @param path The directory path to create.
 * @return true if the directory exists or was created successfully, false otherwise.
 *
 * @example
 * @code
 * char *config_path = wlf_standard_path_writable(NULL, WLF_LOCATION_CONFIG, "myapp");
 * if (config_path && wlf_standard_path_ensure_dir(config_path)) {
 *     printf("Config directory ready: %s\n", config_path);
 * }
 * free(config_path);
 * @endcode
 */
bool wlf_standard_path_ensure_dir(const char *path);

/**
 * @brief Get the display name for a standard location.
 *
 * Returns a human-readable name for a standard location type.
 * The returned string is static and should not be freed.
 *
 * @param location The standard location to query.
 * @return A static string containing the display name.
 *
 * @example
 * @code
 * const char *name = wlf_standard_path_display_name(WLF_LOCATION_DOCUMENTS);
 * printf("Location name: %s\n", name); // "Documents"
 * @endcode
 */
const char *wlf_standard_path_display_name(enum wlf_standard_location location);

#endif // UTILS_WLF_STANDARD_PATHS_H
