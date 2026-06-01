/**
 * @file        wlf_scene_node_width_animation.h
 * @brief       Width animation binding for scene nodes.
 */

#ifndef ANIMATOR_WLF_SCENE_NODE_WIDTH_ANIMATION_H
#define ANIMATOR_WLF_SCENE_NODE_WIDTH_ANIMATION_H

#include "wlf/animator/wlf_double_animation.h"
#include "wlf/scene/wlf_scene_node.h"

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_SCENE_NODE_WIDTH_ANIMATION;

struct wlf_scene_node_width_animation {
	struct wlf_double_animation base;
	struct wlf_scene_node *node;
};

struct wlf_scene_node_width_animation *wlf_scene_node_width_animation_create(
	struct wlf_scene_node *node);
void wlf_scene_node_width_animation_init(
	struct wlf_scene_node_width_animation *animation,
	struct wlf_scene_node *node);
void wlf_scene_node_width_animation_destroy(
	struct wlf_scene_node_width_animation *animation);

bool wlf_animator_job_is_scene_node_width_animation(const struct wlf_animator_job *job);
struct wlf_scene_node_width_animation *wlf_scene_node_width_animation_from_job(
	struct wlf_animator_job *job);

#endif // ANIMATOR_WLF_SCENE_NODE_WIDTH_ANIMATION_H
