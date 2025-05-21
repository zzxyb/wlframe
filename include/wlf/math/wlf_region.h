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

#include "wlf_rect.h"
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Region data structure, contains an array of rectangles and its size.
 */
struct wlf_region_data {
	long size;                /**< Capacity of the data area */
	long numRects;            /**< Number of rectangles */
	struct wlf_rect *rects;   /**< Pointer to rectangle array */
};

/**
 * @brief Region structure, contains bounding rectangle and data pointer.
 */
struct wlf_region {
	struct wlf_rect extents;           /**< Bounding rectangle of the region */
	struct wlf_region_data *data;      /**< Pointer to region data */
};

/**
 * @brief Create a new region object.
 * @return Pointer to the newly allocated region.
 */
struct wlf_region* wlf_region_create(void);

/**
 * @brief Destroy a region object and free memory.
 * @param region Pointer to the region object.
 */
void wlf_region_destroy(struct wlf_region *region);

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
bool wlf_region_add_rect(struct wlf_region *region, const struct wlf_rect *rect);

/**
 * @brief Check if a point is inside the region.
 * @param region Pointer to the region object.
 * @param x X coordinate of the point.
 * @param y Y coordinate of the point.
 * @return true if the point is inside, false otherwise.
 */
bool wlf_region_contains_point(const struct wlf_region *region, int x, int y);

/**
 * @brief Check if the region intersects with a given rectangle.
 * @param region Pointer to the region object.
 * @param rect Pointer to the rectangle to test.
 * @return true if intersecting, false otherwise.
 */
bool wlf_region_intersects_rect(const struct wlf_region *region, const struct wlf_rect *rect);

/**
 * @brief Compute the union of two regions.
 * @param dst Destination region object pointer, result will be written here.
 * @param src Source region object pointer.
 */
void wlf_region_union(struct wlf_region *dst, const struct wlf_region *src);

/**
 * @brief Compute the intersection of two regions.
 * @param dst Destination region object pointer, result will be written here.
 * @param src Source region object pointer.
 */
void wlf_region_intersect(struct wlf_region *dst, const struct wlf_region *src);

#endif // MATH_WLF_REGION_H
