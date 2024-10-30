#ifndef WLF_DMABUF_H
#define WLF_DMABUF_H

#include <stdbool.h>
#include <stdint.h>

#define WLF_DMABUF_MAX_PLANES 4

/**
 * @brief A structure representing a Linux DMA-BUF pixel buffer
 *
 * If the buffer was allocated with explicit modifiers enabled, the `modifier`
 * field must not be INVALID.
 *
 * If the buffer was allocated with explicit modifiers disabled (either because
 * the driver doesn't support it, or because the user didn't specify a valid
 * modifier list), the `modifier` field can have two values: INVALID means that
 * an implicit vendor-defined modifier is in use, LINEAR means that the buffer
 * is linear. The `modifier` field must not have any other value.
 *
 * When importing a DMA-BUF, users must not ignore the modifier unless it's
 * INVALID or LINEAR. In particular, users must not import a DMA-BUF to a
 * legacy API which doesn't support specifying an explicit modifier unless the
 * modifier is set to INVALID or LINEAR.
 */
struct wlf_dmabuf_attributes {
	int32_t width;                /**< Width of the DMA-BUF pixel buffer */
	int32_t height;               /**< Height of the DMA-BUF pixel buffer */
	uint32_t format;              /**< FourCC code, see DRM_FORMAT_* in <drm_fourcc.h> */
	uint64_t modifier;            /**< Modifier, see DRM_FORMAT_MOD_* in <drm_fourcc.h> */

	int n_planes;                 /**< Number of planes in the buffer */
	uint32_t offset[WLF_DMABUF_MAX_PLANES];  /**< Offsets for each plane */
	uint32_t stride[WLF_DMABUF_MAX_PLANES];  /**< Strides for each plane */
	int fd[WLF_DMABUF_MAX_PLANES]; /**< File descriptors for each plane */
};

/**
 * @brief Closes all file descriptors in the DMA-BUF attributes
 * @param attribs Pointer to the DMA-BUF attributes to finish
 */
void wlf_dmabuf_attributes_finish(struct wlf_dmabuf_attributes *attribs);

/**
 * @brief Clones the DMA-BUF attributes
 * @param dst Pointer to the destination DMA-BUF attributes
 * @param src Pointer to the source DMA-BUF attributes to copy from
 * @return true if the cloning was successful, false otherwise
 */
bool wlf_dmabuf_attributes_copy(struct wlf_dmabuf_attributes *dst,
	const struct wlf_dmabuf_attributes *src);

#endif
