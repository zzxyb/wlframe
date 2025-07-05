#include "wlf/blit/wlf_blit.h"
#include "wlf/blit/wlf_gl_blit.h"
#include "wlf/framebuffer/wlf_gl_framebuffer.h"
#include "wlf/texture/wlf_gl_texture.h"

#include <GLES3/gl3.h>

static bool gl_blit_framebuffer_to_framebuffer(struct wlf_render_context* context,
												struct wlf_framebuffer* src,
												struct wlf_framebuffer* dst,
												struct wlf_rect src_rect,
												struct wlf_rect dst_rect,
												enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	struct wlf_gl_framebuffer* gl_src = (struct wlf_gl_framebuffer*)src;
	struct wlf_gl_framebuffer* gl_dst = (struct wlf_gl_framebuffer*)dst;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_src->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_dst->fbo);

	GLenum gl_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? GL_LINEAR : GL_NEAREST;

	glBlitFramebuffer(
		src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
		dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
		GL_COLOR_BUFFER_BIT, gl_filter
	);

	if (src->depth_attachment && dst->depth_attachment) {
		glBlitFramebuffer(
			src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
			dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST
		);
	}

	if (src->stencil_attachment && dst->stencil_attachment) {
		glBlitFramebuffer(
			src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
			dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
			GL_STENCIL_BUFFER_BIT, GL_NEAREST
		);
	}

	return glGetError() == GL_NO_ERROR;
}

static bool gl_blit_texture_to_framebuffer(struct wlf_render_context* context,
											struct wlf_texture* src,
											struct wlf_framebuffer* dst,
											struct wlf_rect src_rect,
											struct wlf_rect dst_rect,
											enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	GLuint temp_fbo;
	glGenFramebuffers(1, &temp_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, temp_fbo);

	GLuint src_texture_id = wlf_texture_get_gl_id(src);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_texture_id, 0);

	struct wlf_gl_framebuffer* gl_dst = (struct wlf_gl_framebuffer*)dst;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl_dst->fbo);

	GLenum gl_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? GL_LINEAR : GL_NEAREST;

	glBlitFramebuffer(
		src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
		dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
		GL_COLOR_BUFFER_BIT, gl_filter
	);

	glDeleteFramebuffers(1, &temp_fbo);

	return glGetError() == GL_NO_ERROR;
}

static bool gl_blit_framebuffer_to_texture(struct wlf_render_context* context,
											struct wlf_framebuffer* src,
											struct wlf_texture* dst,
											struct wlf_rect src_rect,
											struct wlf_rect dst_rect,
											enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	struct wlf_gl_framebuffer* gl_src = (struct wlf_gl_framebuffer*)src;

	GLuint temp_fbo;
	glGenFramebuffers(1, &temp_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, temp_fbo);

	GLuint dst_texture_id = wlf_texture_get_gl_id(dst);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_texture_id, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gl_src->fbo);

	GLenum gl_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? GL_LINEAR : GL_NEAREST;

	glBlitFramebuffer(
		src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
		dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
		GL_COLOR_BUFFER_BIT, gl_filter
	);

	glDeleteFramebuffers(1, &temp_fbo);

	return glGetError() == GL_NO_ERROR;
}

static bool gl_blit_texture_to_texture(struct wlf_render_context* context,
										struct wlf_texture* src,
										struct wlf_texture* dst,
										struct wlf_rect src_rect,
										struct wlf_rect dst_rect,
										enum wlf_blit_filter filter) {
	if (!src || !dst) return false;

	GLuint temp_src_fbo, temp_dst_fbo;
	glGenFramebuffers(1, &temp_src_fbo);
	glGenFramebuffers(1, &temp_dst_fbo);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, temp_src_fbo);
	GLuint src_texture_id = wlf_texture_get_gl_id(src);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_texture_id, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, temp_dst_fbo);
	GLuint dst_texture_id = wlf_texture_get_gl_id(dst);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_texture_id, 0);

	GLenum gl_filter = (filter == WLF_BLIT_FILTER_LINEAR) ? GL_LINEAR : GL_NEAREST;

	glBlitFramebuffer(
		src_rect.x, src_rect.y, src_rect.x + src_rect.width, src_rect.y + src_rect.height,
		dst_rect.x, dst_rect.y, dst_rect.x + dst_rect.width, dst_rect.y + dst_rect.height,
		GL_COLOR_BUFFER_BIT, gl_filter
	);

	glDeleteFramebuffers(1, &temp_src_fbo);
	glDeleteFramebuffers(1, &temp_dst_fbo);

	return glGetError() == GL_NO_ERROR;
}

static void gl_blit_sync(struct wlf_render_context* context) {
	glFinish();
}

static const struct wlf_blit_impl gl_blit_vtable = {
	.framebuffer_to_framebuffer = gl_blit_framebuffer_to_framebuffer,
	.texture_to_framebuffer = gl_blit_texture_to_framebuffer,
	.framebuffer_to_texture = gl_blit_framebuffer_to_texture,
	.texture_to_texture = gl_blit_texture_to_texture,
	.sync = gl_blit_sync,
};

const struct wlf_blit_impl* wlf_gl_blit_get_vtable(void) {
	return &gl_blit_vtable;
}
