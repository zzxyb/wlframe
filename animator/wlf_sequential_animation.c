#include "wlf/animator/wlf_sequential_animation.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_SEQUENTIAL_ANIMATION = {
	.name = "wlf_sequential_animation",
	.parent = &WLF_ANIMATOR_JOB_TYPE_ANIMATION_GROUP,
};

static void sequential_animation_destroy_job(struct wlf_animator_job *job) {
	struct wlf_sequential_animation *animation =
		wlf_sequential_animation_from_job(job);
	wlf_animation_group_fini(&animation->base);
	if (job->owns_self) {
		free(animation);
	}
}

static void sequential_animation_update_current_time(struct wlf_animator_job *job,
		int64_t current_time_msec) {
	struct wlf_sequential_animation *animation =
		wlf_sequential_animation_from_job(job);
	int64_t remaining = current_time_msec;

	struct wlf_animator_job *child;
	wlf_linked_list_for_each(child, &animation->base.children, link) {
		int64_t child_duration = wlf_animator_job_total_duration(child);
		if (remaining <= 0) {
			wlf_animator_job_set_current_time(child, 0);
			continue;
		}
		if (child_duration >= 0 && remaining >= child_duration) {
			wlf_animator_job_set_current_time(child, child_duration);
			remaining -= child_duration;
		} else {
			wlf_animator_job_set_current_time(child, remaining);
			remaining = 0;
		}
	}
}

static const struct wlf_animator_job_impl sequential_animation_impl = {
	.destroy = sequential_animation_destroy_job,
	.update_current_time = sequential_animation_update_current_time,
};

struct wlf_sequential_animation *wlf_sequential_animation_create(void) {
	struct wlf_sequential_animation *animation = malloc(sizeof(*animation));
	if (animation == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_sequential_animation");
		return NULL;
	}

	wlf_sequential_animation_init(animation);
	animation->base.base.owns_self = true;
	return animation;
}

void wlf_sequential_animation_init(struct wlf_sequential_animation *animation) {
	wlf_animation_group_init(&animation->base, &sequential_animation_impl,
		&WLF_ANIMATOR_JOB_TYPE_SEQUENTIAL_ANIMATION);
}

void wlf_sequential_animation_destroy(struct wlf_sequential_animation *animation) {
	wlf_animator_job_destroy(&animation->base.base);
}

bool wlf_animator_job_is_sequential_animation(const struct wlf_animator_job *job) {
	return wlf_animator_job_is_type(job,
		&WLF_ANIMATOR_JOB_TYPE_SEQUENTIAL_ANIMATION);
}

struct wlf_sequential_animation *wlf_sequential_animation_from_job(
		struct wlf_animator_job *job) {
	assert(wlf_animator_job_is_sequential_animation(job));
	struct wlf_sequential_animation *animation = NULL;
	return wlf_container_of(job, animation, base.base);
}
