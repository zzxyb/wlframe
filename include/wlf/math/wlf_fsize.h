/**
 * @file        wlf_fsize.h
 * @brief       2D floating-point size math utility for wlframe.
 * @details     This file provides structures and functions for 2D floating-point size operations,
 *              including creation, conversion, arithmetic, comparison, and rounding.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_FSIZE_H
#define MATH_WLF_FSIZE_H

#include "wlf/math/wlf_size.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a 2D floating-point size.
 */
struct wlf_fsize {
	double width;   /**< The width value */
	double height;  /**< The height value */
};

static const struct wlf_fsize WLF_FSIZE_ZERO = {0.0, 0.0};  /**< Zero size (0.0,0.0) */
static const struct wlf_fsize WLF_FSIZE_UNIT = {1.0, 1.0};  /**< Unit size (1.0,1.0) */

/**
 * @brief Converts a 2D floating-point size to a string representation. 3 decimal places.
 * @param size Source floating-point size.
 * @return A string representing the floating-point size.
 */
char *wlf_fsize_to_str(const struct wlf_fsize *size);

/**
 * @brief Converts a floating-point size to a string representation with specified precision.
 * @param size Source floating-point size.
 * @param precision Number of decimal places (0-15).
 * @return A string representing the floating-point size, or NULL if memory allocation fails.
 */
char *wlf_fsize_to_str_prec(const struct wlf_fsize *size, uint8_t precision);

/**
 * @brief Checks if two floating-point sizes are equal.
 * @param a First size.
 * @param b Second size.
 * @return true if sizes are equal, false otherwise.
 */
bool wlf_fsize_equal(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Checks if two floating-point sizes are nearly equal within epsilon.
 * @param a First size.
 * @param b Second size.
 * @param epsilon Maximum allowed difference.
 * @return true if sizes are nearly equal, false otherwise.
 */
bool wlf_fsize_nearly_equal(const struct wlf_fsize *a, const struct wlf_fsize *b, double epsilon);

/**
 * @brief Adds two floating-point sizes together.
 * @param a First floating-point size operand.
 * @param b Second floating-point size operand.
 * @return A new wlf_fsize structure where:
 *         - width = a->width + b->width
 *         - height = a->height + b->height
 */
struct wlf_fsize wlf_fsize_add(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Subtracts the second floating-point size from the first.
 * @param a First floating-point size (minuend).
 * @param b Second floating-point size (subtrahend).
 * @return A new wlf_fsize structure where:
 *         - width = a->width - b->width
 *         - height = a->height - b->height
 */
struct wlf_fsize wlf_fsize_subtract(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Multiplies a floating-point size by a scalar value.
 * @param size The floating-point size to be multiplied.
 * @param scalar The scalar value to multiply by.
 * @return A new wlf_fsize structure where:
 *         - width = size->width * scalar
 *         - height = size->height * scalar
 */
struct wlf_fsize wlf_fsize_multiply(const struct wlf_fsize *size, double scalar);

/**
 * @brief Divides a floating-point size by a scalar value.
 * @param size The floating-point size to be divided.
 * @param scalar The scalar value to divide by (must not be zero).
 * @return A new wlf_fsize structure where:
 *         - width = size->width / scalar
 *         - height = size->height / scalar
 */
struct wlf_fsize wlf_fsize_divide(const struct wlf_fsize *size, double scalar);

/**
 * @brief Calculates the area of the floating-point size.
 * @param size Size to calculate area for.
 * @return Area (width * height).
 */
double wlf_fsize_area(const struct wlf_fsize *size);

/**
 * @brief Converts integer size to floating-point size.
 * @param size Integer size to convert.
 * @return Equivalent floating-point size.
 */
struct wlf_fsize wlf_size_to_fsize(const struct wlf_size *size);

/**
 * @brief Converts floating-point size to integer size.
 * @param size Floating-point size to convert.
 * @return Equivalent integer size.
 */
struct wlf_size wlf_fsize_to_size(const struct wlf_fsize *size);

/**
 * @brief Rounds floating-point size to nearest integers.
 * @param size Size to round.
 * @return Rounded integer size.
 */
struct wlf_size wlf_fsize_round(const struct wlf_fsize *size);

/**
 * @brief Floors floating-point size to integers.
 * @param size Size to floor.
 * @return Floored integer size.
 */
struct wlf_size wlf_fsize_floor(const struct wlf_fsize *size);

/**
 * @brief Ceils floating-point size to integers.
 * @param size Size to ceil.
 * @return Ceiled integer size.
 */
struct wlf_size wlf_fsize_ceil(const struct wlf_fsize *size);

/**
 * @brief Converts a string representation of a point to a wlf_fsize structure.
 * @param str String in the format "(width, height)".
 * @param point Pointer to the wlf_fsize structure to fill.
 * @return true if conversion was successful, false otherwise.
 */
bool wlf_fsize_from_str(const char *str, struct wlf_fsize *size);

/**
 * @brief Checks if a floating-point size is valid.
 * @param size Size to validate.
 * @return true if size is valid (width > 0 and height > 0), false otherwise.
 */
bool wlf_fsize_is_valid(const struct wlf_fsize *size);

#endif // MATH_WLF_FSIZE_H
