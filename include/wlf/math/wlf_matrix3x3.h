#ifndef WLF_MATRIX3X3_H
#define WLF_MATRIX3X3_H

#include <stdbool.h>
#include <math.h>

/**
 * @brief A structure representing a 3x3 matrix
 */
struct wlf_matrix3x3 {
	double elements[3][3];  /**< 3x3 array of matrix elements */
};

/**
 * @brief Creates a new 3x3 matrix initialized to zero
 * @return A new wlf_matrix3x3 structure
 */
struct wlf_matrix3x3 wlf_matrix3x3_create_zero(void);

/**
 * @brief Creates a new 3x3 identity matrix
 * @return A new identity matrix
 */
struct wlf_matrix3x3 wlf_matrix3x3_identity(void);

/**
 * @brief Converts a 3x3 matrix to a string representation
 * @param matrix Source matrix
 * @return A string representing the matrix
 */
char* wlf_matrix3x3_to_str(const struct wlf_matrix3x3 *matrix);

/**
 * @brief Gets an element from the 3x3 matrix
 * @param matrix Source matrix
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @return Value at the specified position
 */
double wlf_matrix3x3_get(const struct wlf_matrix3x3 *matrix, int row, int col);

/**
 * @brief Sets an element in the 3x3 matrix
 * @param matrix Target matrix
 * @param row Row index (0-based)
 * @param col Column index (0-based)
 * @param value Value to set
 */
void wlf_matrix3x3_set(struct wlf_matrix3x3 *matrix, int row, int col, double value);

/**
 * @brief Adds two 3x3 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a + b
 */
struct wlf_matrix3x3 wlf_matrix3x3_add(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b);

/**
 * @brief Subtracts two 3x3 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a - b
 */
struct wlf_matrix3x3 wlf_matrix3x3_subtract(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b);

/**
 * @brief Multiplies a 3x3 matrix by a scalar
 * @param matrix Input matrix
 * @param scalar Scalar value
 * @return Scaled matrix
 */
struct wlf_matrix3x3 wlf_matrix3x3_multiply_scalar(const struct wlf_matrix3x3 *matrix, double scalar);

/**
 * @brief Multiplies two 3x3 matrices
 * @param a First matrix
 * @param b Second matrix
 * @return Result of a × b
 */
struct wlf_matrix3x3 wlf_matrix3x3_multiply(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b);

/**
 * @brief Transposes a 3x3 matrix
 * @param matrix Input matrix
 * @return Transposed matrix
 */
struct wlf_matrix3x3 wlf_matrix3x3_transpose(const struct wlf_matrix3x3 *matrix);

/**
 * @brief Calculates the determinant of a 3x3 matrix
 * @param matrix Input matrix
 * @return Determinant value
 */
double wlf_matrix3x3_determinant(const struct wlf_matrix3x3 *matrix);

/**
 * @brief Inverts a 3x3 matrix
 * @param matrix Input matrix (must be invertible)
 * @return Inverted matrix
 */
struct wlf_matrix3x3 wlf_matrix3x3_inverse(const struct wlf_matrix3x3 *matrix);

/**
 * @brief Checks if two 3x3 matrices are equal
 * @param a First matrix
 * @param b Second matrix
 * @return true if matrices are equal, false otherwise
 */
bool wlf_matrix3x3_equal(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b);

/**
 * @brief Checks if two 3x3 matrices are nearly equal within epsilon
 * @param a First matrix
 * @param b Second matrix
 * @param epsilon Maximum allowed difference
 * @return true if matrices are nearly equal, false otherwise
 */
bool wlf_matrix3x3_nearly_equal(const struct wlf_matrix3x3 *a, const struct wlf_matrix3x3 *b, double epsilon);

#endif
