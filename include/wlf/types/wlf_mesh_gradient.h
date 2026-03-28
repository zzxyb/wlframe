/**
 * @file        wlf_mesh_gradient.h
 * @brief       Mesh gradient implementation (SVG2-style patches) for wlframe.
 * @author      YaoBing Xiao
 * @date        2026-03-21
 * @version     v1.0
 */

#ifndef TYPES_WLF_MESH_GRADIENT_H
#define TYPES_WLF_MESH_GRADIENT_H

#include "wlf/types/wlf_gradient.h"

/**
 * @brief A 4x4 patch for mesh gradients.
 * @details The current mesh gradient implementation samples colors over a regular
 *   grid defined by @ref wlf_mesh_gradient::origin, @ref wlf_mesh_gradient::size,
 *   and the patch row/column counts. The @ref control_points are stored with each
 *   patch for optional geometric metadata and future use, but they are not read
 *   by the current mesh gradient sampling code, which only interpolates @ref colors.
 */
struct wlf_mesh_gradient_patch {
	struct wlf_fpoint control_points[4][4]; /**< Reserved Bezier control points (not used by current sampling) */
	struct wlf_color colors[4][4];         /**< Patch vertex colors used for interpolation */
};

/**
 * @brief Mesh gradient definition (SVG2-style grid of patches).
 */
struct wlf_mesh_gradient {
	struct wlf_gradient base;   /**< Base gradient */
	struct wlf_fpoint origin;   /**< Mesh origin (top-left) */
	struct wlf_fpoint size;     /**< Mesh size (width/height) */
	uint32_t patch_columns;     /**< Number of patch columns (>=1) */
	uint32_t patch_rows;        /**< Number of patch rows (>=1) */
	struct wlf_mesh_gradient_patch *patches; /**< Patch array (rows * columns) */
};

/**
 * @brief Creates a mesh gradient.
 * @param origin Mesh origin (top-left).
 * @param size Mesh size (width/height).
 * @param patch_columns Number of patch columns (>=1).
 * @param patch_rows Number of patch rows (>=1).
 * @param patches Patch array (rows * columns).
 * @return Newly allocated mesh gradient or NULL on failure.
 */
struct wlf_mesh_gradient *wlf_mesh_gradient_create(
	struct wlf_fpoint origin, struct wlf_fpoint size,
	uint32_t patch_columns, uint32_t patch_rows,
	const struct wlf_mesh_gradient_patch *patches);

/**
 * @brief Checks whether a gradient is a mesh gradient.
 * @param gradient Gradient to test.
 * @return true if the gradient is a mesh gradient, false otherwise.
 */
bool wlf_gradient_is_mesh(const struct wlf_gradient *gradient);

/**
 * @brief Obtains the mesh gradient from a base gradient pointer.
 * @param gradient Base gradient pointer. Must be a mesh gradient; passing any
 *   other gradient type triggers an assertion failure.
 * @return The enclosing wlf_mesh_gradient.
 */
struct wlf_mesh_gradient *wlf_mesh_gradient_from_gradient(
	struct wlf_gradient *gradient);

#endif // TYPES_WLF_MESH_GRADIENT_H
