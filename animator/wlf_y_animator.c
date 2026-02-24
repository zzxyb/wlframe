#include "wlf/animator/wlf_y_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void y_animator_start(struct wlf_animator *animator) {
	struct wlf_y_animator *y = wlf_y_animator_from_animator(animator);
	y->current = y->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void y_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void y_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void y_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void y_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_y_animator *y = wlf_y_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	y->current = y->from + (y->to - y->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool y_animator_write_back(struct wlf_animator *animator) {
	struct wlf_y_animator *y = wlf_y_animator_from_animator(animator);
	if (y->target) {
		*y->target = y->current;
		return true;
	}

	return false;
}

static void y_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void y_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void y_animator_destroy(struct wlf_animator *animator) {
	struct wlf_y_animator *y_animator =
		wlf_y_animator_from_animator(animator);
	free(y_animator);
}

static const struct wlf_animator_impl y_animator_impl = {
	.start = y_animator_start,
	.stop = y_animator_stop,
	.pause = y_animator_pause,
	.resume = y_animator_resume,
	.update = y_animator_update,
	.write_back = y_animator_write_back,
	.pre_sync = y_animator_pre_sync,
	.post_sync = y_animator_post_sync,
	.destroy = y_animator_destroy,
};

struct wlf_y_animator *wlf_y_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_y_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_y_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &y_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_y(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &y_animator_impl;
}

struct wlf_y_animator *wlf_y_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_y(animator)) {
		return NULL;
	}

	struct wlf_y_animator *y_animator = wlf_container_of(animator, y_animator, base);

	return y_animator;
}
