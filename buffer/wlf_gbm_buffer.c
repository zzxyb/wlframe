/**
 * @file        wlf_gbm_buffer.c
 * @brief       GBM buffer implementation.
 * @author      YaoBing Xiao
 * @date        2026-01-22
 */

#include "wlf/buffer/wlf_gbm_buffer.h"
#include "wlf/allocator/wlf_gbm_allocator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <errno.h>

static const struct wlf_buffer_impl buffer_impl;

static struct wlf_gbm_buffer *get_gbm_buffer_from_buffer(
		struct wlf_buffer *wlf_buffer) {
	assert(wlf_buffer->impl == &buffer_impl);
	return wlf_container_of(wlf_buffer, struct wlf_gbm_buffer, base);
}

/**
 * @brief Exports a GBM buffer object as DMA-BUF attributes.
 */
static bool export_gbm_bo(struct gbm_bo *bo,
		struct wlf_dmabuf_attributes *out) {
	struct wlf_dmabuf_attributes attribs = {0};

	attribs.n_planes = gbm_bo_get_plane_count(bo);
	if (attribs.n_planes > GBM_MAX_PLANES) {
		wlf_log(WLF_ERROR, "GBM BO contains too many planes (%d)",
			attribs.n_planes);
		return false;
	}

	attribs.width = gbm_bo_get_width(bo);
	attribs.height = gbm_bo_get_height(bo);
	attribs.format = gbm_bo_get_format(bo);
	attribs.modifier = gbm_bo_get_modifier(bo);

	int i;
	for (i = 0; i < attribs.n_planes; ++i) {
		attribs.fd[i] = gbm_bo_get_fd_for_plane(bo, i);
		if (attribs.fd[i] < 0) {
			wlf_log(WLF_ERROR, "gbm_bo_get_fd_for_plane failed");
			goto error_fd;
		}

		attribs.offset[i] = gbm_bo_get_offset(bo, i);
		attribs.stride[i] = gbm_bo_get_stride_for_plane(bo, i);
	}

	*out = attribs;
	return true;

error_fd:
	for (int j = 0; j < i; ++j) {
		close(attribs.fd[j]);
	}
	return false;
}

/**
 * @brief Creates a GBM buffer with the specified format.
 *
 * This function is called by the GBM allocator to create buffers.
 */
struct wlf_gbm_buffer *wlf_gbm_buffer_create(struct wlf_gbm_allocator *alloc,
		int width, int height, uint32_t format, uint64_t modifier) {
	struct gbm_device *gbm_device = alloc->gbm_device;

	bool has_modifier = true;
	uint64_t fallback_modifier = DRM_FORMAT_MOD_INVALID;
	errno = 0;
	struct gbm_bo *bo = NULL;

	// Try to create with explicit modifier first
	if (modifier != DRM_FORMAT_MOD_INVALID) {
		bo = gbm_bo_create_with_modifiers(gbm_device, width, height,
			format, &modifier, 1);
	}

	// Fallback to implicit modifier allocation
	if (bo == NULL) {
		uint32_t usage = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
		if (modifier == DRM_FORMAT_MOD_LINEAR) {
			usage |= GBM_BO_USE_LINEAR;
			fallback_modifier = DRM_FORMAT_MOD_LINEAR;
		} else {
			fallback_modifier = DRM_FORMAT_MOD_INVALID;
		}

		errno = 0;
		bo = gbm_bo_create(gbm_device, width, height, format, usage);
		has_modifier = false;
	}

	if (bo == NULL) {
		wlf_log_errno(WLF_ERROR, "gbm_bo_create failed");
		return NULL;
	}

	struct wlf_gbm_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		gbm_bo_destroy(bo);
		return NULL;
	}

	wlf_buffer_init(&buffer->base, &buffer_impl, width, height);
	buffer->gbm_bo = bo;

	if (!export_gbm_bo(bo, &buffer->dmabuf)) {
		free(buffer);
		gbm_bo_destroy(bo);
		return NULL;
	}

	// If the buffer has been allocated with an implicit modifier, update it
	if (!has_modifier) {
		buffer->dmabuf.modifier = fallback_modifier;
	}

	wlf_linked_list_insert(&alloc->buffers, &buffer->link);

	char *format_name = drmGetFormatName(buffer->dmabuf.format);
	char *modifier_name = drmGetFormatModifierName(buffer->dmabuf.modifier);
	wlf_log(WLF_DEBUG, "Allocated %dx%d GBM buffer "
		"with format %s (0x%08X), modifier %s (0x%016lX)",
		buffer->base.width, buffer->base.height,
		format_name ? format_name : "<unknown>", buffer->dmabuf.format,
		modifier_name ? modifier_name : "<unknown>", buffer->dmabuf.modifier);
	free(format_name);
	free(modifier_name);

	return buffer;
}

/**
 * @brief Destroys a GBM buffer.
 */
static void buffer_destroy(struct wlf_buffer *wlf_buffer) {
	struct wlf_gbm_buffer *buffer = get_gbm_buffer_from_buffer(wlf_buffer);

	wlf_dmabuf_attributes_finish(&buffer->dmabuf);
	if (buffer->gbm_bo != NULL) {
		gbm_bo_destroy(buffer->gbm_bo);
	}
	wlf_linked_list_remove(&buffer->link);
	free(buffer);
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
};

// Public API implementations

struct wlf_gbm_buffer *wlf_gbm_buffer_from_buffer(struct wlf_buffer *buffer) {
	if (buffer->impl != &buffer_impl) {
		return NULL;
	}
	return get_gbm_buffer_from_buffer(buffer);
}

bool wlf_buffer_is_gbm(struct wlf_buffer *buffer) {
	return buffer->impl == &buffer_impl;
}

bool wlf_gbm_buffer_get_dmabuf(struct wlf_gbm_buffer *buffer,
		struct wlf_dmabuf_attributes *attribs) {
	if (buffer == NULL || attribs == NULL) {
		return false;
	}
	*attribs = buffer->dmabuf;
	return true;
}
