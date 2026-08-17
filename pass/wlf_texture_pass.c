#include "wlf/pass/wlf_texture_pass.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_log.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/pass/gles/texture_pass.h"
#include "wlf/pass/pixman/texture_pass.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#endif

#include <assert.h>
#include <stdlib.h>

struct wlf_texture_pass *wlf_texture_pass_auto_create(struct wlf_renderer *renderer) {
	struct wlf_texture_pass *pass = NULL;
#if WLF_HAS_LINUX_PLATFORM
	if (wlf_renderer_is_gles(renderer)) {
		pass = wlf_gles_texture_pass_create();
	} else if (wlf_renderer_is_pixman(renderer)) {
		pass = wlf_pixman_texture_pass_create();
	} else {
		wlf_log(WLF_ERROR, "Scene rendering is unsupported by this renderer");
	}
#endif

	return pass;
}

void wlf_render_texture_pass_init(struct wlf_texture_pass *pass,
		const struct wlf_texture_pass_impl *impl) {
	assert(pass != NULL);
	assert(impl != NULL && impl->destroy != NULL && impl->render != NULL);

	*pass = (struct wlf_texture_pass){
		.impl = impl,
	};
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_texture_pass_destroy(struct wlf_texture_pass *pass) {
	if (pass == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&pass->events.destroy, pass);
	assert(wlf_linked_list_empty(&pass->events.destroy.listener_list));
	pass->impl->destroy(pass);
}

void wlf_render_texture_options_get_src_box(
		const struct wlf_render_texture_options *options,
		struct wlf_frect *box) {
	if (options->src_box.width > 0 && options->src_box.height > 0) {
		*box = options->src_box;
	} else {
		*box = (struct wlf_frect){
			.width = options->texture->width,
			.height = options->texture->height,
		};
	}
}

void wlf_render_texture_options_get_dst_box(
		const struct wlf_render_texture_options *options,
		struct wlf_frect *box) {
	if (options->dst_box.width > 0 && options->dst_box.height > 0) {
		*box = options->dst_box;
	} else {
		*box = (struct wlf_frect){
			.x = options->dst_box.x,
			.y = options->dst_box.y,
			.width = options->texture->width,
			.height = options->texture->height,
		};
	}
}

void wlf_render_pass_add_texture(struct wlf_texture_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_texture_options *options) {
	assert(pass != NULL && render_target_info != NULL);
	assert(options != NULL && options->texture != NULL);
	assert(options->opacity >= 0.0f && options->opacity <= 1.0f);

	pass->impl->render(pass, render_target_info, options);
}
