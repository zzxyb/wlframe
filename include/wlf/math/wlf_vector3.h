/**
 * @file        wlf_vector3.h
 * @brief       3D vector math utility for wlframe.
 * @details     This file provides a structure and functions for 3D vector operations,
 *              including creation, conversion, arithmetic, normalization, dot and cross product,
 *              magnitude calculation, and comparison.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_VECTOR3_H
#define MATH_WLF_VECTOR3_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a 3D vector.
 */
struct wlf_vector3 {
	double x;  /**< The x component */
	double y;  /**< The y component */
	double z;  /**< The z component */
};

static const struct wlf_vector3 WLF_VECTOR3_ZERO   = {0.0, 0.0, 0.0};    /**< Zero vector (0,0,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_X = {1.0, 0.0, 0.0};    /**< Unit vector in x direction (1,0,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_Y = {0.0, 1.0, 0.0};    /**< Unit vector in y direction (0,1,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_Z = {0.0, 0.0, 1.0};    /**< Unit vector in z direction (0,0,1) */

/**
 * @brief Make a new 3D vector from x, y, and z components.
 * @param x X component of the vector.
 * @param y Y component of the vector.
 * @param z Z component of the vector.
 * @return A new wlf_vector3 structure with the specified components.
 */
struct wlf_vector3 wlf_vector3_make(double x, double y, double z);

/**
 * @brief Converts a 3D vector to a string representation.
 * @param vector Source 3D vector to convert.
 * @return A dynamically allocated string representing the 3D vector in format "(x,y,z)",
 *         or NULL if memory allocation fails. The caller is responsible for freeing the returned string.
 */
char* wlf_vector3_to_str(const struct wlf_vector3 *vector);

/**
 * @brief Converts a 3D vector to a string representation with specified precision.
 * @param size Source 3D vector.
 * @param precision Number of decimal places (0-15).
 * @return A string representing the 3D vector, or NULL if memory allocation fails.
 */
char *wlf_vector3_to_str_prec(const struct wlf_vector3 *vector, uint8_t precision);

/**
 * @brief Adds two 3D vectors component-wise.
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return A new vector representing the sum (a + b).
 */
struct wlf_vector3 wlf_vector3_add(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Subtracts two 3D vectors component-wise.
 * @param a First vector operand (minuend).
 * @param b Second vector operand (subtrahend).
 * @return A new vector representing the difference (a - b).
 */
struct wlf_vector3 wlf_vector3_subtract(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Multiplies a 3D vector by a scalar value.
 * @param vec Vector to multiply.
 * @param scalar Scalar multiplier.
 * @return A new vector with each component multiplied by the scalar.
 */
struct wlf_vector3 wlf_vector3_multiply(const struct wlf_vector3 *vec, double scalar);

/**
 * @brief Divides a 3D vector by a scalar value.
 * @param vec Vector to divide.
 * @param scalar Scalar divisor (must not be zero).
 * @return A new vector with each component divided by the scalar.
 * @warning Behavior is undefined if scalar is zero.
 */
struct wlf_vector3 wlf_vector3_divide(const struct wlf_vector3 *vec, double scalar);

/**
 * @brief Calculates the dot product of two 3D vectors.
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return The dot product (a·b) as a scalar value.
 */
double wlf_vector3_dot(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Calculates the cross product of two 3D vectors.
 * @param a First vector operand.
 * @param b Second vector operand.
 * @return A new vector representing the cross product (a×b), perpendicular to both input vectors.
 */
struct wlf_vector3 wlf_vector3_cross(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Calculates the magnitude (length) of a 3D vector.
 * @param vec Input vector.
 * @return The magnitude of the vector as a non-negative scalar value.
 */
double wlf_vector3_magnitude(const struct wlf_vector3 *vec);

/**
 * @brief Normalizes a 3D vector to unit length.
 * @param vec Vector to normalize.
 * @return A new vector with the same direction but magnitude of 1.0.
 * @warning Behavior is undefined if the input vector has zero magnitude.
 */
struct wlf_vector3 wlf_vector3_normalize(const struct wlf_vector3 *vec);

/**
 * @brief Checks if two 3D vectors are exactly equal.
 * @param a First vector to compare.
 * @param b Second vector to compare.
 * @return true if vectors are exactly equal (all components match), false otherwise.
 */
bool wlf_vector3_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Checks if two 3D vectors are approximately equal within an epsilon tolerance.
 * @param a First vector to compare.
 * @param b Second vector to compare.
 * @param epsilon Maximum allowed difference between corresponding components.
 * @return true if the absolute difference between all corresponding components is less than epsilon,
 *         false otherwise.
 */
bool wlf_vector3_nearly_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b, double epsilon);

#endif // MATH_WLF_VECTOR3_H
