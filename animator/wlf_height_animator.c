#include "wlf/animator/wlf_height_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void height_animator_start(struct wlf_animator *animator) {
	struct wlf_height_animator *height = wlf_height_animator_from_animator(animator);
	height->current = height->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void height_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void height_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void height_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void height_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_height_animator *height = wlf_height_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	height->current = height->from + (height->to - height->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool height_animator_write_back(struct wlf_animator *animator) {
	struct wlf_height_animator *height = wlf_height_animator_from_animator(animator);
	if (height->target) {
		*height->target = height->current;
		return true;
	}

	return false;
}

static void height_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void height_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void height_animator_destroy(struct wlf_animator *animator) {
	struct wlf_height_animator *height_animator =
		wlf_height_animator_from_animator(animator);
	free(height_animator);
}

static const struct wlf_animator_impl height_animator_impl = {
	.start = height_animator_start,
	.stop = height_animator_stop,
	.pause = height_animator_pause,
	.resume = height_animator_resume,
	.update = height_animator_update,
	.write_back = height_animator_write_back,
	.pre_sync = height_animator_pre_sync,
	.post_sync = height_animator_post_sync,
	.destroy = height_animator_destroy,
};

struct wlf_height_animator *wlf_height_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_height_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_height_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &height_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_height(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &height_animator_impl;
}

struct wlf_height_animator *wlf_height_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_height(animator)) {
		return NULL;
	}

	struct wlf_height_animator *height_animator = wlf_container_of(animator, height_animator, base);

	return height_animator;
}
