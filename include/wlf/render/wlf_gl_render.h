#ifndef RENDER_WLF_GL_RENDER_H
#define RENDER_WLF_GL_RENDER_H

#include "wlf/render/wlf_render.h"

#include <stdbool.h>

struct wlf_egl;
struct wlf_backend;

struct wlf_gl_render {
	struct wlf_render base; /**< Base render structure */

	struct wlf_egl *egl;    /**< EGL context for OpenGL rendering */
};

struct wlf_render *wlf_gl_render_create(struct wlf_backend *backend);
bool wlf_gl_render_check_ext(struct wlf_render *render, const char *ext);
struct wlr_egl *wlr_gl_render_get_egl(struct wlf_render *render);
// GLuint wlr_gl_renderer_get_buffer_fbo(struct wlf_render *render, struct wlf_buffer *buffer);

#endif // RENDER_WLF_GL_RENDER_H
