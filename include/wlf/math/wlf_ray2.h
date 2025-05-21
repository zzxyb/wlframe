/**
 * @file        wlf_ray2.h
 * @brief       2D ray math utility for wlframe.
 * @details     This file provides a structure and functions for 2D ray operations,
 *              including creation, conversion, point calculation, and comparison.
 * @author      YaoBing Xiao
 * @date        2025-06-22
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-22, initial version\n
 */

#ifndef MATH_WLF_RAY2_H
#define MATH_WLF_RAY2_H

#include "wlf/math/wlf_vector2.h"

#include <stdbool.h>

/**
 * @brief A structure representing a 2D ray.
 */
struct wlf_ray2 {
	struct wlf_vector2 origin;    /**< The origin point of the ray */
	struct wlf_vector2 direction; /**< The direction vector of the ray */
};

static const struct wlf_ray2 WLF_RAY2_ZERO = {{0.0, 0.0}, {0.0, 0.0}}; /**< Zero ray (origin at (0,0) with zero direction) */

/**
 * @brief Creates a new 2D ray from origin and direction.
 * @param origin The origin point of the ray.
 * @param direction The direction vector of the ray (must be normalized).
 * @return A new wlf_ray2 structure.
 */
struct wlf_ray2 wlf_ray2_make(struct wlf_vector2 origin, struct wlf_vector2 direction);

/**
 * @brief Converts a ray to a string representation.
 * @param ray Source ray.
 * @return A string representing the ray.
 */
char* wlf_ray2_to_str(const struct wlf_ray2 *ray);

/**
 * @brief Computes a point along the ray at a given distance.
 * @param ray The ray.
 * @param t The distance along the ray.
 * @return The point at distance t from the origin.
 */
struct wlf_vector2 wlf_ray2_point_at_parameter(const struct wlf_ray2 *ray, double t);

/**
 * @brief Checks if two rays are equal.
 * @param a First ray.
 * @param b Second ray.
 * @return true if rays are equal, false otherwise.
 */
bool wlf_ray2_equal(const struct wlf_ray2 *a, const struct wlf_ray2 *b);

/**
 * @brief Checks if two rays are nearly equal within epsilon.
 * @param a First ray.
 * @param b Second ray.
 * @param epsilon Maximum allowed difference.
 * @return true if rays are nearly equal, false otherwise.
 */
bool wlf_ray2_nearly_equal(const struct wlf_ray2 *a, const struct wlf_ray2 *b, double epsilon);

#endif // MATH_WLF_RAY2_H
