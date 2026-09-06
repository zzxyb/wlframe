#include "wlf/animation/wlf_sequential_animation_group.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

static struct wlf_animation *sequential_first_animation(
		struct wlf_sequential_animation_group *sequential) {
	if (wlf_linked_list_empty(&sequential->base.childrens)) {
		return NULL;
	}

	struct wlf_animation *child;
	return wlf_container_of(sequential->base.childrens.next, child, link);
}

static struct wlf_animation *sequential_next_animation(
		struct wlf_sequential_animation_group *sequential,
		struct wlf_animation *animation) {
	if (animation->link.next == &sequential->base.childrens) {
		return NULL;
	}

	struct wlf_animation *child;
	return wlf_container_of(animation->link.next, child, link);
}

static void sequential_start_current(
		struct wlf_sequential_animation_group *sequential) {
	if (sequential->current != NULL) {
		wlf_animation_start(sequential->current);
	}
}

static void sequential_start(struct wlf_animation_group *group) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		if (child->state != WLF_ANIMATION_STATE_STOPPED) {
			wlf_animation_stop(child);
		}
	}

	sequential->current = sequential_first_animation(sequential);
	sequential_start_current(sequential);
	if (sequential->current == NULL) {
		group->state = WLF_ANIMATION_STATE_STOPPED;
	}
}

static void sequential_stop(struct wlf_animation_group *group) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_stop(child);
	}
	sequential->current = NULL;
}

static void sequential_pause(struct wlf_animation_group *group) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);

	if (sequential->current != NULL) {
		wlf_animation_pause(sequential->current);
	}
}

static void sequential_resume(struct wlf_animation_group *group) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);

	if (sequential->current != NULL) {
		wlf_animation_resume(sequential->current);
	}
}

static void sequential_update(struct wlf_animation_group *group, int64_t dt) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);

	while (sequential->current != NULL) {
		dt = wlf_animation_group_advance_animation(
			sequential->current, dt);
		if (sequential->current->state != WLF_ANIMATION_STATE_STOPPED) {
			return;
		}

		sequential->current = sequential_next_animation(
			sequential, sequential->current);
		sequential_start_current(sequential);
		if (dt <= 0 && sequential->current != NULL &&
				sequential->current->duration > 0) {
			return;
		}
	}

	group->state = WLF_ANIMATION_STATE_STOPPED;
}

static void sequential_destroy(struct wlf_animation_group *group) {
	struct wlf_sequential_animation_group *sequential =
		wlf_sequential_animation_group_from_group(group);

	free(sequential);
}

static const struct wlf_animation_group_impl sequential_impl = {
	.start = sequential_start,
	.stop = sequential_stop,
	.pause = sequential_pause,
	.resume = sequential_resume,
	.update = sequential_update,
	.destroy = sequential_destroy,
};

struct wlf_sequential_animation_group *wlf_sequential_animation_group_create(void) {
	struct wlf_sequential_animation_group *sequential =
		malloc(sizeof(*sequential));
	if (sequential == NULL) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_sequential_animation_group");
		return NULL;
	}

	wlf_animation_group_init(&sequential->base, &sequential_impl);
	sequential->current = NULL;

	return sequential;
}

bool wlf_animation_group_is_sequential(const struct wlf_animation_group *group) {
	return group != NULL && group->impl == &sequential_impl;
}

struct wlf_sequential_animation_group *wlf_sequential_animation_group_from_group(
		struct wlf_animation_group *group) {
	assert(group && group->impl == &sequential_impl);

	struct wlf_sequential_animation_group *sequential =
		wlf_container_of(group, sequential, base);

	return sequential;
}
