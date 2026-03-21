#include "wlf/buffer/pixman/buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <inttypes.h>

#include <drm_fourcc.h>

static const struct wlf_pixman_pixel_format formats[] = {
	{
		.drm_format = DRM_FORMAT_ARGB8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8a8,
#else
		.pixman_format = PIXMAN_a8r8g8b8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_XBGR8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8x8,
#else
		.pixman_format = PIXMAN_x8b8g8r8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_XRGB8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_b8g8r8x8,
#else
		.pixman_format = PIXMAN_x8r8g8b8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_ABGR8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_r8g8b8a8,
#else
		.pixman_format = PIXMAN_a8b8g8r8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_RGBA8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_a8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8a8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_RGBX8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_x8b8g8r8,
#else
		.pixman_format = PIXMAN_r8g8b8x8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_BGRA8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_a8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8a8,
#endif
	},
	{
		.drm_format = DRM_FORMAT_BGRX8888,
#if WLF_BIG_ENDIAN
		.pixman_format = PIXMAN_x8r8g8b8,
#else
		.pixman_format = PIXMAN_b8g8r8x8,
#endif
	},
#if WLF_LITTLE_ENDIAN
	{
		.drm_format = DRM_FORMAT_RGB565,
		.pixman_format = PIXMAN_r5g6b5,
	},
	{
		.drm_format = DRM_FORMAT_BGR565,
		.pixman_format = PIXMAN_b5g6r5,
	},
	{
		.drm_format = DRM_FORMAT_ARGB2101010,
		.pixman_format = PIXMAN_a2r10g10b10,
	},
	{
		.drm_format = DRM_FORMAT_XRGB2101010,
		.pixman_format = PIXMAN_x2r10g10b10,
	},
	{
		.drm_format = DRM_FORMAT_ABGR2101010,
		.pixman_format = PIXMAN_a2b10g10r10,
	},
	{
		.drm_format = DRM_FORMAT_XBGR2101010,
		.pixman_format = PIXMAN_x2b10g10r10,
	},
	{
		.drm_format = DRM_FORMAT_ABGR16161616,
		.pixman_format = PIXMAN_a16b16g16r16,
	},
#endif
};

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
	struct wlf_pixman_renderer *renderer,struct wlf_buffer *wlf_buffer) {
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

pixman_format_code_t get_pixman_format_from_drm(uint32_t fmt) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (formats[i].drm_format == fmt) {
			return formats[i].pixman_format;
		}
	}

	wlf_log(WLF_ERROR, "DRM format 0x%"PRIX32" has no pixman equivalent", fmt);
	return 0;
}

uint32_t get_drm_format_from_pixman(pixman_format_code_t fmt) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (formats[i].pixman_format == fmt) {
			return formats[i].drm_format;
		}
	}

	wlf_log(WLF_ERROR, "pixman format 0x%"PRIX32" has no drm equivalent", fmt);
	return DRM_FORMAT_INVALID;
}
const uint32_t *get_pixman_drm_formats(size_t *len) {
	static uint32_t drm_formats[sizeof(formats) / sizeof(formats[0])];
	*len = sizeof(formats) / sizeof(formats[0]);
	for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
		drm_formats[i] = formats[i].drm_format;
	}

	return drm_formats;
}
