#include "wlf/pass/wlf_render_target_info.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

void wlf_render_target_info_init(struct wlf_render_target_info *render_target,
		const struct wlf_render_target_info_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	render_target->impl = impl;
	render_target->scale = 1.0;

	wlf_signal_init(&render_target->events.destroy);
}

void wlf_render_target_info_scale_region(
		const struct wlf_render_target_info *render_target,
		const pixman_region32_t *logical, pixman_region32_t *buffer) {
	pixman_region32_clear(buffer);
	int count = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(
		(pixman_region32_t *)logical, &count);
	for (int i = 0; i < count; i++) {
		int x1 = (int)floor(rects[i].x1 * render_target->scale);
		int y1 = (int)floor(rects[i].y1 * render_target->scale);
		int x2 = (int)ceil(rects[i].x2 * render_target->scale);
		int y2 = (int)ceil(rects[i].y2 * render_target->scale);
		pixman_region32_union_rect(buffer, buffer, x1, y1,
			(uint32_t)(x2 - x1), (uint32_t)(y2 - y1));
	}
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
