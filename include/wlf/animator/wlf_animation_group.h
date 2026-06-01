/**
 * @file        wlf_animation_group.h
 * @brief       Base container animation for wlframe.
 */

#ifndef ANIMATOR_WLF_ANIMATION_GROUP_H
#define ANIMATOR_WLF_ANIMATION_GROUP_H

#include "wlf/animator/wlf_animator_job.h"

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_ANIMATION_GROUP;

struct wlf_animation_group {
	struct wlf_animator_job base;
	struct wlf_linked_list children;
	bool parallel_mode;
};

void wlf_animation_group_init(struct wlf_animation_group *group,
	const struct wlf_animator_job_impl *impl,
	const struct wlf_animator_job_type *type);
void wlf_animation_group_fini(struct wlf_animation_group *group);

bool wlf_animator_job_is_animation_group(const struct wlf_animator_job *job);
struct wlf_animation_group *wlf_animation_group_from_job(
	struct wlf_animator_job *job);

void wlf_animation_group_add_job(struct wlf_animation_group *group,
	struct wlf_animator_job *job);
void wlf_animation_group_remove_job(struct wlf_animator_job *job);

#endif // ANIMATOR_WLF_ANIMATION_GROUP_H
