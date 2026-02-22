#include "wlf/animator/wlf_height_animator.h"
#include <stdlib.h>

static void height_animator_start(struct wlf_animator *animator) {
	struct wlf_height_animator *height = (struct wlf_height_animator *)animator;
	height->current = height->from;
}

static void height_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_height_animator *height = (struct wlf_height_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	height->current = height->from + (height->to - height->from) * progress;
}

static bool height_animator_write_back(struct wlf_animator *animator) {
	struct wlf_height_animator *height = (struct wlf_height_animator *)animator;
	if (height->target) {
		*height->target = height->current;
		return true;
	}
	return false;
}

static const struct wlf_animator_impl height_animator_impl = {
	.start = height_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = height_animator_update,
	.write_back = height_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_height_animator *wlf_height_animator_create(int64_t duration,
                                                        float from,
                                                        float to,
                                                        float *target) {
	struct wlf_height_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &height_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

void wlf_height_animator_destroy(struct wlf_height_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

struct wlf_animator *wlf_height_animator_get_base(struct wlf_height_animator *animator) {
	return animator ? &animator->base : NULL;
}
