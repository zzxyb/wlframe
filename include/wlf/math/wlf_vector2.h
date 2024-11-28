#ifndef WLF_VECTOR2_H
#define WLF_VECTOR2_H

#include <stdbool.h>

/**
 * @brief A structure representing a 2D vector
 */
struct wlf_vector2 {
	double u;  /**< The u component (x-axis) */
	double v;  /**< The v component (y-axis) */
};

static const struct wlf_vector2 WLF_VECTOR2_ZERO = {0.0, 0.0};    /**< Zero vector (0,0) */
static const struct wlf_vector2 WLF_VECTOR2_UNIT_U = {1.0, 0.0};  /**< Unit vector in u direction (1,0) */
static const struct wlf_vector2 WLF_VECTOR2_UNIT_V = {0.0, 1.0};  /**< Unit vector in v direction (0,1) */

/**
 * @brief Creates a new 2D vector
 * @param u The u component
 * @param v The v component
 * @return A new wlf_vector2 structure
 */
struct wlf_vector2 wlf_vector2_create(double u, double v);

/**
 * @brief Converts a 2D vector to a string representation
 * @param vector Source 2D vector
 * @return A string representing the 2D vector
 */
char* wlf_vector2_to_str(const struct wlf_vector2 *vector);

/**
 * @brief Adds two vectors
 * @param a First vector
 * @param b Second vector
 * @return Result of a + b
 */
struct wlf_vector2 wlf_vector2_add(const struct wlf_vector2 *a, const struct wlf_vector2 *b);

/**
 * @brief Subtracts two vectors
 * @param a First vector
 * @param b Second vector
 * @return Result of a - b
 */
struct wlf_vector2 wlf_vector2_subtract(const struct wlf_vector2 *a, const struct wlf_vector2 *b);

/**
 * @brief Multiplies a vector by a scalar
 * @param vec Vector to multiply
 * @param scalar Scalar value
 * @return Scaled vector
 */
struct wlf_vector2 wlf_vector2_multiply(const struct wlf_vector2 *vec, double scalar);

/**
 * @brief Divides a vector by a scalar
 * @param vec Vector to divide
 * @param scalar Scalar value (must not be zero)
 * @return Divided vector
 */
struct wlf_vector2 wlf_vector2_divide(const struct wlf_vector2 *vec, double scalar);

/**
 * @brief Calculates the dot product of two vectors
 * @param a First vector
 * @param b Second vector
 * @return Dot product (a·b)
 */
double wlf_vector2_dot(const struct wlf_vector2 *a, const struct wlf_vector2 *b);

/**
 * @brief Calculates the magnitude (length) of a vector
 * @param vec Input vector
 * @return Magnitude of the vector
 */
double wlf_vector2_magnitude(const struct wlf_vector2 *vec);

/**
 * @brief Normalizes a vector to unit length
 * @param vec Vector to normalize
 * @return Normalized vector
 */
struct wlf_vector2 wlf_vector2_normalize(const struct wlf_vector2 *vec);

/**
 * @brief Checks if two vectors are equal
 * @param a First vector
 * @param b Second vector
 * @return true if vectors are equal, false otherwise
 */
bool wlf_vector2_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b);

/**
 * @brief Checks if two vectors are nearly equal within epsilon
 * @param a First vector
 * @param b Second vector
 * @param epsilon Maximum allowed difference
 * @return true if vectors are nearly equal, false otherwise
 */
bool wlf_vector2_nearly_equal(const struct wlf_vector2 *a, const struct wlf_vector2 *b, double epsilon);

#endif
