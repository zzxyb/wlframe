#include "wlf/pass/wlf_rect_shape_pass.h"
#include "wlf_shape_geometry.h"

#include <stdlib.h>

#define CORNER_SEGMENTS 8

struct wlf_rect_shape_pass { struct wlf_vector_pass *vector; };

struct wlf_rect_shape_pass *wlf_rect_shape_pass_create(
		struct wlf_vector_pass *vector_pass) {
	if (vector_pass == NULL) return NULL;
	struct wlf_rect_shape_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_vector_pass_destroy(vector_pass);
		return NULL;
	}
	pass->vector = vector_pass;
	return pass;
}

void wlf_render_rect_shape_pass_destroy(struct wlf_rect_shape_pass *pass) {
	if (pass == NULL) return;
	wlf_vector_pass_destroy(pass->vector);
	free(pass);
}

void wlf_render_pass_add_rect_shape(struct wlf_rect_shape_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_render_rect_shape_options *options) {
	if (pass == NULL || target == NULL || options == NULL || options->shape == NULL) return;
	const struct wlf_rect_shape *shape = options->shape;
	const struct wlf_shape_state *state = &shape->state;
	float points[4 * (CORNER_SEGMENTS + 1) * 2];
	int count = wlf_shape_rounded_rect_points(shape, points, CORNER_SEGMENTS);
	struct wlf_shape_vertices vertices = {0};
	if (state->has_fill) {
		wlf_shape_add_polygon_fill(&vertices, points, count,
			options->offset_x, options->offset_y);
		wlf_shape_add_polygon_fringe(&vertices, points, count,
			options->offset_x, options->offset_y, 1);
		wlf_shape_submit(pass->vector, target, &vertices, state->fill_color,
			wlf_shape_state_fill_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	vertices.len = 0;
	if (state->has_stroke && state->stroke_width > 0) {
		wlf_shape_add_polygon_stroke(&vertices, points, count, true,
			state->stroke_width, options->offset_x, options->offset_y);
		wlf_shape_submit(pass->vector, target, &vertices, state->stroke_color,
			wlf_shape_state_stroke_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	wlf_shape_vertices_finish(&vertices);
}
