#include "wlf/pass/pixman/rect_pass.h"

#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/utils/wlf_log.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static uint16_t clamp_channel(double value) {
	if (value <= 0.0) {
		return 0;
	}
	if (value >= 1.0) {
		return UINT16_MAX;
	}

	return (uint16_t)(value * UINT16_MAX + 0.5);
}

static void pixman_rect_pass_destroy(struct wlf_rect_pass *pass) {
	free(pass);
}

static void pixman_rect_pass_render(struct wlf_rect_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_rect_options *options) {
	(void)pass;

	if (!wlf_render_target_info_is_pixman(render_target_info)) {
		wlf_log(WLF_ERROR, "pixman rect pass requires a pixman render target");
		return;
	}

	struct wlf_pixman_render_target_info *target =
		wlf_pixman_render_target_info_from_info(render_target_info);
	if (target->buffer == NULL || target->buffer->image == NULL) {
		return;
	}

	double scale = render_target_info->scale;
	int x = (int)floor(options->box.x * scale);
	int y = (int)floor(options->box.y * scale);
	int width = (int)ceil((options->box.x + options->box.width) * scale) - x;
	int height = (int)ceil((options->box.y + options->box.height) * scale) - y;
	if (width <= 0 || height <= 0) {
		return;
	}

	pixman_region32_t rect_region;
	pixman_region32_init_rect(&rect_region, x, y, (uint32_t)width, (uint32_t)height);

	pixman_region32_t clipped_region;
	pixman_region32_init(&clipped_region);
	if (options->clip != NULL) {
		pixman_region32_t scaled_clip;
		pixman_region32_init(&scaled_clip);
		wlf_render_target_info_scale_region(render_target_info,
			options->clip, &scaled_clip);
		pixman_region32_intersect(&clipped_region, &rect_region, &scaled_clip);
		pixman_region32_fini(&scaled_clip);
	} else {
		pixman_region32_copy(&clipped_region, &rect_region);
	}

	int nrects = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped_region, &nrects);
	if (nrects <= 0) {
		goto out;
	}

	struct wlf_color color = wlf_color_clamp(&options->color);
	pixman_color_t pixman_color = {
		.red = clamp_channel(color.r * color.a),
		.green = clamp_channel(color.g * color.a),
		.blue = clamp_channel(color.b * color.a),
		.alpha = clamp_channel(color.a),
	};
	pixman_image_t *solid = pixman_image_create_solid_fill(&pixman_color);
	if (solid == NULL) {
		wlf_log(WLF_ERROR, "failed to create pixman solid fill");
		goto out;
	}

	pixman_op_t op = options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		PIXMAN_OP_SRC : PIXMAN_OP_OVER;
	for (int i = 0; i < nrects; i++) {
		const pixman_box32_t *r = &rects[i];
		pixman_image_composite32(op, solid, NULL, target->buffer->image,
			0, 0, 0, 0, r->x1, r->y1, r->x2 - r->x1, r->y2 - r->y1);
	}

	pixman_image_unref(solid);

out:
	pixman_region32_fini(&clipped_region);
	pixman_region32_fini(&rect_region);
}

static const struct wlf_rect_pass_impl pixman_rect_pass_impl = {
	.destroy = pixman_rect_pass_destroy,
	.render = pixman_rect_pass_render,
};

struct wlf_rect_pass *wlf_pixman_rect_pass_create(void) {
	struct wlf_rect_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_rect_pass");
		return NULL;
	}

	wlf_render_rect_pass_init(pass, &pixman_rect_pass_impl);
	return pass;
}
