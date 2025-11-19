/**
 * @file        wlf_video_buffer.c
 * @brief       Video buffer base implementation.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-26
 */

#include "wlf/va/wlf_video_buffer.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>

/* Base video buffer implementation */

void wlf_video_buffer_init(struct wlf_video_buffer *buffer,
	const struct wlf_video_buffer_impl *impl,
	uint32_t width, uint32_t height) {

	wlf_buffer_init(&buffer->base, &impl->base, width, height);
	buffer->impl = impl;
	buffer->wl_display = NULL;
}

void wlf_video_buffer_set_wayland_display(
	struct wlf_video_buffer *buffer,
	struct wl_display *wl_display) {

	if (buffer) {
		buffer->wl_display = wl_display;
	}
}

struct wl_buffer *wlf_video_buffer_export_to_wl_buffer(
	struct wlf_video_buffer *buffer) {

	if (!buffer || !buffer->impl || !buffer->impl->export_to_wl_buffer) {
		wlf_log(WLF_ERROR, "Invalid buffer or export not supported");
		return NULL;
	}

	if (!buffer->wl_display) {
		wlf_log(WLF_ERROR, "Wayland display not set for buffer");
		return NULL;
	}

	return buffer->impl->export_to_wl_buffer(buffer, buffer->wl_display);
}

struct wlf_video_buffer *wlf_video_buffer_from_buffer(
	struct wlf_buffer *buffer) {

	/* TODO: Add proper type checking mechanism */
	return (struct wlf_video_buffer *)buffer;
}

bool wlf_buffer_is_video_buffer(struct wlf_buffer *buffer) {
	/* TODO: Add proper type checking mechanism */
	return buffer != NULL;
}
