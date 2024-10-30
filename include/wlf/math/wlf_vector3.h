#ifndef WLF_VECTOR3_H
#define WLF_VECTOR3_H

/**
 * @brief A structure representing a 3D vector
 */
struct wlf_vector3 {
	double x;  /**< The x component */
	double y;  /**< The y component */
	double z;  /**< The z component */
};

static const struct wlf_vector3 WLF_VECTOR3_ZERO = {0.0, 0.0, 0.0};    /**< Zero vector (0,0,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_X = {1.0, 0.0, 0.0};  /**< Unit vector in x direction (1,0,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_Y = {0.0, 1.0, 0.0};  /**< Unit vector in y direction (0,1,0) */
static const struct wlf_vector3 WLF_VECTOR3_UNIT_Z = {0.0, 0.0, 1.0};  /**< Unit vector in z direction (0,0,1) */

/**
 * @brief Creates a new 3D vector
 * @param x The x component
 * @param y The y component
 * @param z The z component
 * @return A new wlf_vector3 structure
 */
struct wlf_vector3 wlf_vector3_create(double x, double y, double z);

/**
 * @brief Converts a 3D vector to a string representation
 * @param vector Source 3D vector
 * @return A string representing the 3D vector
 */
char* wlf_vector3_to_str(const struct wlf_vector3 *vector);

/**
 * @brief Adds two vectors
 * @param a First vector
 * @param b Second vector
 * @return Result of a + b
 */
struct wlf_vector3 wlf_vector3_add(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Subtracts two vectors
 * @param a First vector
 * @param b Second vector
 * @return Result of a - b
 */
struct wlf_vector3 wlf_vector3_subtract(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Multiplies a vector by a scalar
 * @param vec Vector to multiply
 * @param scalar Scalar value
 * @return Scaled vector
 */
struct wlf_vector3 wlf_vector3_multiply(const struct wlf_vector3 *vec, double scalar);

/**
 * @brief Divides a vector by a scalar
 * @param vec Vector to divide
 * @param scalar Scalar value (must not be zero)
 * @return Divided vector
 */
struct wlf_vector3 wlf_vector3_divide(const struct wlf_vector3 *vec, double scalar);

/**
 * @brief Calculates the dot product of two vectors
 * @param a First vector
 * @param b Second vector
 * @return Dot product (a·b)
 */
double wlf_vector3_dot(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Calculates the cross product of two vectors
 * @param a First vector
 * @param b Second vector
 * @return Cross product (a×b)
 */
struct wlf_vector3 wlf_vector3_cross(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Calculates the magnitude (length) of a vector
 * @param vec Input vector
 * @return Magnitude of the vector
 */
double wlf_vector3_magnitude(const struct wlf_vector3 *vec);

/**
 * @brief Normalizes a vector to unit length
 * @param vec Vector to normalize
 * @return Normalized vector
 */
struct wlf_vector3 wlf_vector3_normalize(const struct wlf_vector3 *vec);

/**
 * @brief Checks if two vectors are equal
 * @param a First vector
 * @param b Second vector
 * @return true if vectors are equal, false otherwise
 */
bool wlf_vector3_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b);

/**
 * @brief Checks if two vectors are nearly equal within epsilon
 * @param a First vector
 * @param b Second vector
 * @param epsilon Maximum allowed difference
 * @return true if vectors are nearly equal, false otherwise
 */
bool wlf_vector3_nearly_equal(const struct wlf_vector3 *a, const struct wlf_vector3 *b, double epsilon);

#endif
