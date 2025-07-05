#ifndef BUFFER_WLF_GL_BUFFER_H
#define BUFFER_WLF_GL_BUFFER_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

struct wlf_gl_renderer;
struct wlf_buffer;

struct wlf_gl_buffer {
	struct wlf_buffer *buffer;
	struct wlf_gl_renderer *renderer;
	bool external_only;

	EGLImageKHR image;
	GLuint rbo;
	GLuint fbo;
	GLuint tex;
};

#endif
