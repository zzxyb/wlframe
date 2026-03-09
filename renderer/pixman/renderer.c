#include "wlf/renderer/pixman/renderer.h"
#include "wlf/renderer/wlf_renderer.h"
#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/buffer/pixman/render_buffer.h"
#include "wlf/texture/pixman/texture.h"

#include <pixman.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void pixman_renderer_destroy(struct wlf_renderer *render) {
	struct wlf_pixman_renderer *pixman_render =
		wlf_pixman_renderer_from_renderer(render);

	struct wlf_pixman_render_buffer *buffer, *buffer_tmp;
	wlf_linked_list_for_each_safe(buffer, buffer_tmp, &pixman_render->buffers, link) {
		wlf_pixman_render_buffer_destroy(buffer);
	}

	struct wlf_pixman_texture *tex, *tex_tmp;
	wlf_linked_list_for_each_safe(tex, tex_tmp, &pixman_render->textures, link) {
		wlf_texture_destroy(&tex->wlf_texture);
	}

	if (pixman_render == NULL) {
		return;
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
	wlf_buffer_end_data_ptr_access(wlf_buffer);

	struct wlf_pixman_texture *texture = wlf_pixman_texture_create(renderer,
		drm_format, wlf_buffer->width, wlf_buffer->height);
	if (texture == NULL) {
		return NULL;
	}

	texture->image = pixman_image_create_bits_no_clear(texture->format,
		wlf_buffer->width, wlf_buffer->height, data, stride);
	if (!texture->image) {
		wlf_log(WLF_ERROR, "Failed to create pixman image");
		wlf_linked_list_remove(&texture->link);
		free(texture);
		return NULL;
	}

	texture->buffer = wlf_buffer_lock(wlf_buffer);

	return &texture->wlf_texture;
}

static const struct wlf_renderer_impl pixman_renderer_impl = {
	.destroy = pixman_renderer_destroy,
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
	renderer->base.type = CPU;
	renderer->backend = backend;

	return &renderer->base;
}
