#include "wlf/pass/gles/render_target_info.h"

#include "wlf/renderer/gles/egl.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/utils/wlf_log.h"

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
		struct wlf_egl_buffer *buffer, struct wlf_gles_renderer *renderer) {
	if (buffer == NULL || renderer == NULL ||
			wlf_egl_buffer_get_egl(buffer) != renderer->egl) {
		return NULL;
	}

	struct wlf_gles_render_target_info *target = calloc(1, sizeof(*target));
	if (target == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_gles_render_target_info");
		return NULL;
	}

	EGLSurface surface = wlf_egl_buffer_get_surface(buffer);
	if (!wlf_egl_make_current(renderer->egl, surface, surface)) {
		wlf_log(WLF_ERROR, "failed to make GLES render target current");
		free(target);
		return NULL;
	}
	if (!wlf_egl_buffer_configure_swap_interval(buffer)) {
		free(target);
		return NULL;
	}

	wlf_render_target_info_init(&target->base, &render_target_info_impl);
	target->base.logical_width = (int)buffer->base.width;
	target->base.logical_height = (int)buffer->base.height;
	target->base.buffer_width = (int)buffer->base.width;
	target->base.buffer_height = (int)buffer->base.height;
	target->buffer = buffer;
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
