/**
 * @file        wlf_dmabuf.h
 * @brief       DMA-BUF utilities and attributes handling.
 * @details     This file provides helper structures and functions for working
 *              with Linux DMA-BUF buffers, including attribute management and
 *              explicit synchronization via sync_file fences.
 *
 *              It is intended to be used in graphics pipelines involving
 *              Wayland, DRM, GBM, Vulkan, or OpenGL, where buffers are shared
 *              across devices and processes.
 *
 * @author      YaoBing Xiao
 * @date        2025-12-27
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-12-27, initial version\n
 */

#ifndef BUFFER_WLF_DMABUF_H
#define BUFFER_WLF_DMABUF_H

#include <stdbool.h>
#include <stdint.h>

#include <gbm.h>

/**
 * @brief DMA-BUF attribute description.
 *
 * Describes the layout and metadata of a DMA-BUF, including size, format,
 * modifier, and per-plane information.
 *
 * This structure is commonly used when importing or exporting buffers via
 * Wayland linux-dmabuf, GBM, or DRM APIs.
 */
struct wlf_dmabuf_attributes {
	int32_t width;                 /**< Buffer width in pixels */
	int32_t height;                /**< Buffer height in pixels */
	uint32_t format;               /**< DRM FourCC format code */
	uint64_t modifier;             /**< DRM format modifier */

	int n_planes;                  /**< Number of planes in the buffer */
	uint32_t offset[GBM_MAX_PLANES]; /**< Byte offset for each plane */
	uint32_t stride[GBM_MAX_PLANES]; /**< Stride (bytes per row) for each plane */
	int fd[GBM_MAX_PLANES];        /**< File descriptor for each plane */
};

/**
 * @brief Releases resources held by DMA-BUF attributes.
 *
 * Closes all plane file descriptors stored in the attributes and resets
 * the structure to a safe state.
 *
 * @param attribs DMA-BUF attributes to clean up.
 */
void wlf_dmabuf_attributes_finish(struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Copies DMA-BUF attributes.
 *
 * Performs a deep copy of DMA-BUF attributes, duplicating plane file
 * descriptors where necessary.
 *
 * @param dst Destination attributes.
 * @param src Source attributes.
 * @return true on success, false on failure.
 */
bool wlf_dmabuf_attributes_copy(struct wlf_dmabuf_attributes *dst,
	const struct wlf_dmabuf_attributes *src);

/**
 * @brief Checks whether sync_file import/export is supported.
 *
 * Tests whether the kernel supports DMA_BUF_IOCTL_IMPORT_SYNC_FILE and
 * DMA_BUF_IOCTL_EXPORT_SYNC_FILE ioctls.
 *
 * @return true if both import and export are supported, false otherwise.
 */
bool wlf_dmabuf_check_sync_file_import_export(void);

/**
 * @brief Imports a sync_file fence into a DMA-BUF.
 *
 * Attaches a sync_file fence to the given DMA-BUF, ensuring that future
 * accesses to the buffer will wait until the fence is signaled.
 *
 * @param dmabuf_fd File descriptor of the DMA-BUF.
 * @param flags Synchronization flags (e.g. DMA_BUF_SYNC_READ/WRITE).
 * @param sync_file_fd File descriptor of the sync_file fence.
 * @return true on success, false on failure.
 */
bool wlf_dmabuf_import_sync_file(int dmabuf_fd, uint32_t flags, int sync_file_fd);

/**
 * @brief Exports a sync_file fence from a DMA-BUF.
 *
 * Creates a sync_file representing the current or future synchronization
 * state of the DMA-BUF.
 *
 * @param dmabuf_fd File descriptor of the DMA-BUF.
 * @param flags Synchronization flags (e.g. DMA_BUF_SYNC_READ/WRITE).
 * @return A new sync_file file descriptor on success, or -1 on failure.
 */
int wlf_dmabuf_export_sync_file(int dmabuf_fd, uint32_t flags);

#endif // BUFFER_WLF_DMABUF_H
