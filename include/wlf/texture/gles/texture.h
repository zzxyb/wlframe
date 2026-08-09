#ifndef TEXTURE_GLES_TEXTURE_H
#define TEXTURE_GLES_TEXTURE_H

#include "wlf/texture/wlf_texture.h"
#include "wlf/utils/wlf_linked_list.h"

#include <GLES2/gl2.h>

struct wlf_gles_renderer;

struct wlf_gles_texture {
	struct wlf_texture base;
	struct wlf_gles_renderer *renderer;
	struct wlf_linked_list link;
	GLuint tex;
};

bool wlf_texture_is_gles(const struct wlf_texture *texture);
struct wlf_gles_texture *wlf_gles_texture_from_texture(
	struct wlf_texture *texture);
struct wlf_texture *wlf_gles_texture_from_buffer(
	struct wlf_gles_renderer *renderer, struct wlf_buffer *buffer);

#endif // TEXTURE_GLES_TEXTURE_H
