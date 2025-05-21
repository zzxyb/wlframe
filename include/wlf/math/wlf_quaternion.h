/**
 * @file        wlf_quaternion.h
 * @brief       Quaternion math utility for wlframe.
 * @details     This file provides a structure and functions for quaternion operations,
 *              including creation, conversion, arithmetic, normalization, conjugation,
 *              inversion, norm calculation, and comparison.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_QUATERNION_H
#define MATH_WLF_QUATERNION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a quaternion.
 */
struct wlf_quaternion {
	double w;  /**< The scalar part */
	double x;  /**< The x component */
	double y;  /**< The y component */
	double z;  /**< The z component */
};

static const struct wlf_quaternion WLF_QUATERNION_IDENTITY = {1.0, 0.0, 0.0, 0.0}; /**< Identity quaternion (1,0,0,0) */

/**
 * @brief Creates a new quaternion from w, x, y, and z components.
 * @param w The scalar part.
 * @param x The x component.
 * @param y The y component.
 * @param z The z component.
 * @return A new wlf_quaternion structure.
 */
struct wlf_quaternion wlf_quaternion_make(double w, double x, double y, double z);

/**
 * @brief Converts a quaternion to a string representation.
 * @param quaternion Source quaternion.
 * @return A string representing the quaternion, Quaternion(w, x, y, z).
 */
char* wlf_quaternion_to_str(const struct wlf_quaternion *quaternion);


char* wlf_quaternion_to_str_prec(const struct wlf_quaternion *quaternion, uint8_t precision);

/**
 * @brief Adds two quaternions.
 * @param a First quaternion.
 * @param b Second quaternion.
 * @return Result of a + b.
 */
struct wlf_quaternion wlf_quaternion_add(const struct wlf_quaternion *a, const struct wlf_quaternion *b);

/**
 * @brief Subtracts two quaternions.
 * @param a First quaternion.
 * @param b Second quaternion.
 * @return Result of a - b.
 */
struct wlf_quaternion wlf_quaternion_subtract(const struct wlf_quaternion *a, const struct wlf_quaternion *b);

/**
 * @brief Multiplies two quaternions.
 * @param a First quaternion.
 * @param b Second quaternion.
 * @return Result of a * b.
 */
struct wlf_quaternion wlf_quaternion_multiply(const struct wlf_quaternion *a, const struct wlf_quaternion *b);

/**
 * @brief Conjugates a quaternion.
 * @param q Input quaternion.
 * @return Conjugated quaternion.
 */
struct wlf_quaternion wlf_quaternion_conjugate(const struct wlf_quaternion *q);

/**
 * @brief Calculates the norm (magnitude) of a quaternion.
 * @param q Input quaternion.
 * @return Norm of the quaternion.
 */
double wlf_quaternion_norm(const struct wlf_quaternion *q);

/**
 * @brief Normalizes a quaternion to unit length.
 * @param q Input quaternion.
 * @return Normalized quaternion.
 */
struct wlf_quaternion wlf_quaternion_normalize(const struct wlf_quaternion *q);

/**
 * @brief Calculates the inverse of a quaternion.
 * @param q Input quaternion (must be non-zero).
 * @return Inverted quaternion.
 */
struct wlf_quaternion wlf_quaternion_inverse(const struct wlf_quaternion *q);

/**
 * @brief Checks if two quaternions are equal.
 * @param a First quaternion.
 * @param b Second quaternion.
 * @return true if quaternions are equal, false otherwise.
 */
bool wlf_quaternion_equal(const struct wlf_quaternion *a, const struct wlf_quaternion *b);

/**
 * @brief Checks if two quaternions are nearly equal within epsilon.
 * @param a First quaternion.
 * @param b Second quaternion.
 * @param epsilon Maximum allowed difference.
 * @return true if quaternions are nearly equal, false otherwise.
 */
bool wlf_quaternion_nearly_equal(const struct wlf_quaternion *a, const struct wlf_quaternion *b, double epsilon);

#endif // MATH_WLF_QUATERNION_H
