#include "wlf/animator/wlf_animator_job.h"
#include "wlf/animator/wlf_animator.h"
#include "wlf/animator/wlf_animation_group.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_BASE = {
	.name = "wlf_animator_job",
	.parent = NULL,
};

static int64_t clamp_i64(int64_t value, int64_t min_value, int64_t max_value) {
	if (value < min_value) {
		return min_value;
	}
	if (value > max_value) {
		return max_value;
	}
	return value;
}

static void animator_job_emit_state_change(struct wlf_animator_job *job,
		enum wlf_animator_job_state old_state) {
	if (job->state == old_state) {
		return;
	}

	struct wlf_animator_job_state_event event = {
		.job = job,
		.new_state = job->state,
		.old_state = old_state,
	};
	wlf_signal_emit_mutable(&job->events.state_changed, &event);

	if (job->impl->state_changed != NULL) {
		job->impl->state_changed(job, job->state, old_state);
	}
}

static void animator_job_apply_time(struct wlf_animator_job *job,
		int64_t current_time_msec) {
	int64_t duration = job->duration_msec;
	int64_t total_duration = wlf_animator_job_total_duration(job);
	int64_t clamped_time = current_time_msec;

	if (job->loop_count >= 0) {
		clamped_time = clamp_i64(current_time_msec, 0, total_duration);
	} else if (current_time_msec < 0) {
		clamped_time = 0;
	}

	job->current_time_msec = clamped_time;

	if (duration <= 0) {
		job->current_loop = 0;
		job->current_loop_time_msec = 0;
		job->current_progress = clamped_time > 0 ? 1.0f : 0.0f;
	} else if (job->loop_count >= 0 && clamped_time == total_duration &&
			total_duration > 0) {
		job->current_loop = job->loop_count - 1;
		job->current_loop_time_msec = duration;
		job->current_progress = 1.0f;
	} else {
		job->current_loop = (int32_t)(clamped_time / duration);
		job->current_loop_time_msec = clamped_time % duration;
		job->current_progress =
			(float)job->current_loop_time_msec / (float)duration;
	}

	if (job->impl->update_current_time != NULL) {
		job->impl->update_current_time(job, job->current_loop_time_msec);
	}
}

void wlf_animator_job_init(struct wlf_animator_job *job,
		const struct wlf_animator_job_impl *impl,
		const struct wlf_animator_job_type *type) {
	assert(job != NULL);
	assert(impl != NULL);
	assert(impl->destroy != NULL);
	assert(type != NULL);

	*job = (struct wlf_animator_job){
		.impl = impl,
		.type = type,
		.duration_msec = 250,
		.loop_count = 1,
		.direction = WLF_ANIMATOR_FORWARD,
		.state = WLF_ANIMATOR_JOB_STOPPED,
	};

	wlf_linked_list_init(&job->link);
	wlf_signal_init(&job->events.destroy);
	wlf_signal_init(&job->events.state_changed);
	wlf_signal_init(&job->events.finished);
}

void wlf_animator_job_destroy(struct wlf_animator_job *job) {
	if (job == NULL) {
		return;
	}

	wlf_animator_remove_job(job);
	if (job->parent_group != NULL) {
		wlf_animation_group_remove_job(job);
	}

	wlf_signal_emit_mutable(&job->events.destroy, job);

	assert(wlf_linked_list_empty(&job->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&job->events.state_changed.listener_list));
	assert(wlf_linked_list_empty(&job->events.finished.listener_list));

	job->impl->destroy(job);
}

void wlf_animator_job_set_duration(struct wlf_animator_job *job,
		int64_t duration_msec) {
	assert(job != NULL);
	job->duration_msec = duration_msec < 0 ? 0 : duration_msec;
}

void wlf_animator_job_set_loop_count(struct wlf_animator_job *job,
		int32_t loop_count) {
	assert(job != NULL);
	job->loop_count = loop_count == 0 ? 1 : loop_count;
}

void wlf_animator_job_set_direction(struct wlf_animator_job *job,
		enum wlf_animator_direction direction) {
	assert(job != NULL);
	job->direction = direction == WLF_ANIMATOR_BACKWARD ?
		WLF_ANIMATOR_BACKWARD : WLF_ANIMATOR_FORWARD;
}

