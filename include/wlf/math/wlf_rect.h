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
// #include "wlf/types/wlf_output.h"

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
 * @brief Parses a string representation of a rectangle and creates a wlf_rect structure.
 * @param str String representation of rectangle in format "(x,y,width,height)"
 *             or "(x, y, width, height)".
 * @param rect Pointer to the rectangle structure to fill.
 * @return true if parsing was successful, false otherwise.
 */
bool wlf_rect_from_str(const char *str, struct wlf_rect *rect);

/**
 * @brief Computes the intersection of two point rectangles.
 * @param dest Pointer to the rectangle where the intersection result will be written.
 * @param a First input rectangle.
 * @param b Second input rectangle.
 * @return true if the two rectangles have a non-empty intersection, false otherwise.
 * @note If there is no intersection, or if either input rectangle is empty, `dest` is set to a zero rectangle.
 */
bool wlf_rect_intersection(struct wlf_rect *dest, const struct wlf_rect *a,
	const struct wlf_rect *b);

/**
 * @brief Checks whether a point lies inside a point rectangle.
 * @param rect Rectangle to test against.
 * @param x X coordinate of the point.
 * @param y Y coordinate of the point.
 * @return true if the point is inside the rectangle, false otherwise.
 * @note Empty rectangles never contain any point.
 */
bool wlf_rect_contains_point(const struct wlf_rect *rect, int x, int y);

// /**
//  * @brief Checks whether one point rectangle fully contains another.
//  * @param bigger Candidate containing rectangle.
//  * @param smaller Candidate contained rectangle.
//  * @return true if `bigger` fully contains `smaller`, false otherwise.
//  * @note If either rectangle is empty, the result is false.
//  */
bool wlf_rect_contains_frect(const struct wlf_rect *bigger, const struct wlf_rect *smaller);

// /**
//  * @brief Applies an output transform to a point rectangle.
//  * @param dest Pointer to the rectangle where the transformed result will be written.
//  * @param rect Source rectangle to transform.
//  * @param transform Output transform to apply.
//  * @param width Width of the source or target coordinate space used by the transform.
//  * @param height Height of the source or target coordinate space used by the transform.
//  * @note For 90-degree and 270-degree rotations, the resulting rectangle width and height are swapped.
//  */
// void wlf_rect_transform(struct wlf_rect *dest, const struct wlf_rect *rect,
// 	enum wlf_output_transform transform, int width, int height);

#endif // MATH_WLF_RECT_H
