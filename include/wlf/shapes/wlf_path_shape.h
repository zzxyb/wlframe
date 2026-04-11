/**
 * @file        wlf_path_shape.h
 * @brief       Path shape definitions for wlframe.
 * @details     Provides linked-list path segments and path-shape wrappers used by SVG
 *              parsing and rendering.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_PATH_SHAPE_H
#define SHAPES_WLF_PATH_SHAPE_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

/**
 * @brief One path node containing packed point coordinates.
 */
struct wlf_path {
	float *pts; /**< Packed coordinates: x0,y0,x1,y1,... */
	int npts; /**< Number of points (not float entries). */
	char closed; /**< Non-zero if the path is closed. */
	float bounds[4]; /**< Bounding box [minx,miny,maxx,maxy]. */
	struct wlf_path *next; /**< Next path node. */
};

/**
 * @brief Shape wrapper for a path list.
 */
struct wlf_path_shape {
	struct wlf_shape base;
	struct wlf_path *paths; /**< Head of path list. */
	bool owns_paths; /**< Whether destroy should free paths. */
	struct wlf_shape_state state; /**< Shared fill/stroke style payload. */
};

/**
 * @brief Create a path shape.
 * @param paths Path list head.
 * @param owns_paths true if the shape takes ownership of paths.
 * @return New shape pointer, or NULL on failure.
 */
struct wlf_shape *wlf_path_shape_create(struct wlf_path *paths, bool owns_paths);

/**
 * @brief Check whether a shape is a path shape.
 * @param shape Shape to test.
 * @return true if shape is path shape, false otherwise.
 */
bool wlf_shape_is_path(struct wlf_shape *shape);

/**
 * @brief Cast a base shape to a path shape.
 * @param shape Base shape pointer.
 * @return Path shape pointer (asserts if type mismatch).
 */
struct wlf_path_shape *wlf_path_shape_from_shape(struct wlf_shape *shape);

/**
 * @brief Duplicate one path node.
 * @param path Source path node.
 * @return Duplicated path node, or NULL on failure.
 */
struct wlf_path *wlf_path_duplicate(const struct wlf_path *path);

/**
 * @brief Clone a full linked list of paths.
 * @param path Source list head.
 * @return Cloned list head, or NULL on failure.
 */
struct wlf_path *wlf_path_clone_list(const struct wlf_path *path);

/**
 * @brief Destroy a linked list of paths.
 * @param path List head to destroy.
 */
void wlf_path_destroy_list(struct wlf_path *path);

#endif // SHAPES_WLF_PATH_SHAPE_H
