#include "wlf/animation/wlf_animation_group.h"

#include <assert.h>

static bool animation_group_contains(
		const struct wlf_animation_group *group,
		const struct wlf_animation *animation) {
	const struct wlf_animation *child;

	wlf_linked_list_for_each(child, &group->childrens, link) {
		if (child == animation) {
			return true;
		}
	}

	return false;
}

int64_t wlf_animation_group_advance_animation(
		struct wlf_animation *animation, int64_t dt) {
	while (animation->state == WLF_ANIMATION_STATE_RUNNING) {
		if (animation->duration < 0) {
			wlf_animation_stop(animation);
			return dt;
		}
		if (animation->duration == 0) {
			int current_loop = animation->current_loop;
			wlf_animation_update(animation, 0);
			if (animation->loop_count == WLF_ANIMATION_LOOP_INFINITE) {
				return dt;
			}
			if (animation->state == WLF_ANIMATION_STATE_RUNNING &&
					animation->current_loop == current_loop) {
				return dt;
			}
			continue;
		}
		if (dt <= 0) {
			return dt;
		}

		int64_t remaining = animation->duration - animation->current_time;
		int64_t step = dt < remaining ? dt : remaining;
		wlf_animation_update(animation, step);
		dt -= step;
	}

	return dt;
}

void wlf_animation_group_init(struct wlf_animation_group *group,
		const struct wlf_animation_group_impl *impl) {
	assert(group);
	assert(impl);
	assert(impl->start);
	assert(impl->stop);
	assert(impl->pause);
	assert(impl->update);
	assert(impl->destroy);

	*group = (struct wlf_animation_group) {
		.impl = impl,
		.listener = NULL,
		.user_data = NULL,
		.state = WLF_ANIMATION_STATE_STOPPED,
	};
	wlf_linked_list_init(&group->childrens);
}

void wlf_animation_group_destroy(struct wlf_animation_group *group) {
	if (group == NULL) {
		return;
	}

	struct wlf_animation *child, *tmp;
	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_destroy(child);
	}

	if (group->listener != NULL && group->listener->destroy != NULL) {
		group->listener->destroy(group->user_data, group);
	}

	group->impl->destroy(group);
}

void wlf_animation_group_add_listener(struct wlf_animation_group *group,
		const struct wlf_animation_group_listener *listener, void *data) {
	group->listener = listener;
	group->user_data = data;
}

void wlf_animation_group_start(struct wlf_animation_group *group) {
	group->state = WLF_ANIMATION_STATE_RUNNING;
	group->impl->start(group);

	if (group->listener != NULL && group->listener->started != NULL) {
		group->listener->started(group->user_data, group);
	}
}

void wlf_animation_group_stop(struct wlf_animation_group *group) {
	if (group->state == WLF_ANIMATION_STATE_STOPPED) {
		return;
	}

	group->state = WLF_ANIMATION_STATE_STOPPED;
	group->impl->stop(group);

	if (group->listener != NULL && group->listener->stopped != NULL) {
		group->listener->stopped(group->user_data, group);
	}
}

void wlf_animation_group_pause(struct wlf_animation_group *group) {
	if (group->state != WLF_ANIMATION_STATE_RUNNING) {
		return;
	}

	group->state = WLF_ANIMATION_STATE_PAUSED;
	group->impl->pause(group);

	if (group->listener != NULL && group->listener->paused != NULL) {
		group->listener->paused(group->user_data, group);
	}
}

void wlf_animation_group_resume(struct wlf_animation_group *group) {
	if (group->state != WLF_ANIMATION_STATE_PAUSED) {
		return;
	}

	group->state = WLF_ANIMATION_STATE_RUNNING;
	if (group->impl->resume != NULL) {
		group->impl->resume(group);
	}

	if (group->listener != NULL && group->listener->resumed != NULL) {
		group->listener->resumed(group->user_data, group);
	}
}

void wlf_animation_group_update(struct wlf_animation_group *group, int64_t dt) {
	if (group->state != WLF_ANIMATION_STATE_RUNNING || dt < 0) {
		return;
	}

	group->impl->update(group, dt);
}

bool wlf_animation_group_add_animation(struct wlf_animation_group *group,
		struct wlf_animation *animation) {
	if (group->state != WLF_ANIMATION_STATE_STOPPED ||
			animation->state != WLF_ANIMATION_STATE_STOPPED) {
		return false;
	}
	if (animation->link.prev != &animation->link ||
			animation->link.next != &animation->link) {
		return false;
	}

	wlf_linked_list_insert(group->childrens.prev, &animation->link);
	return true;
}

bool wlf_animation_group_remove_animation(struct wlf_animation_group *group,
		struct wlf_animation *animation) {
	if (group->state != WLF_ANIMATION_STATE_STOPPED ||
			!animation_group_contains(group, animation)) {
		return false;
	}

	wlf_linked_list_remove(&animation->link);
	wlf_linked_list_init(&animation->link);
	return true;
}

int wlf_animation_group_animation_count(
		const struct wlf_animation_group *group) {
	return wlf_linked_list_length(&group->childrens);
}
