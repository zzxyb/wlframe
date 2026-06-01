#include "wlf/animator/wlf_animator.h"
#include "wlf/animator/wlf_animation_group.h"
#include "wlf/utils/wlf_time.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_animator *wlf_animator_create(void) {
	struct wlf_animator *animator = malloc(sizeof(*animator));
	if (animator == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_animator");
		return NULL;
	}

	wlf_animator_init(animator);
	return animator;
}

void wlf_animator_destroy(struct wlf_animator *animator) {
	if (animator == NULL) {
		return;
	}

	wlf_animator_fini(animator);
	free(animator);
}

void wlf_animator_init(struct wlf_animator *animator) {
	assert(animator != NULL);

	*animator = (struct wlf_animator){0};
	wlf_linked_list_init(&animator->jobs);
}

void wlf_animator_fini(struct wlf_animator *animator) {
	assert(animator != NULL);

	struct wlf_animator_job *job, *tmp;
	wlf_linked_list_for_each_safe(job, tmp, &animator->jobs, link) {
		wlf_animator_job_destroy(job);
	}

	animator->has_last_tick = false;
	wlf_linked_list_init(&animator->jobs);
}

void wlf_animator_add_job(struct wlf_animator *animator,
		struct wlf_animator_job *job) {
	assert(animator != NULL);
	assert(job != NULL);

	if (job->animator == animator) {
		return;
	}

	wlf_animator_remove_job(job);
	if (job->parent_group != NULL) {
		wlf_animation_group_remove_job(job);
	}

	wlf_linked_list_insert(animator->jobs.prev, &job->link);
	job->animator = animator;
}

void wlf_animator_remove_job(struct wlf_animator_job *job) {
	if (job == NULL || job->animator == NULL) {
		return;
	}

	wlf_linked_list_remove(&job->link);
	wlf_linked_list_init(&job->link);
	job->animator = NULL;
}

void wlf_animator_tick(struct wlf_animator *animator, int64_t now_msec) {
	assert(animator != NULL);

	if (!animator->has_last_tick) {
		animator->has_last_tick = true;
		animator->last_tick_msec = now_msec;
		return;
	}

	int64_t delta_msec = now_msec - animator->last_tick_msec;
	animator->last_tick_msec = now_msec;
	if (delta_msec <= 0) {
		return;
	}

	struct wlf_animator_job *job, *tmp;
	wlf_linked_list_for_each_safe(job, tmp, &animator->jobs, link) {
		wlf_animator_job_advance(job, delta_msec);
	}
}

void wlf_animator_tick_now(struct wlf_animator *animator) {
	wlf_animator_tick(animator, get_current_time_msec());
}

bool wlf_animator_has_running_jobs(const struct wlf_animator *animator) {
	assert(animator != NULL);

	struct wlf_animator_job *job;
	wlf_linked_list_for_each(job, &((struct wlf_animator *)animator)->jobs, link) {
		if (wlf_animator_job_is_running(job)) {
			return true;
		}
	}

	return false;
}
