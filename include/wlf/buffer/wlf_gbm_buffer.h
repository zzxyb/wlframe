/**
 * @file        wlf_gbm_buffer.h
 * @brief       GBM buffer implementation.
 * @details     This file provides a buffer implementation backed by GBM (Generic Buffer Manager).
 *              GBM buffers are hardware-accelerated and can be used for GPU rendering
 *              and direct scanout on DRM/KMS displays. Buffers are exported as DMA-BUF
 *              file descriptors for zero-copy sharing between processes and devices.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-01-22, initial version\n
 */

#ifndef BUFFER_WLF_GBM_BUFFER_H
#define BUFFER_WLF_GBM_BUFFER_H

#include "wlf/buffer/wlf_buffer.h"
#include "wlf/dmabuf/wlf_dmabuf.h"
#include "wlf/utils/wlf_linked_list.h"

#include <gbm.h>
#include <stdbool.h>

struct wlf_gbm_allocator;

/**
 * @brief A buffer backed by a GBM buffer object.
 *
 * This buffer implementation wraps a GBM buffer object (gbm_bo) and provides
 * DMA-BUF export capabilities for hardware-accelerated rendering and display.
 */
struct wlf_gbm_buffer {
	struct wlf_buffer base;              /**< Base buffer structure */

	struct wlf_linked_list link;         /**< Link in the allocator's buffer list */

	struct gbm_bo *gbm_bo;               /**< GBM buffer object (NULL if device destroyed) */
	struct wlf_dmabuf_attributes dmabuf; /**< Exported DMA-BUF attributes */
};

/**
 * @brief Gets the GBM buffer from a generic buffer.
 *
 * @param buffer Generic buffer pointer.
 * @return GBM buffer pointer, or NULL if the buffer is not a GBM buffer.
 */
struct wlf_gbm_buffer *wlf_gbm_buffer_from_buffer(struct wlf_buffer *buffer);

/**
 * @brief Checks if a buffer is a GBM buffer.
 *
 * @param buffer Buffer to check.
 * @return true if the buffer is a GBM buffer, false otherwise.
 */
bool wlf_buffer_is_gbm(struct wlf_buffer *buffer);

/**
 * @brief Gets the DMA-BUF attributes of a GBM buffer.
 *
 * @param buffer GBM buffer.
 * @param attribs Output DMA-BUF attributes.
 * @return true on success, false on failure.
 */
bool wlf_gbm_buffer_get_dmabuf(struct wlf_gbm_buffer *buffer,
	struct wlf_dmabuf_attributes *attribs);

#endif // BUFFER_WLF_GBM_BUFFER_H
