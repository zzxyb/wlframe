#ifndef TEXTURE_WLF_GL_TEXTURE_H
#define TEXTURE_WLF_GL_TEXTURE_H

#include "wlf/texture/wlf_texture.h"

#include <GLES2/gl2.h>

struct wlf_gl_renderer;

struct wlf_gl_texture {
	struct wlf_texture base;
	struct wlf_gl_renderer *renderer;

	GLenum target;
	GLuint tex;
	GLuint fbo;

	bool has_alpha;
};

// ===== Utility Functions =====

/**
 * @brief Get OpenGL texture ID from a generic texture
 * @param texture Generic texture pointer
 * @return OpenGL texture ID, or 0 if not a GL texture
 */
GLuint wlf_texture_get_gl_id(struct wlf_texture* texture);

#endif // TEXTURE_WLF_GL_TEXTURE_H
