#include "wlf/pass/wlf_lame_texture_pass.h"

#include <assert.h>
#include <stdlib.h>

void wlf_render_lame_texture_pass_init(struct wlf_render_lame_texture_pass *pass,
		const struct wlf_render_lame_texture_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_render_lame_texture_pass){ .impl = impl };
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_lame_texture_pass_destroy(struct wlf_render_lame_texture_pass *pass) {
	if (pass == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&pass->events.destroy, pass);
	assert(wlf_linked_list_empty(&pass->events.destroy.listener_list));
	if (pass->impl->destroy != NULL) {
		pass->impl->destroy(pass);
	} else {
		free(pass);
	}
}

void wlf_render_lame_texture_pass_add_texture(struct wlf_render_lame_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_lame_texture_options *options) {
	assert(options->src_box.width >= 0.0 && options->src_box.height >= 0.0);
	assert(options->dst_box.width >= 0.0 && options->dst_box.height >= 0.0);
	assert(options->lame_m > 0.0f);
	assert(options->lame_n > 0.0f);
	pass->impl->render(pass, render_target_info, options);
}

void wlf_render_lame_texture_options_get_src_box(
		const struct wlf_render_lame_texture_options *options,
		struct wlf_frect *box) {
	*box = options->src_box;
	if (wlf_frect_is_empty(box)) {
		*box = (struct wlf_frect){
			.width = options->texture->width,
			.height = options->texture->height,
		};
	}
}

void wlf_render_lame_texture_options_get_dst_box(
		const struct wlf_render_lame_texture_options *options,
		struct wlf_frect *box) {
	*box = options->dst_box;
	if (wlf_frect_is_empty(box)) {
		box->width = options->texture->width;
		box->height = options->texture->height;
	}
}

float wlf_render_lame_texture_options_get_alpha(
		const struct wlf_render_lame_texture_options *options) {
	return options->alpha == NULL ? 1.0f : *options->alpha;
}
