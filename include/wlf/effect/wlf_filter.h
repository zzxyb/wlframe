/**
 * @file        wlf_filter.h
 * @brief       Filter effect container for wlframe.
 * @details     This file provides a shape-based filter container.
 *              A filter owns an ordered list of effect primitives and describes
 *              the coordinate spaces and region in which those primitives run.
 * @author      YaoBing Xiao
 * @date        2026-09-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-09, initial version\n
 */

#ifndef EFFECT_WLF_FILTER_H
#define EFFECT_WLF_FILTER_H

#include "wlf/shapes/wlf_shape.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdbool.h>

/**
 * @brief Coordinate spaces used by filters and their effect primitives.
 */
enum wlf_filter_units {
	WLF_FILTER_UNITS_OBJECT_BOUNDING_BOX, /**< Coordinates relative to the target bounds */
	WLF_FILTER_UNITS_USER_SPACE,          /**< Coordinates in the current user space */
};

/**
 * @brief A shape-based container for an ordered filter effect chain.
 *
 * The filter owns every shape added with wlf_filter_add(). Removing an effect
 * transfers ownership back to the caller.
 */
struct wlf_filter {
	struct wlf_shape base;               /**< Base shape */
	char id[64];                         /**< Identifier used by SVG url() references */
	enum wlf_filter_units units;         /**< Coordinate space for the filter region */
	enum wlf_filter_units primitive_units; /**< Coordinate space for effect primitives */
	float x, y;                          /**< Filter region origin */
	float width, height;                 /**< Filter region size */
	struct wlf_linked_list effects;      /**< Ordered list of owned effect shapes */
	struct wlf_filter *next;             /**< Next filter in an SVG image */
};

/**
 * @brief Creates an empty filter effect container.
 *
 * @param id Filter identifier, or NULL for an unnamed filter.
 * @return New filter or NULL if allocation fails.
 */
struct wlf_filter *wlf_filter_create(const char *id);

/**
 * @brief Adds an effect shape to the end of a filter chain.
 *
 * @param filter Filter that takes ownership of the effect.
 * @param effect Effect shape to add.
 */
void wlf_filter_add(struct wlf_filter *filter, struct wlf_shape *effect);

/**
 * @brief Removes an effect shape from a filter chain.
 *
 * @param filter Filter containing the effect.
 * @param effect Effect shape to remove.
 */
void wlf_filter_remove(struct wlf_filter *filter, struct wlf_shape *effect);

/**
 * @brief Gets the number of effect shapes in a filter chain.
 *
 * @param filter Filter to inspect.
 * @return Number of effects in the filter.
 */
int wlf_filter_effect_count(const struct wlf_filter *filter);

/**
 * @brief Checks whether a shape is a filter.
 *
 * @param shape Shape to test.
 * @return true if the shape is a filter, otherwise false.
 */
bool wlf_shape_is_filter(struct wlf_shape *shape);

/**
 * @brief Gets a filter from its base shape.
 *
 * @param shape Base shape belonging to a filter.
 * @return Filter containing the base shape.
 */
struct wlf_filter *wlf_filter_from_shape(struct wlf_shape *shape);

#endif // EFFECT_WLF_FILTER_H
