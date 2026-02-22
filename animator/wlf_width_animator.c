#include "wlf/animator/wlf_width_animator.h"
#include <stdlib.h>

static void width_animator_start(struct wlf_animator *animator) {
	struct wlf_width_animator *width = (struct wlf_width_animator *)animator;
	width->current = width->from;
}

static void width_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_width_animator *width = (struct wlf_width_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	width->current = width->from + (width->to - width->from) * progress;
}

static bool width_animator_write_back(struct wlf_animator *animator) {
	struct wlf_width_animator *width = (struct wlf_width_animator *)animator;
	if (width->target) {
		*width->target = width->current;
		return true;
	}
	return false;
}

static const struct wlf_animator_impl width_animator_impl = {
	.start = width_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = width_animator_update,
	.write_back = width_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_width_animator *wlf_width_animator_create(int64_t duration,
                                                      float from,
                                                      float to,
                                                      float *target) {
	struct wlf_width_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &width_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

void wlf_width_animator_destroy(struct wlf_width_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

struct wlf_animator *wlf_width_animator_get_base(struct wlf_width_animator *animator) {
	return animator ? &animator->base : NULL;
}
