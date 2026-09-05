#include "wlf/animation/wlf_animation.h"
#include "wlf/curve/wlf_curve_linear.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void wlf_animation_init(struct wlf_animation *animation,
		const struct wlf_animation_impl *impl) {
	assert(impl);
	assert(impl->destroy);
	assert(impl->start);
	assert(impl->stop);
	assert(impl->pause);

	*animation = (struct wlf_animation) {
		.impl = impl,
		.listener = NULL,
		.user_data = NULL,
		.state = WLF_ANIMATION_STATE_STOPPED,
		.duration = 0,
		.current_time = 0,
		.loop_count = WLF_ANIMATION_LOOP_ONCE,
		.current_loop = 0,
		.direction = WLF_ANIMATION_DIRECTION_FORWARD,
		.alternate_reverse = false,
	};

	struct wlf_curve_linear *curve = wlf_curve_linear_create();
	animation->curve = &curve->base;
	wlf_linked_list_init(&animation->link);
}

void wlf_animation_destroy(struct wlf_animation *animation) {
	if (animation == NULL) {
		return;
	}

	if (animation->curve) {
		wlf_curve_destroy(animation->curve);
		animation->curve = NULL;
	}

	if (animation->listener != NULL && animation->listener->destroy != NULL) {
		animation->listener->destroy(animation->user_data, animation);
	}

	wlf_linked_list_remove(&animation->link);
	if (animation->impl && animation->impl->destroy) {
		animation->impl->destroy(animation);
	} else {
		free(animation);
	}
}

void wlf_animation_add_listener(struct wlf_animation *animation,
		const struct wlf_animation_listener *listener, void *data) {
	animation->listener = listener;
	animation->user_data = data;
}

void wlf_animation_start(struct wlf_animation *animation) {
	animation->current_time = 0;
	animation->current_loop = 0;
	animation->alternate_reverse = false;
	animation->state = WLF_ANIMATION_STATE_RUNNING;

	if (animation->impl && animation->impl->pre_sync) {
		animation->impl->pre_sync(animation);
	}

	if (animation->impl && animation->impl->start) {
		animation->impl->start(animation);
	}

	if (animation->listener != NULL && animation->listener->started != NULL) {
		animation->listener->started(animation->user_data, animation);
	}
}

void wlf_animation_stop(struct wlf_animation *animation) {
	if (animation->state == WLF_ANIMATION_STATE_STOPPED) {
		return;
	}

	animation->state = WLF_ANIMATION_STATE_STOPPED;
	animation->current_time = 0;
	animation->current_loop = 0;

	if (animation->impl && animation->impl->stop) {
		animation->impl->stop(animation);
	}

	if (animation->impl && animation->impl->post_sync) {
		animation->impl->post_sync(animation);
	}

	if (animation->listener != NULL && animation->listener->stopped != NULL) {
		animation->listener->stopped(animation->user_data, animation);
	}
}

void wlf_animation_pause(struct wlf_animation *animation) {
	if (animation->state != WLF_ANIMATION_STATE_RUNNING) {
		return;
	}

	animation->state = WLF_ANIMATION_STATE_PAUSED;
	animation->pause_time = animation->current_time;
	
	if (animation->impl && animation->impl->pause) {
		animation->impl->pause(animation);
	}

	if (animation->listener != NULL && animation->listener->paused != NULL) {
		animation->listener->paused(animation->user_data, animation);
	}
}

void wlf_animation_resume(struct wlf_animation *animation) {
	if (animation->state != WLF_ANIMATION_STATE_PAUSED) {
		return;
	}

	animation->state = WLF_ANIMATION_STATE_RUNNING;
	
	if (animation->impl && animation->impl->resume) {
		animation->impl->resume(animation);
	}

	if (animation->listener != NULL && animation->listener->resumed != NULL) {
		animation->listener->resumed(animation->user_data, animation);
	}
}

void wlf_animation_update(struct wlf_animation *animation, int64_t dt) {
	if (animation->state != WLF_ANIMATION_STATE_RUNNING) {
		return;
	}

	if (animation->duration < 0 || dt < 0) {
		return;
	}

	int64_t remaining = animation->current_time < animation->duration ?
		animation->duration - animation->current_time : 0;
	int64_t elapsed = dt < remaining ? dt : remaining;
	animation->current_time += elapsed;
	bool loop_complete = animation->current_time >= animation->duration;

	if (animation->impl && animation->impl->update) {
		animation->impl->update(animation, elapsed);
	}

	if (animation->impl && animation->impl->write_back) {
		animation->impl->write_back(animation);
	}

	if (loop_complete) {
		animation->current_loop++;
		bool should_continue = (animation->loop_count == WLF_ANIMATION_LOOP_INFINITE) ||
			(animation->current_loop < animation->loop_count);

		if (should_continue) {
			if (animation->direction == WLF_ANIMATION_DIRECTION_ALTERNATE) {
				animation->alternate_reverse = !animation->alternate_reverse;
			}

			animation->current_time = 0;
		} else {
			animation->state = WLF_ANIMATION_STATE_STOPPED;
			
			if (animation->impl && animation->impl->post_sync) {
				animation->impl->post_sync(animation);
			}

			if (animation->listener != NULL && animation->listener->finished != NULL) {
				animation->listener->finished(animation->user_data, animation);
			}
		}
	}
}

void wlf_animation_set_duration(struct wlf_animation *animation, int64_t duration) {
	animation->duration = duration;
}

void wlf_animation_set_curve(struct wlf_animation *animation,
		struct wlf_curve *curve) {
	if (animation->curve) {
		wlf_curve_destroy(animation->curve);
	}

	animation->curve = curve;
}

void wlf_animation_set_loop_count(struct wlf_animation *animation, int count) {
	animation->loop_count = count;
}

void wlf_animation_set_direction(struct wlf_animation *animation,
		enum wlf_animation_direction direction) {
	animation->direction = direction;
}

float wlf_animation_get_progress(const struct wlf_animation *animation) {
	if (animation->duration <= 0) {
		return 0.0f;
	}

	float t = (float)animation->current_time / (float)animation->duration;
	bool reverse = (animation->direction == WLF_ANIMATION_DIRECTION_BACKWARD) ||
		(animation->direction == WLF_ANIMATION_DIRECTION_ALTERNATE && animation->alternate_reverse);

	if (reverse) {
		t = 1.0f - t;
	}

	return wlf_curve_value_at(animation->curve, t);
}
