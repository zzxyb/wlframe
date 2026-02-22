#include "wlf/animator/wlf_rotation_animator.h"
#include <stdlib.h>

static void rotation_animator_start(struct wlf_animator *animator) {
	struct wlf_rotation_animator *rotation = (struct wlf_rotation_animator *)animator;
	rotation->current = rotation->from;
}

static void rotation_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_rotation_animator *rotation = (struct wlf_rotation_animator *)animator;
	float progress = wlf_animator_get_progress(animator);
	rotation->current = rotation->from + (rotation->to - rotation->from) * progress;
}

static bool rotation_animator_write_back(struct wlf_animator *animator) {
	struct wlf_rotation_animator *rotation = (struct wlf_rotation_animator *)animator;
	if (rotation->target) {
		*rotation->target = rotation->current;
		return true;
	}
	return false;
}

static const struct wlf_animator_impl rotation_animator_impl = {
	.start = rotation_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = rotation_animator_update,
	.write_back = rotation_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = NULL,
};

struct wlf_rotation_animator *wlf_rotation_animator_create(int64_t duration,
                                                            float from,
                                                            float to,
                                                            float *target) {
	struct wlf_rotation_animator *animator = calloc(1, sizeof(*animator));
	if (!animator)
		return NULL;

	wlf_animator_init(&animator->base, &rotation_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;
	animator->axis = WLF_ROTATION_AXIS_Z;  // Default to Z axis (2D rotation)
	animator->origin_x = 0.5f;  // Center
	animator->origin_y = 0.5f;  // Center

	return animator;
}

void wlf_rotation_animator_destroy(struct wlf_rotation_animator *animator) {
	if (!animator)
		return;
	wlf_animator_destroy(&animator->base);
	free(animator);
}

void wlf_rotation_animator_set_axis(struct wlf_rotation_animator *animator,
                                     enum wlf_rotation_axis axis) {
	if (!animator)
		return;
	animator->axis = axis;
}

void wlf_rotation_animator_set_origin(struct wlf_rotation_animator *animator,
                                       float origin_x, float origin_y) {
	if (!animator)
		return;
	animator->origin_x = origin_x;
	animator->origin_y = origin_y;
}

struct wlf_animator *wlf_rotation_animator_get_base(struct wlf_rotation_animator *animator) {
	return animator ? &animator->base : NULL;
}
