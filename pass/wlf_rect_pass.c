#include "wlf/pass/wlf_rect_pass.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/config.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/renderer/wlf_renderer.h"
#if WLF_HAS_LINUX_PLATFORM
#include "wlf/pass/gles/rect_pass.h"
#include "wlf/pass/pixman/rect_pass.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/renderer/pixman/renderer.h"
#endif

#include <assert.h>
#include <stdlib.h>

struct wlf_rect_pass *wlf_rect_pass_auto_create(struct wlf_renderer *renderer) {
	struct wlf_rect_pass *pass = NULL;
#if WLF_HAS_LINUX_PLATFORM
	if (wlf_renderer_is_gles(renderer)) {
		pass = wlf_gles_rect_pass_create();
	} else if (wlf_renderer_is_pixman(renderer)) {
		pass = wlf_pixman_rect_pass_create();
	} else {
		wlf_log(WLF_ERROR, "Scene rendering is unsupported by this renderer");
	}
#endif

	return pass;
}

void wlf_rect_pass_init(struct wlf_rect_pass *pass,
		const struct wlf_rect_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_rect_pass){
		.impl = impl,
	};

	wlf_signal_init(&pass->events.destroy);
}

void wlf_rect_pass_destroy(struct wlf_rect_pass *pass) {
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

void wlf_render_pass_add_rect(struct wlf_rect_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_rect_options *options) {
	assert(options->box.width >= 0 && options->box.height >= 0);

	pass->impl->render(pass, render_target_info, options);
}
