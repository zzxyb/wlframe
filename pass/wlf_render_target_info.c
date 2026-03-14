#include "wlf/pass/wlf_render_target_info.h"

#include <stdlib.h>
#include <assert.h>

void wlf_render_target_info_init(struct wlf_render_target_info *info,
		const struct wlf_render_target_info_impl *impl) {
	*info = (struct wlf_render_target_info){
		.impl = impl,
	};
	wlf_signal_init(&info->events.destroy);
}

void wlf_render_target_info_destroy(struct wlf_render_target_info *info) {
	if (info == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&info->events.destroy, info);

	assert(wlf_linked_list_empty(&info->events.destroy.listener_list));

	if (info->impl && info->impl->destroy) {
		info->impl->destroy(info);
	} else {
		free(info);
	}
}
