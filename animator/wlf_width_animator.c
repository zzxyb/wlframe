#include "wlf/animator/wlf_width_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void width_animator_start(struct wlf_animator *animator) {
	struct wlf_width_animator *width = wlf_width_animator_from_animator(animator);
	width->current = width->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void width_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void width_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void width_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void width_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_width_animator *width = wlf_width_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	width->current = width->from + (width->to - width->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool width_animator_write_back(struct wlf_animator *animator) {
	struct wlf_width_animator *width = wlf_width_animator_from_animator(animator);
	if (width->target) {
		*width->target = width->current;
		return true;
	}

	return false;
}

static void width_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void width_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void width_animator_destroy(struct wlf_animator *animator) {
	struct wlf_width_animator *width_animator =
		wlf_width_animator_from_animator(animator);
	free(width_animator);
}

static const struct wlf_animator_impl width_animator_impl = {
	.start = width_animator_start,
	.stop = width_animator_stop,
	.pause = width_animator_pause,
	.resume = width_animator_resume,
	.update = width_animator_update,
	.write_back = width_animator_write_back,
	.pre_sync = width_animator_pre_sync,
	.post_sync = width_animator_post_sync,
	.destroy = width_animator_destroy,
};

struct wlf_width_animator *wlf_width_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_width_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_width_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &width_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_width(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &width_animator_impl;
}

struct wlf_width_animator *wlf_width_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_width(animator)) {
		return NULL;
	}

	struct wlf_width_animator *width_animator = wlf_container_of(animator, width_animator, base);

	return width_animator;
}
