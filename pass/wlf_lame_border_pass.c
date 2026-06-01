#include "wlf/pass/wlf_lame_border_pass.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>

void wlf_render_lame_border_pass_init(struct wlf_render_lame_border_pass *pass,
		const struct wlf_render_lame_border_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_render_lame_border_pass){ .impl = impl };
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_lame_border_pass_destroy(struct wlf_render_lame_border_pass *pass) {
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

void wlf_render_lame_border_pass_add_border(struct wlf_render_lame_border_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_lame_border_options *options) {
	assert(options->box.width >= 0.0 && options->box.height >= 0.0);
	assert(options->border_width >= 0.0f);
	assert(options->lame_m > 0.0f);
	assert(options->lame_n > 0.0f);
	pass->impl->render(pass, render_target_info, options);
}
