#include "wlf/buffer/wlf_shm_buffer.h"
#include "wlf/allocator/wlf_shm_allocator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static const struct wlf_buffer_impl buffer_impl;

static struct wlf_shm_buffer *shm_buffer_from_buffer(
		struct wlf_buffer *wlf_buffer) {
	assert(wlf_buffer->impl == &buffer_impl);
	return wlf_container_of(wlf_buffer, struct wlf_shm_buffer, base);
}

static int get_min_stride(uint32_t format, int width) {
	switch (format) {
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_XBGR8888:
			return width * 4;
		case DRM_FORMAT_RGB888:
		case DRM_FORMAT_BGR888:
			return width * 3;
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_BGR565:
			return width * 2;
		default:
			wlf_log(WLF_ERROR, "Unsupported pixel format 0x%X", format);
			return -1;
	}
}

struct wlf_shm_buffer *wlf_shm_buffer_create(struct wlf_shm_allocator *alloc,
		int width, int height, uint32_t format) {
	(void)alloc; // Unused for now

	int stride = get_min_stride(format, width);
	if (stride < 0) {
		return NULL;
	}

	struct wlf_shm_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}

	wlf_buffer_init(&buffer->base, &buffer_impl, width, height);

	buffer->size = stride * height;
	buffer->shm.fd = wlf_allocate_shm_file(buffer->size);
	if (buffer->shm.fd < 0) {
		free(buffer);
		return NULL;
	}

	buffer->shm.format = format;
	buffer->shm.width = width;
	buffer->shm.height = height;
	buffer->shm.stride = stride;
	buffer->shm.offset = 0;

	buffer->data = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		buffer->shm.fd, 0);
	if (buffer->data == MAP_FAILED) {
		wlf_log_errno(WLF_ERROR, "mmap failed");
		close(buffer->shm.fd);
		free(buffer);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Allocated %dx%d SHM buffer with format 0x%08X, stride %d",
		width, height, format, stride);

	return buffer;
}

static void buffer_destroy(struct wlf_buffer *wlf_buffer) {
	struct wlf_shm_buffer *buffer = shm_buffer_from_buffer(wlf_buffer);

	munmap(buffer->data, buffer->size);
	close(buffer->shm.fd);
	free(buffer);
}

static bool buffer_get_shm(struct wlf_buffer *wlf_buffer,
		struct wlf_shm_attributes *shm) {
	struct wlf_shm_buffer *buffer = shm_buffer_from_buffer(wlf_buffer);
	*shm = buffer->shm;
	return true;
}

static bool buffer_begin_data_ptr_access(struct wlf_buffer *wlf_buffer,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlf_shm_buffer *buffer = shm_buffer_from_buffer(wlf_buffer);
	(void)flags; // Unused
	*data = buffer->data;
	*format = buffer->shm.format;
	*stride = buffer->shm.stride;
	return true;
}

static void buffer_end_data_ptr_access(struct wlf_buffer *wlf_buffer) {
	(void)wlf_buffer; // Nothing to do
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.begin_data_ptr_access = buffer_begin_data_ptr_access,
	.end_data_ptr_access = buffer_end_data_ptr_access,
};

struct wlf_shm_buffer *wlf_shm_buffer_from_buffer(struct wlf_buffer *buffer) {
	if (buffer->impl != &buffer_impl) {
		return NULL;
	}
	return shm_buffer_from_buffer(buffer);
}

bool wlf_buffer_is_shm(struct wlf_buffer *buffer) {
	return buffer->impl == &buffer_impl;
}

bool wlf_shm_buffer_get_shm(struct wlf_shm_buffer *buffer,
		struct wlf_shm_attributes *attribs) {
	if (buffer == NULL || attribs == NULL) {
		return false;
	}
	*attribs = buffer->shm;
	return true;
}
