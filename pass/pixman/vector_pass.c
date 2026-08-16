#include "wlf/pass/pixman/vector_pass.h"

#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/utils/wlf_log.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

static uint16_t channel(double value) {
	if (value <= 0) {
		return 0;
	}
	if (value >= 1) {
		return UINT16_MAX;
	}
	return (uint16_t)(value * UINT16_MAX + 0.5);
}

static void vector_pass_destroy(struct wlf_vector_pass *pass) {
	free(pass);
}

static void vector_pass_render(struct wlf_vector_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_vector_options *options) {
	(void)pass;
	if (!wlf_render_target_info_is_pixman(render_target_info)) {
		wlf_log(WLF_ERROR, "pixman vector pass requires a pixman target");
		return;
	}
	struct wlf_pixman_render_target_info *target =
		wlf_pixman_render_target_info_from_info(render_target_info);
	if (target->buffer == NULL || target->buffer->image == NULL) {
		return;
	}

	struct wlf_color color = wlf_color_clamp(&options->color);
	pixman_color_t solid_color = {
		.red = channel(color.r * color.a),
		.green = channel(color.g * color.a),
		.blue = channel(color.b * color.a),
		.alpha = channel(color.a),
	};
	pixman_image_t *solid = pixman_image_create_solid_fill(&solid_color);
	if (solid == NULL) {
		return;
	}

	size_t triangle_count = 0;
	for (size_t i = 0; i < options->vertex_count; i += 3) {
		if (options->vertices[i].coverage >= 1 &&
				options->vertices[i + 1].coverage >= 1 &&
				options->vertices[i + 2].coverage >= 1) triangle_count++;
	}
	if (triangle_count == 0) {
		pixman_image_unref(solid);
		return;
	}
	if (triangle_count > INT_MAX) {
		wlf_log(WLF_ERROR, "Too many triangles for pixman vector pass");
		pixman_image_unref(solid);
		return;
	}
	pixman_triangle_t *triangles = malloc(triangle_count * sizeof(*triangles));
	if (triangles == NULL) {
		pixman_image_unref(solid);
		return;
	}
	size_t triangle_index = 0;
	for (size_t i = 0; i < options->vertex_count; i += 3) {
		const struct wlf_vector_vertex *v = &options->vertices[i];
		if (v[0].coverage < 1 || v[1].coverage < 1 || v[2].coverage < 1) continue;
		double scale = render_target_info->scale;
		triangles[triangle_index++] = (pixman_triangle_t){
			.p1 = { pixman_double_to_fixed(v[0].x * scale),
				pixman_double_to_fixed(v[0].y * scale) },
			.p2 = { pixman_double_to_fixed(v[1].x * scale),
				pixman_double_to_fixed(v[1].y * scale) },
			.p3 = { pixman_double_to_fixed(v[2].x * scale),
				pixman_double_to_fixed(v[2].y * scale) },
		};
	}

	pixman_region32_t scaled_clip;
	pixman_region32_init(&scaled_clip);
	if (options->clip != NULL) {
		wlf_render_target_info_scale_region(render_target_info,
			options->clip, &scaled_clip);
		pixman_image_set_clip_region32(target->buffer->image,
			&scaled_clip);
	}
	pixman_composite_triangles(
		options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ? PIXMAN_OP_SRC : PIXMAN_OP_OVER,
		solid, target->buffer->image, PIXMAN_a8, 0, 0, 0, 0,
		(int)triangle_count, triangles);
	if (options->clip != NULL) {
		pixman_image_set_clip_region32(target->buffer->image, NULL);
	}
	pixman_region32_fini(&scaled_clip);

	free(triangles);
	pixman_image_unref(solid);
}

static const struct wlf_vector_pass_impl vector_pass_impl = {
	.destroy = vector_pass_destroy,
	.render = vector_pass_render,
};

struct wlf_vector_pass *wlf_pixman_vector_pass_create(void) {
	struct wlf_vector_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		return NULL;
	}
	wlf_render_vector_pass_init(pass, &vector_pass_impl);
	return pass;
}
