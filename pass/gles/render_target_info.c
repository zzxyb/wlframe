#include "wlf/pass/gles/render_target_info.h"

#include "wlf/renderer/gles/egl.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <stdlib.h>

static void render_target_info_destroy(struct wlf_render_target_info *render_target) {
	struct wlf_gles_render_target_info *target =
		wlf_gles_render_target_info_from_info(render_target);
	free(target);
}

static struct wlf_renderer *render_target_info_get_renderer(
		struct wlf_render_target_info *render_target) {
	struct wlf_gles_render_target_info *target =
		wlf_gles_render_target_info_from_info(render_target);
	return &target->renderer->base;
}

static const struct wlf_render_target_info_impl render_target_info_impl = {
	.destroy = render_target_info_destroy,
	.get_renderer = render_target_info_get_renderer,
};

struct wlf_gles_render_target_info *wlf_gles_begin_egl_render_pass(
		struct wlf_egl_swapchain *swapchain) {
	struct wlf_gles_render_target_info *target = calloc(1, sizeof(*target));
	if (target == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_gles_render_target_info");
		return NULL;
	}

	struct wlf_gles_renderer *renderer =
		wlf_gles_renderer_from_renderer(swapchain->base.window->state.renderer);
	if (!wlf_egl_make_current(renderer->egl, swapchain->surface, swapchain->surface)) {
		wlf_log(WLF_ERROR, "failed to make GLES render target current");
		free(target);
		return NULL;
	}

	wlf_render_target_info_init(&target->base, &render_target_info_impl);
	target->swapchain = swapchain;
	target->renderer = renderer;

	return target;
}

bool wlf_render_target_info_is_gles(
		const struct wlf_render_target_info *render_target) {
	return render_target->impl == &render_target_info_impl;
}

struct wlf_gles_render_target_info *wlf_gles_render_target_info_from_info(
		struct wlf_render_target_info *render_target) {
	assert(render_target->impl == &render_target_info_impl);

	struct wlf_gles_render_target_info *target =
		wlf_container_of(render_target, target, base);
	return target;
}
