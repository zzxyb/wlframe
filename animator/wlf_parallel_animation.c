#include "wlf/animator/wlf_parallel_animation.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_PARALLEL_ANIMATION = {
	.name = "wlf_parallel_animation",
	.parent = &WLF_ANIMATOR_JOB_TYPE_ANIMATION_GROUP,
};

static void parallel_animation_destroy_job(struct wlf_animator_job *job) {
	struct wlf_parallel_animation *animation =
		wlf_parallel_animation_from_job(job);
	wlf_animation_group_fini(&animation->base);
	if (job->owns_self) {
		free(animation);
	}
}

static void parallel_animation_update_current_time(struct wlf_animator_job *job,
		int64_t current_time_msec) {
	struct wlf_parallel_animation *animation =
		wlf_parallel_animation_from_job(job);
	struct wlf_animator_job *child;
	wlf_linked_list_for_each(child, &animation->base.children, link) {
		int64_t child_duration = wlf_animator_job_total_duration(child);
		if (child_duration >= 0 && current_time_msec > child_duration) {
			wlf_animator_job_set_current_time(child, child_duration);
		} else {
			wlf_animator_job_set_current_time(child, current_time_msec);
		}
	}
}

static const struct wlf_animator_job_impl parallel_animation_impl = {
	.destroy = parallel_animation_destroy_job,
	.update_current_time = parallel_animation_update_current_time,
};

struct wlf_parallel_animation *wlf_parallel_animation_create(void) {
	struct wlf_parallel_animation *animation = malloc(sizeof(*animation));
	if (animation == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_parallel_animation");
		return NULL;
	}

	wlf_parallel_animation_init(animation);
	animation->base.base.owns_self = true;
	return animation;
}

void wlf_parallel_animation_init(struct wlf_parallel_animation *animation) {
	wlf_animation_group_init(&animation->base, &parallel_animation_impl,
		&WLF_ANIMATOR_JOB_TYPE_PARALLEL_ANIMATION);
	animation->base.parallel_mode = true;
}

void wlf_parallel_animation_destroy(struct wlf_parallel_animation *animation) {
	wlf_animator_job_destroy(&animation->base.base);
}

bool wlf_animator_job_is_parallel_animation(const struct wlf_animator_job *job) {
	return wlf_animator_job_is_type(job,
		&WLF_ANIMATOR_JOB_TYPE_PARALLEL_ANIMATION);
}

struct wlf_parallel_animation *wlf_parallel_animation_from_job(
		struct wlf_animator_job *job) {
	assert(wlf_animator_job_is_parallel_animation(job));
	struct wlf_parallel_animation *animation = NULL;
	return wlf_container_of(job, animation, base.base);
}
