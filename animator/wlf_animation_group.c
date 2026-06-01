#include "wlf/animator/wlf_animation_group.h"
#include "wlf/animator/wlf_animator.h"

#include <assert.h>
#include <limits.h>
#include <string.h>
const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_ANIMATION_GROUP = {
	.name = "wlf_animation_group",
	.parent = &WLF_ANIMATOR_JOB_TYPE_BASE,
};

static int64_t group_safe_add_duration(int64_t lhs, int64_t rhs) {
	if (lhs == INT64_MAX || rhs == INT64_MAX) {
		return INT64_MAX;
	}
	if (INT64_MAX - lhs < rhs) {
		return INT64_MAX;
	}
	return lhs + rhs;
}

static void animation_group_recalculate_duration(struct wlf_animation_group *group) {
	if (group->base.type == NULL) {
		return;
	}

	if (group->parallel_mode) {
		int64_t max_duration = 0;
		struct wlf_animator_job *child;
		wlf_linked_list_for_each(child, &group->children, link) {
			int64_t child_duration = wlf_animator_job_total_duration(child);
			if (child_duration > max_duration) {
				max_duration = child_duration;
			}
		}
		group->base.duration_msec = max_duration;
		return;
	}

	int64_t total_duration = 0;
	struct wlf_animator_job *child;
	wlf_linked_list_for_each(child, &group->children, link) {
		total_duration = group_safe_add_duration(total_duration,
			wlf_animator_job_total_duration(child));
	}
	group->base.duration_msec = total_duration;
}

void wlf_animation_group_init(struct wlf_animation_group *group,
		const struct wlf_animator_job_impl *impl,
		const struct wlf_animator_job_type *type) {
	assert(group != NULL);

	*group = (struct wlf_animation_group){0};
	wlf_animator_job_init(&group->base, impl, type);
	wlf_linked_list_init(&group->children);
	group->base.loop_count = 1;
}

void wlf_animation_group_fini(struct wlf_animation_group *group) {
	if (group == NULL) {
		return;
	}

	struct wlf_animator_job *child, *tmp;
	wlf_linked_list_for_each_safe(child, tmp, &group->children, link) {
		wlf_animation_group_remove_job(child);
		wlf_animator_job_destroy(child);
	}
}

bool wlf_animator_job_is_animation_group(const struct wlf_animator_job *job) {
	return wlf_animator_job_is_type(job, &WLF_ANIMATOR_JOB_TYPE_ANIMATION_GROUP);
}

struct wlf_animation_group *wlf_animation_group_from_job(
		struct wlf_animator_job *job) {
	assert(wlf_animator_job_is_animation_group(job));
	struct wlf_animation_group *group = NULL;
	return wlf_container_of(job, group, base);
}

void wlf_animation_group_add_job(struct wlf_animation_group *group,
		struct wlf_animator_job *job) {
	assert(group != NULL);
	assert(job != NULL);

	if (job->parent_group == group) {
		return;
	}

	wlf_animator_remove_job(job);
	if (job->parent_group != NULL) {
		wlf_animation_group_remove_job(job);
	}

	wlf_linked_list_insert(group->children.prev, &job->link);
	job->parent_group = group;
	animation_group_recalculate_duration(group);
}

void wlf_animation_group_remove_job(struct wlf_animator_job *job) {
	if (job == NULL || job->parent_group == NULL) {
		return;
	}

	struct wlf_animation_group *group = job->parent_group;
	wlf_linked_list_remove(&job->link);
	wlf_linked_list_init(&job->link);
	job->parent_group = NULL;
	animation_group_recalculate_duration(group);
}
