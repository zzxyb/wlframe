#ifndef WLF_RAY_H
#define WLF_RAY_H

#include <stdbool.h>

#include "wlf_vector3.h"

/**
 * @brief A structure representing a 3D ray
 */
struct wlf_ray {
	struct wlf_vector3 origin;    /**< The origin point of the ray */
	struct wlf_vector3 direction; /**< The direction vector of the ray */
};

static const struct wlf_ray WLF_RAY_ZERO = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}; /**< Zero ray (origin at (0,0,0) with zero direction) */

/**
 * @brief Creates a new ray
 * @param origin The origin point of the ray
 * @param direction The direction vector of the ray (must be normalized)
 * @return A new wlf_ray structure
 */
struct wlf_ray wlf_ray_create(struct wlf_vector3 origin, struct wlf_vector3 direction);

/**
 * @brief Converts a ray to a string representation
 * @param ray Source ray
 * @return A string representing the ray
 */
char* wlf_ray_to_str(const struct wlf_ray *ray);

/**
 * @brief Computes a point along the ray at a given distance
 * @param ray The ray
 * @param t The distance along the ray
 * @return The point at distance t from the origin
 */
struct wlf_vector3 wlf_ray_point_at_parameter(const struct wlf_ray *ray, double t);

/**
 * @brief Checks if two rays are equal
 * @param a First ray
 * @param b Second ray
 * @return true if rays are equal, false otherwise
 */
bool wlf_ray_equal(const struct wlf_ray *a, const struct wlf_ray *b);

/**
 * @brief Checks if two rays are nearly equal within epsilon
 * @param a First ray
 * @param b Second ray
 * @param epsilon Maximum allowed difference
 * @return true if rays are nearly equal, false otherwise
 */
bool wlf_ray_nearly_equal(const struct wlf_ray *a, const struct wlf_ray *b, double epsilon);

#endif
