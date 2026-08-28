/**
 * @file        wlf_texture_node.h
 * @brief       Texture scene-node interface.
 * @details     Provides a scene node that owns a renderer texture and draws
 *              it into a destination rectangle.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef SCENE_WLF_TEXTURE_NODE_H
#define SCENE_WLF_TEXTURE_NODE_H

#include "wlf/pass/wlf_texture_pass.h"
#include "wlf/node/wlf_scene_node.h"
#include "wlf/texture/wlf_texture.h"

/**
 * @brief A scene-graph node displaying a renderer texture.
 *
 * The node owns the texture and releases it when the node is destroyed or the
 * texture is replaced.
 */
struct wlf_texture_node {
	struct wlf_scene_node base;
	struct wlf_texture *texture;
	enum wlf_scale_filter_mode filter_mode;
	enum wlf_render_blend_mode blend_mode;
	struct wlf_listener renderer_destroy;
};

/**
 * @brief Creates a texture node.
 * @details The node takes ownership of @p texture. A zero destination width
 *          or height uses the texture's natural size.
 * @param parent Parent scene node.
 * @param texture Texture to own, or NULL for an initially invisible node.
 * @param x Initial x position relative to @p parent.
 * @param y Initial y position relative to @p parent.
 * @param width Destination width, or zero for the natural width.
 * @param height Destination height, or zero for the natural height.
 * @return New texture node, or NULL on allocation failure.
 */
struct wlf_texture_node *wlf_texture_node_create(
	struct wlf_scene_node *parent, struct wlf_texture *texture,
	int x, int y, uint32_t width, uint32_t height);

/**
 * @brief Replaces the owned texture.
 * @details Passing NULL makes the node invisible.
 * @param node Texture node to update.
 * @param texture New texture owned by the node, or NULL.
 */
void wlf_texture_node_set_texture(struct wlf_texture_node *node,
	struct wlf_texture *texture);

/**
 * @brief Changes the destination size.
 * @param node Texture node to update.
 * @param width Destination width; zero uses the natural width.
 * @param height Destination height; zero uses the natural height.
 */
void wlf_texture_node_set_dest_size(struct wlf_texture_node *node,
	uint32_t width, uint32_t height);

/**
 * @brief Checks whether a scene node is a texture node.
 * @param node Scene node to inspect.
 * @return true when @p node is a texture node, false otherwise.
 */
bool wlf_scene_node_is_texture(const struct wlf_scene_node *node);

/**
 * @brief Casts a scene node to a texture node.
 * @param node Scene node known to be a texture node.
 * @return The enclosing texture node.
 * @note The function asserts when @p node has another type.
 */
struct wlf_texture_node *wlf_texture_node_from_node(
	struct wlf_scene_node *node);

/**
 * @brief Renders a texture node through the supplied texture pass.
 * @param node Texture node to render.
 * @param pass Texture pass used for rendering.
 * @param render_target_info Destination render target.
 * @param clip Optional clip region.
 */
void wlf_texture_node_render(struct wlf_texture_node *node,
	struct wlf_texture_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_TEXTURE_NODE_H
