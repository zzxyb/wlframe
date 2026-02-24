#include "wlf/animator/wlf_uniform_animator.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>
#include <string.h>

static void uniform_animator_start(struct wlf_animator *animator) {
	struct wlf_uniform_animator *uniform =
		wlf_uniform_animator_from_animator(animator);
	uniform->current = uniform->from;

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

static void uniform_animator_stop(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

static void uniform_animator_pause(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

static void uniform_animator_resume(struct wlf_animator *animator) {
	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

static void uniform_animator_update(struct wlf_animator *animator, int64_t dt) {
	WLF_UNUSED(dt);
	struct wlf_uniform_animator *uniform =
		wlf_uniform_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	uniform->current = uniform->from + (uniform->to - uniform->from) * progress;
	wlf_signal_emit_mutable(&animator->events.updated, animator);
}

static bool uniform_animator_write_back(struct wlf_animator *animator) {
	struct wlf_uniform_animator *uniform =
		wlf_uniform_animator_from_animator(animator);
	if (uniform->target) {
		*uniform->target = uniform->current;
		return true;
	}

	return false;
}

static void uniform_animator_pre_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void uniform_animator_post_sync(struct wlf_animator *animator) {
	WLF_UNUSED(animator);
}

static void uniform_animator_destroy(struct wlf_animator *animator) {
	struct wlf_uniform_animator *uniform_animator =
		wlf_uniform_animator_from_animator(animator);
	free(uniform_animator);
}

static const struct wlf_animator_impl uniform_animator_impl = {
	.start = uniform_animator_start,
	.stop = uniform_animator_stop,
	.pause = uniform_animator_pause,
	.resume = uniform_animator_resume,
	.update = uniform_animator_update,
	.write_back = uniform_animator_write_back,
	.pre_sync = uniform_animator_pre_sync,
	.post_sync = uniform_animator_post_sync,
	.destroy = uniform_animator_destroy,
};

struct wlf_uniform_animator *wlf_uniform_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_uniform_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_uniform_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &uniform_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_uniform(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &uniform_animator_impl;
}

struct wlf_uniform_animator *wlf_uniform_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_uniform(animator)) {
		return NULL;
	}

	struct wlf_uniform_animator *uniform_animator = wlf_container_of(animator, uniform_animator, base);

	return uniform_animator;
}
