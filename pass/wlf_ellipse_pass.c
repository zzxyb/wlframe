#include "wlf/pass/wlf_ellipse_pass.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>

void wlf_render_ellipse_pass_init(struct wlf_render_ellipse_pass *pass,
		const struct wlf_render_ellipse_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_render_ellipse_pass){ .impl = impl };
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_ellipse_pass_destroy(struct wlf_render_ellipse_pass *pass) {
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

void wlf_render_ellipse_pass_add_ellipse(struct wlf_render_ellipse_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_ellipse_options *options) {
	assert(options->box.width >= 0.0 && options->box.height >= 0.0);
	pass->impl->render(pass, render_target_info, options);
}
