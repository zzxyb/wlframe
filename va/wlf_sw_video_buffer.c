/**
 * @file        wlf_sw_video_buffer.c
 * @brief       Software video buffer implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 */

#include "wlf/va/wlf_sw_video_buffer.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

/* Software video buffer implementation */

static void sw_video_buffer_destroy(struct wlf_buffer *buffer) {
	struct wlf_sw_video_buffer *sw_buffer =
		(struct wlf_sw_video_buffer *)buffer;

	if (sw_buffer->wl_buffer) {
		wl_buffer_destroy(sw_buffer->wl_buffer);
	}

	if (sw_buffer->shm_pool) {
		wl_shm_pool_destroy(sw_buffer->shm_pool);
	}

	if (sw_buffer->data && sw_buffer->shm_fd >= 0) {
		munmap(sw_buffer->data, sw_buffer->size);
	}

	if (sw_buffer->shm_fd >= 0) {
		close(sw_buffer->shm_fd);
	}

	wlf_signal_emit(&buffer->events.destroy, buffer);
	free(sw_buffer);
}

static bool sw_video_buffer_begin_data_ptr_access(struct wlf_buffer *buffer,
	uint32_t flags, void **data, uint32_t *format, size_t *stride) {

	struct wlf_sw_video_buffer *sw_buffer =
		(struct wlf_sw_video_buffer *)buffer;

	if (buffer->accessing_data_ptr) {
		wlf_log(WLF_ERROR, "Buffer already being accessed");
		return false;
	}

	*data = sw_buffer->data;
	*format = sw_buffer->pixel_format;
	*stride = sw_buffer->stride;

	buffer->accessing_data_ptr = true;
	return true;
}

static void sw_video_buffer_end_data_ptr_access(struct wlf_buffer *buffer) {
	buffer->accessing_data_ptr = false;
}

static const struct wlf_region *sw_video_buffer_opaque_region(
	struct wlf_buffer *buffer) {
	return NULL;  /* Fully opaque */
}

static struct wl_buffer *sw_video_buffer_export_to_wl_buffer(
	struct wlf_video_buffer *buffer, struct wl_display *wl_display) {

	struct wlf_sw_video_buffer *sw_buffer =
		wlf_sw_video_buffer_from_video_buffer(buffer);

	if (!sw_buffer) {
		return NULL;
	}

	/* Reuse cached buffer if available */
	if (sw_buffer->wl_buffer) {
		return sw_buffer->wl_buffer;
	}

	/* Get wl_shm from registry */
	/* TODO: Proper wl_shm retrieval from registry */
	struct wl_shm *shm = NULL;  /* Would get from registry */
	if (!shm) {
		wlf_log(WLF_ERROR, "wl_shm not available");
		return NULL;
	}

	/* Create wl_shm_pool if not exists */
	if (!sw_buffer->shm_pool) {
		sw_buffer->shm_pool = wl_shm_create_pool(shm,
			sw_buffer->shm_fd, sw_buffer->size);
		if (!sw_buffer->shm_pool) {
			wlf_log(WLF_ERROR, "Failed to create wl_shm_pool");
			return NULL;
		}
	}

	/* Create wl_buffer from pool */
	sw_buffer->wl_buffer = wl_shm_pool_create_buffer(
		sw_buffer->shm_pool, 0,
		buffer->base.base.width, buffer->base.base.height,
		sw_buffer->stride,
		WL_SHM_FORMAT_ARGB8888
	);

	if (!sw_buffer->wl_buffer) {
		wlf_log(WLF_ERROR, "Failed to create wl_buffer");
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Created wl_buffer from software video buffer");

	return sw_buffer->wl_buffer;
}

const struct wlf_video_buffer_impl sw_video_buffer_impl = {
	.base = {
		.destroy = sw_video_buffer_destroy,
		.begin_data_ptr_access = sw_video_buffer_begin_data_ptr_access,
		.end_data_ptr_access = sw_video_buffer_end_data_ptr_access,
		.opaque_region = sw_video_buffer_opaque_region,
	},
	.export_to_wl_buffer = sw_video_buffer_export_to_wl_buffer,
};

struct wlf_sw_video_buffer *wlf_sw_video_buffer_create(
	uint32_t width, uint32_t height,
	uint32_t pixel_format) {

	struct wlf_sw_video_buffer *buffer = calloc(1, sizeof(*buffer));
	if (!buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate software video buffer");
		return NULL;
	}

	wlf_video_buffer_init(&buffer->base, &sw_video_buffer_impl,
		width, height);

	buffer->pixel_format = pixel_format;
	buffer->stride = width * 4;  /* Assuming ARGB8888 */
	buffer->size = buffer->stride * height;
	buffer->shm_fd = -1;

	/* Create shared memory */
	buffer->shm_fd = memfd_create("wlframe-video", MFD_CLOEXEC);
	if (buffer->shm_fd < 0) {
		wlf_log(WLF_ERROR, "Failed to create memfd");
		free(buffer);
		return NULL;
	}

	if (ftruncate(buffer->shm_fd, buffer->size) < 0) {
		wlf_log(WLF_ERROR, "Failed to truncate memfd");
		close(buffer->shm_fd);
		free(buffer);
		return NULL;
	}

	buffer->data = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE,
		MAP_SHARED, buffer->shm_fd, 0);
	if (buffer->data == MAP_FAILED) {
		wlf_log(WLF_ERROR, "Failed to mmap memfd");
		close(buffer->shm_fd);
		free(buffer);
		return NULL;
	}

	return buffer;
}