void wlf_animator_job_set_current_time(struct wlf_animator_job *job,
		int64_t current_time_msec) {
	assert(job != NULL);
	animator_job_apply_time(job, current_time_msec);
}

void wlf_animator_job_start(struct wlf_animator_job *job) {
	assert(job != NULL);

	enum wlf_animator_job_state old_state = job->state;
	job->state = WLF_ANIMATOR_JOB_RUNNING;

	if (job->duration_msec <= 0) {
		job->current_time_msec = 0;
		job->current_loop = 0;
		job->current_loop_time_msec = 0;
		job->current_progress = job->direction == WLF_ANIMATOR_BACKWARD ?
			0.0f : 1.0f;
		if (job->impl->update_current_time != NULL) {
			job->impl->update_current_time(job, job->current_loop_time_msec);
		}
		animator_job_emit_state_change(job, old_state);
		job->state = WLF_ANIMATOR_JOB_STOPPED;
		animator_job_emit_state_change(job, WLF_ANIMATOR_JOB_RUNNING);
		wlf_signal_emit_mutable(&job->events.finished, job);
		return;
	}

	if (job->direction == WLF_ANIMATOR_BACKWARD) {
		animator_job_apply_time(job, wlf_animator_job_total_duration(job));
	} else {
		animator_job_apply_time(job, 0);
	}

	animator_job_emit_state_change(job, old_state);
}

void wlf_animator_job_stop(struct wlf_animator_job *job) {
	assert(job != NULL);

	enum wlf_animator_job_state old_state = job->state;
	job->state = WLF_ANIMATOR_JOB_STOPPED;
	animator_job_emit_state_change(job, old_state);
}

void wlf_animator_job_pause(struct wlf_animator_job *job) {
	assert(job != NULL);
	if (job->state != WLF_ANIMATOR_JOB_RUNNING) {
		return;
	}

	enum wlf_animator_job_state old_state = job->state;
	job->state = WLF_ANIMATOR_JOB_PAUSED;
	animator_job_emit_state_change(job, old_state);
}

void wlf_animator_job_resume(struct wlf_animator_job *job) {
	assert(job != NULL);
	if (job->state != WLF_ANIMATOR_JOB_PAUSED) {
		return;
	}

	enum wlf_animator_job_state old_state = job->state;
	job->state = WLF_ANIMATOR_JOB_RUNNING;
	animator_job_emit_state_change(job, old_state);
}

bool wlf_animator_job_is_running(const struct wlf_animator_job *job) {
	return job != NULL && job->state == WLF_ANIMATOR_JOB_RUNNING;
}

bool wlf_animator_job_is_finished(const struct wlf_animator_job *job) {
	if (job == NULL) {
		return true;
	}

	if (job->direction == WLF_ANIMATOR_BACKWARD) {
		return job->current_time_msec <= 0;
	}
	if (job->loop_count < 0) {
		return false;
	}
	return job->current_time_msec >= wlf_animator_job_total_duration(job);
}

bool wlf_animator_job_is_type(const struct wlf_animator_job *job,
		const struct wlf_animator_job_type *type) {
	if (job == NULL || type == NULL) {
		return false;
	}

	for (const struct wlf_animator_job_type *cursor = job->type;
			cursor != NULL; cursor = cursor->parent) {
		if (cursor == type) {
			return true;
		}
	}
	return false;
}

int64_t wlf_animator_job_total_duration(const struct wlf_animator_job *job) {
	assert(job != NULL);

	if (job->loop_count < 0) {
		return INT64_MAX;
	}
	if (job->duration_msec <= 0) {
		return 0;
	}
	return job->duration_msec * job->loop_count;
}

void wlf_animator_job_advance(struct wlf_animator_job *job,
		int64_t delta_msec) {
	assert(job != NULL);

	if (job->state != WLF_ANIMATOR_JOB_RUNNING || delta_msec <= 0) {
		return;
	}

	int64_t next_time = job->direction == WLF_ANIMATOR_BACKWARD ?
		job->current_time_msec - delta_msec :
		job->current_time_msec + delta_msec;

	animator_job_apply_time(job, next_time);
	if (!wlf_animator_job_is_finished(job)) {
		return;
	}

	job->state = WLF_ANIMATOR_JOB_STOPPED;
	animator_job_emit_state_change(job, WLF_ANIMATOR_JOB_RUNNING);
	wlf_signal_emit_mutable(&job->events.finished, job);
}
