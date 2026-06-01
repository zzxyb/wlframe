/**
 * @file        wlf_ellipse_pass.h
 * @brief       Ellipse rendering pass interface in wlframe.
 */

#ifndef PASS_WLF_ELLIPSE_PASS_H
#define PASS_WLF_ELLIPSE_PASS_H

#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_render_ellipse_options {
	struct wlf_frect box;
	struct wlf_color color;
	const struct wlf_region *clip;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_render_ellipse_pass;

struct wlf_render_ellipse_pass_impl {
	void (*destroy)(struct wlf_render_ellipse_pass *pass);
	void (*render)(struct wlf_render_ellipse_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_ellipse_options *options);
};

struct wlf_render_ellipse_pass {
	const struct wlf_render_ellipse_pass_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;
};

void wlf_render_ellipse_pass_init(struct wlf_render_ellipse_pass *pass,
	const struct wlf_render_ellipse_pass_impl *impl);
void wlf_render_ellipse_pass_destroy(struct wlf_render_ellipse_pass *pass);
void wlf_render_ellipse_pass_add_ellipse(struct wlf_render_ellipse_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_ellipse_options *options);

#endif // PASS_WLF_ELLIPSE_PASS_H
