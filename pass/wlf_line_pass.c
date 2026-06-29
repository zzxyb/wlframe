#include "wlf/pass/wlf_line_pass.h"
#include "wlf_shape_geometry.h"

#include <stdlib.h>

struct wlf_line_pass { struct wlf_vector_pass *vector; };

struct wlf_line_pass *wlf_line_pass_create(struct wlf_vector_pass *vector_pass) {
	if (vector_pass == NULL) return NULL;
	struct wlf_line_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_render_vector_pass_destroy(vector_pass);
		return NULL;
	}
	pass->vector = vector_pass;
	return pass;
}

void wlf_render_line_pass_destroy(struct wlf_line_pass *pass) {
	if (pass == NULL) return;
	wlf_render_vector_pass_destroy(pass->vector);
	free(pass);
}

void wlf_render_pass_add_line(struct wlf_line_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_render_line_options *options) {
	if (pass == NULL || target == NULL || options == NULL || options->shape == NULL) return;
	const struct wlf_line_shape *shape = options->shape;
	const struct wlf_shape_state *state = &shape->state;
	if (!state->has_stroke || state->stroke_width <= 0) return;
	struct wlf_shape_vertices vertices = {0};
	wlf_shape_add_segment(&vertices,
		shape->x1 + options->offset_x, shape->y1 + options->offset_y,
		shape->x2 + options->offset_x, shape->y2 + options->offset_y,
		state->stroke_width);
	wlf_shape_submit(pass->vector, target, &vertices, state->stroke_color,
		wlf_shape_state_stroke_alpha(state) * options->opacity,
		options->clip, options->blend_mode);
	wlf_shape_vertices_finish(&vertices);
}
