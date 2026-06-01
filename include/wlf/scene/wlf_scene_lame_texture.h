/**
 * @file        wlf_scene_lame_texture.h
 * @brief       Lamé-curve rounded texture scene node type for wlframe.
 */

#ifndef SCENE_WLF_SCENE_LAME_TEXTURE_H
#define SCENE_WLF_SCENE_LAME_TEXTURE_H

#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/types/wlf_output.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

struct wlf_texture;

struct wlf_scene_lame_texture {
	struct wlf_scene_node node;

	struct wlf_texture *texture;
	float opacity;
	enum wlf_scale_filter_mode filter_mode;
	struct wlf_frect src_box;
	int dst_width, dst_height;
	enum wlf_output_transform transform;
	struct wlf_region *opaque_region;

	double lame_m;
	double lame_n;

	bool own_texture;
};

struct wlf_scene_lame_texture *wlf_scene_lame_texture_create(
	struct wlf_scene_tree *parent, struct wlf_texture *texture, bool own_texture);
void wlf_scene_lame_texture_set_texture(
	struct wlf_scene_lame_texture *scene_texture, struct wlf_texture *texture);
void wlf_scene_lame_texture_set_texture_with_damage(
	struct wlf_scene_lame_texture *scene_texture, struct wlf_texture *texture,
	const struct wlf_region *region);
void wlf_scene_lame_texture_set_opaque_region(
	struct wlf_scene_lame_texture *scene_texture, const struct wlf_region *region);
void wlf_scene_lame_texture_set_source_box(
	struct wlf_scene_lame_texture *scene_texture, const struct wlf_frect *box);
void wlf_scene_lame_texture_set_dest_size(
	struct wlf_scene_lame_texture *scene_texture, int width, int height);
void wlf_scene_lame_texture_set_transform(
	struct wlf_scene_lame_texture *scene_texture,
	enum wlf_output_transform transform);
void wlf_scene_lame_texture_set_filter_mode(
	struct wlf_scene_lame_texture *scene_texture,
	enum wlf_scale_filter_mode filter_mode);
void wlf_scene_lame_texture_set_lame_exponents(
	struct wlf_scene_lame_texture *scene_texture, double lame_m, double lame_n);

bool wlf_scene_node_is_lame_texture(const struct wlf_scene_node *node);
struct wlf_scene_lame_texture *wlf_scene_lame_texture_from_node(
	struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_LAME_TEXTURE_H
