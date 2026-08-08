#include "wlf/pass/gles/rect_pass.h"

#include "wlf/pass/gles/render_target_info.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/utils/wlf_log.h"

#include <GLES2/gl2.h>
#include <math.h>
#include <stdlib.h>

struct wlf_gles_rect_pass {
	struct wlf_rect_pass base;
	GLuint program;
	GLint attrib_pos;
	GLint uniform_viewport;
	GLint uniform_color;
};

static const char vertex_shader_src[] =
	"attribute vec2 pos;\n"
	"uniform vec2 viewport;\n"
	"void main() {\n"
	"	vec2 ndc = pos / viewport * 2.0 - 1.0;\n"
	"	gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);\n"
	"}\n";

static const char fragment_shader_src[] =
	"precision mediump float;\n"
	"uniform vec4 color;\n"
	"void main() {\n"
	"	gl_FragColor = color;\n"
	"}\n";

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
		wlf_log(WLF_ERROR, "failed to compile GLES shader: %s", log);
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

static bool link_program(struct wlf_gles_rect_pass *pass) {
	GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
	if (vertex_shader == 0) {
		return false;
	}

	GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
	if (fragment_shader == 0) {
		glDeleteShader(vertex_shader);
		return false;
	}

	pass->program = glCreateProgram();
	if (pass->program == 0) {
		wlf_log(WLF_ERROR, "glCreateProgram failed: %s",
			wlf_gles_error_str(glGetError()));
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return false;
	}

	glAttachShader(pass->program, vertex_shader);
	glAttachShader(pass->program, fragment_shader);
	glBindAttribLocation(pass->program, 0, "pos");
	glLinkProgram(pass->program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLint ok = GL_FALSE;
	glGetProgramiv(pass->program, GL_LINK_STATUS, &ok);
	if (ok != GL_TRUE) {
		char log[512] = {0};
		glGetProgramInfoLog(pass->program, sizeof(log), NULL, log);
		wlf_log(WLF_ERROR, "failed to link GLES rect program: %s", log);
		glDeleteProgram(pass->program);
		pass->program = 0;
		return false;
	}

	pass->attrib_pos = glGetAttribLocation(pass->program, "pos");
	pass->uniform_viewport = glGetUniformLocation(pass->program, "viewport");
	pass->uniform_color = glGetUniformLocation(pass->program, "color");
	if (pass->attrib_pos < 0 || pass->uniform_viewport < 0 ||
			pass->uniform_color < 0) {
		wlf_log(WLF_ERROR, "failed to query GLES rect program locations");
		glDeleteProgram(pass->program);
		pass->program = 0;
		return false;
	}

	return true;
}

static void gles_rect_pass_destroy(struct wlf_rect_pass *base) {
	struct wlf_gles_rect_pass *pass =
		wlf_container_of(base, pass, base);
	if (pass->program != 0) {
		glDeleteProgram(pass->program);
	}
	free(pass);
}

static void gles_rect_pass_render(struct wlf_rect_pass *base,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_rect_options *options) {
	struct wlf_gles_rect_pass *pass =
		wlf_container_of(base, pass, base);

	if (!wlf_render_target_info_is_gles(render_target_info)) {
		wlf_log(WLF_ERROR, "GLES rect pass requires a GLES render target");
		return;
	}

	struct wlf_gles_render_target_info *target =
		wlf_gles_render_target_info_from_info(render_target_info);
	int target_width = target->swapchain->base.width;
	int target_height = target->swapchain->base.height;
	if (target_width <= 0 || target_height <= 0) {
		return;
	}

	int x = (int)floor(options->box.x);
	int y = (int)floor(options->box.y);
	int width = (int)ceil(options->box.x + options->box.width) - x;
	int height = (int)ceil(options->box.y + options->box.height) - y;
	if (width <= 0 || height <= 0) {
		return;
	}

	pixman_region32_t rect_region;
	pixman_region32_init_rect(&rect_region, x, y, (uint32_t)width, (uint32_t)height);

	pixman_region32_t clipped_region;
	pixman_region32_init(&clipped_region);
	if (options->clip != NULL) {
		pixman_region32_intersect(&clipped_region, &rect_region, options->clip);
	} else {
		pixman_region32_copy(&clipped_region, &rect_region);
	}

	int nrects = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped_region, &nrects);
	if (nrects <= 0) {
		goto out;
	}

	struct wlf_color color = wlf_color_clamp(&options->color);
	float rgba[4] = {
		(float)(color.r * color.a),
		(float)(color.g * color.a),
		(float)(color.b * color.a),
		(float)color.a,
	};
	float vertices[8] = {
		(float)x, (float)y,
		(float)(x + width), (float)y,
		(float)x, (float)(y + height),
		(float)(x + width), (float)(y + height),
	};

	glViewport(0, 0, target_width, target_height);
	glUseProgram(pass->program);
	glUniform2f(pass->uniform_viewport, target_width, target_height);
	glUniform4fv(pass->uniform_color, 1, rgba);
	glVertexAttribPointer((GLuint)pass->attrib_pos, 2, GL_FLOAT,
		GL_FALSE, 0, vertices);
	glEnableVertexAttribArray((GLuint)pass->attrib_pos);

	if (options->blend_mode == WLF_RENDER_BLEND_MODE_NONE) {
		glDisable(GL_BLEND);
	} else {
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	glEnable(GL_SCISSOR_TEST);
	for (int i = 0; i < nrects; i++) {
		const pixman_box32_t *r = &rects[i];
		int scissor_width = r->x2 - r->x1;
		int scissor_height = r->y2 - r->y1;
		glScissor(r->x1, target_height - r->y2,
			scissor_width, scissor_height);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glDisable(GL_SCISSOR_TEST);
	glDisableVertexAttribArray((GLuint)pass->attrib_pos);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		wlf_log(WLF_ERROR, "GLES rect render failed: %s",
			wlf_gles_error_str(error));
	}

out:
	pixman_region32_fini(&clipped_region);
	pixman_region32_fini(&rect_region);
}

static const struct wlf_rect_pass_impl gles_rect_pass_impl = {
	.destroy = gles_rect_pass_destroy,
	.render = gles_rect_pass_render,
};

struct wlf_rect_pass *wlf_gles_rect_pass_create(void) {
	struct wlf_gles_rect_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_gles_rect_pass");
		return NULL;
	}

	if (!link_program(pass)) {
		free(pass);
		return NULL;
	}

	wlf_render_rect_pass_init(&pass->base, &gles_rect_pass_impl);
	return &pass->base;
}
