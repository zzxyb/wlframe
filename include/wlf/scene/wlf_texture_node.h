#ifndef SCENE_WLF_TEXTURE_NODE_H
#define SCENE_WLF_TEXTURE_NODE_H

#include "wlf/pass/wlf_texture_pass.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/texture/wlf_texture.h"

/** A scene-graph node displaying a renderer texture. */
struct wlf_texture_node {
	struct wlf_scene_node base;
	struct wlf_texture *texture;
	enum wlf_scale_filter_mode filter_mode;
	enum wlf_render_blend_mode blend_mode;
	struct wlf_listener renderer_destroy;
};

/**
 * Creates a texture node. The node takes ownership of @p texture.
 * A zero destination width or height uses the texture's natural size.
 */
struct wlf_texture_node *wlf_texture_node_create(
	struct wlf_scene_node *parent, struct wlf_texture *texture,
	double x, double y, double width, double height);

/** Replaces the owned texture. Passing NULL makes the node invisible. */
void wlf_texture_node_set_texture(struct wlf_texture_node *node,
	struct wlf_texture *texture);

void wlf_texture_node_set_dest_size(struct wlf_texture_node *node,
	double width, double height);

bool wlf_scene_node_is_texture(const struct wlf_scene_node *node);
struct wlf_texture_node *wlf_texture_node_from_node(
	struct wlf_scene_node *node);

void wlf_texture_node_render(struct wlf_texture_node *node,
	struct wlf_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_TEXTURE_NODE_H
