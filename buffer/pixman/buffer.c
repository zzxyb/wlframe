#include "wlf/buffer/pixman/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <inttypes.h>
#include <stdlib.h>

static void destroy_buffer(struct wlf_pixman_buffer *buffer) {
	wlf_linked_list_remove(&buffer->link);
	wlf_linked_list_remove(&buffer->buffer_destroy.link);

	pixman_image_unref(buffer->image);
	free(buffer);
}

static void handle_destroy_buffer(struct wlf_listener *listener, void *data) {
	struct wlf_pixman_buffer *buffer =
		wlf_container_of(listener, buffer, buffer_destroy);
	destroy_buffer(buffer);
}

struct wlf_pixman_buffer *wlf_pixman_buffer_create(
	struct wlf_pixman_renderer *renderer, struct wlf_buffer *wlf_buffer) {
	struct wlf_pixman_buffer *buffer = malloc(sizeof(*buffer));
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_pixman_buffer");
		return NULL;
	}

	buffer->buffer = wlf_buffer;
	buffer->renderer = renderer;
	void *data = NULL;
	uint32_t drm_format;
	size_t stride;
	if (!wlf_buffer_begin_data_ptr_access(wlf_buffer,
			WLF_BUFFER_DATA_PTR_ACCESS_READ | WLF_BUFFER_DATA_PTR_ACCESS_WRITE,
			&data, &drm_format, &stride)) {
		wlf_log(WLF_ERROR, "Failed to get buffer data");
		goto failed;
	}
	wlf_buffer_end_data_ptr_access(wlf_buffer);

	pixman_format_code_t format = get_pixman_format_from_drm(drm_format);
	if (format == 0) {
		wlf_log(WLF_ERROR, "Unsupported pixman drm format 0x%"PRIX32,
			drm_format);
		goto failed;
	}

	buffer->image = pixman_image_create_bits(format, wlf_buffer->width,
		wlf_buffer->height, data, stride);
	if (!buffer->image) {
		wlf_log(WLF_ERROR, "Failed to allocate pixman image");
		goto failed;
	}

	buffer->buffer_destroy.notify = handle_destroy_buffer;
	wlf_signal_add(&wlf_buffer->events.destroy, &buffer->buffer_destroy);

	wlf_linked_list_insert(&renderer->buffers, &buffer->link);

	wlf_log(WLF_DEBUG, "Created pixman buffer %dx%d",
		wlf_buffer->width, wlf_buffer->height);

	return buffer;

failed:
	free(buffer);
	return NULL;
}

void wlf_pixman_buffer_destroy(struct wlf_pixman_buffer *buffer) {
	wlf_linked_list_remove(&buffer->link);
	wlf_linked_list_remove(&buffer->buffer_destroy.link);

	pixman_image_unref(buffer->image);

	free(buffer);
}

struct wlf_pixman_buffer *wlf_pixman_buffer_get(
		struct wlf_pixman_renderer *renderer, struct wlf_buffer *wlf_buffer) {
	struct wlf_pixman_buffer *buffer;
	wlf_linked_list_for_each(buffer, &renderer->buffers, link) {
		if (buffer->buffer == wlf_buffer) {
			return buffer;
		}
	}

	return NULL;
}
