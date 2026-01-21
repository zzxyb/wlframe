/**
 * @file        wlf_gbm_allocator.h
 * @brief       GBM-based buffer allocator.
 * @details     This file provides a buffer allocator implementation using GBM
 *              (Generic Buffer Manager). The allocator creates hardware-accelerated
 *              buffers suitable for GPU rendering and direct display scanout.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef ALLOCATOR_WLF_GBM_ALLOCATOR_H
#define ALLOCATOR_WLF_GBM_ALLOCATOR_H

#include "wlf/allocator/wlf_allocator.h"
#include "wlf/utils/wlf_linked_list.h"

#include <gbm.h>
#include <stdbool.h>

/**
 * @brief A GBM-based buffer allocator.
 *
 * This allocator creates buffers backed by GBM (Generic Buffer Manager),
 * which provides hardware-accelerated buffer allocation suitable for
 * GPU rendering and direct display scanout.
 */
struct wlf_gbm_allocator {
	struct wlf_allocator base;       /**< Base allocator structure */

	int fd;                          /**< DRM device file descriptor */
	struct gbm_device *gbm_device;   /**< GBM device handle */

	struct wlf_linked_list buffers;  /**< List of allocated buffers */
};

/**
 * @brief Creates a new GBM allocator.
 *
 * Creates an allocator that uses GBM to allocate hardware-accelerated buffers.
 * The DRM device must support PRIME buffer export.
 *
 * @param fd DRM device file descriptor. Must remain valid for the allocator's lifetime.
 * @return Pointer to the created allocator, or NULL on failure.
 */
struct wlf_allocator *wlf_gbm_allocator_create(int fd);

/**
 * @brief Gets the GBM allocator from a generic allocator.
 *
 * @param allocator Generic allocator pointer.
 * @return GBM allocator pointer, or NULL if the allocator is not a GBM allocator.
 */
struct wlf_gbm_allocator *wlf_gbm_allocator_from_allocator(
	struct wlf_allocator *allocator);

/**
 * @brief Checks if an allocator is a GBM allocator.
 *
 * @param allocator Allocator to check.
 * @return true if the allocator is a GBM allocator, false otherwise.
 */
bool wlf_allocator_is_gbm(struct wlf_allocator *allocator);

#endif // ALLOCATOR_WLF_GBM_ALLOCATOR_H
