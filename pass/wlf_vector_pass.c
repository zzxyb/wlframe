#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>

void wlf_render_vector_pass_init(struct wlf_vector_pass *pass,
		const struct wlf_vector_pass_impl *impl) {
	assert(pass != NULL);
	assert(impl != NULL && impl->destroy != NULL && impl->render != NULL);
	*pass = (struct wlf_vector_pass){ .impl = impl };
	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_vector_pass_destroy(struct wlf_vector_pass *pass) {
	if (pass == NULL) {
		return;
	}
	wlf_signal_emit_mutable(&pass->events.destroy, pass);
	assert(wlf_linked_list_empty(&pass->events.destroy.listener_list));
	pass->impl->destroy(pass);
}

void wlf_render_pass_add_triangles(struct wlf_vector_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_vector_options *options) {
	assert(pass != NULL && render_target_info != NULL && options != NULL);
	assert(options->vertices != NULL && options->vertex_count % 3 == 0);
	if (options->vertex_count == 0 || options->color.a <= 0) {
		return;
	}
	pass->impl->render(pass, render_target_info, options);
}
