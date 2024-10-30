#ifndef WLF_RECT_H
#define WLF_RECT_H

#include "wlf_point.h"
#include "wlf_size.h"

#include <stdbool.h>

/**
 * @brief A structure representing a 2D integer rectangle
 */
struct wlf_rect {
	int x;        /**< X coordinate of the top-left corner */
	int y;        /**< Y coordinate of the top-left corner */
	int width;    /**< Width of the rectangle */
	int height;   /**< Height of the rectangle */
};

/**
 * @brief A structure representing a 2D floating-point rectangle
 */
struct wlf_frect {
	double x;      /**< X coordinate of the top-left corner */
	double y;      /**< Y coordinate of the top-left corner */
	double width;  /**< Width of the rectangle */
	double height; /**< Height of the rectangle */
};

static const struct wlf_rect WLF_RECT_ZERO = {0, 0, 0, 0};          /**< Zero rectangle */
static const struct wlf_rect WLF_RECT_UNIT = {0, 0, 1, 1};          /**< Unit rectangle */
static const struct wlf_frect WLF_FRECT_ZERO = {0.0, 0.0, 0.0, 0.0}; /**< Zero rectangle */
static const struct wlf_frect WLF_FRECT_UNIT = {0.0, 0.0, 1.0, 1.0}; /**< Unit rectangle */

/**
 * @brief Creates a new rectangle from position and size
 * @param x X coordinate
 * @param y Y coordinate
 * @param width Width value
 * @param height Height value
 * @return A new wlf_rect structure
 */
struct wlf_rect wlf_rect_create(int x, int y, int width, int height);

/**
 * @brief Converts a rectangle to a string representation
 * @param rect Source rectangle
 * @return A string representing the rectangle, Rect(x, y, width, height)
 */
char* wlf_rect_to_str(const struct wlf_rect *rect);

/**
 * @brief Creates a rectangle from point and size
 * @param pos Top-left position
 * @param size Rectangle size
 * @return A new wlf_rect structure
 */
struct wlf_rect wlf_rect_from_size(const struct wlf_point *pos, const struct wlf_size *size);

/**
 * @brief Creates a rectangle from two points
 * @param p1 First point (top-left)
 * @param p2 Second point (bottom-right)
 * @return A new wlf_rect structure
 */
