/**
 * @file        wlf_shape.h
 * @brief       Base shape abstraction for wlframe.
 * @details     Defines the common interface and lifecycle hooks for all shape types.
 *              Concrete shapes embed struct wlf_shape as their first member and provide
 *              type-specific implementations via struct wlf_shape_impl.
 * @author      YaoBing Xiao
 * @date        2026-04-07
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-07, initial version\n
 */

#ifndef SHAPES_WLF_SHAPE_H
#define SHAPES_WLF_SHAPE_H

#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/types/wlf_color.h"

#include <stdbool.h>

struct wlf_shape;
struct wlf_gradient;

/**
 * @brief Common fill/stroke style payload shared by concrete shapes.
 */
struct wlf_shape_state {
	struct wlf_color fill_color; /**< Fill color. */
	struct wlf_color stroke_color; /**< Stroke color. */
	float stroke_width; /**< Stroke width. */
	float opacity; /**< Shape opacity [0..1]. */
	float fill_opacity; /**< Fill opacity [0..1]. */
	float stroke_opacity; /**< Stroke opacity [0..1]. */
	bool has_fill; /**< Non-zero when fill is enabled. */
	bool has_stroke; /**< Non-zero when stroke is enabled. */
	struct wlf_gradient *fill_gradient; /**< Optional non-owning fill gradient reference. */
	struct wlf_gradient *stroke_gradient; /**< Optional non-owning stroke gradient reference. */
};

/**
 * @brief Virtual operations for concrete shape types.
 */
struct wlf_shape_impl {
	/** @brief Destroy a shape instance. */
	void (*destroy)(struct wlf_shape *shape);
	/** @brief Clone a shape instance. */
	struct wlf_shape *(*clone)(struct wlf_shape *shape);
};

/**
 * @brief Base object embedded in every shape.
 */
struct wlf_shape {
	const struct wlf_shape_impl *impl;

	struct wlf_linked_list link; /**< List link used by shape tree children. */
	float x, y; /**< Position relative to parent. */

	struct {
		/** @brief Emitted before the shape object is destroyed. */
		struct wlf_signal destroy;
	} events;
};

/**
 * @brief Initialize a base shape with the provided implementation.
 *
 * @param shape Shape object to initialize.
 * @param impl Virtual function table for the concrete shape type.
 */
void wlf_shape_init(struct wlf_shape *shape,
	const struct wlf_shape_impl *impl);

/**
 * @brief Initialize common fill/stroke style with SVG-like defaults.
 *
 * @param paint Style payload to initialize.
 */
void wlf_shape_state_init(struct wlf_shape_state *paint);

/**
 * @brief Compute effective fill alpha from paint opacity fields.
 *
 * @param paint Style payload.
 * @return Effective fill alpha in [0, 1].
 */
float wlf_shape_state_fill_alpha(const struct wlf_shape_state *paint);

/**
 * @brief Compute effective stroke alpha from paint opacity fields.
 *
 * @param paint Style payload.
 * @return Effective stroke alpha in [0, 1].
 */
float wlf_shape_state_stroke_alpha(const struct wlf_shape_state *paint);

/**
 * @brief Destroy a shape object.
 *
 * @param shape Shape object to destroy. NULL is accepted.
 */
void wlf_shape_destroy(struct wlf_shape *shape);

/**
 * @brief Clone a shape object.
 *
 * @param shape Shape object to clone.
 * @return A new shape clone, or NULL if cloning is unsupported/failed.
 */
struct wlf_shape *wlf_shape_clone(struct wlf_shape *shape);

#endif // SHAPES_WLF_SHAPE_H
