#include "wlf/animator/wlf_y_animator.h"
#include <stdlib.h>

static void y_animator_start(struct wlf_animator *animator) {
	struct wlf_y_animator *y = (struct wlf_y_animator *)animator;
	y->current = y->from;
}

static void y_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_y_animator *y = (struct wlf_y_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	y->current = y->from + (y->to - y->from) * progress;
}

static bool y_animator_write_back(struct wlf_animator *animator) {
	struct wlf_y_animator *y = (struct wlf_y_animator *)animator;
	if (y->target) {
		*y->target = y->current;
		return true;
	}
	return false;
}

static const struct wlf_animator_impl y_animator_impl = {
	.start = y_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = y_animator_update,
	.write_back = y_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_y_animator *wlf_y_animator_create(int64_t duration,
                                              float from,
                                              float to,
                                              float *target) {
	struct wlf_y_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &y_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

void wlf_y_animator_destroy(struct wlf_y_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

struct wlf_animator *wlf_y_animator_get_base(struct wlf_y_animator *animator) {
	return animator ? &animator->base : NULL;
}