struct wlf_rect wlf_rect_from_points(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Checks if two rectangles are equal
 * @param a First rectangle
 * @param b Second rectangle
 * @return true if rectangles are equal, false otherwise
 */
bool wlf_rect_equal(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Checks if rectangle is empty (zero width or height)
 * @param rect Rectangle to check
 * @return true if rectangle is empty, false otherwise
 */
bool wlf_rect_is_empty(const struct wlf_rect *rect);

/**
 * @brief Checks if rectangle is valid (positive width and height)
 * @param rect Rectangle to check
 * @return true if rectangle is valid, false otherwise
 */
bool wlf_rect_is_valid(const struct wlf_rect *rect);

/**
 * @brief Gets the position (top-left corner) of the rectangle
 * @param rect Source rectangle
 * @return Top-left point
 */
struct wlf_point wlf_rect_get_position(const struct wlf_rect *rect);

/**
 * @brief Gets the size of the rectangle
 * @param rect Source rectangle
 * @return Rectangle size
 */
struct wlf_size wlf_rect_get_size(const struct wlf_rect *rect);

/**
 * @brief Gets the center point of the rectangle
 * @param rect Source rectangle
 * @return Center point
 */
struct wlf_point wlf_rect_get_center(const struct wlf_rect *rect);

/**
 * @brief Gets the top-left corner of the rectangle
 * @param rect Source rectangle
 * @return Top-left point
 */
struct wlf_point wlf_rect_get_top_left(const struct wlf_rect *rect);

/**
 * @brief Gets the bottom-right corner of the rectangle
 * @param rect Source rectangle
 * @return Bottom-right point
 */
struct wlf_point wlf_rect_get_bottom_right(const struct wlf_rect *rect);

/**
 * @brief Calculates the area of the rectangle
 * @param rect Source rectangle
 * @return Area of the rectangle
 */
int wlf_rect_area(const struct wlf_rect *rect);

/**
 * @brief Calculates the perimeter of the rectangle
 * @param rect Source rectangle
 * @return Perimeter of the rectangle
 */
int wlf_rect_perimeter(const struct wlf_rect *rect);

/**
 * @brief Moves rectangle by given offset
 * @param rect Rectangle to move
 * @param offset Movement offset
 * @return Moved rectangle
 */
struct wlf_rect wlf_rect_offset(const struct wlf_rect *rect, const struct wlf_point *offset);

/**
 * @brief Inflates rectangle by given amount
 * @param rect Rectangle to inflate
 * @param dx Horizontal inflation
 * @param dy Vertical inflation
 * @return Inflated rectangle
 */
struct wlf_rect wlf_rect_inflate(const struct wlf_rect *rect, int dx, int dy);

/**
 * @brief Scales rectangle by given factors
 * @param rect Rectangle to scale
 * @param sx Horizontal scale factor
 * @param sy Vertical scale factor
 * @return Scaled rectangle
 */
struct wlf_rect wlf_rect_scale(const struct wlf_rect *rect, double sx, double sy);

/**
 * @brief Checks if point with double coordinates is inside rectangle
 * @param rect Rectangle to check
 * @param x X coordinate of the point
 * @param y Y coordinate of the point
 * @return true if point is inside rectangle, false otherwise
 */
bool wlf_rect_contains_point_d(const struct wlf_rect *rect, double x, double y);

/**
 * @brief Checks if point is inside rectangle
 * @param rect Rectangle to check
 * @param point Point to test
 * @return true if point is inside rectangle, false otherwise
 */
bool wlf_rect_contains_point(const struct wlf_rect *rect, const struct wlf_point *point);

/**
 * @brief Checks if one rectangle contains another
 * @param outer Outer rectangle
 * @param inner Inner rectangle to test
 * @return true if outer contains inner, false otherwise
 */
bool wlf_rect_contains_rect(const struct wlf_rect *outer, const struct wlf_rect *inner);

/**
 * @brief Checks if two rectangles intersect
 * @param a First rectangle
 * @param b Second rectangle
 * @return true if rectangles intersect, false otherwise
 */
bool wlf_rect_intersects(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Calculates intersection of two rectangles
 * @param a First rectangle
 * @param b Second rectangle
 * @return Intersection rectangle (empty if no intersection)
 */
struct wlf_rect wlf_rect_intersection(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Calculates union (bounding box) of two rectangles
 * @param a First rectangle
 * @param b Second rectangle
 * @return Union rectangle
 */
struct wlf_rect wlf_rect_union(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Creates a new floating-point rectangle from position and size
 * @param x X coordinate of the top-left corner
 * @param y Y coordinate of the top-left corner
 * @param width Width of the rectangle
 * @param height Height of the rectangle
 * @return A new wlf_frect structure
 */
struct wlf_frect wlf_frect_create(double x, double y, double width, double height);

/**
 * @brief Converts a floating-point rectangle to a string representation
 * @param rect Source floating-point rectangle
 * @return A string representing the floating-point rectangle
 */
char* wlf_frect_to_str(const struct wlf_frect *rect);

/**
 * @brief Checks if two floating-point rectangles are exactly equal
 * @param a First floating-point rectangle
 * @param b Second floating-point rectangle
 * @return true if rectangles are exactly equal (all components match), false otherwise
 */
bool wlf_frect_equal(const struct wlf_frect *a, const struct wlf_frect *b);

/**
 * @brief Checks if two floating-point rectangles are approximately equal within an epsilon
 * @param a First floating-point rectangle
 * @param b Second floating-point rectangle
 * @param epsilon Maximum allowed difference between corresponding components
 * @return true if the absolute difference between all corresponding components is less than epsilon,
 *         false otherwise
 */
bool wlf_frect_nearly_equal(const struct wlf_frect *a, const struct wlf_frect *b, double epsilon);

/**
 * @brief Converts integer rectangle to floating-point rectangle
 * @param rect Integer rectangle to convert
 * @return Equivalent floating-point rectangle
 */
struct wlf_frect wlf_rect_to_frect(const struct wlf_rect *rect);

/**
 * @brief Converts floating-point rectangle to integer rectangle
 * @param rect Floating-point rectangle to convert
 * @return Equivalent integer rectangle
 */
struct wlf_rect wlf_frect_to_rect(const struct wlf_frect *rect);

/**
 * @brief Rounds floating-point rectangle to nearest integers
 * @param rect Rectangle to round
 * @return Rounded integer rectangle
 */
struct wlf_rect wlf_frect_round(const struct wlf_frect *rect);

/**
 * @brief Floors floating-point rectangle to integers
 * @param rect Rectangle to floor
 * @return Floored integer rectangle
 */
struct wlf_rect wlf_frect_floor(const struct wlf_frect *rect);

/**
 * @brief Ceils floating-point rectangle to integers
 * @param rect Rectangle to ceil
 * @return Ceiled integer rectangle
 */
struct wlf_rect wlf_frect_ceil(const struct wlf_frect *rect);

#endif
