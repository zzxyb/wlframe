/**
 * @file        wlf_ray3.h
 * @brief       3D ray math utility for wlframe.
 * @details     This file provides a structure and functions for 3D ray operations,
 *              including creation, conversion, point calculation, and comparison.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_RAY3_H
#define MATH_WLF_RAY3_H

#include "wlf/math/wlf_vector3.h"

#include <stdbool.h>

/**
 * @brief A structure representing a 3D ray.
 */
struct wlf_ray3 {
	struct wlf_vector3 origin;    /**< The origin point of the ray */
	struct wlf_vector3 direction; /**< The direction vector of the ray */
};

static const struct wlf_ray3 WLF_RAY_ZERO = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}; /**< Zero ray (origin at (0,0,0) with zero direction) */

/**
 * @brief Creates a new 3D ray from origin and direction.
 * @param origin The origin point of the ray.
 * @param direction The direction vector of the ray (must be normalized).
 * @return A new wlf_ray3 structure.
 */
struct wlf_ray3 wlf_ray3_make(struct wlf_vector3 origin, struct wlf_vector3 direction);

/**
 * @brief Converts a ray to a string representation.
 * @param ray Source ray.
 * @return A string representing the ray.
 */
char* wlf_ray3_to_str(const struct wlf_ray3 *ray);

/**
 * @brief Computes a point along the ray at a given distance.
 * @param ray The ray.
 * @param t The distance along the ray.
 * @return The point at distance t from the origin.
 */
struct wlf_vector3 wlf_ray3_point_at_parameter(const struct wlf_ray3 *ray, double t);

/**
 * @brief Checks if two rays are equal.
 * @param a First ray.
 * @param b Second ray.
 * @return true if rays are equal, false otherwise.
 */
bool wlf_ray3_equal(const struct wlf_ray3 *a, const struct wlf_ray3 *b);

/**
 * @brief Checks if two rays are nearly equal within epsilon.
 * @param a First ray.
 * @param b Second ray.
 * @param epsilon Maximum allowed difference.
 * @return true if rays are nearly equal, false otherwise.
 */
bool wlf_ray3_nearly_equal(const struct wlf_ray3 *a, const struct wlf_ray3 *b, double epsilon);

#endif // MATH_WLF_RAY_H
