#include "wlf/animator/wlf_double_animation.h"
#include "wlf/curve/wlf_curve_linear.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_DOUBLE_ANIMATION = {
	.name = "wlf_double_animation",
	.parent = &WLF_ANIMATOR_JOB_TYPE_BASE,
};

static void double_animation_destroy_job(struct wlf_animator_job *job) {
	struct wlf_double_animation *animation =
		wlf_double_animation_from_job(job);
	wlf_double_animation_fini(animation);
	if (job->owns_self) {
		free(animation);
	}
}

static void double_animation_update_current_time(struct wlf_animator_job *job,
		int64_t current_time_msec) {
	struct wlf_double_animation *animation =
		wlf_double_animation_from_job(job);
	(void)current_time_msec;

	if (animation->binding.set == NULL || animation->binding.target == NULL) {
		return;
	}

	if (!animation->has_explicit_from && animation->binding.get != NULL) {
		animation->from = animation->binding.get(animation->binding.target,
			animation->binding.user_data);
		animation->has_explicit_from = true;
	}

	float progress = job->current_progress;
	if (animation->curve != NULL) {
		progress = wlf_curve_value_at(animation->curve, progress);
	}

	animation->value =
		animation->from + (animation->to - animation->from) * (double)progress;
	animation->binding.set(animation->binding.target, animation->value,
		animation->binding.user_data);
}

static const struct wlf_animator_job_impl double_animation_impl = {
	.destroy = double_animation_destroy_job,
	.update_current_time = double_animation_update_current_time,
};

struct wlf_double_animation *wlf_double_animation_create(void) {
	struct wlf_double_animation *animation = malloc(sizeof(*animation));
	if (animation == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_double_animation");
		return NULL;
	}

	wlf_double_animation_init(animation);
	animation->base.owns_self = true;
	return animation;
}

void wlf_double_animation_init(struct wlf_double_animation *animation) {
	wlf_double_animation_init_with_type(animation,
		&WLF_ANIMATOR_JOB_TYPE_DOUBLE_ANIMATION);
}

void wlf_double_animation_init_with_type(struct wlf_double_animation *animation,
		const struct wlf_animator_job_type *type) {
	assert(animation != NULL);

	*animation = (struct wlf_double_animation){0};
	wlf_animator_job_init(&animation->base, &double_animation_impl, type);

	animation->curve = wlf_curve_linear_create();
	if (animation->curve == NULL) {
		wlf_log(WLF_ERROR, "failed to create default linear curve for animation");
		return;
	}
	animation->owns_curve = true;
}

void wlf_double_animation_fini(struct wlf_double_animation *animation) {
	if (animation == NULL) {
		return;
	}

	if (animation->owns_curve && animation->curve != NULL) {
		wlf_curve_destroy(animation->curve);
	}
	animation->curve = NULL;
	animation->owns_curve = false;
}

void wlf_double_animation_destroy(struct wlf_double_animation *animation) {
	wlf_animator_job_destroy(&animation->base);
}

bool wlf_animator_job_is_double_animation(const struct wlf_animator_job *job) {
	return wlf_animator_job_is_type(job, &WLF_ANIMATOR_JOB_TYPE_DOUBLE_ANIMATION);
}

struct wlf_double_animation *wlf_double_animation_from_job(
		struct wlf_animator_job *job) {
	assert(wlf_animator_job_is_double_animation(job));
	struct wlf_double_animation *animation = NULL;
	return wlf_container_of(job, animation, base);
}

void wlf_double_animation_set_binding(struct wlf_double_animation *animation,
		const struct wlf_double_animation_binding *binding) {
	assert(animation != NULL);
	assert(binding != NULL);
	animation->binding = *binding;
}

void wlf_double_animation_set_curve(struct wlf_double_animation *animation,
		struct wlf_curve *curve, bool take_ownership) {
	assert(animation != NULL);

	if (animation->owns_curve && animation->curve != NULL) {
		wlf_curve_destroy(animation->curve);
	}

	animation->curve = curve;
	animation->owns_curve = take_ownership;
}

void wlf_double_animation_set_from(struct wlf_double_animation *animation,
		double from) {
	assert(animation != NULL);
	animation->from = from;
	animation->has_explicit_from = true;
}

void wlf_double_animation_set_to(struct wlf_double_animation *animation,
		double to) {
	assert(animation != NULL);
	animation->to = to;
}

void wlf_double_animation_clear_from(struct wlf_double_animation *animation) {
	assert(animation != NULL);
	animation->has_explicit_from = false;
}
