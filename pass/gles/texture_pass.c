#include "wlf/pass/gles/texture_pass.h"

#include "wlf/pass/gles/render_target_info.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/texture/gles/texture.h"
#include "wlf/utils/wlf_log.h"

#include "texture_frag_src.h"
#include "texture_vert_src.h"

#include <GLES2/gl2.h>
#include <math.h>
#include <stdlib.h>

struct wlf_gles_texture_pass {
	struct wlf_texture_pass base;
	GLuint program;
	GLint attrib_pos;
	GLint attrib_texcoord;
	GLint uniform_viewport;
	GLint uniform_texture;
	GLint uniform_opacity;
};

static GLuint compile_shader(GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	if (shader == 0) {
		wlf_log(WLF_ERROR, "glCreateShader failed: %s",
			wlf_gles_error_str(glGetError()));
		return 0;
	}
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok != GL_TRUE) {
		char log[512] = {0};
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		wlf_log(WLF_ERROR, "failed to compile GLES texture shader: %s", log);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static bool link_program(struct wlf_gles_texture_pass *pass) {
	GLuint vert = compile_shader(GL_VERTEX_SHADER, texture_vert_src);
	GLuint frag = compile_shader(GL_FRAGMENT_SHADER, texture_frag_src);
	if (vert == 0 || frag == 0) {
		glDeleteShader(vert);
		glDeleteShader(frag);
		return false;
	}

	pass->program = glCreateProgram();
	if (pass->program == 0) {
		glDeleteShader(vert);
		glDeleteShader(frag);
		return false;
	}
	glAttachShader(pass->program, vert);
	glAttachShader(pass->program, frag);
	glBindAttribLocation(pass->program, 0, "pos");
	glBindAttribLocation(pass->program, 1, "texcoord");
	glLinkProgram(pass->program);
	glDeleteShader(vert);
	glDeleteShader(frag);
	GLint ok = GL_FALSE;
	glGetProgramiv(pass->program, GL_LINK_STATUS, &ok);
	if (ok != GL_TRUE) {
		char log[512] = {0};
		glGetProgramInfoLog(pass->program, sizeof(log), NULL, log);
		wlf_log(WLF_ERROR, "failed to link GLES texture program: %s", log);
		glDeleteProgram(pass->program);
		pass->program = 0;
		return false;
	}

	pass->attrib_pos = glGetAttribLocation(pass->program, "pos");
	pass->attrib_texcoord = glGetAttribLocation(pass->program, "texcoord");
	pass->uniform_viewport = glGetUniformLocation(pass->program, "viewport");
	pass->uniform_texture = glGetUniformLocation(pass->program, "tex");
	pass->uniform_opacity = glGetUniformLocation(pass->program, "opacity");
	bool locations_ok = pass->attrib_pos >= 0 && pass->attrib_texcoord >= 0 &&
		pass->uniform_viewport >= 0 && pass->uniform_texture >= 0 &&
		pass->uniform_opacity >= 0;
	if (!locations_ok) {
		wlf_log(WLF_ERROR, "failed to query GLES texture program locations");
		glDeleteProgram(pass->program);
		pass->program = 0;
	}
	return locations_ok;
}

static void texture_pass_destroy(struct wlf_texture_pass *base) {
	struct wlf_gles_texture_pass *pass =
		wlf_container_of(base, pass, base);
	if (pass->program != 0) {
		glDeleteProgram(pass->program);
	}
	free(pass);
}

static void texture_pass_render(struct wlf_texture_pass *base,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_texture_options *options) {
	struct wlf_gles_texture_pass *pass =
		wlf_container_of(base, pass, base);
	if (!wlf_render_target_info_is_gles(render_target_info) ||
			!wlf_texture_is_gles(options->texture)) {
		wlf_log(WLF_ERROR, "GLES texture pass requires GLES target and texture");
		return;
	}

	struct wlf_gles_render_target_info *target =
		wlf_gles_render_target_info_from_info(render_target_info);
	struct wlf_gles_texture *texture =
		wlf_gles_texture_from_texture(options->texture);
	int target_width = target->swapchain->base.width;
	int target_height = target->swapchain->base.height;
	if (target_width <= 0 || target_height <= 0 || options->opacity <= 0.0f) {
		return;
	}

	struct wlf_frect src, dst;
	wlf_render_texture_options_get_src_box(options, &src);
	wlf_render_texture_options_get_dst_box(options, &dst);
	if (src.width <= 0 || src.height <= 0 || dst.width <= 0 || dst.height <= 0) {
		return;
	}

	float vertices[8] = {
		dst.x, dst.y,
		dst.x + dst.width, dst.y,
		dst.x, dst.y + dst.height,
		dst.x + dst.width, dst.y + dst.height,
	};
	float texcoords[8] = {
		src.x / texture->base.width, src.y / texture->base.height,
		(src.x + src.width) / texture->base.width, src.y / texture->base.height,
		src.x / texture->base.width, (src.y + src.height) / texture->base.height,
		(src.x + src.width) / texture->base.width,
			(src.y + src.height) / texture->base.height,
	};

	glViewport(0, 0, target_width, target_height);
	glUseProgram(pass->program);
	glUniform2f(pass->uniform_viewport, target_width, target_height);
	glUniform1i(pass->uniform_texture, 0);
	glUniform1f(pass->uniform_opacity, options->opacity);
	glVertexAttribPointer(pass->attrib_pos, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(pass->attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
	glEnableVertexAttribArray(pass->attrib_pos);
	glEnableVertexAttribArray(pass->attrib_texcoord);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		options->filter_mode == WLF_SCALE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		options->filter_mode == WLF_SCALE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR);

	if (options->blend_mode == WLF_RENDER_BLEND_MODE_NONE) {
		glDisable(GL_BLEND);
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	pixman_region32_t dst_region, clipped;
	int x = (int)floor(dst.x);
	int y = (int)floor(dst.y);
	int width = (int)ceil(dst.x + dst.width) - x;
	int height = (int)ceil(dst.y + dst.height) - y;
	pixman_region32_init_rect(&dst_region, x, y, width, height);
	pixman_region32_init(&clipped);
	if (options->clip != NULL) {
		pixman_region32_intersect(&clipped, &dst_region, options->clip);
	} else {
		pixman_region32_copy(&clipped, &dst_region);
	}

	int nrects = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &nrects);
	glEnable(GL_SCISSOR_TEST);
	for (int i = 0; i < nrects; i++) {
		pixman_box32_t *r = &rects[i];
		glScissor(r->x1, target_height - r->y2,
			r->x2 - r->x1, r->y2 - r->y1);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glDisable(GL_SCISSOR_TEST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(pass->attrib_texcoord);
	glDisableVertexAttribArray(pass->attrib_pos);
	pixman_region32_fini(&clipped);
	pixman_region32_fini(&dst_region);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		wlf_log(WLF_ERROR, "GLES texture render failed: %s",
			wlf_gles_error_str(error));
	}
}

static const struct wlf_texture_pass_impl texture_pass_impl = {
	.destroy = texture_pass_destroy,
	.render = texture_pass_render,
};

struct wlf_texture_pass *wlf_gles_texture_pass_create(void) {
	struct wlf_gles_texture_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL || !link_program(pass)) {
		free(pass);
		return NULL;
	}
	wlf_render_texture_pass_init(&pass->base, &texture_pass_impl);
	return &pass->base;
}
