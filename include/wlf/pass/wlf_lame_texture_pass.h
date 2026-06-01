/**
 * @file        wlf_lame_texture_pass.h
 * @brief       Superellipse-texture rendering pass interface in wlframe.
 */

#ifndef PASS_WLF_LAME_TEXTURE_PASS_H
#define PASS_WLF_LAME_TEXTURE_PASS_H

#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/types/wlf_output.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_texture;

struct wlf_render_lame_texture_options {
	struct wlf_texture *texture;
	struct wlf_frect src_box;
	struct wlf_frect dst_box;
	float lame_m;
	float lame_n;
	const float *alpha;
	const struct wlf_region *clip;
	enum wlf_output_transform transform;
	enum wlf_scale_filter_mode filter_mode;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_render_lame_texture_pass;

struct wlf_render_lame_texture_pass_impl {
	void (*destroy)(struct wlf_render_lame_texture_pass *pass);
	void (*render)(struct wlf_render_lame_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_lame_texture_options *options);
};

struct wlf_render_lame_texture_pass {
	const struct wlf_render_lame_texture_pass_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;
};

void wlf_render_lame_texture_pass_init(struct wlf_render_lame_texture_pass *pass,
	const struct wlf_render_lame_texture_pass_impl *impl);
void wlf_render_lame_texture_pass_destroy(struct wlf_render_lame_texture_pass *pass);
void wlf_render_lame_texture_pass_add_texture(struct wlf_render_lame_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_lame_texture_options *options);
void wlf_render_lame_texture_options_get_src_box(
	const struct wlf_render_lame_texture_options *options,
	struct wlf_frect *box);
void wlf_render_lame_texture_options_get_dst_box(
	const struct wlf_render_lame_texture_options *options,
	struct wlf_frect *box);
float wlf_render_lame_texture_options_get_alpha(
	const struct wlf_render_lame_texture_options *options);

#endif // PASS_WLF_LAME_TEXTURE_PASS_H
