#include "wlf/pass/wlf_ellipse_pass.h"
#include "wlf_shape_geometry.h"

#include <math.h>
#include <stdlib.h>

#define CURVE_SEGMENTS 48
#define WLF_PI 3.14159265358979323846

struct wlf_ellipse_pass { struct wlf_vector_pass *vector; };

struct wlf_ellipse_pass *wlf_ellipse_pass_create(struct wlf_vector_pass *vector_pass) {
	if (vector_pass == NULL) return NULL;
	struct wlf_ellipse_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_render_vector_pass_destroy(vector_pass);
		return NULL;
	}
	pass->vector = vector_pass;
	return pass;
}

void wlf_render_ellipse_pass_destroy(struct wlf_ellipse_pass *pass) {
	if (pass == NULL) return;
	wlf_render_vector_pass_destroy(pass->vector);
	free(pass);
}

void wlf_render_pass_add_ellipse(struct wlf_ellipse_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_render_ellipse_options *options) {
	if (pass == NULL || target == NULL || options == NULL || options->shape == NULL) return;
	const struct wlf_ellipse_shape *s = options->shape;
	const struct wlf_shape_state *state = &s->state;
	struct wlf_shape_vertices vertices = {0};
	if (state->has_fill) {
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			wlf_shape_add_triangle(&vertices,
				s->cx + options->offset_x, s->cy + options->offset_y,
				s->cx + cos(a) * s->rx + options->offset_x,
				s->cy + sin(a) * s->ry + options->offset_y,
				s->cx + cos(b) * s->rx + options->offset_x,
				s->cy + sin(b) * s->ry + options->offset_y);
			wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(a) * s->rx + options->offset_x,
				s->cy + sin(a) * s->ry + options->offset_y, 1,
				s->cx + cos(b) * s->rx + options->offset_x,
				s->cy + sin(b) * s->ry + options->offset_y, 1,
				s->cx + cos(b) * (s->rx + 1) + options->offset_x,
				s->cy + sin(b) * (s->ry + 1) + options->offset_y, 0,
				s->cx + cos(a) * (s->rx + 1) + options->offset_x,
				s->cy + sin(a) * (s->ry + 1) + options->offset_y, 0);
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->fill_color,
			wlf_shape_state_fill_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	vertices.len = 0;
	if (state->has_stroke && state->stroke_width > 0) {
		double orx = s->rx + state->stroke_width / 2;
		double ory = s->ry + state->stroke_width / 2;
		double irx = fmax(0, s->rx - state->stroke_width / 2);
		double iry = fmax(0, s->ry - state->stroke_width / 2);
		for (int i = 0; i < CURVE_SEGMENTS; i++) {
			double a = i * 2 * WLF_PI / CURVE_SEGMENTS;
			double b = (i + 1) * 2 * WLF_PI / CURVE_SEGMENTS;
			wlf_shape_add_quad(&vertices,
				s->cx + cos(a) * orx + options->offset_x,
				s->cy + sin(a) * ory + options->offset_y,
				s->cx + cos(b) * orx + options->offset_x,
				s->cy + sin(b) * ory + options->offset_y,
				s->cx + cos(b) * irx + options->offset_x,
				s->cy + sin(b) * iry + options->offset_y,
				s->cx + cos(a) * irx + options->offset_x,
				s->cy + sin(a) * iry + options->offset_y);
			wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(a) * orx + options->offset_x,
				s->cy + sin(a) * ory + options->offset_y, 1,
				s->cx + cos(b) * orx + options->offset_x,
				s->cy + sin(b) * ory + options->offset_y, 1,
				s->cx + cos(b) * (orx + 1) + options->offset_x,
				s->cy + sin(b) * (ory + 1) + options->offset_y, 0,
				s->cx + cos(a) * (orx + 1) + options->offset_x,
				s->cy + sin(a) * (ory + 1) + options->offset_y, 0);
			if (irx > 0 && iry > 0) wlf_shape_add_quad_coverage(&vertices,
				s->cx + cos(b) * irx + options->offset_x,
				s->cy + sin(b) * iry + options->offset_y, 1,
				s->cx + cos(a) * irx + options->offset_x,
				s->cy + sin(a) * iry + options->offset_y, 1,
				s->cx + cos(a) * fmax(0, irx - 1) + options->offset_x,
				s->cy + sin(a) * fmax(0, iry - 1) + options->offset_y, 0,
				s->cx + cos(b) * fmax(0, irx - 1) + options->offset_x,
				s->cy + sin(b) * fmax(0, iry - 1) + options->offset_y, 0);
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->stroke_color,
			wlf_shape_state_stroke_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	wlf_shape_vertices_finish(&vertices);
}
