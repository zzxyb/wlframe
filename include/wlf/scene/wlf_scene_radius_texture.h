/**
 * @file        wlf_scene_radius_texture.h
 * @brief       Rounded texture scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_RADIUS_TEXTURE_H
#define SCENE_WLF_SCENE_RADIUS_TEXTURE_H

#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/types/wlf_output.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

struct wlf_texture;

struct wlf_scene_radius_texture {
	struct wlf_scene_node node;

	struct wlf_texture *texture;
	float opacity;
	enum wlf_scale_filter_mode filter_mode;
	struct wlf_frect src_box;
	int dst_width, dst_height;
	enum wlf_output_transform transform;
	struct wlf_region *opaque_region;

	double radius_top_left;
	double radius_top_right;
	double radius_bottom_right;
	double radius_bottom_left;

	bool own_texture;
};

struct wlf_scene_radius_texture *wlf_scene_radius_texture_create(
	struct wlf_scene_tree *parent, struct wlf_texture *texture, bool own_texture);

void wlf_scene_radius_texture_set_texture(
	struct wlf_scene_radius_texture *scene_texture, struct wlf_texture *texture);
void wlf_scene_radius_texture_set_texture_with_damage(
	struct wlf_scene_radius_texture *scene_texture, struct wlf_texture *texture,
	const struct wlf_region *region);
void wlf_scene_radius_texture_set_opaque_region(
	struct wlf_scene_radius_texture *scene_texture, const struct wlf_region *region);
void wlf_scene_radius_texture_set_source_box(
	struct wlf_scene_radius_texture *scene_texture, const struct wlf_frect *box);
void wlf_scene_radius_texture_set_dest_size(
	struct wlf_scene_radius_texture *scene_texture, int width, int height);
void wlf_scene_radius_texture_set_transform(
	struct wlf_scene_radius_texture *scene_texture,
	enum wlf_output_transform transform);
void wlf_scene_radius_texture_set_filter_mode(
	struct wlf_scene_radius_texture *scene_texture,
	enum wlf_scale_filter_mode filter_mode);
void wlf_scene_radius_texture_set_corner_radii(
	struct wlf_scene_radius_texture *scene_texture, double top_left,
	double top_right, double bottom_right, double bottom_left);

bool wlf_scene_node_is_radius_texture(const struct wlf_scene_node *node);
struct wlf_scene_radius_texture *wlf_scene_radius_texture_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_RADIUS_TEXTURE_H
