#include "wlf/animator/wlf_scale_animator.h"
#include <stdlib.h>

static void scale_animator_start(struct wlf_animator *animator) {
	struct wlf_scale_animator *scale = (struct wlf_scale_animator *)animator;
	scale->current_x = scale->from_x;
	scale->current_y = scale->from_y;
}

static void scale_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_scale_animator *scale = (struct wlf_scale_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	scale->current_x = scale->from_x + (scale->to_x - scale->from_x) * progress;
	scale->current_y = scale->from_y + (scale->to_y - scale->from_y) * progress;
}

static bool scale_animator_write_back(struct wlf_animator *animator) {
	struct wlf_scale_animator *scale = (struct wlf_scale_animator *)animator;
	bool success = false;

	if (scale->target_x) {
		*scale->target_x = scale->current_x;
		success = true;
	}

	if (scale->target_y) {
		*scale->target_y = scale->current_y;
		success = true;
	}

	return success;
}

static const struct wlf_animator_impl scale_animator_impl = {
	.start = scale_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = scale_animator_update,
	.write_back = scale_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_scale_animator *wlf_scale_animator_create(int64_t duration,
                                                      float from,
                                                      float to,
                                                      float *target_x,
                                                      float *target_y) {
	return wlf_scale_animator_create_xy(duration, from, to, from, to, target_x, target_y);
}

struct wlf_scale_animator *wlf_scale_animator_create_xy(int64_t duration,
                                                         float from_x, float to_x,
                                                         float from_y, float to_y,
                                                         float *target_x,
                                                         float *target_y) {
	struct wlf_scale_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &scale_animator_impl);
	animator->base.duration = duration;
	animator->from_x = from_x;
	animator->to_x = to_x;
	animator->from_y = from_y;
	animator->to_y = to_y;
	animator->current_x = from_x;
	animator->current_y = from_y;
	animator->target_x = target_x;
	animator->target_y = target_y;
	animator->origin_x = 0.5f;  // Center
	animator->origin_y = 0.5f;  // Center

	return animator;
}

void wlf_scale_animator_destroy(struct wlf_scale_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

void wlf_scale_animator_set_origin(struct wlf_scale_animator *animator,
                                    float origin_x, float origin_y) {
	if (!animator)
		return;
	animator->origin_x = origin_x;
	animator->origin_y = origin_y;
}

struct wlf_animator *wlf_scale_animator_get_base(struct wlf_scale_animator *animator) {
	return animator ? &animator->base : NULL;
}
