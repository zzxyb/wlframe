/**
 * @file        wlf_region.h
 * @brief       2D region math utility for wlframe.
 * @details     This file provides structures and functions for 2D region operations (region is a set of rectangles),
 *              including creation, destruction, initialization, cleanup, emptiness check, rectangle addition,
 *              point containment, rectangle intersection, union, and intersection.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_REGION_H
#define MATH_WLF_REGION_H

#include "wlf/math/wlf_frect.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Region data structure, contains an array of rectangles and its size.
 */
struct wlf_region_data {
	long size;                /**< Capacity of the data area */
	long numRects;            /**< Number of rectangles */
	struct wlf_frect *rects;   /**< Pointer to rectangle array */
};

/**
 * @brief Region structure, contains bounding rectangle and data pointer.
 */
struct wlf_region {
	struct wlf_frect extents;           /**< Bounding rectangle of the region */
	struct wlf_region_data *data;      /**< Pointer to region data */
};

/**
 * @brief Initialize a region object.
 * @param region Pointer to the region object.
 */
void wlf_region_init(struct wlf_region *region);

/**
 * @brief Finalize a region object and release its resources.
 * @param region Pointer to the region object.
 */
void wlf_region_fini(struct wlf_region *region);

/**
 * @brief Converts a region to a string representation.
 * @param region Source region.
 * @return A string representing the region.
 */
char* wlf_region_to_str(const struct wlf_region *region);

/**
 * @brief Parse a region from a string representation.
 * @param str The input string. eg: {[0, 0, 100, 100], [150, 150, 50, 50], ....}
 * @param out_region Pointer to the region object to fill.
 * @return true if parsing succeeded, false otherwise.
 */
bool wlf_region_from_str(const char *str, struct wlf_region *out_region);

/**
 * @brief Get the bounding rectangle of the region.
 * @param region Pointer to the region object.
 * @return The bounding rectangle of the region.
 */
struct wlf_frect wlf_region_bounding_rect(const struct wlf_region *region);

/**
 * @brief Check if the region is empty (no valid rectangles).
 * @param region Pointer to the region object.
 * @return true if empty, false otherwise.
 */
bool wlf_region_is_nil(const struct wlf_region *region);

/**
 * @brief Add a rectangle to the region.
 * @param region Pointer to the region object.
 * @param rect Pointer to the rectangle to add.
 * @return true if added successfully, false otherwise.
 */
bool wlf_region_add_rect(struct wlf_region *region, const struct wlf_frect *rect);

/**
 * @brief Check if a point is inside the region.
 * @param region Pointer to the region object.
 * @param x X coordinate of the point.
 * @param y Y coordinate of the point.
 * @return true if the point is inside, false otherwise.
 */
bool wlf_region_contains_point(const struct wlf_region *region, double x, double y);

/**
 * @brief Check if the region intersects with a given rectangle.
 * @param region Pointer to the region object.
 * @param rect Pointer to the rectangle to test.
 * @param result Pointer to the region object where the intersection result will be written.
 * If the intersection is empty, the result will be an empty region.
 */
void wlf_region_intersects_rect(const struct wlf_region *region, const struct wlf_frect *rect,
	struct wlf_region *result);

/**
 * @brief Compute the union of two regions.
 * @param dst Destination region object pointer, result will be written here.
 * @param src Source region object pointer.
 */
void wlf_region_union(struct wlf_region *dst, const struct wlf_region *src);

/**
 * @brief Compute the intersection of two regions.
 * @param dst First region object pointer.
 * @param src Second region object pointer.
 * @param result Pointer to the region object where the intersection result will be written.
 * If the intersection is empty, the result will be an empty region.
 */
void wlf_region_intersect(const struct wlf_region *dst, const struct wlf_region *src, struct wlf_region *result);

#endif // MATH_WLF_REGION_H
