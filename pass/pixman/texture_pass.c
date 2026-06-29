#include "wlf/pass/pixman/texture_pass.h"

#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/texture/pixman/texture.h"
#include "wlf/utils/wlf_log.h"

#include <math.h>
#include <stdlib.h>

static void pixman_texture_pass_destroy(struct wlf_texture_pass *pass) {
	free(pass);
}

static void pixman_texture_pass_render(struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_texture_options *options) {
	(void)pass;
	if (!wlf_render_target_info_is_pixman(render_target_info) ||
			!wlf_texture_is_pixman(options->texture)) {
		wlf_log(WLF_ERROR, "pixman texture pass requires pixman target and texture");
		return;
	}

	struct wlf_pixman_render_target_info *target =
		wlf_pixman_render_target_info_from_info(render_target_info);
	struct wlf_pixman_texture *texture =
		wlf_pixman_texture_from_texture(options->texture);
	if (target->buffer == NULL || target->buffer->image == NULL ||
			texture->image == NULL || options->opacity <= 0.0f) {
		return;
	}

	struct wlf_frect src, dst;
	wlf_render_texture_options_get_src_box(options, &src);
	wlf_render_texture_options_get_dst_box(options, &dst);
	if (src.width <= 0 || src.height <= 0 || dst.width <= 0 || dst.height <= 0) {
		return;
	}

	double target_scale = render_target_info->scale;
	dst.x *= target_scale;
	dst.y *= target_scale;
	dst.width *= target_scale;
	dst.height *= target_scale;
	int x = (int)floor(dst.x);
	int y = (int)floor(dst.y);
	int width = (int)ceil(dst.x + dst.width) - x;
	int height = (int)ceil(dst.y + dst.height) - y;
	pixman_region32_t dst_region, clipped;
	pixman_region32_init_rect(&dst_region, x, y, width, height);
	pixman_region32_init(&clipped);
	if (options->clip != NULL) {
		pixman_region32_t scaled_clip;
		pixman_region32_init(&scaled_clip);
		wlf_render_target_info_scale_region(render_target_info,
			options->clip, &scaled_clip);
		pixman_region32_intersect(&clipped, &dst_region, &scaled_clip);
		pixman_region32_fini(&scaled_clip);
	} else {
		pixman_region32_copy(&clipped, &dst_region);
	}

	double scale_x = src.width / dst.width;
	double scale_y = src.height / dst.height;
	pixman_transform_t transform = {
		.matrix = {
			{ pixman_double_to_fixed(scale_x), 0,
				pixman_double_to_fixed(src.x - dst.x * scale_x) },
			{ 0, pixman_double_to_fixed(scale_y),
				pixman_double_to_fixed(src.y - dst.y * scale_y) },
			{ 0, 0, pixman_fixed_1 },
		},
	};
	pixman_image_set_transform(texture->image, &transform);
	pixman_image_set_filter(texture->image,
		options->filter_mode == WLF_SCALE_FILTER_NEAREST ?
		PIXMAN_FILTER_NEAREST : PIXMAN_FILTER_BILINEAR, NULL, 0);
	pixman_image_set_repeat(texture->image, PIXMAN_REPEAT_NONE);

	pixman_image_t *mask = NULL;
	if (options->opacity < 1.0f) {
		pixman_color_t alpha = {
			.red = 0xffff,
			.green = 0xffff,
			.blue = 0xffff,
			.alpha = (uint16_t)lround(options->opacity * 65535.0),
		};
		mask = pixman_image_create_solid_fill(&alpha);
	}

	int nrects = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(&clipped, &nrects);
	pixman_op_t op = options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		PIXMAN_OP_SRC : PIXMAN_OP_OVER;
	for (int i = 0; i < nrects; i++) {
		pixman_box32_t *r = &rects[i];
		pixman_image_composite32(op, texture->image, mask, target->buffer->image,
			r->x1, r->y1, 0, 0, r->x1, r->y1,
			r->x2 - r->x1, r->y2 - r->y1);
	}

	if (mask != NULL) {
		pixman_image_unref(mask);
	}
	pixman_image_set_transform(texture->image, NULL);
	pixman_region32_fini(&clipped);
	pixman_region32_fini(&dst_region);
}

static const struct wlf_texture_pass_impl pixman_texture_pass_impl = {
	.destroy = pixman_texture_pass_destroy,
	.render = pixman_texture_pass_render,
};

struct wlf_texture_pass *wlf_pixman_texture_pass_create(void) {
	struct wlf_texture_pass *pass = malloc(sizeof(*pass));
	if (pass == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate pixman texture pass");
		return NULL;
	}
	wlf_render_texture_pass_init(pass, &pixman_texture_pass_impl);
	return pass;
}
