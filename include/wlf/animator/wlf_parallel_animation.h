/**
 * @file        wlf_parallel_animation.h
 * @brief       Parallel animation container for wlframe.
 */

#ifndef ANIMATOR_WLF_PARALLEL_ANIMATION_H
#define ANIMATOR_WLF_PARALLEL_ANIMATION_H

#include "wlf/animator/wlf_animation_group.h"

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_PARALLEL_ANIMATION;

struct wlf_parallel_animation {
	struct wlf_animation_group base;
};

struct wlf_parallel_animation *wlf_parallel_animation_create(void);
void wlf_parallel_animation_init(struct wlf_parallel_animation *animation);
void wlf_parallel_animation_destroy(struct wlf_parallel_animation *animation);

bool wlf_animator_job_is_parallel_animation(const struct wlf_animator_job *job);
struct wlf_parallel_animation *wlf_parallel_animation_from_job(
	struct wlf_animator_job *job);

#endif // ANIMATOR_WLF_PARALLEL_ANIMATION_H
