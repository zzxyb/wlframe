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
 * @brief Relationship between a region and a rectangle, mirroring pixman_region_overlap_t.
 */
enum wlf_region_overlap {
	WLF_REGION_OUT = 0,   /**< Rectangle is completely outside the region */
	WLF_REGION_IN = 1,    /**< Rectangle is fully contained in the region */
	WLF_REGION_PART = 2,  /**< Rectangle is partially covered by the region */
};

/**
 * @brief Initialize a region object.
 * @param region Pointer to the region object.
 */
void wlf_region_init(struct wlf_region *region);

/**
 * @brief Initialize a region object with a rectangle.
 * @param region Pointer to the region object.
 * @param rect Pointer to the initial rectangle.
 */
void wlf_region_init_rect(struct wlf_region *region, const struct wlf_frect *rect);

/**
 * @brief Finalize a region object and release its resources.
 * @param region Pointer to the region object.
 */
void wlf_region_fini(struct wlf_region *region);

/**
 * @brief Remove all rectangles from a region while keeping it initialized.
 * @param region Pointer to the region object.
 */
void wlf_region_clear(struct wlf_region *region);

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
 * @brief Check whether the region contains at least one rectangle.
 * @param region Pointer to the region object.
 * @return true if the region is not empty, false otherwise.
 */
bool wlf_region_not_empty(const struct wlf_region *region);

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
 * @brief Check how a rectangle overlaps with the region.
 * @param region Pointer to the region object.
 * @param rect Rectangle to test.
 * @return Overlap classification relative to the region.
 */
enum wlf_region_overlap wlf_region_contains_rect(const struct wlf_region *region,
	const struct wlf_frect *rect);

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
 * @param new_region Pointer to the destination region object where the union result will be written.
 * @param reg1 First source region.
 * @param reg2 Second source region.
 * @return true on success, false if memory allocation fails or the destination is invalid.
 * @note The result is: new_region = reg1 U reg2
 * @note new_region may alias reg1 or reg2. In that case, the function preserves source data
 *       until the merged result has been built successfully.
 */
bool wlf_region_union(struct wlf_region *new_region, const struct wlf_region *reg1,
	const struct wlf_region *reg2);

/**
 * @brief Compute the union of a region and a rectangle.
 * @param dst Destination region object pointer, result will be written here.
 * @param src Source region object pointer.
 * @param rect Pointer to the rectangle to union.
 * @note The result is: dst = src ∪ rect
 */
void wlf_region_union_rect(struct wlf_region *dst, struct wlf_region *src, const struct wlf_frect *rect);

/**
 * @brief Compute the intersection of two regions.
 * @param dst First region object pointer.
 * @param src Second region object pointer.
 * @param result Pointer to the region object where the intersection result will be written.
 * If the intersection is empty, the result will be an empty region.
 */
void wlf_region_intersect(const struct wlf_region *dst, const struct wlf_region *src, struct wlf_region *result);

/**
 * @brief Subtract one region from another.
 * @param dst Destination region object where the result will be written.
 * @param minuend Source region to subtract from.
 * @param subtrahend Region to subtract.
 * @return true on success, false if memory allocation fails or the destination is invalid.
 */
bool wlf_region_subtract(struct wlf_region *dst, const struct wlf_region *minuend,
	const struct wlf_region *subtrahend);

/**
 * @brief Translate all rectangles in the region by the given offset.
 * @param region Pointer to the region object.
 * @param dx Horizontal offset.
 * @param dy Vertical offset.
 */
void wlf_region_translate(struct wlf_region *region, double dx, double dy);

/**
 * @brief Reset the region to contain exactly one rectangle.
 * @param region Pointer to the region object.
 * @param rect Rectangle to store in the region. If NULL, the region becomes empty.
 * @return true on success, false if memory allocation fails or the region is invalid.
 */
bool wlf_region_reset(struct wlf_region *region, const struct wlf_frect *rect);

/**
 * @brief Get the number of rectangles stored in the region.
 * @param region Pointer to the region object.
 * @return Number of rectangles in the region, or 0 for an empty/invalid region.
 */
long wlf_region_n_rects(const struct wlf_region *region);

/**
 * @brief Get a read-only pointer to the internal rectangle array.
 * @param region Pointer to the region object.
 * @param n_rects Optional output for rectangle count.
 * @return Pointer to the internal rectangle array, or NULL if the region is empty/invalid.
 */
const struct wlf_frect *wlf_region_rectangles(const struct wlf_region *region, long *n_rects);

/**
 * @brief Compare two regions for exact rectangle-array equality.
 * @param region1 First region.
 * @param region2 Second region.
 * @return true if both regions contain the same rectangles in the same order, false otherwise.
 */
bool wlf_region_equal(const struct wlf_region *region1, const struct wlf_region *region2);

/**
 * @brief Copy one region into another.
 * @param dst Destination region object that receives the copy.
 * @param src Source region object to copy from.
 * @return true on success, false if memory allocation fails or either region is invalid.
 */
bool wlf_region_copy(struct wlf_region *dst, const struct wlf_region *src);

#endif // MATH_WLF_REGION_H
