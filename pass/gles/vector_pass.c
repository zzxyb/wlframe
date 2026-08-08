#include "wlf/pass/gles/vector_pass.h"

#include "wlf/pass/gles/render_target_info.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/utils/wlf_log.h"

#include "vector_frag_src.h"
#include "vector_vert_src.h"

#include <GLES2/gl2.h>
#include <limits.h>
#include <stdlib.h>

struct wlf_gles_vector_pass {
	struct wlf_vector_pass base;
	GLuint program;
	GLint attrib_pos;
	GLint attrib_coverage;
	GLint uniform_viewport;
	GLint uniform_color;
};

static GLuint compile_shader(GLenum type, const char *source) {
	GLuint shader = glCreateShader(type);
	if (shader == 0) {
		return 0;
	}
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static bool link_program(struct wlf_gles_vector_pass *pass) {
	GLuint vert = compile_shader(GL_VERTEX_SHADER, vector_vert_src);
	GLuint frag = compile_shader(GL_FRAGMENT_SHADER, vector_frag_src);
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
	glBindAttribLocation(pass->program, 1, "coverage");
	glLinkProgram(pass->program);
	glDeleteShader(vert);
	glDeleteShader(frag);
	GLint ok = GL_FALSE;
	glGetProgramiv(pass->program, GL_LINK_STATUS, &ok);
	if (!ok) {
		glDeleteProgram(pass->program);
		pass->program = 0;
		return false;
	}
	pass->attrib_pos = glGetAttribLocation(pass->program, "pos");
	pass->attrib_coverage = glGetAttribLocation(pass->program, "coverage");
	pass->uniform_viewport = glGetUniformLocation(pass->program, "viewport");
	pass->uniform_color = glGetUniformLocation(pass->program, "color");
	if (pass->attrib_pos < 0 || pass->attrib_coverage < 0 ||
			pass->uniform_viewport < 0 ||
			pass->uniform_color < 0) {
		glDeleteProgram(pass->program);
		pass->program = 0;
		return false;
	}
	return true;
}

static void vector_pass_destroy(struct wlf_vector_pass *base) {
	struct wlf_gles_vector_pass *pass = wlf_container_of(base, pass, base);
	if (pass->program != 0) {
		glDeleteProgram(pass->program);
	}
	free(pass);
}

static void vector_pass_render(struct wlf_vector_pass *base,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_vector_options *options) {
	struct wlf_gles_vector_pass *pass = wlf_container_of(base, pass, base);
	if (!wlf_render_target_info_is_gles(render_target_info)) {
		wlf_log(WLF_ERROR, "GLES vector pass requires a GLES target");
		return;
	}
	struct wlf_gles_render_target_info *target =
		wlf_gles_render_target_info_from_info(render_target_info);
	int width = target->swapchain->base.width;
	int height = target->swapchain->base.height;
	if (width <= 0 || height <= 0) {
		return;
	}
	if (options->vertex_count > INT_MAX) {
		wlf_log(WLF_ERROR, "Too many vertices for GLES vector pass");
		return;
	}
	GLsizei vertex_count = (GLsizei)options->vertex_count;

	struct wlf_color color = wlf_color_clamp(&options->color);
	float rgba[4] = {
		color.r * color.a, color.g * color.a,
		color.b * color.a, color.a,
	};
	glViewport(0, 0, width, height);
	glUseProgram(pass->program);
	glUniform2f(pass->uniform_viewport, width, height);
	glUniform4fv(pass->uniform_color, 1, rgba);
	glVertexAttribPointer(pass->attrib_pos, 2, GL_FLOAT, GL_FALSE,
		sizeof(struct wlf_vector_vertex), options->vertices);
	glVertexAttribPointer(pass->attrib_coverage, 1, GL_FLOAT, GL_FALSE,
		sizeof(struct wlf_vector_vertex), &options->vertices[0].coverage);
	glEnableVertexAttribArray(pass->attrib_pos);
	glEnableVertexAttribArray(pass->attrib_coverage);
	if (options->blend_mode == WLF_RENDER_BLEND_MODE_NONE) {
		glDisable(GL_BLEND);
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	if (options->clip != NULL) {
		int nrects = 0;
		pixman_box32_t *rects = pixman_region32_rectangles(
			(pixman_region32_t *)options->clip, &nrects);
		glEnable(GL_SCISSOR_TEST);
		for (int i = 0; i < nrects; i++) {
			pixman_box32_t *r = &rects[i];
			glScissor(r->x1, height - r->y2, r->x2 - r->x1, r->y2 - r->y1);
			glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		}
		glDisable(GL_SCISSOR_TEST);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
	}
	glDisableVertexAttribArray(pass->attrib_pos);
	glDisableVertexAttribArray(pass->attrib_coverage);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		wlf_log(WLF_ERROR, "GLES vector render failed: %s",
			wlf_gles_error_str(error));
	}
}

static const struct wlf_vector_pass_impl vector_pass_impl = {
	.destroy = vector_pass_destroy,
	.render = vector_pass_render,
};

struct wlf_vector_pass *wlf_gles_vector_pass_create(void) {
	struct wlf_gles_vector_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL || !link_program(pass)) {
		free(pass);
		return NULL;
	}
	wlf_render_vector_pass_init(&pass->base, &vector_pass_impl);
	return &pass->base;
}
