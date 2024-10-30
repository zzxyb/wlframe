#ifndef WLF_POINT_H
#define WLF_POINT_H

#include <stdbool.h>
#include <math.h>

/**
 * @brief A structure representing a 2D integer point
 */
struct wlf_point {
	int x;  /**< The x coordinate */
	int y;  /**< The y coordinate */
};

/**
 * @brief A structure representing a 2D floating-point point
 */
struct wlf_fpoint {
	double x;  /**< The x coordinate */
	double y;  /**< The y coordinate */
};

/**
 * @brief Common constant points
 */
static const struct wlf_point WLF_POINT_ZERO = {0, 0};        /**< Origin point (0,0) */
static const struct wlf_point WLF_POINT_UNIT = {1, 1};        /**< Unit point (1,1) */
static const struct wlf_point WLF_POINT_UNIT_X = {1, 0};      /**< Unit vector in x direction */
static const struct wlf_point WLF_POINT_UNIT_Y = {0, 1};      /**< Unit vector in y direction */

static const struct wlf_fpoint WLF_FPOINT_ZERO = {0.0, 0.0};    /**< Origin point (0.0,0.0) */
static const struct wlf_fpoint WLF_FPOINT_UNIT = {1.0, 1.0};    /**< Unit point (1.0,1.0) */
static const struct wlf_fpoint WLF_FPOINT_UNIT_X = {1.0, 0.0};  /**< Unit vector in x direction */
static const struct wlf_fpoint WLF_FPOINT_UNIT_Y = {0.0, 1.0};  /**< Unit vector in y direction */

/**
 * @brief Creates a new integer point
 * @param x The x coordinate
 * @param y The y coordinate
 * @return A new wlf_point structure
 */
struct wlf_point wlf_point_create(int x, int y);

/**
 * @brief Converts a 2D integer point to a string representation
 * @param point Source point
 * @return A string representing the point
 */
char* wlf_point_to_str(const struct wlf_point *point);

/**
 * @brief Checks if two integer points are equal
 * @param a First point
 * @param b Second point
 * @return true if points are equal, false otherwise
 */
