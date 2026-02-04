/**
 * @file        device.h
 * @brief       Metal device management for wlframe renderer.
 * @details     This file defines the Metal device abstraction layer for wlframe.
 *              It provides functions for device creation, management, and querying.
 *
 * @author      YaoBing Xiao
 * @date        2026-02-04
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, 2026-02-04, initial version.
 */

#ifndef METAL_MTL_DEVICE_H
#define METAL_MTL_DEVICE_H

#include <stdbool.h>

/**
 * @struct wlf_mtl_device
 * @brief Metal device wrapper for wlframe.
 *
 * This structure wraps a Metal device (MTLDevice) and provides
 * additional metadata and functionality.
 */
struct wlf_mtl_device {
	void *device;        /**< Metal device handle (id<MTLDevice>). */
	char *name;          /**< Device name. */
	bool is_low_power;   /**< Whether this is a low-power device. */
};

/**
 * @brief Creates a Metal device.
 *
 * This function selects the default or best available Metal device.
 *
 * @return Pointer to the created Metal device, or NULL on failure.
 */
struct wlf_mtl_device *wlf_mtl_device_create(void);

/**
 * @brief Destroys a Metal device.
 *
 * Releases all resources associated with the Metal device.
 *
 * @param device Pointer to the Metal device to destroy.
 */
void wlf_mtl_device_destroy(struct wlf_mtl_device *device);

#endif // METAL_MTL_DEVICE_H
