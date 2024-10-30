#ifndef WLF_MATRIX4X4_H
#define WLF_MATRIX4X4_H

#include <stdbool.h>
#include <math.h>

/**
 * @brief A structure representing a 4x4 matrix
 */
struct wlf_matrix4x4 {
	double elements[4][4];  /**< 4x4 array of matrix elements */
};

/**
 * @brief Creates a new 4x4 matrix initialized to zero
 * @return A new wlf_matrix4x4 structure
 */
struct wlf_matrix4x4 wlf_matrix4x4_create_zero(void);

/**
 * @brief Creates a new 4x4 identity matrix
 * @return A new identity matrix
 */
struct wlf_matrix4x4 wlf_matrix4x4_identity(void);

/**
 * @brief Converts a 4x4 matrix to a string representation
 * @param matrix Source matrix
 * @return A string representing the matrix
 */
char* wlf_matrix4x4_to_str(const struct wlf_matrix4x4 *matrix);

/**
 * @brief Gets an element from the 4x4 matrix
 * @param matrix Source matrix
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @return Value at the specified position
 */
double wlf_matrix4x4_get(const struct wlf_matrix4x4 *matrix, int row, int col);

/**
 * @brief Sets an element in the 4x4 matrix
 * @param matrix Target matrix
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @param value Value to set
 */
void wlf_matrix4x4_set(struct wlf_matrix4x4 *matrix, int row, int col, double value);

/**
 * @brief Adds two 4x4 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a + b
 */
struct wlf_matrix4x4 wlf_matrix4x4_add(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b);

/**
 * @brief Subtracts two 4x4 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a - b
 */
struct wlf_matrix4x4 wlf_matrix4x4_subtract(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b);

/**
 * @brief Multiplies a 4x4 matrix by a scalar
 * @param matrix Input matrix
 * @param scalar Scalar value
 * @return Scaled matrix
 */
struct wlf_matrix4x4 wlf_matrix4x4_multiply_scalar(const struct wlf_matrix4x4 *matrix, double scalar);

/**
 * @brief Multiplies two 4x4 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a × b
 */
struct wlf_matrix4x4 wlf_matrix4x4_multiply(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b);

/**
 * @brief Transposes a 4x4 matrix
 * @param matrix Input matrix
 * @return Transposed matrix
 */
struct wlf_matrix4x4 wlf_matrix4x4_transpose(const struct wlf_matrix4x4 *matrix);

/**
 * @brief Calculates the determinant of a 4x4 matrix
 * @param matrix Input matrix
 * @return Determinant value
 */
double wlf_matrix4x4_determinant(const struct wlf_matrix4x4 *matrix);

/**
 * @brief Inverts a 4x4 matrix
 * @param matrix Input matrix (must be invertible)
 * @return Inverted matrix
 */
struct wlf_matrix4x4 wlf_matrix4x4_inverse(const struct wlf_matrix4x4 *matrix);

/**
 * @brief Checks if two 4x4 matrices are equal
 * @param a First matrix
 * @param b Second matrix
 * @return true if matrices are equal, false otherwise
 */
bool wlf_matrix4x4_equal(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b);

/**
 * @brief Checks if two 4x4 matrices are nearly equal within epsilon
 * @param a First matrix
 * @param b Second matrix
 * @param epsilon Maximum allowed difference
 * @return true if matrices are nearly equal, false otherwise
 */
bool wlf_matrix4x4_nearly_equal(const struct wlf_matrix4x4 *a, const struct wlf_matrix4x4 *b, double epsilon);

#endif
