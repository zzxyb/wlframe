/**
 * @file        wlf_rect.h
 * @brief       2D integer rectangle math utility for wlframe.
 * @details     This file provides structures and functions for 2D integer rectangle operations,
 *              including creation, conversion, arithmetic, geometric queries, intersection, and union.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef MATH_WLF_RECT_H
#define MATH_WLF_RECT_H

#include "wlf/math/wlf_point.h"
#include "wlf/math/wlf_size.h"
#include <stdbool.h>

/**
 * @brief A structure representing a 2D integer rectangle.
 */
struct wlf_rect {
	int x;      /**< X coordinate of the top-left corner */
	int y;      /**< Y coordinate of the top-left corner */
	int width;  /**< Width of the rectangle */
	int height; /**< Height of the rectangle */
};

static const struct wlf_rect WLF_RECT_ZERO = {0, 0, 0, 0};            /**< Zero rectangle */
static const struct wlf_rect WLF_RECT_UNIT = {0, 0, 1, 1};            /**< Unit rectangle */

/**
 * @brief Creates a new rectangle from position and size.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param width Width value.
 * @param height Height value.
 * @return A new wlf_rect structure.
 */
struct wlf_rect wlf_rect_make(int x, int y, int width, int height);

/**
 * @brief Converts a rectangle to a string representation.
 * @param rect Source rectangle.
 * @return A string representing the rectangle, Rect(x, y, width, height).
 */
char* wlf_rect_to_str(const struct wlf_rect *rect);

/**
 * @brief Creates a rectangle from point and size.
 * @param pos Top-left position.
 * @param size Rectangle size.
 * @return A new wlf_rect structure.
 */
struct wlf_rect wlf_rect_from_point_size(const struct wlf_point *pos, const struct wlf_size *size);

/**
 * @brief Creates a rectangle from two points.
 * @param p1 First point (top-left).
 * @param p2 Second point (bottom-right).
 * @return A new wlf_rect structure.
 */
struct wlf_rect wlf_rect_from_points(const struct wlf_point *p1, const struct wlf_point *p2);

/**
 * @brief Checks if two rectangles are equal.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @return true if rectangles are equal, false otherwise.
 */
bool wlf_rect_equal(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Checks if rectangle is empty (zero width or height).
 * @param rect Rectangle to check.
 * @return true if rectangle is empty, false otherwise.
 */
bool wlf_rect_is_empty(const struct wlf_rect *rect);

/**
 * @brief Checks if rectangle is valid (positive width and height).
 * @param rect Rectangle to check.
 * @return true if rectangle is valid, false otherwise.
 */
bool wlf_rect_is_valid(const struct wlf_rect *rect);

/**
 * @brief Gets the position (top-left corner) of the rectangle.
 * @param rect Source rectangle.
 * @return Top-left point.
 */
struct wlf_point wlf_rect_get_position(const struct wlf_rect *rect);

/**
 * @brief Gets the size of the rectangle.
 * @param rect Source rectangle.
 * @return Rectangle size.
 */
struct wlf_size wlf_rect_get_size(const struct wlf_rect *rect);

/**
 * @brief Gets the center point of the rectangle.
 * @param rect Source rectangle.
 * @return Center point.
 */
struct wlf_point wlf_rect_get_center(const struct wlf_rect *rect);

/**
 * @brief Gets the top-left corner of the rectangle.
 * @param rect Source rectangle.
 * @return Top-left point.
 */
struct wlf_point wlf_rect_get_top_left(const struct wlf_rect *rect);

/**
 * @brief Gets the bottom-right corner of the rectangle.
 * @param rect Source rectangle.
 * @return Bottom-right point.
 */
struct wlf_point wlf_rect_get_bottom_right(const struct wlf_rect *rect);

/**
 * @brief Calculates the area of the rectangle.
 * @param rect Source rectangle.
 * @return Area of the rectangle.
 */
int wlf_rect_area(const struct wlf_rect *rect);

/**
 * @brief Calculates the perimeter of the rectangle.
 * @param rect Source rectangle.
 * @return Perimeter of the rectangle.
 */
int wlf_rect_perimeter(const struct wlf_rect *rect);

/**
 * @brief Moves rectangle by given offset.
 * @param rect Rectangle to move.
 * @param offset Movement offset.
 * @return Moved rectangle.
 */
struct wlf_rect wlf_rect_offset(const struct wlf_rect *rect, const struct wlf_point *offset);

/**
 * @brief Inflates rectangle by given amount.
 * @param rect Rectangle to inflate.
 * @param dx Horizontal inflation.
 * @param dy Vertical inflation.
 * @return Inflated rectangle.
 */
struct wlf_rect wlf_rect_inflate(const struct wlf_rect *rect, int dx, int dy);

/**
 * @brief Scales rectangle by given factors.
 * @param rect Rectangle to scale.
 * @param sx Horizontal scale factor.
 * @param sy Vertical scale factor.
 * @return Scaled rectangle.
 */
struct wlf_rect wlf_rect_scale(const struct wlf_rect *rect, double sx, double sy);

/**
 * @brief Checks if point with double coordinates is inside rectangle.
 * @param rect Rectangle to check.
 * @param x X coordinate of the point.
 * @param y Y coordinate of the point.
 * @return true if point is inside rectangle, false otherwise.
 */
bool wlf_rect_contains_point_d(const struct wlf_rect *rect, double x, double y);

/**
 * @brief Checks if point is inside rectangle.
 * @param rect Rectangle to check.
 * @param point Point to test.
 * @return true if point is inside rectangle, false otherwise.
 */
bool wlf_rect_contains_point(const struct wlf_rect *rect, const struct wlf_point *point);

/**
 * @brief Checks if one rectangle contains another.
 * @param outer Outer rectangle.
 * @param inner Inner rectangle to test.
 * @return true if outer contains inner, false otherwise.
 */
bool wlf_rect_contains_rect(const struct wlf_rect *outer, const struct wlf_rect *inner);

/**
 * @brief Checks if two rectangles intersect.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @return true if rectangles intersect, false otherwise.
 */
bool wlf_rect_intersects(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Calculates intersection of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @return Intersection rectangle (empty if no intersection).
 */
struct wlf_rect wlf_rect_intersection(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Calculates union (bounding box) of two rectangles.
 * @param a First rectangle.
 * @param b Second rectangle.
 * @return Union rectangle.
 */
struct wlf_rect wlf_rect_union(const struct wlf_rect *a, const struct wlf_rect *b);

/**
 * @brief Parses a string representation of a rectangle and creates a wlf_rect structure.
 * @param str String representation of rectangle in format "(x,y,width,height)"
 *             or "(x, y, width, height)".
 * @param rect Pointer to the rectangle structure to fill.
 * @return true if parsing was successful, false otherwise.
 */
bool wlf_rect_from_str(const char *str, struct wlf_rect *rect);

#endif // MATH_WLF_RECT_H
