/**
 * @file        wlf_sequential_animation.h
 * @brief       Sequential animation container for wlframe.
 */

#ifndef ANIMATOR_WLF_SEQUENTIAL_ANIMATION_H
#define ANIMATOR_WLF_SEQUENTIAL_ANIMATION_H

#include "wlf/animator/wlf_animation_group.h"

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_SEQUENTIAL_ANIMATION;

struct wlf_sequential_animation {
	struct wlf_animation_group base;
};

struct wlf_sequential_animation *wlf_sequential_animation_create(void);
void wlf_sequential_animation_init(struct wlf_sequential_animation *animation);
void wlf_sequential_animation_destroy(struct wlf_sequential_animation *animation);

bool wlf_animator_job_is_sequential_animation(const struct wlf_animator_job *job);
struct wlf_sequential_animation *wlf_sequential_animation_from_job(
	struct wlf_animator_job *job);

#endif // ANIMATOR_WLF_SEQUENTIAL_ANIMATION_H
