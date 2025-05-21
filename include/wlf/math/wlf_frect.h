/**
 * @file        wlf_frect.h
 * @brief       2D floating-point rectangle math utility for wlframe.
 * @details     This file provides structures and functions for 2D floating-point rectangle operations,
 *              including creation, conversion, arithmetic, geometric queries, intersection, union, and rounding.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_FRECT_H
#define MATH_WLF_FRECT_H

#include "wlf/math/wlf_fpoint.h"
#include "wlf/math/wlf_fsize.h"
#include "wlf/math/wlf_rect.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a 2D floating-point rectangle.
 */
struct wlf_frect {
	double x;      /**< X coordinate of the top-left corner */
	double y;      /**< Y coordinate of the top-left corner */
	double width;  /**< Width of the rectangle */
	double height; /**< Height of the rectangle */
};

static const struct wlf_frect WLF_FRECT_ZERO = {0.0, 0.0, 0.0, 0.0};  /**< Zero rectangle */
static const struct wlf_frect WLF_FRECT_UNIT = {0.0, 0.0, 1.0, 1.0};  /**< Unit rectangle */

/**
 * @brief Creates a new floating-point rectangle from position and size.
 * @param x X coordinate of the top-left corner.
 * @param y Y coordinate of the top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @return A new wlf_frect structure.
 */
struct wlf_frect wlf_frect_make(double x, double y, double width, double height);

/**
 * @brief Creates a rectangle from point and size.
 * @param pos Top-left position.
 * @param size Rectangle size.
 * @return A new wlf_rect structure.
 */
struct wlf_frect wlf_frect_from_point_size(const struct wlf_fpoint *pos, const struct wlf_fsize *size);

/**
 * @brief Creates a rectangle from two points.
 * @param p1 First point (top-left).
 * @param p2 Second point (bottom-right).
 * @return A new wlf_rect structure.
 */
struct wlf_frect wlf_frect_from_points(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2);

/**
 * @brief Converts a floating-point rectangle to a string representation with specified precision.
 * @param rect Source floating-point rectangle.
 * @param precision Number of decimal places (0-15).
 * @return A string representing the floating-point rectangle, or NULL if memory allocation fails.
 */
char* wlf_frect_to_str_prec(const struct wlf_frect *rect, uint8_t precision);

/**
 * @brief Checks if two floating-point rectangles are exactly equal.
 * @param a First floating-point rectangle.
 * @param b Second floating-point rectangle.
 * @return true if rectangles are exactly equal (all components match), false otherwise.
 */
bool wlf_frect_equal(const struct wlf_frect *a, const struct wlf_frect *b);

/**
 * @brief Checks if two floating-point rectangles are approximately equal within an epsilon.
 * @param a First floating-point rectangle.
 * @param b Second floating-point rectangle.
 * @param epsilon Maximum allowed difference between corresponding components.
 * @return true if the absolute difference between all corresponding components is less than epsilon,
 *         false otherwise.
 */
bool wlf_frect_nearly_equal(const struct wlf_frect *a, const struct wlf_frect *b, double epsilon);

/**
 * @brief Converts integer rectangle to floating-point rectangle.
 * @param rect Integer rectangle to convert.
 * @return Equivalent floating-point rectangle.
 */
struct wlf_frect wlf_rect_to_frect(const struct wlf_rect *rect);

/**
 * @brief Converts floating-point rectangle to integer rectangle.
 * @param rect Floating-point rectangle to convert.
 * @return Equivalent integer rectangle.
 */
struct wlf_rect wlf_frect_to_rect(const struct wlf_frect *rect);

/**
 * @brief Rounds floating-point rectangle to nearest integers.
 * @param rect Rectangle to round.
 * @return Rounded integer rectangle.
 */
struct wlf_rect wlf_frect_round(const struct wlf_frect *rect);

/**
 * @brief Floors floating-point rectangle to integers.
 * @param rect Rectangle to floor.
 * @return Floored integer rectangle.
 */
struct wlf_rect wlf_frect_floor(const struct wlf_frect *rect);

/**
 * @brief Ceils floating-point rectangle to integers.
 * @param rect Rectangle to ceil.
 * @return Ceiled integer rectangle.
 */
struct wlf_rect wlf_frect_ceil(const struct wlf_frect *rect);

/**
 * @brief Checks if floating-point rectangle is valid (positive width and height).
 * @param rect Rectangle to check.
 * @return true if rectangle is valid (width > 0 and height > 0), false otherwise.
 */
bool wlf_frect_is_valid(const struct wlf_frect *rect);

/**
 * @brief Parses a string representation of a floating-point rectangle and creates a wlf_frect structure.
 * @param str String representation of rectangle in format "(x,y,width,height)"
 *            or "(x, y, width, height)".
 * @param rect Pointer to the rectangle structure to fill.
 * @return true if parsing was successful, false otherwise.
 */
bool wlf_frect_from_str(const char *str, struct wlf_frect *rect);

#endif // MATH_WLF_FRECT_H