bool wlf_point_equal(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Checks if an integer point is zero (origin)
 * @param p Point to check
 * @return true if point is zero, false otherwise
 */
bool wlf_point_is_zero(const struct wlf_point *p);

/**
 * @brief Adds two integer points
 * @param a First point
 * @param b Second point
 * @return Result of a + b
 */
struct wlf_point wlf_point_add(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Subtracts two integer points
 * @param a First point
 * @param b Second point
 * @return Result of a - b
 */
struct wlf_point wlf_point_subtract(const struct wlf_point *a, const struct wlf_point *b);

/**
 * @brief Multiplies an integer point by a scalar
 * @param p Point to multiply
 * @param scalar Scalar value
 * @return Result of p * scalar
 */
struct wlf_point wlf_point_multiply(const struct wlf_point *p, int scalar);

/**
 * @brief Calculates Manhattan distance between two points
 * @param p1 First point
 * @param p2 Second point
 * @return Manhattan distance |x1-x2| + |y1-y2|
 */
int wlf_point_manhattan_distance(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Calculates Euclidean distance between two points
 * @param p1 First point
 * @param p2 Second point
 * @return Euclidean distance sqrt((x1-x2)² + (y1-y2)²)
 */
double wlf_point_euclidean_distance(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Creates a new floating-point point
 * @param x The x coordinate
 * @param y The y coordinate
 * @return A new wlf_fpoint structure
 */
struct wlf_fpoint wlf_fpoint_create(double x, double y);

/**
 * @brief Converts a 2D floating-point point to a string representation
 * @param point Source floating-point point
 * @return A string representing the floating-point point
 */
char* wlf_fpoint_to_str(const struct wlf_fpoint *point);

/**
 * @brief Checks if two floating-point points are equal
 * @param a First point
 * @param b Second point
 * @return true if points are equal, false otherwise
 */
bool wlf_fpoint_equal(const struct wlf_fpoint *a, const struct wlf_fpoint *b);

/**
 * @brief Checks if two floating-point points are nearly equal within epsilon
 * @param a First point
 * @param b Second point
 * @param epsilon Maximum allowed difference
 * @return true if points are nearly equal, false otherwise
 */
bool wlf_fpoint_nearly_equal(const struct wlf_fpoint *a, const struct wlf_fpoint *b, double epsilon);

/**
 * @brief Checks if a floating-point point is zero (origin)
 * @param p Point to check
 * @return true if point is zero, false otherwise
 */
bool wlf_fpoint_is_zero(const struct wlf_fpoint *p);

/**
 * @brief Adds two floating-point points
 * @param a First point
 * @param b Second point
 * @return Result of a + b
 */
struct wlf_fpoint wlf_fpoint_add(const struct wlf_fpoint *a, const struct wlf_fpoint *b);

/**
 * @brief Subtracts two floating-point points
 * @param a First point
 * @param b Second point
 * @return Result of a - b
 */
struct wlf_fpoint wlf_fpoint_subtract(const struct wlf_fpoint *a, const struct wlf_fpoint *b);

/**
 * @brief Multiplies a floating-point point by a scalar
 * @param p Point to multiply
 * @param scalar Scalar value
 * @return Result of p * scalar
 */
struct wlf_fpoint wlf_fpoint_multiply(const struct wlf_fpoint *p, double scalar);

/**
 * @brief Divides a floating-point point by a scalar
 * @param p Point to divide
 * @param scalar Scalar value
 * @return Result of p / scalar
 */
struct wlf_fpoint wlf_fpoint_divide(const struct wlf_fpoint *p, double scalar);

/**
 * @brief Negates a floating-point point
 * @param p Point to negate
 * @return Negated point (-x, -y)
 */
struct wlf_fpoint wlf_fpoint_negate(const struct wlf_fpoint *p);

/**
 * @brief Calculates Manhattan distance between two floating-point points
 * @param p1 First point
 * @param p2 Second point
 * @return Manhattan distance |x1-x2| + |y1-y2|
 */
double wlf_fpoint_manhattan_distance(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2);

/**
 * @brief Calculates Euclidean distance between two floating-point points
 * @param p1 First point
 * @param p2 Second point
 * @return Euclidean distance sqrt((x1-x2)² + (y1-y2)²)
 */
double wlf_fpoint_euclidean_distance(const struct wlf_fpoint *p1, const struct wlf_fpoint *p2);

/**
 * @brief Calculates dot product of two floating-point points
 * @param a First point
 * @param b Second point
 * @return Dot product (a.x * b.x + a.y * b.y)
 */
double wlf_fpoint_dot_product(const struct wlf_fpoint *a, const struct wlf_fpoint *b);

/**
 * @brief Calculates the angle of a point relative to positive x-axis
 * @param p Point to calculate angle for
 * @return Angle in radians
 */
double wlf_fpoint_angle(const struct wlf_fpoint *p);

/**
 * @brief Calculates the angle between two points
 * @param a First point
 * @param b Second point
 * @return Angle in radians
 */
double wlf_fpoint_angle_between(const struct wlf_fpoint *a, const struct wlf_fpoint *b);

/**
 * @brief Rotates a point by given angle
 * @param p Point to rotate
 * @param angle_radians Angle in radians
 * @return Rotated point
 */
struct wlf_fpoint wlf_fpoint_rotate(const struct wlf_fpoint *p, double angle_radians);

/**
 * @brief Gets the length (magnitude) of a point vector
 * @param p Point to get length of
 * @return Length of the point vector
 */
double wlf_fpoint_length(const struct wlf_fpoint *p);

/**
 * @brief Gets the squared length of a point vector
 * @param p Point to get squared length of
 * @return Squared length of the point vector
 */
double wlf_fpoint_length_squared(const struct wlf_fpoint *p);

/**
 * @brief Checks if point is inside a circle
 * @param p Point to check
 * @param center Circle center
 * @param radius Circle radius
 * @return true if point is inside circle, false otherwise
 */
bool wlf_fpoint_in_circle(const struct wlf_fpoint *p, const struct wlf_fpoint *center, double radius);

/**
 * @brief Rounds floating-point coordinates to nearest integers
 * @param p Point to round
 * @return Rounded integer point
 */
struct wlf_point wlf_fpoint_round(const struct wlf_fpoint *p);

/**
 * @brief Floors floating-point coordinates to integers
 * @param p Point to floor
 * @return Floored integer point
 */
struct wlf_point wlf_fpoint_floor(const struct wlf_fpoint *p);

/**
 * @brief Ceils floating-point coordinates to integers
 * @param p Point to ceil
 * @return Ceiled integer point
 */
struct wlf_point wlf_fpoint_ceil(const struct wlf_fpoint *p);

/**
 * @brief Normalizes a floating-point point (makes it unit length)
 * @param p Point to normalize
 * @return Normalized point
 */
struct wlf_fpoint wlf_fpoint_normalize(const struct wlf_fpoint *p);

/**
 * @brief Performs linear interpolation between two points
 * @param a Start point
 * @param b End point
 * @param t Interpolation parameter (0.0 to 1.0)
 * @return Interpolated point
 */
struct wlf_fpoint wlf_fpoint_lerp(const struct wlf_fpoint *a, const struct wlf_fpoint *b, double t);

/**
 * @brief Calculates quadratic Bezier curve point
 * @param p0 Start point
 * @param p1 Control point
 * @param p2 End point
 * @param t Curve parameter (0.0 to 1.0)
 * @return Point on Bezier curve
 */
struct wlf_fpoint wlf_fpoint_bezier(const struct wlf_fpoint *p0, const struct wlf_fpoint *p1, 
                                   const struct wlf_fpoint *p2, double t);

/**
 * @brief Converts integer point to floating-point point
 * @param p Integer point to convert
 * @return Equivalent floating-point point
 */
struct wlf_fpoint wlf_point_to_fpoint(const struct wlf_point *p);

/**
 * @brief Converts floating-point point to integer point (truncates)
 * @param p Floating-point point to convert
 * @return Equivalent integer point
 */
struct wlf_point wlf_fpoint_to_point(const struct wlf_fpoint *p);

#endif
