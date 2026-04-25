#include "wlf/buffer/shm/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <linux/memfd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <assert.h>
#include <drm_fourcc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define RANDNAME_PATTERN "/wlframe-XXXXXX"

static int get_min_stride(uint32_t format, int width) {
	// Simple implementation for common formats
	// A real implementation should use a pixel format info table
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

static void buffer_destroy(struct wlf_buffer *buffer) {
	struct wlf_shm_buffer *shm_buffer =
		wlf_shm_buffer_from_buffer(buffer);

	munmap(shm_buffer->data, shm_buffer->size);
	close(shm_buffer->shm.fd);
	free(buffer);
}

static bool buffer_begin_data_ptr_access(struct wlf_buffer *buffer,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlf_shm_buffer *shm_buffer =
		wlf_shm_buffer_from_buffer(buffer);
	WLF_UNUSED(flags);
	*data = shm_buffer->data;
	*format = shm_buffer->shm.format;
	*stride = shm_buffer->shm.stride;

	return true;
}

static void buffer_end_data_ptr_access(struct wlf_buffer *buffer) {
	WLF_UNUSED(buffer);
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.begin_data_ptr_access = buffer_begin_data_ptr_access,
	.end_data_ptr_access = buffer_end_data_ptr_access,
};

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

bool wlf_buffer_is_shm(struct wlf_buffer *buffer) {
	return buffer->impl == &buffer_impl;
}

struct wlf_shm_buffer *wlf_shm_buffer_from_buffer(struct wlf_buffer *buffer) {
	assert(buffer->impl == &buffer_impl);

	struct wlf_shm_buffer *shm_buffer =
		wlf_container_of(buffer, shm_buffer, base);

	return shm_buffer;
}

static void randname(char *buf) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	long r = ts.tv_nsec;
	for (int i = 0; i < 6; ++i) {
		buf[i] = 'A'+(r&15)+(r&16)*2;
		r >>= 5;
	}
}

bool wlf_shm_buffer_get_shm(struct wlf_shm_buffer *buffer,
		struct wlf_shm_attributes *attribs) {
	if (buffer == NULL || attribs == NULL) {
		return false;
	}
	*attribs = buffer->shm;
	return true;
}

static int excl_shm_open(char *name) {
	int retries = 100;
	do {
		randname(name + strlen(RANDNAME_PATTERN) - 6);

		--retries;
		// CLOEXEC is guaranteed to be set by shm_open
		int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
		if (fd >= 0) {
			return fd;
		}
	} while (retries > 0 && errno == EEXIST);

	return -1;
}

int wlf_allocate_shm_file(size_t size) {
	char name[] = RANDNAME_PATTERN;
	int fd = excl_shm_open(name);
	if (fd < 0) {
		return -1;
	}
	shm_unlink(name);

	int ret;
	do {
		ret = ftruncate(fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

bool wlf_allocate_shm_file_pair(size_t size, int *rw_fd_ptr, int *ro_fd_ptr) {
	char name[] = RANDNAME_PATTERN;
	int rw_fd = excl_shm_open(name);
	if (rw_fd < 0) {
		return false;
	}

	// CLOEXEC is guaranteed to be set by shm_open
	int ro_fd = shm_open(name, O_RDONLY, 0);
	if (ro_fd < 0) {
		shm_unlink(name);
		close(rw_fd);
		return false;
	}

	shm_unlink(name);

	// Make sure the file cannot be re-opened in read-write mode (e.g. via
	// "/proc/self/fd/" on Linux)
	if (fchmod(rw_fd, 0) != 0) {
		close(rw_fd);
		close(ro_fd);
		return false;
	}

	int ret;
	do {
		ret = ftruncate(rw_fd, size);
	} while (ret < 0 && errno == EINTR);
	if (ret < 0) {
		close(rw_fd);
		close(ro_fd);
		return false;
	}

	*rw_fd_ptr = rw_fd;
	*ro_fd_ptr = ro_fd;
	return true;
}
