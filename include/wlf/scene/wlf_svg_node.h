/**
 * @file        wlf_svg_node.h
 * @brief       Composite scene node for parsed SVG images.
 * @details     This file integrates parsed SVG images with the scene graph.
 *              SVG geometry is represented by regular wlframe shape and text
 *              nodes so rendering, damage tracking, and hit testing use the
 *              normal scene pipeline.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef SCENE_WLF_SVG_NODE_H
#define SCENE_WLF_SVG_NODE_H

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/svg/wlf_svg.h"

/**
 * A composite scene node backed by a parsed SVG image.
 *
 * The node owns @ref image and exposes its generated scene children through
 * @ref children. SVG geometry is represented by regular wlframe shape and text
 * nodes, so rendering, damage tracking and hit testing use the normal scene
 * pipeline on every renderer backend.
 */
struct wlf_svg_node {
	struct wlf_scene_node base;
	struct wlf_linked_list children;
	struct wlf_svg_image *image;
};

/**
 * Creates an SVG node from an already parsed image.
 *
 * On success the node takes ownership of @p image. On failure ownership stays
 * with the caller. SVG text requires a renderer to be associated with the
 * parent node's window.
 *
 * @param parent Parent container node.
 * @param x Horizontal position relative to the parent.
 * @param y Vertical position relative to the parent.
 * @param image Parsed SVG image.
 * @return A new SVG node, or NULL on failure.
 */
struct wlf_svg_node *wlf_svg_node_create(struct wlf_scene_node *parent,
	int x, int y, struct wlf_svg_image *image);

/**
 * Parses a file and creates an SVG node.
 *
 * @param parent Parent container node.
 * @param x Horizontal position relative to the parent.
 * @param y Vertical position relative to the parent.
 * @param filename SVG file path.
 * @param units Output units passed to the SVG parser, or NULL for pixels.
 * @param dpi Resolution passed to the SVG parser.
 * @return A new SVG node, or NULL on parse/allocation failure.
 */
struct wlf_svg_node *wlf_svg_node_create_from_file(
	struct wlf_scene_node *parent, int x, int y,
	const char *filename, const char *units, float dpi);

/** @return true when @p node is an SVG node. */
bool wlf_scene_node_is_svg(const struct wlf_scene_node *node);

/**
 * Casts a scene node to an SVG node.
 *
 * @param node Scene node known to be an SVG node.
 * @return SVG node containing @p node.
 */
struct wlf_svg_node *wlf_svg_node_from_node(struct wlf_scene_node *node);

#endif // SCENE_WLF_SVG_NODE_H
