#include "wlf/animator/wlf_uniform_animator.h"
#include <stdlib.h>

static void uniform_animator_start(struct wlf_animator *animator) {
	struct wlf_uniform_animator *uniform = (struct wlf_uniform_animator *)animator;
	uniform->current = uniform->from;
}

static void uniform_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_uniform_animator *uniform = (struct wlf_uniform_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	uniform->current = uniform->from + (uniform->to - uniform->from) * progress;
}

static bool uniform_animator_write_back(struct wlf_animator *animator) {
	struct wlf_uniform_animator *uniform = (struct wlf_uniform_animator *)animator;
	if (uniform->target) {
		*uniform->target = uniform->current;
		return true;
	}
	return false;
}

static const struct wlf_animator_impl uniform_animator_impl = {
	.start = uniform_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = uniform_animator_update,
	.write_back = uniform_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_uniform_animator *wlf_uniform_animator_create(int64_t duration,
                                                          float from,
                                                          float to,
                                                          float *target) {
	struct wlf_uniform_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &uniform_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

void wlf_uniform_animator_destroy(struct wlf_uniform_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

struct wlf_animator *wlf_uniform_animator_get_base(struct wlf_uniform_animator *animator) {
	return animator ? &animator->base : NULL;
}
