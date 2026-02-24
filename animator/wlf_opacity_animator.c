#include "wlf/animator/wlf_opacity_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void opacity_animator_start(struct wlf_animator *animator) {
	struct wlf_opacity_animator *opacity =
		wlf_opacity_animator_from_animator(animator);
	opacity->current = opacity->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void opacity_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void opacity_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void opacity_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void opacity_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_opacity_animator *opacity =
		wlf_opacity_animator_from_animator(animator);

	float progress = wlf_animator_get_progress(animator);
	opacity->current = opacity->from + (opacity->to - opacity->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool opacity_animator_write_back(struct wlf_animator *animator) {
	struct wlf_opacity_animator *opacity =
		wlf_opacity_animator_from_animator(animator);

	if (opacity->target) {
		*opacity->target = opacity->current;
		return true;
	}

	return false;
}

static void opacity_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void opacity_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void opacity_animator_destroy(struct wlf_animator *animator) {
	struct wlf_opacity_animator *opacity_animator =
		wlf_opacity_animator_from_animator(animator);
	free(opacity_animator);
}

static const struct wlf_animator_impl opacity_animator_impl = {
	.start = opacity_animator_start,
	.stop = opacity_animator_stop,
	.pause = opacity_animator_pause,
	.resume = opacity_animator_resume,
	.update = opacity_animator_update,
	.write_back = opacity_animator_write_back,
	.pre_sync = opacity_animator_pre_sync,
	.post_sync = opacity_animator_post_sync,
	.destroy = opacity_animator_destroy,
};

struct wlf_opacity_animator *wlf_opacity_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_opacity_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_opacity_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &opacity_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_opacity(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &opacity_animator_impl;
}

struct wlf_opacity_animator *wlf_opacity_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_opacity(animator)) {
		return NULL;
	}

	struct wlf_opacity_animator *opacity_animator = wlf_container_of(animator, opacity_animator, base);

	return opacity_animator;
}
