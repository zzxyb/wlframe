/**
 * @file        wlf_animator_job.h
 * @brief       Base animation job interface for wlframe.
 */

#ifndef ANIMATOR_WLF_ANIMATOR_JOB_H
#define ANIMATOR_WLF_ANIMATOR_JOB_H

#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stdint.h>

struct wlf_animator;
struct wlf_animation_group;
struct wlf_animator_job;

enum wlf_animator_job_state {
	WLF_ANIMATOR_JOB_STOPPED,
	WLF_ANIMATOR_JOB_PAUSED,
	WLF_ANIMATOR_JOB_RUNNING,
};

enum wlf_animator_direction {
	WLF_ANIMATOR_FORWARD = 1,
	WLF_ANIMATOR_BACKWARD = -1,
};

struct wlf_animator_job_type {
	const char *name;
	const struct wlf_animator_job_type *parent;
};

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_BASE;

struct wlf_animator_job_impl {
	void (*destroy)(struct wlf_animator_job *job);
	void (*update_current_time)(struct wlf_animator_job *job,
		int64_t current_time_msec);
	void (*state_changed)(struct wlf_animator_job *job,
		enum wlf_animator_job_state new_state,
		enum wlf_animator_job_state old_state);
};

struct wlf_animator_job {
	const struct wlf_animator_job_impl *impl;
	const struct wlf_animator_job_type *type;

	struct wlf_animator *animator;
	struct wlf_animation_group *parent_group;
	struct wlf_linked_list link;
	bool owns_self;

	int64_t duration_msec;
	int32_t loop_count;
	int32_t current_loop;
	int64_t current_time_msec;
	int64_t current_loop_time_msec;
	float current_progress;

	enum wlf_animator_job_state state;
	enum wlf_animator_direction direction;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal state_changed;
		struct wlf_signal finished;
	} events;
};

struct wlf_animator_job_state_event {
	struct wlf_animator_job *job;
	enum wlf_animator_job_state new_state;
	enum wlf_animator_job_state old_state;
};

void wlf_animator_job_init(struct wlf_animator_job *job,
	const struct wlf_animator_job_impl *impl,
	const struct wlf_animator_job_type *type);
void wlf_animator_job_destroy(struct wlf_animator_job *job);

void wlf_animator_job_set_duration(struct wlf_animator_job *job,
	int64_t duration_msec);
void wlf_animator_job_set_loop_count(struct wlf_animator_job *job,
	int32_t loop_count);
void wlf_animator_job_set_direction(struct wlf_animator_job *job,
	enum wlf_animator_direction direction);
void wlf_animator_job_set_current_time(struct wlf_animator_job *job,
	int64_t current_time_msec);

void wlf_animator_job_start(struct wlf_animator_job *job);
void wlf_animator_job_stop(struct wlf_animator_job *job);
void wlf_animator_job_pause(struct wlf_animator_job *job);
void wlf_animator_job_resume(struct wlf_animator_job *job);

bool wlf_animator_job_is_running(const struct wlf_animator_job *job);
bool wlf_animator_job_is_finished(const struct wlf_animator_job *job);
bool wlf_animator_job_is_type(const struct wlf_animator_job *job,
	const struct wlf_animator_job_type *type);

int64_t wlf_animator_job_total_duration(const struct wlf_animator_job *job);
void wlf_animator_job_advance(struct wlf_animator_job *job,
	int64_t delta_msec);

#endif // ANIMATOR_WLF_ANIMATOR_JOB_H
