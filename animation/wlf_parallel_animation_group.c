#include "wlf/animation/wlf_parallel_animation_group.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

static void parallel_start(struct wlf_animation_group *group) {
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		if (child->state != WLF_ANIMATION_STATE_STOPPED) {
			wlf_animation_stop(child);
		}
		wlf_animation_start(child);
	}

	if (wlf_linked_list_empty(&group->childrens)) {
		group->state = WLF_ANIMATION_STATE_STOPPED;
	}
}

static void parallel_stop(struct wlf_animation_group *group) {
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_stop(child);
	}
}

static void parallel_pause(struct wlf_animation_group *group) {
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_pause(child);
	}
}

static void parallel_resume(struct wlf_animation_group *group) {
	struct wlf_animation *child, *tmp;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_resume(child);
	}
}

static void parallel_update(struct wlf_animation_group *group, int64_t dt) {
	struct wlf_animation *child, *tmp;
	bool finished = true;

	wlf_linked_list_for_each_safe(child, tmp, &group->childrens, link) {
		wlf_animation_group_advance_animation(child, dt);
		if (child->state != WLF_ANIMATION_STATE_STOPPED) {
			finished = false;
		}
	}

	if (finished) {
		group->state = WLF_ANIMATION_STATE_STOPPED;
	}
}

static void parallel_destroy(struct wlf_animation_group *group) {
	struct wlf_parallel_animation_group *parallel =
		wlf_parallel_animation_group_from_group(group);

	free(parallel);
}

static const struct wlf_animation_group_impl parallel_impl = {
	.start = parallel_start,
	.stop = parallel_stop,
	.pause = parallel_pause,
	.resume = parallel_resume,
	.update = parallel_update,
	.destroy = parallel_destroy,
};

struct wlf_parallel_animation_group *wlf_parallel_animation_group_create(void) {
	struct wlf_parallel_animation_group *parallel = malloc(sizeof(*parallel));
	if (parallel == NULL) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_parallel_animation_group");
		return NULL;
	}

	wlf_animation_group_init(&parallel->base, &parallel_impl);

	return parallel;
}

bool wlf_animation_group_is_parallel(const struct wlf_animation_group *group) {
	return group != NULL && group->impl == &parallel_impl;
}

struct wlf_parallel_animation_group *wlf_parallel_animation_group_from_group(
		struct wlf_animation_group *group) {
	assert(group && group->impl == &parallel_impl);

	struct wlf_parallel_animation_group *parallel =
		wlf_container_of(group, parallel, base);

	return parallel;
}
