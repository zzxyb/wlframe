#include "wlf/texture/gles/texture.h"

#include "wlf/renderer/gles/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static void texture_destroy(struct wlf_texture *base) {
	struct wlf_gles_texture *texture = wlf_gles_texture_from_texture(base);
	wlf_linked_list_remove(&texture->link);
	if (texture->tex != 0) {
		glDeleteTextures(1, &texture->tex);
	}
	free(texture);
}

static const struct wlf_texture_impl texture_impl = {
	.destroy = texture_destroy,
};

struct wlf_texture *wlf_gles_texture_from_buffer(
		struct wlf_gles_renderer *renderer, struct wlf_buffer *buffer) {
	void *data = NULL;
	uint32_t format = WLF_FORMAT_INVALID;
	size_t stride = 0;
	if (!wlf_buffer_begin_data_ptr_access(buffer,
			WLF_BUFFER_DATA_PTR_ACCESS_READ, &data, &format, &stride)) {
		return NULL;
	}

	const struct wlf_gles_pixel_format *gles_format =
		wlf_gles_pixel_format_from_wlf(format);
	const struct wlf_pixel_format_info *format_info =
		wlf_get_pixel_format_info(format);
	int32_t packed_stride = format_info != NULL ?
		pixel_format_info_min_stride(format_info, buffer->width) : 0;
	if (gles_format == NULL ||
			!wlf_gles_pixel_format_is_supported(renderer, gles_format) ||
			packed_stride <= 0 || stride != (size_t)packed_stride) {
		wlf_log(WLF_ERROR, "unsupported GLES texture format or row stride");
		wlf_buffer_end_data_ptr_access(buffer);
		return NULL;
	}

	struct wlf_gles_texture *texture = calloc(1, sizeof(*texture));
	if (texture == NULL) {
		wlf_buffer_end_data_ptr_access(buffer);
		return NULL;
	}
	wlf_texture_init(&texture->base, &renderer->base, &texture_impl,
		buffer->width, buffer->height);
	texture->renderer = renderer;

	glGenTextures(1, &texture->tex);
	glBindTexture(GL_TEXTURE_2D, texture->tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLint internal_format = gles_format->gl_internalformat != 0 ?
		gles_format->gl_internalformat : (GLint)gles_format->gl_format;
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
		buffer->width, buffer->height, 0,
		gles_format->gl_format, gles_format->gl_type, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	wlf_buffer_end_data_ptr_access(buffer);

	GLenum error = glGetError();
	if (texture->tex == 0 || error != GL_NO_ERROR) {
		wlf_log(WLF_ERROR, "failed to upload GLES texture: %s",
			wlf_gles_error_str(error));
		if (texture->tex != 0) {
			glDeleteTextures(1, &texture->tex);
		}
		free(texture);
		return NULL;
	}

	wlf_linked_list_insert(&renderer->textures, &texture->link);
	return &texture->base;
}

bool wlf_texture_is_gles(const struct wlf_texture *texture) {
	return texture != NULL && texture->impl == &texture_impl;
}

struct wlf_gles_texture *wlf_gles_texture_from_texture(
		struct wlf_texture *texture) {
	assert(wlf_texture_is_gles(texture));
	struct wlf_gles_texture *gles_texture =
		wlf_container_of(texture, gles_texture, base);
	return gles_texture;
}
