/**
 * @file        wlf_va_video_buffer.c
 * @brief       VA-API video buffer implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 */

#include "wlf/va/wlf_va_video_buffer.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <va/va_wayland.h>

/* VA-API video buffer implementation */

static void va_video_buffer_destroy(struct wlf_buffer *buffer) {
	struct wlf_va_video_buffer *va_buffer =
		(struct wlf_va_video_buffer *)buffer;

	if (va_buffer->wl_buffer) {
		wl_buffer_destroy(va_buffer->wl_buffer);
	}

	/* Note: VA surface is managed by decoder, not destroyed here */

	wlf_signal_emit(&buffer->events.destroy, buffer);
	free(va_buffer);
}

static bool va_video_buffer_begin_data_ptr_access(struct wlf_buffer *buffer,
	uint32_t flags, void **data, uint32_t *format, size_t *stride) {

	struct wlf_va_video_buffer *va_buffer =
		(struct wlf_va_video_buffer *)buffer;

	/* Use vaMapBuffer for CPU access if needed */
	VAImage va_image;
	VAStatus status = vaDeriveImage(va_buffer->va_display,
		va_buffer->surface_id, &va_image);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to derive VA image: %d", status);
		return false;
	}

	void *mapped_data;
	status = vaMapBuffer(va_buffer->va_display, va_image.buf, &mapped_data);
	if (status != VA_STATUS_SUCCESS) {
		vaDestroyImage(va_buffer->va_display, va_image.image_id);
		wlf_log(WLF_ERROR, "Failed to map VA buffer: %d", status);
		return false;
	}

	*data = mapped_data;
	*format = 0;  /* TODO: Convert VA format to common format */
	*stride = va_image.pitches[0];

	return true;
}

static void va_video_buffer_end_data_ptr_access(struct wlf_buffer *buffer) {
	struct wlf_va_video_buffer *va_buffer =
		(struct wlf_va_video_buffer *)buffer;

	/* Unmap the buffer */
	VAImage va_image;
	vaDeriveImage(va_buffer->va_display, va_buffer->surface_id, &va_image);
	vaUnmapBuffer(va_buffer->va_display, va_image.buf);
	vaDestroyImage(va_buffer->va_display, va_image.image_id);
}

static const struct wlf_region *va_video_buffer_opaque_region(
	struct wlf_buffer *buffer) {
	return NULL;  /* Fully opaque */
}

static struct wl_buffer *va_video_buffer_export_to_wl_buffer(
	struct wlf_video_buffer *buffer, struct wl_display *wl_display) {

	struct wlf_va_video_buffer *va_buffer =
		wlf_va_video_buffer_from_video_buffer(buffer);

	if (!va_buffer) {
		return NULL;
	}

	/* Reuse cached buffer if available */
	if (va_buffer->wl_buffer) {
		return va_buffer->wl_buffer;
	}

	/* Get wl_buffer from VA surface using VA-API Wayland extension */
	struct wl_buffer *wl_buffer = NULL;
	VAStatus status = vaGetSurfaceBufferWl(
		va_buffer->va_display,
		va_buffer->surface_id,
		VA_FRAME_PICTURE,
		&wl_buffer
	);

	if (status != VA_STATUS_SUCCESS) {
		wlf_log(WLF_ERROR, "Failed to get wl_buffer from VA surface: %d", status);
		return NULL;
	}

	va_buffer->wl_buffer = wl_buffer;
	wlf_log(WLF_DEBUG, "Exported VA surface %u to wl_buffer", va_buffer->surface_id);

	return wl_buffer;
}

const struct wlf_video_buffer_impl va_video_buffer_impl = {
	.base = {
		.destroy = va_video_buffer_destroy,
		.begin_data_ptr_access = va_video_buffer_begin_data_ptr_access,
		.end_data_ptr_access = va_video_buffer_end_data_ptr_access,
		.opaque_region = va_video_buffer_opaque_region,
	},
	.export_to_wl_buffer = va_video_buffer_export_to_wl_buffer,
};

struct wlf_va_video_buffer *wlf_va_video_buffer_create(
	VADisplay va_display,
	VASurfaceID surface_id,
	uint32_t width, uint32_t height) {

	struct wlf_va_video_buffer *buffer = calloc(1, sizeof(*buffer));
	if (!buffer) {
		wlf_log(WLF_ERROR, "Failed to allocate VA-API video buffer");
		return NULL;
	}

	wlf_video_buffer_init(&buffer->base, &va_video_buffer_impl,
		width, height);

	buffer->va_display = va_display;
	buffer->surface_id = surface_id;
	buffer->wl_buffer = NULL;

	return buffer;
}
