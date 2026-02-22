#include "wlf/animator/wlf_opacity_animator.h"
#include <stdlib.h>
#include <string.h>

/* Implementation functions */
static void opacity_animator_start(struct wlf_animator *animator) {
	struct wlf_opacity_animator *opacity = (struct wlf_opacity_animator *)animator;
	opacity->current = opacity->from;
}

static void opacity_animator_stop(struct wlf_animator *animator) {
	(void)animator;
	// Nothing special to do
}

static void opacity_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;  // dt is handled by base animator
	struct wlf_opacity_animator *opacity = (struct wlf_opacity_animator *)animator;

	float progress = wlf_animator_get_progress(animator);
	opacity->current = opacity->from + (opacity->to - opacity->from) * progress;
}

static bool opacity_animator_write_back(struct wlf_animator *animator) {
	struct wlf_opacity_animator *opacity = (struct wlf_opacity_animator *)animator;

	if (opacity->target) {
		*opacity->target = opacity->current;
		return true;
	}
	return false;
}

static void opacity_animator_destroy(struct wlf_animator *animator) {
	(void)animator;
	// Nothing special to cleanup
}

static const struct wlf_animator_impl opacity_animator_impl = {
	.start = opacity_animator_start,
	.stop = opacity_animator_stop,
	.pause = NULL,
	.resume = NULL,
	.update = opacity_animator_update,
	.write_back = opacity_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = opacity_animator_destroy,
};

struct wlf_opacity_animator *wlf_opacity_animator_create(int64_t duration,
                                                          float from,
                                                          float to,
                                                          float *target) {
	struct wlf_opacity_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &opacity_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

void wlf_opacity_animator_destroy(struct wlf_opacity_animator *animator) {
	if (!animator)
		return;

	wlf_animator_destroy(&animator->base);
	free(animator);
}

struct wlf_animator *wlf_opacity_animator_get_base(struct wlf_opacity_animator *animator) {
	return animator ? &animator->base : NULL;
}
