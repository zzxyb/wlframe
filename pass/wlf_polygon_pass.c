#include "wlf/pass/wlf_polygon_pass.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <stdlib.h>

void wlf_render_polygon_pass_init(struct wlf_render_polygon_pass *pass,
		const struct wlf_render_polygon_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_render_polygon_pass){ .impl = impl };
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_polygon_pass_destroy(struct wlf_render_polygon_pass *pass) {
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

void wlf_render_polygon_pass_add_polygon(struct wlf_render_polygon_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_polygon_options *options) {
	assert(options->point_count == 0 || options->points != NULL);
	pass->impl->render(pass, render_target_info, options);
}
