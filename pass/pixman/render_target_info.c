#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void render_target_info_destroy(struct wlf_render_target_info *render_target) {
	struct wlf_pixman_render_target_info *target_info =
		wlf_pixman_render_target_info_from_info(render_target);
	free(target_info);
}

static struct wlf_renderer *render_target_info_get_renderer(
		struct wlf_render_target_info *render_target) {
	struct wlf_pixman_render_target_info *target_info =
		wlf_pixman_render_target_info_from_info(render_target);
	struct wlf_pixman_renderer *renderer = target_info->buffer->renderer;

	return &renderer->base;
}

static const struct wlf_render_target_info_impl render_target_info_impl = {
	.destroy = render_target_info_destroy,
	.get_renderer = render_target_info_get_renderer,
};

struct wlf_pixman_render_target_info *wlf_pixman_begin_pixman_render_pass(
		struct wlf_pixman_buffer *buffer) {
	struct wlf_pixman_render_target_info *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_pixman_render_target_info");
		return NULL;
	}

	wlf_render_target_info_init(&pass->base, &render_target_info_impl);

	if (!begin_pixman_data_ptr_access(buffer->buffer, &buffer->image,
			WLF_BUFFER_DATA_PTR_ACCESS_READ | WLF_BUFFER_DATA_PTR_ACCESS_WRITE)) {
		free(pass);
		return NULL;
	}

	wlf_buffer_lock(buffer->buffer);
	pass->buffer = buffer;

	return pass;
}

bool begin_pixman_data_ptr_access(struct wlf_buffer *buffer, pixman_image_t **image_ptr,
		uint32_t flags) {
	pixman_image_t *image = *image_ptr;

	void *data = NULL;
	uint32_t drm_format;
	size_t stride;
	if (!wlf_buffer_begin_data_ptr_access(buffer, flags,
			&data, &drm_format, &stride)) {
		return false;
	}

	if (data != pixman_image_get_data(image)) {
		pixman_format_code_t format = get_pixman_format_from_drm(drm_format);
		assert(format != 0);

		pixman_image_t *new_image = pixman_image_create_bits_no_clear(format,
			buffer->width, buffer->height, data, stride);
		if (new_image == NULL) {
			wlf_buffer_end_data_ptr_access(buffer);
			return false;
		}

		pixman_image_unref(image);
		image = new_image;
	}

	*image_ptr = image;
	return true;
}

bool wlf_render_target_info_is_pixman(
		const struct wlf_render_target_info *render_target) {
	return render_target->impl == &render_target_info_impl;
}

struct wlf_pixman_render_target_info *wlf_pixman_render_target_info_from_info(
		struct wlf_render_target_info *render_target) {
	assert(render_target->impl == &render_target_info_impl);

	struct wlf_pixman_render_target_info *pixman_target_info =
		wlf_container_of(render_target, pixman_target_info, base);

	return pixman_target_info;
}
