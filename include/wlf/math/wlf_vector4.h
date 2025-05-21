/**
 * @file        wlf_vector4.h
 * @brief       4D vector math utility for wlframe.
 * @details     This file provides a structure and functions for 4D vector operations,
 *              including creation, conversion, arithmetic, normalization, dot product,
 *              magnitude calculation, and comparison.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_VECTOR4_H
#define MATH_WLF_VECTOR4_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a 4D vector.
 */
struct wlf_vector4 {
	double x;  /**< The x component */
	double y;  /**< The y component */
	double z;  /**< The z component */
	double w;  /**< The w component */
};

static const struct wlf_vector4 WLF_VECTOR4_ZERO   = {0.0, 0.0, 0.0, 0.0};    /**< Zero vector (0,0,0,0) */
static const struct wlf_vector4 WLF_VECTOR4_UNIT_X = {1.0, 0.0, 0.0, 0.0};    /**< Unit vector in x direction (1,0,0,0) */
static const struct wlf_vector4 WLF_VECTOR4_UNIT_Y = {0.0, 1.0, 0.0, 0.0};    /**< Unit vector in y direction (0,1,0,0) */
static const struct wlf_vector4 WLF_VECTOR4_UNIT_Z = {0.0, 0.0, 1.0, 0.0};    /**< Unit vector in z direction (0,0,1,0) */
static const struct wlf_vector4 WLF_VECTOR4_UNIT_W = {0.0, 0.0, 0.0, 1.0};    /**< Unit vector in w direction (0,0,0,1) */

/**
 * @brief  Make a new 4D vector from x, y, and z components.
 * @param x The x component.
 * @param y The y component.
 * @param z The z component.
 * @param w The w component.
 * @return A new wlf_vector4 structure.
 */
struct wlf_vector4 wlf_vector4_make(double x, double y, double z, double w);

/**
 * @brief Converts a 4D vector to a string representation.
 * @param vector Source 4D vector.
 * @return A string representing the 4D vector.
 */
char* wlf_vector4_to_str(const struct wlf_vector4 *vector);

/**
 * @brief Converts a 4D vector to a string representation with specified precision.
 * @param size Source 4D vector.
 * @param precision Number of decimal places (0-15).
 * @return A string representing the 4D vector, or NULL if memory allocation fails.
 */
char *wlf_vector4_to_str_prec(const struct wlf_vector4 *vector, uint8_t precision);

/**
 * @brief Adds two vectors.
 * @param a First vector.
 * @param b Second vector.
 * @return Result of a + b.
 */
struct wlf_vector4 wlf_vector4_add(const struct wlf_vector4 *a, const struct wlf_vector4 *b);

/**
 * @brief Subtracts two vectors.
 * @param a First vector.
 * @param b Second vector.
 * @return Result of a - b.
 */
struct wlf_vector4 wlf_vector4_subtract(const struct wlf_vector4 *a, const struct wlf_vector4 *b);

/**
 * @brief Multiplies a vector by a scalar.
 * @param vec Vector to multiply.
 * @param scalar Scalar value.
 * @return Scaled vector.
 */
struct wlf_vector4 wlf_vector4_multiply(const struct wlf_vector4 *vec, double scalar);

/**
 * @brief Divides a vector by a scalar.
 * @param vec Vector to divide.
 * @param scalar Scalar value (must not be zero).
 * @return Divided vector.
 */
struct wlf_vector4 wlf_vector4_divide(const struct wlf_vector4 *vec, double scalar);

/**
 * @brief Calculates the dot product of two vectors.
 * @param a First vector.
 * @param b Second vector.
 * @return Dot product (a·b).
 */
double wlf_vector4_dot(const struct wlf_vector4 *a, const struct wlf_vector4 *b);

/**
 * @brief Calculates the magnitude (length) of a vector.
 * @param vec Input vector.
 * @return Magnitude of the vector.
 */
double wlf_vector4_magnitude(const struct wlf_vector4 *vec);

/**
 * @brief Normalizes a vector to unit length.
 * @param vec Vector to normalize.
 * @return Normalized vector.
 */
struct wlf_vector4 wlf_vector4_normalize(const struct wlf_vector4 *vec);

/**
 * @brief Checks if two vectors are equal.
 * @param a First vector.
 * @param b Second vector.
 * @return true if vectors are equal, false otherwise.
 */
bool wlf_vector4_equal(const struct wlf_vector4 *a, const struct wlf_vector4 *b);

/**
 * @brief Checks if two vectors are nearly equal within epsilon.
 * @param a First vector.
 * @param b Second vector.
 * @param epsilon Maximum allowed difference.
 * @return true if vectors are nearly equal, false otherwise.
 */
bool wlf_vector4_nearly_equal(const struct wlf_vector4 *a, const struct wlf_vector4 *b, double epsilon);

#endif // MATH_WLF_VECTOR4_H
