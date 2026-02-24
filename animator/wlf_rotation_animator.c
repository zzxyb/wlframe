#include "wlf/animator/wlf_rotation_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void rotation_animator_start(struct wlf_animator *animator) {
	struct wlf_rotation_animator *rotation =
		wlf_rotation_animator_from_animator(animator);
	rotation->current = rotation->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void rotation_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void rotation_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void rotation_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void rotation_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_rotation_animator *rotation =
		wlf_rotation_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	rotation->current = rotation->from + (rotation->to - rotation->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool rotation_animator_write_back(struct wlf_animator *animator) {
	struct wlf_rotation_animator *rotation =
		wlf_rotation_animator_from_animator(animator);
	if (rotation->target) {
		*rotation->target = rotation->current;
		return true;
	}

	return false;
}

static void rotation_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void rotation_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void rotation_animator_destroy(struct wlf_animator *animator) {
	struct wlf_rotation_animator *rotation_animator =
		wlf_rotation_animator_from_animator(animator);
	free(rotation_animator);
}

static const struct wlf_animator_impl rotation_animator_impl = {
	.start = rotation_animator_start,
	.stop = rotation_animator_stop,
	.pause = rotation_animator_pause,
	.resume = rotation_animator_resume,
	.update = rotation_animator_update,
	.write_back = rotation_animator_write_back,
	.pre_sync = rotation_animator_pre_sync,
	.post_sync = rotation_animator_post_sync,
	.destroy = rotation_animator_destroy,
};

struct wlf_rotation_animator *wlf_rotation_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_rotation_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_rotation_animator");
		return NULL;
	}

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

bool wlf_animator_is_rotation(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &rotation_animator_impl;
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

struct wlf_rotation_animator *wlf_rotation_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_rotation(animator)) {
		return NULL;
	}

	struct wlf_rotation_animator *rotation_animator = wlf_container_of(animator, rotation_animator, base);

	return rotation_animator;
}
