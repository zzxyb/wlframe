#include "wlf/renderer/pixman/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/buffer/pixman/buffer.h"
#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/texture/pixman/texture.h"

#include <pixman.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void pixman_renderer_destroy(struct wlf_renderer *render) {
	struct wlf_pixman_renderer *pixman_render =
		wlf_pixman_renderer_from_renderer(render);

	struct wlf_pixman_buffer *buffer, *buffer_tmp;
	wlf_linked_list_for_each_safe(buffer, buffer_tmp, &pixman_render->buffers, link) {
		wlf_pixman_buffer_destroy(buffer);
	}

	struct wlf_pixman_texture *tex, *tex_tmp;
	wlf_linked_list_for_each_safe(tex, tex_tmp, &pixman_render->textures, link) {
		wlf_texture_destroy(&tex->wlf_texture);
	}

	free(pixman_render);
}

static struct wlf_texture *pixman_renderer_texture_from_buffer(struct wlf_renderer *wlf_renderer,
		struct wlf_buffer *wlf_buffer) {
	struct wlf_pixman_renderer *renderer = wlf_pixman_renderer_from_renderer(wlf_renderer);

	void *data = NULL;
	uint32_t drm_format;
	size_t stride;
	if (!wlf_buffer_begin_data_ptr_access(wlf_buffer, WLF_BUFFER_DATA_PTR_ACCESS_READ,
			&data, &drm_format, &stride)) {
		return NULL;
	}
	if (stride == 0 || wlf_buffer->height > SIZE_MAX / stride) {
		wlf_buffer_end_data_ptr_access(wlf_buffer);
		return NULL;
	}
	size_t size = stride * wlf_buffer->height;
	void *copy = malloc(size);
	if (copy == NULL) {
		wlf_buffer_end_data_ptr_access(wlf_buffer);
		return NULL;
	}
	memcpy(copy, data, size);
	wlf_buffer_end_data_ptr_access(wlf_buffer);

	struct wlf_pixman_texture *texture = wlf_pixman_texture_create(renderer,
		drm_format, wlf_buffer->width, wlf_buffer->height);
	if (texture == NULL) {
		free(copy);
		return NULL;
	}

	texture->image = pixman_image_create_bits_no_clear(texture->format,
		(int)wlf_buffer->width, (int)wlf_buffer->height, copy, (int)stride);
	if (!texture->image) {
		wlf_log(WLF_ERROR, "Failed to create pixman image");
		wlf_linked_list_remove(&texture->link);
		free(copy);
		free(texture);
		return NULL;
	}

	texture->data = copy;

	return &texture->wlf_texture;
}

static struct wlf_render_target_info *pixman_renderer_begin_buffer_pass(
		struct wlf_renderer *renderer, struct wlf_buffer *buffer,
		const struct wlf_buffer_pass_options *options) {
	(void)options;
	if (buffer == NULL) {
		return NULL;
	}

	struct wlf_pixman_renderer *pixman_renderer =
		wlf_pixman_renderer_from_renderer(renderer);
	struct wlf_pixman_buffer *pixman_buffer =
		wlf_pixman_buffer_get(pixman_renderer, buffer);
	if (pixman_buffer == NULL) {
		pixman_buffer = wlf_pixman_buffer_create(pixman_renderer, buffer);
	}
	if (pixman_buffer == NULL) {
		return NULL;
	}

	struct wlf_pixman_render_target_info *target =
		wlf_pixman_begin_pixman_render_pass(pixman_buffer);
	return target != NULL ? &target->base : NULL;
}

static const struct wlf_renderer_impl pixman_renderer_impl = {
	.destroy = pixman_renderer_destroy,
	.begin_buffer_pass = pixman_renderer_begin_buffer_pass,
	.texture_from_buffer = pixman_renderer_texture_from_buffer,
};

bool wlf_renderer_is_pixman(const struct wlf_renderer *renderer) {
	return renderer->impl == &pixman_renderer_impl;
}

struct wlf_pixman_renderer *wlf_pixman_renderer_from_renderer(
		struct wlf_renderer *renderer) {
	if (!wlf_renderer_is_pixman(renderer)) {
		return NULL;
	}

	struct wlf_pixman_renderer *pixman_renderer =
		wlf_container_of(renderer, pixman_renderer, base);

	return pixman_renderer;
}

struct wlf_renderer *wlf_pixman_renderer_create_from_backend(
		struct wlf_backend *backend) {
	struct wlf_pixman_renderer *renderer = malloc(sizeof(*renderer));
	if (renderer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_pixman_renderer");
		return NULL;
	}

	wlf_renderer_init(&renderer->base, &pixman_renderer_impl);
	wlf_linked_list_init(&renderer->buffers);
	wlf_linked_list_init(&renderer->textures);
	renderer->base.type = CPU;
	renderer->backend = backend;
	renderer->base.features.damage = true;

	return &renderer->base;
}
