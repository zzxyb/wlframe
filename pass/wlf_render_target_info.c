#include "wlf/pass/wlf_render_target_info.h"

#include <assert.h>
#include <stdlib.h>

void wlf_render_target_info_init(struct wlf_render_target_info *render_target,
		const struct wlf_render_target_info_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	render_target->impl = impl;

	wlf_signal_init(&render_target->events.destroy);
}

void wlf_render_target_info_destroy(struct wlf_render_target_info *render_target) {
	if (render_target == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&render_target->events.destroy, render_target);

	assert(wlf_linked_list_empty(&render_target->events.destroy.listener_list));

	if (render_target->impl && render_target->impl->destroy) {
		render_target->impl->destroy(render_target);
	} else {
		free(render_target);
	}
}
