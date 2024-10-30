#ifndef WLF_SIZE_H
#define WLF_SIZE_H

#include <stdbool.h>

/**
 * @brief A structure representing a 2D integer size
 */
struct wlf_size {
	int width;   /**< The width value */
	int height;  /**< The height value */
};

/**
 * @brief A structure representing a 2D floating-point size
 */
struct wlf_fsize {
	double width;   /**< The width value */
	double height;  /**< The height value */
};

static const struct wlf_size WLF_SIZE_ZERO = {0, 0};        /**< Zero size (0,0) */
static const struct wlf_size WLF_SIZE_UNIT = {1, 1};        /**< Unit size (1,1) */
static const struct wlf_fsize WLF_FSIZE_ZERO = {0.0, 0.0};  /**< Zero size (0.0,0.0) */
static const struct wlf_fsize WLF_FSIZE_UNIT = {1.0, 1.0};  /**< Unit size (1.0,1.0) */

/**
 * @brief Creates a new integer size
 * @param width The width value
 * @param height The height value
 * @return A new wlf_size structure
 */
struct wlf_size wlf_size_create(int width, int height);

/**
 * @brief Converts a size to a string representation
 * @param size Source size
 * @return A string representing the size
 */
char* wlf_size_to_str(const struct wlf_size *size);

/**
 * @brief Checks if two sizes are equal
 * @param a First size
 * @param b Second size
 * @return true if sizes are equal, false otherwise
 */
bool wlf_size_equal(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Checks if size is empty (zero width or height)
 * @param size Size to check
 * @return true if size is empty, false otherwise
 */
bool wlf_size_is_empty(const struct wlf_size *size);

/**
 * @brief Checks if size is valid (positive width and height)
 * @param size Size to check
 * @return true if size is valid, false otherwise
 */
bool wlf_size_is_valid(const struct wlf_size *size);

/**
 * @brief Adds two sizes
 * @param a First size
 * @param b Second size
 * @return Result of a + b
 */
struct wlf_size wlf_size_add(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Subtracts two sizes
 * @param a First size
 * @param b Second size
 * @return Result of a - b
 */
struct wlf_size wlf_size_subtract(const struct wlf_size *a, const struct wlf_size *b);

/**
 * @brief Multiplies a size by a scalar
 * @param size Size to multiply
 * @param scalar Scalar value
 * @return Scaled size
 */
struct wlf_size wlf_size_multiply(const struct wlf_size *size, int scalar);

/**
 * @brief Divides a size by a scalar
 * @param size Size to divide
 * @param scalar Scalar value
 * @return Divided size
 */
struct wlf_size wlf_size_divide(const struct wlf_size *size, int scalar);

/**
 * @brief Calculates the area of the size
 * @param size Size to calculate area for
 * @return Area (width * height)
 */
int wlf_size_area(const struct wlf_size *size);

/**
 * @brief Creates a new floating-point size
 * @param width The width value
 * @param height The height value
 * @return A new wlf_fsize structure
 */
struct wlf_fsize wlf_fsize_create(double width, double height);

/**
 * @brief Converts a floating-point size to a string representation
 * @param size Source floating-point size
 * @return A string representing the floating-point size
 */
char* wlf_fsize_to_str(const struct wlf_fsize *size);

/**
 * @brief Checks if two floating-point sizes are equal
 * @param a First size
 * @param b Second size
 * @return true if sizes are equal, false otherwise
 */
bool wlf_fsize_equal(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Checks if two floating-point sizes are nearly equal within epsilon
 * @param a First size
 * @param b Second size
 * @param epsilon Maximum allowed difference
 * @return true if sizes are nearly equal, false otherwise
 */
bool wlf_fsize_nearly_equal(const struct wlf_fsize *a, const struct wlf_fsize *b, double epsilon);

/**
 * @brief Adds two floating-point sizes together
 * @param a First floating-point size operand
 * @param b Second floating-point size operand
 * @return A new wlf_fsize structure where:
 *         - width = a->width + b->width
 *         - height = a->height + b->height
 */
struct wlf_fsize wlf_fsize_add(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Subtracts the second floating-point size from the first
 * @param a First floating-point size (minuend)
 * @param b Second floating-point size (subtrahend)
 * @return A new wlf_fsize structure where:
 *         - width = a->width - b->width
 *         - height = a->height - b->height
 */
struct wlf_fsize wlf_fsize_subtract(const struct wlf_fsize *a, const struct wlf_fsize *b);

/**
 * @brief Multiplies a floating-point size by a scalar value
 * @param size The floating-point size to be multiplied
 * @param scalar The scalar value to multiply by
 * @return A new wlf_fsize structure where:
 *         - width = size->width * scalar
 *         - height = size->height * scalar
 */
struct wlf_fsize wlf_fsize_multiply(const struct wlf_fsize *size, double scalar);

/**
 * @brief Divides a floating-point size by a scalar value
 * @param size The floating-point size to be divided
 * @param scalar The scalar value to divide by (must not be zero)
 * @return A new wlf_fsize structure where:
 *         - width = size->width / scalar
 *         - height = size->height / scalar
 */
struct wlf_fsize wlf_fsize_divide(const struct wlf_fsize *size, double scalar);

/**
 * @brief Calculates the area of the floating-point size
 * @param size Size to calculate area for
 * @return Area (width * height)
 */
double wlf_fsize_area(const struct wlf_fsize *size);

/**
 * @brief Converts integer size to floating-point size
 * @param size Integer size to convert
 * @return Equivalent floating-point size
 */
struct wlf_fsize wlf_size_to_fsize(const struct wlf_size *size);

/**
 * @brief Converts floating-point size to integer size
 * @param size Floating-point size to convert
 * @return Equivalent integer size
 */
struct wlf_size wlf_fsize_to_size(const struct wlf_fsize *size);

/**
 * @brief Rounds floating-point size to nearest integers
 * @param size Size to round
 * @return Rounded integer size
 */
struct wlf_size wlf_fsize_round(const struct wlf_fsize *size);

/**
 * @brief Floors floating-point size to integers
 * @param size Size to floor
 * @return Floored integer size
 */
struct wlf_size wlf_fsize_floor(const struct wlf_fsize *size);

/**
 * @brief Ceils floating-point size to integers
 * @param size Size to ceil
 * @return Ceiled integer size
 */
struct wlf_size wlf_fsize_ceil(const struct wlf_fsize *size);

#endif
