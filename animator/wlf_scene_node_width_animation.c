#include "wlf/animator/wlf_scene_node_width_animation.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_SCENE_NODE_WIDTH_ANIMATION = {
	.name = "wlf_scene_node_width_animation",
	.parent = &WLF_ANIMATOR_JOB_TYPE_DOUBLE_ANIMATION,
};

static double scene_node_get_width(void *target, void *user_data) {
	struct wlf_scene_node *node = target;
	(void)user_data;
	return node->state.width;
}

static void scene_node_set_width(void *target, double value, void *user_data) {
	struct wlf_scene_node *node = target;
	(void)user_data;
	node->state.width = value < 0.0 ? 0.0 : value;
	wlf_scene_node_update(node, NULL);
}

struct wlf_scene_node_width_animation *wlf_scene_node_width_animation_create(
		struct wlf_scene_node *node) {
	struct wlf_scene_node_width_animation *animation = malloc(sizeof(*animation));
	if (animation == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_scene_node_width_animation");
		return NULL;
	}

	wlf_scene_node_width_animation_init(animation, node);
	animation->base.base.owns_self = true;
	return animation;
}

void wlf_scene_node_width_animation_init(
		struct wlf_scene_node_width_animation *animation,
		struct wlf_scene_node *node) {
	wlf_double_animation_init_with_type(&animation->base,
		&WLF_ANIMATOR_JOB_TYPE_SCENE_NODE_WIDTH_ANIMATION);
	animation->node = node;
	wlf_double_animation_set_binding(&animation->base,
		&(struct wlf_double_animation_binding){
			.target = node,
			.get = scene_node_get_width,
			.set = scene_node_set_width,
		});
}

void wlf_scene_node_width_animation_destroy(
		struct wlf_scene_node_width_animation *animation) {
	wlf_animator_job_destroy(&animation->base.base);
}

bool wlf_animator_job_is_scene_node_width_animation(
		const struct wlf_animator_job *job) {
	return wlf_animator_job_is_type(job,
		&WLF_ANIMATOR_JOB_TYPE_SCENE_NODE_WIDTH_ANIMATION);
}

struct wlf_scene_node_width_animation *wlf_scene_node_width_animation_from_job(
		struct wlf_animator_job *job) {
	assert(wlf_animator_job_is_scene_node_width_animation(job));
	struct wlf_scene_node_width_animation *animation = NULL;
	return wlf_container_of(job, animation, base.base);
}
