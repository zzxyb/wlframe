#include "wlf/buffer/xcb-shm/buffer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

bool wlf_xcb_shm_buffer_wait(struct wlf_xcb_shm_buffer *buffer) {
	if (buffer->pending) {
		/* X requests execute in order. This reply fences all preceding
		 * ShmPutImage requests without stealing events from the backend. */
		xcb_get_input_focus_reply_t *reply =
			xcb_get_input_focus_reply(buffer->xcb, buffer->fence, NULL);
		buffer->pending = false;
		if (reply == NULL) {
			return false;
		}
		free(reply);
	}
	return xcb_connection_has_error(buffer->xcb) == 0;
}

void wlf_xcb_shm_buffer_mark_pending(struct wlf_xcb_shm_buffer *buffer) {
	if (buffer->pending) {
		xcb_discard_reply(buffer->xcb, buffer->fence.sequence);
	}
	buffer->fence = xcb_get_input_focus(buffer->xcb);
	buffer->pending = true;
}

static void buffer_destroy(struct wlf_buffer *base) {
	struct wlf_xcb_shm_buffer *buffer = wlf_xcb_shm_buffer_from_buffer(base);
	wlf_xcb_shm_buffer_wait(buffer);
	wlf_buffer_finish(base);
	xcb_shm_detach(buffer->xcb, buffer->segment);
	xcb_flush(buffer->xcb);
	shmdt(buffer->data);
	free(buffer);
}

static bool buffer_begin_data_ptr_access(struct wlf_buffer *base,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlf_xcb_shm_buffer *buffer = wlf_xcb_shm_buffer_from_buffer(base);
	WLF_UNUSED(flags);
	if (!wlf_xcb_shm_buffer_wait(buffer)) {
		return false;
	}
	*data = buffer->data;
	*format = buffer->format;
	*stride = buffer->stride;
	return true;
}

static void buffer_end_data_ptr_access(struct wlf_buffer *base) {
	WLF_UNUSED(base);
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.begin_data_ptr_access = buffer_begin_data_ptr_access,
	.end_data_ptr_access = buffer_end_data_ptr_access,
};

struct wlf_xcb_shm_buffer *wlf_xcb_shm_buffer_create(xcb_connection_t *xcb,
		int width, int height, uint32_t format) {
	if (xcb == NULL || xcb_connection_has_error(xcb) || width <= 0 || height <= 0 ||
			width > INT16_MAX || height > INT16_MAX ||
			format != WLF_FORMAT_XRGB8888 ||
			(size_t)width > SIZE_MAX / 4 / (size_t)height) {
		return NULL;
	}
	const xcb_query_extension_reply_t *extension = xcb_get_extension_data(xcb, &xcb_shm_id);
	if (extension == NULL || !extension->present) {
		wlf_log(WLF_ERROR, "X server does not support MIT-SHM");
		return NULL;
	}
	struct wlf_xcb_shm_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	buffer->xcb = xcb;
	buffer->format = format;
	buffer->stride = (size_t)width * 4;
	buffer->size = buffer->stride * height;
	int shmid = shmget(IPC_PRIVATE, buffer->size, IPC_CREAT | 0600);
	if (shmid < 0) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate MIT-SHM segment");
		free(buffer);
		return NULL;
	}
	buffer->data = shmat(shmid, NULL, 0);
	if (buffer->data == (void *)-1) {
		wlf_log_errno(WLF_ERROR, "Failed to map MIT-SHM segment");
		shmctl(shmid, IPC_RMID, NULL);
		free(buffer);
		return NULL;
	}
	buffer->segment = xcb_generate_id(xcb);
	xcb_generic_error_t *error = xcb_request_check(xcb,
		xcb_shm_attach_checked(xcb, buffer->segment, shmid, true));
	/* Both processes now hold the segment; remove it when the last detaches.
	 * Also remove it on attachment failure, avoiding leaked SysV objects. */
	shmctl(shmid, IPC_RMID, NULL);
	if (error != NULL || xcb_connection_has_error(xcb)) {
		wlf_log(WLF_ERROR, "Failed to attach MIT-SHM segment to X server");
		free(error);
		shmdt(buffer->data);
		free(buffer);
		return NULL;
	}
	wlf_buffer_init(&buffer->base, &buffer_impl, width, height);
	return buffer;
}

bool wlf_buffer_is_xcb_shm(const struct wlf_buffer *buffer) {
	return buffer->impl == &buffer_impl;
}

struct wlf_xcb_shm_buffer *wlf_xcb_shm_buffer_from_buffer(struct wlf_buffer *base) {
	assert(wlf_buffer_is_xcb_shm(base));
	struct wlf_xcb_shm_buffer *buffer = wlf_container_of(base, buffer, base);
	return buffer;
}
