/**
 * @file        wlf_animator.h
 * @brief       Animation driver for wlframe.
 */

#ifndef ANIMATOR_WLF_ANIMATOR_H
#define ANIMATOR_WLF_ANIMATOR_H

#include "wlf/animator/wlf_animator_job.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_animator {
	struct wlf_linked_list jobs;
	bool has_last_tick;
	int64_t last_tick_msec;
};

struct wlf_animator *wlf_animator_create(void);
void wlf_animator_destroy(struct wlf_animator *animator);

void wlf_animator_init(struct wlf_animator *animator);
void wlf_animator_fini(struct wlf_animator *animator);

void wlf_animator_add_job(struct wlf_animator *animator,
	struct wlf_animator_job *job);
void wlf_animator_remove_job(struct wlf_animator_job *job);

void wlf_animator_tick(struct wlf_animator *animator, int64_t now_msec);
void wlf_animator_tick_now(struct wlf_animator *animator);
bool wlf_animator_has_running_jobs(const struct wlf_animator *animator);

#endif // ANIMATOR_WLF_ANIMATOR_H
