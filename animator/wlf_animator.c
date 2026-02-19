#include "wlf/animator/wlf_animator.h"
#include "wlf/animator/wlf_curve_linear.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void wlf_animator_init(struct wlf_animator *animator,
		const struct wlf_animator_impl *impl) {
	assert(impl);
	assert(impl->destroy);
	assert(impl->start);
	assert(impl->stop);
	assert(impl->pause);

	animator->impl = impl;
	animator->state = WLF_ANIMATOR_STATE_STOPPED;
	animator->duration = 0;
	animator->current_time = 0;
	animator->loop_count = WLF_ANIMATOR_LOOP_ONCE;
	animator->current_loop = 0;
	animator->direction = WLF_ANIMATOR_DIRECTION_FORWARD;
	animator->alternate_reverse = false;

	wlf_signal_init(&animator->events.destroy);
	wlf_signal_init(&animator->events.started);
	wlf_signal_init(&animator->events.stopped);
	wlf_signal_init(&animator->events.finished);
	wlf_signal_init(&animator->events.paused);
	wlf_signal_init(&animator->events.resumed);
	wlf_signal_init(&animator->events.updated);

	animator->curve = wlf_curve_linear_create();
}

void wlf_animator_destroy(struct wlf_animator *animator) {
	if (animator == NULL) {
		return;
	}

	if (animator->curve) {
		wlf_curve_destroy(animator->curve);
		animator->curve = NULL;
	}

	wlf_signal_emit_mutable(&animator->events.destroy, animator);

	assert(wlf_linked_list_empty(&animator->events.destroy.listener_list));
	assert(wlf_linked_list_empty(&animator->events.started.listener_list));
	assert(wlf_linked_list_empty(&animator->events.stopped.listener_list));
	assert(wlf_linked_list_empty(&animator->events.finished.listener_list));
	assert(wlf_linked_list_empty(&animator->events.paused.listener_list));
	assert(wlf_linked_list_empty(&animator->events.resumed.listener_list));
	assert(wlf_linked_list_empty(&animator->events.updated.listener_list));

	if (animator->impl && animator->impl->destroy) {
		animator->impl->destroy(animator);
	} else {
		free(animator);
	}
}

void wlf_animator_start(struct wlf_animator *animator) {
	assert(animator);

	animator->current_time = 0;
	animator->current_loop = 0;
	animator->alternate_reverse = false;
	animator->state = WLF_ANIMATOR_STATE_RUNNING;

	if (animator->impl && animator->impl->pre_sync) {
		animator->impl->pre_sync(animator);
	}

	if (animator->impl && animator->impl->start) {
		animator->impl->start(animator);
	}

	wlf_signal_emit_mutable(&animator->events.started, animator);
}

void wlf_animator_stop(struct wlf_animator *animator) {
	assert(animator);
	
	if (animator->state == WLF_ANIMATOR_STATE_STOPPED) {
		return;
	}

	animator->state = WLF_ANIMATOR_STATE_STOPPED;
	animator->current_time = 0;
	animator->current_loop = 0;

	if (animator->impl && animator->impl->stop) {
		animator->impl->stop(animator);
	}

	if (animator->impl && animator->impl->post_sync) {
		animator->impl->post_sync(animator);
	}

	wlf_signal_emit_mutable(&animator->events.stopped, animator);
}

void wlf_animator_pause(struct wlf_animator *animator) {
	assert(animator);
	
	if (animator->state != WLF_ANIMATOR_STATE_RUNNING) {
		return;
	}

	animator->state = WLF_ANIMATOR_STATE_PAUSED;
	animator->pause_time = animator->current_time;
	
	if (animator->impl && animator->impl->pause) {
		animator->impl->pause(animator);
	}

	wlf_signal_emit_mutable(&animator->events.paused, animator);
}

void wlf_animator_resume(struct wlf_animator *animator) {
	assert(animator);
	
	if (animator->state != WLF_ANIMATOR_STATE_PAUSED) {
		return;
	}

	animator->state = WLF_ANIMATOR_STATE_RUNNING;
	
	if (animator->impl && animator->impl->resume) {
		animator->impl->resume(animator);
	}

	wlf_signal_emit_mutable(&animator->events.resumed, animator);
}

void wlf_animator_update(struct wlf_animator *animator, int64_t dt) {
	assert(animator);

	if (animator->state != WLF_ANIMATOR_STATE_RUNNING) {
		return;
	}

	if (animator->duration <= 0) {
		return;
	}

	animator->current_time += dt;
	bool loop_complete = false;
	if (animator->current_time >= animator->duration) {
		animator->current_time = animator->duration;
		loop_complete = true;
	}

	if (animator->impl && animator->impl->update) {
		animator->impl->update(animator, dt);
	}

	if (animator->impl && animator->impl->write_back) {
		animator->impl->write_back(animator);
	}

	wlf_signal_emit_mutable(&animator->events.updated, animator);

	if (loop_complete) {
		animator->current_loop++;
		bool should_continue = (animator->loop_count == WLF_ANIMATOR_LOOP_INFINITE) ||
			(animator->current_loop < animator->loop_count);

		if (should_continue) {
			if (animator->direction == WLF_ANIMATOR_DIRECTION_ALTERNATE) {
				animator->alternate_reverse = !animator->alternate_reverse;
			}

			animator->current_time = 0;
		} else {
			animator->state = WLF_ANIMATOR_STATE_STOPPED;
			
			if (animator->impl && animator->impl->post_sync) {
				animator->impl->post_sync(animator);
			}

			wlf_signal_emit_mutable(&animator->events.finished, animator);
		}
	}
}

void wlf_animator_set_duration(struct wlf_animator *animator, int64_t duration) {
	assert(animator);

	animator->duration = duration;
}

void wlf_animator_set_curve(struct wlf_animator *animator,
		struct wlf_curve *curve) {
	assert(animator);

	if (animator->curve) {
		wlf_curve_destroy(animator->curve);
	}

	animator->curve = curve;
}

void wlf_animator_set_loop_count(struct wlf_animator *animator, int count) {
	assert(animator);

	animator->loop_count = count;
}

void wlf_animator_set_direction(struct wlf_animator *animator,
		enum wlf_animator_direction direction) {
	assert(animator);

	animator->direction = direction;
}

float wlf_animator_get_progress(const struct wlf_animator *animator) {
	assert(animator);
	if (animator->duration <= 0) {
		return 0.0f;
	}

	float t = (float)animator->current_time / (float)animator->duration;
	bool reverse = (animator->direction == WLF_ANIMATOR_DIRECTION_BACKWARD) ||
		(animator->direction == WLF_ANIMATOR_DIRECTION_ALTERNATE && animator->alternate_reverse);

	if (reverse) {
		t = 1.0f - t;
	}

	return wlf_curve_value_at(animator->curve, t);
}
