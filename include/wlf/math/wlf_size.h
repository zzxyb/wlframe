/**
 * @file        wlf_size.h
 * @brief       2D integer size math utility for wlframe.
 * @details     This file provides structures and functions for 2D integer size operations,
 *              including creation, conversion, arithmetic, comparison, and rounding.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_SIZE_H
#define MATH_WLF_SIZE_H

#include <stdbool.h>

/**
 * @brief A structure representing a 2D integer size.
 */
struct wlf_size {
	int width;   /**< The width value */
	int height;  /**< The height value */
};

static const struct wlf_size WLF_SIZE_ZERO = {0, 0};        /**< Zero size (0,0) */
static const struct wlf_size WLF_SIZE_UNIT = {1, 1};        /**< Unit size (1,1) */

/**
 * @brief Converts a size to a string representation.
 * @param size Source size.
 * @return A string representing the size.
 */
char* wlf_size_to_str(const struct wlf_size *size);

/**
 * @brief Checks if two sizes are equal.
 * @param a First size.
 * @param b Second size.
 * @return true if sizes are equal, false otherwise.
 */
bool wlf_size_equal(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Checks if size is empty (zero width or height).
 * @param size Size to check.
 * @return true if size is empty, false otherwise.
 */
bool wlf_size_is_empty(const struct wlf_size *size);

/**
 * @brief Adds two sizes.
 * @param a First size.
 * @param b Second size.
 * @return Result of a + b.
 */
struct wlf_size wlf_size_add(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Subtracts two sizes.
 * @param a First size.
 * @param b Second size.
 * @return Result of a - b.
 */
struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Multiplies a size by a scalar.
 * @param size Size to multiply.
 * @param scalar Scalar value.
 * @return Scaled size.
 */
struct wlf_size wlf_size_multiply(const struct wlf_size *size, double scalar);

/**
 * @brief Divides a size by a scalar.
 * @param size Size to divide.
 * @param scalar Scalar value.
 * @return Divided size.
 */
struct wlf_size wlf_size_divide(const struct wlf_size *size, double scalar);

/**
 * @brief Calculates the area of the size.
 * @param size Size to calculate area for.
 * @return Area (width * height).
 */
int wlf_size_area(const struct wlf_size *size);

/**
 * @brief Converts a string representation of a point to a wlf_size structure.
 * @param str String in the format "(width, height)".
 * @param point Pointer to the wlf_size structure to fill.
 * @return true if conversion was successful, false otherwise.
 */
bool wlf_size_from_str(const char *str, struct wlf_size *size);

/**
 * @brief Checks if a integer size is valid.
 * @param size Size to validate.
 * @return true if size is valid (width > 0 and height > 0), false otherwise.
 */
bool wlf_size_is_valid(const struct wlf_size *size);

#endif // MATH_WLF_SIZE_H
