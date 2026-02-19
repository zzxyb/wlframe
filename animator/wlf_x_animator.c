#include "wlf/animator/wlf_x_animator.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static void x_animator_start(struct wlf_animator *animator) {
	struct wlf_x_animator *x = wlf_x_animator_from_animator(animator);
	x->current = x->from;
}

static void x_animator_update(struct wlf_animator *animator, int64_t dt) {
	(void)dt;
	struct wlf_x_animator *x = wlf_x_animator_from_animator(animator);
	float progress = wlf_animator_get_progress(animator);
	x->current = x->from + (x->to - x->from) * progress;
}

static bool x_animator_write_back(struct wlf_animator *animator) {
	struct wlf_x_animator *x = wlf_x_animator_from_animator(animator);
	if (x->target) {
		*x->target = x->current;
		return true;
	}

	return false;
}

static void x_animator_destroy(struct wlf_animator *animator) {
	struct wlf_x_animator *x_animator =
		wlf_x_animator_from_animator(animator);
	free(x_animator);
}

static const struct wlf_animator_impl x_animator_impl = {
	.start = x_animator_start,
	.stop = NULL,
	.pause = NULL,
	.resume = NULL,
	.update = x_animator_update,
	.write_back = x_animator_write_back,
	.pre_sync = NULL,
	.post_sync = NULL,
	.destroy = x_animator_destroy,
};

struct wlf_x_animator *wlf_x_animator_create(int64_t duration,
		float from, float to, float *target) {
	struct wlf_x_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_x_animator");
		return NULL;
	}

	wlf_animator_init(&animator->base, &x_animator_impl);
	animator->base.duration = duration;
	animator->from = from;
	animator->to = to;
	animator->current = from;
	animator->target = target;

	return animator;
}

bool wlf_animator_is_x(const struct wlf_animator *animator) {
	if (animator == NULL || animator->impl == NULL) {
		return false;
	}

	return animator->impl == &x_animator_impl;
}

struct wlf_x_animator *wlf_x_animator_from_animator(struct wlf_animator *animator) {
	if (!wlf_animator_is_x(animator)) {
		return NULL;
	}

	struct wlf_x_animator *x_animator = wlf_container_of(animator, x_animator, base);

	return x_animator;
}
