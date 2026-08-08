#include "wlf/pass/wlf_path_pass.h"
#include "wlf_shape_geometry.h"

#include <stdlib.h>

struct wlf_path_pass { struct wlf_vector_pass *vector; };

struct wlf_path_pass *wlf_path_pass_create(struct wlf_vector_pass *vector_pass) {
	if (vector_pass == NULL) return NULL;
	struct wlf_path_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_render_vector_pass_destroy(vector_pass);
		return NULL;
	}
	pass->vector = vector_pass;
	return pass;
}

void wlf_render_path_pass_destroy(struct wlf_path_pass *pass) {
	if (pass == NULL) return;
	wlf_render_vector_pass_destroy(pass->vector);
	free(pass);
}

void wlf_render_pass_add_path(struct wlf_path_pass *pass,
		struct wlf_render_target_info *target,
		const struct wlf_render_path_options *options) {
	if (pass == NULL || target == NULL || options == NULL || options->shape == NULL) return;
	const struct wlf_path_shape *shape = options->shape;
	const struct wlf_shape_state *state = &shape->state;
	struct wlf_shape_vertices vertices = {0};
	if (state->has_fill) {
		for (const struct wlf_path *path = shape->paths; path != NULL; path = path->next) {
			if (path->closed) {
				wlf_shape_add_polygon_fill(&vertices, path->pts, path->npts,
					options->offset_x, options->offset_y);
				wlf_shape_add_polygon_fringe(&vertices, path->pts, path->npts,
					options->offset_x, options->offset_y, 1);
			}
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->fill_color,
			wlf_shape_state_fill_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	vertices.len = 0;
	if (state->has_stroke && state->stroke_width > 0) {
		for (const struct wlf_path *path = shape->paths; path != NULL; path = path->next) {
			wlf_shape_add_polygon_stroke(&vertices, path->pts, path->npts,
				path->closed, state->stroke_width,
				options->offset_x, options->offset_y);
		}
		wlf_shape_submit(pass->vector, target, &vertices, state->stroke_color,
			wlf_shape_state_stroke_alpha(state) * options->opacity,
			options->clip, options->blend_mode);
	}
	wlf_shape_vertices_finish(&vertices);
}
