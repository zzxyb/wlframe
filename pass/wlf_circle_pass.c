#include "wlf/pass/wlf_circle_pass.h"
#include "wlf_shape_geometry.h"

#include <math.h>
#include <stdlib.h>

#define CURVE_SEGMENTS 48
#define WLF_PI 3.14159265358979323846

struct wlf_circle_pass { struct wlf_vector_pass *vector; };

struct wlf_circle_pass *wlf_circle_pass_create(struct wlf_vector_pass *vector_pass) {
	if (vector_pass == NULL) return NULL;
	struct wlf_circle_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_vector_pass_destroy(vector_pass);
		return NULL;
	}
	pass->vector = vector_pass;
	return pass;
}

void wlf_render_circle_pass_destroy(struct wlf_circle_pass *pass) {
	if (pass == NULL) return;
	wlf_vector_pass_destroy(pass->vector);
	free(pass);
}

void wlf_render_pass_add_circle(struct wlf_circle_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_render_circle_options *options) {
	if (pass == NULL || target == NULL || options == NULL || options->shape == NULL) return;
	const struct wlf_circle_shape *s = options->shape;
	const struct wlf_shape_state *state = &s->state;
	struct wlf_shape_vertices vertices = {0};
	if (state->has_fill) {
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			wlf_shape_add_triangle(&vertices,
				s->cx + options->offset_x, s->cy + options->offset_y,
				s->cx + cos(a) * s->r + options->offset_x,
				s->cy + sin(a) * s->r + options->offset_y,
				s->cx + cos(b) * s->r + options->offset_x,
				s->cy + sin(b) * s->r + options->offset_y);
			wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(a) * s->r + options->offset_x,
				s->cy + sin(a) * s->r + options->offset_y, 1,
				s->cx + cos(b) * s->r + options->offset_x,
				s->cy + sin(b) * s->r + options->offset_y, 1,
				s->cx + cos(b) * (s->r + 1) + options->offset_x,
				s->cy + sin(b) * (s->r + 1) + options->offset_y, 0,
				s->cx + cos(a) * (s->r + 1) + options->offset_x,
				s->cy + sin(a) * (s->r + 1) + options->offset_y, 0);
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->fill_color,
			wlf_shape_state_fill_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	vertices.len = 0;
	if (state->has_stroke && state->stroke_width > 0) {
		double outer = s->r + state->stroke_width / 2;
		double inner = fmax(0, s->r - state->stroke_width / 2);
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			wlf_shape_add_quad(&vertices,
				s->cx + cos(a) * outer + options->offset_x,
				s->cy + sin(a) * outer + options->offset_y,
				s->cx + cos(b) * outer + options->offset_x,
				s->cy + sin(b) * outer + options->offset_y,
				s->cx + cos(b) * inner + options->offset_x,
				s->cy + sin(b) * inner + options->offset_y,
				s->cx + cos(a) * inner + options->offset_x,
				s->cy + sin(a) * inner + options->offset_y);
			wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(a) * outer + options->offset_x,
				s->cy + sin(a) * outer + options->offset_y, 1,
				s->cx + cos(b) * outer + options->offset_x,
				s->cy + sin(b) * outer + options->offset_y, 1,
				s->cx + cos(b) * (outer + 1) + options->offset_x,
				s->cy + sin(b) * (outer + 1) + options->offset_y, 0,
				s->cx + cos(a) * (outer + 1) + options->offset_x,
				s->cy + sin(a) * (outer + 1) + options->offset_y, 0);
			if (inner > 0) wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(b) * inner + options->offset_x,
				s->cy + sin(b) * inner + options->offset_y, 1,
				s->cx + cos(a) * inner + options->offset_x,
				s->cy + sin(a) * inner + options->offset_y, 1,
				s->cx + cos(a) * fmax(0, inner - 1) + options->offset_x,
				s->cy + sin(a) * fmax(0, inner - 1) + options->offset_y, 0,
				s->cx + cos(b) * fmax(0, inner - 1) + options->offset_x,
				s->cy + sin(b) * fmax(0, inner - 1) + options->offset_y, 0);
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->stroke_color,
			wlf_shape_state_stroke_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	wlf_shape_vertices_finish(&vertices);
}
