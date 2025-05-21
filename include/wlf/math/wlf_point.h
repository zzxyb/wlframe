/**
 * @file        wlf_point.h
 * @brief       2D integer point math utility for wlframe.
 * @details     This file provides structures and functions for 2D integer point operations,
 *              including creation, conversion, arithmetic, and distance calculation.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_POINT_H
#define MATH_WLF_POINT_H

#include <stdbool.h>
#include <math.h>

/**
 * @brief A structure representing a 2D integer point.
 */
struct wlf_point {
	int x;  /**< The x coordinate */
	int y;  /**< The y coordinate */
};

/**
 * @brief Common constant points.
 */
static const struct wlf_point WLF_POINT_ZERO = {0, 0};        /**< Origin point (0,0) */
static const struct wlf_point WLF_POINT_UNIT = {1, 1};        /**< Unit point (1,1) */
static const struct wlf_point WLF_POINT_UNIT_X = {1, 0};      /**< Unit vector in x direction */
static const struct wlf_point WLF_POINT_UNIT_Y = {0, 1};      /**< Unit vector in y direction */

/**
 * @brief Converts a 2D integer point to a string representation.
 * @param point Source point.
 * @return A string representing the point.
 */
char *wlf_point_to_str(const struct wlf_point *point);

/**
 * @brief Checks if two integer points are equal.
 * @param a First point.
 * @param b Second point.
 * @return true if points are equal, false otherwise.
 */
bool wlf_point_equal(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Checks if an integer point is zero (origin).
 * @param p Point to check.
 * @return true if point is zero, false otherwise.
 */
bool wlf_point_is_zero(const struct wlf_point *p);

/**
 * @brief Adds two integer points.
 * @param a First point.
 * @param b Second point.
 * @return Result of a + b.
 */
struct wlf_point wlf_point_add(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Subtracts two integer points.
 * @param a First point.
 * @param b Second point.
 * @return Result of a - b.
 */
struct wlf_point wlf_point_subtract(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Multiplies an integer point by a scalar.
 * @param p Point to multiply.
 * @param scalar Scalar value.
 * @return Result of p * scalar.
 */
struct wlf_point wlf_point_multiply(const struct wlf_point *p, double scalar);

/**
 * @brief Calculates Manhattan distance between two points.
 * @param p1 First point.
 * @param p2 Second point.
 * @return Manhattan distance |x1-x2| + |y1-y2|.
 */
int wlf_point_manhattan_distance(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Calculates Euclidean distance between two points.
 * @param p1 First point.
 * @param p2 Second point.
 * @return Euclidean distance sqrt((x1-x2)² + (y1-y2)²).
 */
double wlf_point_euclidean_distance(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Converts a string representation of a point to a wlf_point structure.
 * @param str String in the format "(x, y)".
 * @param point Pointer to the wlf_point structure to fill.
 * @return true if conversion was successful, false otherwise.
 */
bool wlf_point_from_str(const char *str, struct wlf_point *point);

#endif // MATH_WLF_POINT_H
