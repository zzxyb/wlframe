
/**
 * @file        wlf_scene_texture.h
 * @brief       Texture scene node type for wlframe.
 * @details     This file provides the texture-based scene node used by wlframe scene graphs.
 *              A texture scene node is a concrete `wlf_scene_node` implementation that draws
 *              a `wlf_texture` with configurable source cropping, destination size, transform,
 *              filtering, and optional opaque region metadata.
 * @author      YaoBing Xiao
 * @date        2026-06-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-06-02, initial version\n
 */

#ifndef SCENE_WLF_SCENE_TEXTURE_H
#define SCENE_WLF_SCENE_TEXTURE_H

#include "wlf/math/wlf_frect.h"
#include "wlf/math/wlf_region.h"
#include "wlf/pass/wlf_pass.h"
#include "wlf/types/wlf_output.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/scene/wlf_scene_tree.h"

struct wlf_texture;

/**
 * @brief Texture scene node.
 *
 * A concrete scene node that renders a texture in the scene graph.
 * It embeds `struct wlf_scene_node` as its base type and stores the
 * texture source, scaling, transform, and ownership metadata.
 */
struct wlf_scene_texture {
	struct wlf_scene_node node;              /**< Base scene node */

	struct wlf_texture *texture;             /**< Texture rendered by this node */
	float opacity;                           /**< Texture-local opacity multiplier */
	enum wlf_scale_filter_mode filter_mode;  /**< Sampling filter used when scaling */
	struct wlf_frect src_box;                /**< Source box sampled from the texture */
	int dst_width, dst_height;               /**< Optional destination size override */
	enum wlf_output_transform transform;     /**< Output transform applied to the texture */
	struct wlf_region *opaque_region;        /**< Optional opaque region in local coordinates */

	bool own_texture;                        /**< Whether the node destroys the texture on teardown */
};

/**
 * @brief Create a texture scene node.
 *
 * Allocates and initializes a texture node attached to the specified parent tree.
 *
 * @param parent Parent scene tree to attach to.
 * @param texture Texture to render, or NULL to create an initially empty node.
 * @param own_texture Whether the created node takes ownership of `texture`.
 * @return Pointer to the newly created texture node, or NULL on allocation failure.
 */
struct wlf_scene_texture *wlf_scene_texture_create(struct wlf_scene_tree *parent,
	struct wlf_texture *texture, bool own_texture);

/**
 * @brief Replace the texture rendered by the scene node.
 *
 * @param scene_texture Texture scene node.
 * @param texture New texture to render.
 */
void wlf_scene_texture_set_texture(struct wlf_scene_texture *scene_texture,
	struct wlf_texture *texture);

/**
 * @brief Replace the texture rendered by the scene node and mark a damage region.
 *
 * @param scene_texture Texture scene node.
 * @param texture New texture to render.
 * @param region Optional damage region associated with the texture update.
 */
void wlf_scene_texture_set_texture_with_damage(struct wlf_scene_texture *scene_texture,
	struct wlf_texture *texture, const struct wlf_region *region);

/**
 * @brief Set the opaque region metadata for the texture node.
 *
 * @param scene_texture Texture scene node.
 * @param region Opaque region in local coordinates, or NULL to clear it.
 */
void wlf_scene_texture_set_opaque_region(struct wlf_scene_texture *scene_texture,
	const struct wlf_region *region);

/**
 * @brief Set the source box sampled from the texture.
 *
 * @param scene_texture Texture scene node.
 * @param box Source rectangle in texture coordinates, or NULL to use the full texture.
 */
void wlf_scene_texture_set_source_box(struct wlf_scene_texture *scene_texture,
	const struct wlf_frect *box);

/**
 * @brief Set the destination size used when rendering the texture.
 *
 * If width and height are zero, the texture node falls back to the source box size.
 *
 * @param scene_texture Texture scene node.
 * @param width Destination width.
 * @param height Destination height.
 */
void wlr_scene_buffer_set_dest_size(struct wlf_scene_texture *scene_texture,
	int width, int height);

/**
 * @brief Set the output transform applied to the texture node.
 *
 * @param scene_texture Texture scene node.
 * @param transform Transform to apply.
 */
void wlr_scene_buffer_set_transform(struct wlf_scene_texture *scene_texture,
	enum wlf_output_transform transform);

/**
 * @brief Set the texture scaling filter mode.
 *
 * @param scene_texture Texture scene node.
 * @param filter_mode Filter mode used when scaling the texture.
 */
void wlr_scene_buffer_set_filter_mode(struct wlf_scene_texture *scene_texture,
	enum wlf_scale_filter_mode filter_mode);

/**
 * @brief Check whether a scene node is a texture node.
 * @param node Scene node to test.
 * @return true if the node is a texture scene node, false otherwise.
 */
bool wlf_scene_node_is_texture(const struct wlf_scene_node *node);

/**
 * @brief Cast a scene node to a texture scene node.
 *
 * The caller must ensure that `node` is actually a texture node, typically by
 * checking `wlf_scene_node_is_texture()` first.
 *
 * @param node Scene node to cast.
 * @return Pointer to the corresponding texture scene node.
 */
struct wlf_scene_texture *wlf_scene_texture_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SCENE_TEXTURE_H
