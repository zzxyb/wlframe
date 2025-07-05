/**
 * @file        wlf_color.h
 * @brief       Color utility for wlframe.
 * @details     This file provides structures and functions for color operations,
 *              including creation, conversion, arithmetic, interpolation, and standard color constants.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef TYPES_WLF_COLOR_H
#define TYPES_WLF_COLOR_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A structure representing a color with RGBA components.
 */
struct wlf_color {
	double r;  /**< Red component (0.0-1.0) */
	double g;  /**< Green component (0.0-1.0) */
	double b;  /**< Blue component (0.0-1.0) */
	double a;  /**< Alpha component (0.0-1.0) */
};

/**
 * @brief Common standard colors.
 */
static const struct wlf_color WLF_COLOR_TRANSPARENT = {0.0, 0.0, 0.0, 0.0};  /**< Transparent color */
static const struct wlf_color WLF_COLOR_BLACK = {0.0, 0.0, 0.0, 1.0};        /**< Black color */
static const struct wlf_color WLF_COLOR_WHITE = {1.0, 1.0, 1.0, 1.0};        /**< White color */
static const struct wlf_color WLF_COLOR_RED = {1.0, 0.0, 0.0, 1.0};          /**< Red color */
static const struct wlf_color WLF_COLOR_GREEN = {0.0, 1.0, 0.0, 1.0};        /**< Green color */
static const struct wlf_color WLF_COLOR_BLUE = {0.0, 0.0, 1.0, 1.0};         /**< Blue color */
static const struct wlf_color WLF_COLOR_YELLOW = {1.0, 1.0, 0.0, 1.0};       /**< Yellow color */
static const struct wlf_color WLF_COLOR_CYAN = {0.0, 1.0, 1.0, 1.0};         /**< Cyan color */
static const struct wlf_color WLF_COLOR_MAGENTA = {1.0, 0.0, 1.0, 1.0};      /**< Magenta color */
static const struct wlf_color WLF_COLOR_ORANGE = {1.0, 0.5, 0.0, 1.0};       /**< Orange color */
static const struct wlf_color WLF_COLOR_PURPLE = {0.5, 0.0, 0.5, 1.0};       /**< Purple color */
static const struct wlf_color WLF_COLOR_GRAY = {0.5, 0.5, 0.5, 1.0};         /**< Gray color */
static const struct wlf_color WLF_COLOR_LIGHT_GRAY = {0.75, 0.75, 0.75, 1.0}; /**< Light gray color */
static const struct wlf_color WLF_COLOR_DARK_GRAY = {0.25, 0.25, 0.25, 1.0}; /**< Dark gray color */

/**
 * @brief Creates a new color from RGBA components.
 * @param r Red component (0.0-1.0).
 * @param g Green component (0.0-1.0).
 * @param b Blue component (0.0-1.0).
 * @param a Alpha component (0.0-1.0).
 * @return A new wlf_color structure.
 */
struct wlf_color wlf_color_make(double r, double g, double b, double a);

/**
 * @brief Creates a new opaque color from RGB components.
 * @param r Red component (0.0-1.0).
 * @param g Green component (0.0-1.0).
 * @param b Blue component (0.0-1.0).
 * @return A new wlf_color structure with alpha set to 1.0.
 */
struct wlf_color wlf_color_make_rgb(double r, double g, double b);

/**
 * @brief Creates a color from 8-bit RGBA values.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @param a Alpha component (0-255).
 * @return A new wlf_color structure.
 */
struct wlf_color wlf_color_from_rgba8(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * @brief Creates a color from 8-bit RGB values with full opacity.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @return A new wlf_color structure with alpha set to 1.0.
 */
struct wlf_color wlf_color_from_rgb8(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Creates a color from a 32-bit hexadecimal value (0xRRGGBBAA).
 * @param hex Hexadecimal color value.
 * @return A new wlf_color structure.
 */
struct wlf_color wlf_color_from_hex(uint32_t hex);

/**
 * @brief Creates a color from a 24-bit hexadecimal value (0xRRGGBB) with full opacity.
 * @param hex Hexadecimal color value.
 * @return A new wlf_color structure with alpha set to 1.0.
 */
struct wlf_color wlf_color_from_hex_rgb(uint32_t hex);

/**
 * @brief Converts a color to a string representation with specified precision.
 * @param color Source color.
 * @param precision Number of decimal places (0-15).
 * @return A string representing the color, or NULL if memory allocation fails.
 */
char* wlf_color_to_str_prec(const struct wlf_color *color, uint8_t precision);

/**
 * @brief Converts a color to a string representation with 3 decimal places.
 * @param color Source color.
 * @return A string representing the color, or NULL if memory allocation fails.
 */
char* wlf_color_to_str(const struct wlf_color *color);

/**
 * @brief Converts a color to a 32-bit hexadecimal value (0xRRGGBBAA).
 * @param color Source color.
 * @return 32-bit hexadecimal representation.
 */
uint32_t wlf_color_to_hex(const struct wlf_color *color);

/**
 * @brief Converts a color to a 24-bit hexadecimal value (0xRRGGBB).
 * @param color Source color.
 * @return 24-bit hexadecimal representation.
 */
uint32_t wlf_color_to_hex_rgb(const struct wlf_color *color);

/**
 * @brief Checks if two colors are exactly equal.
 * @param a First color.
 * @param b Second color.
 * @return true if colors are exactly equal (all components match), false otherwise.
 */
bool wlf_color_equal(const struct wlf_color *a, const struct wlf_color *b);

/**
 * @brief Checks if two colors are approximately equal within an epsilon.
 * @param a First color.
 * @param b Second color.
 * @param epsilon Maximum allowed difference between corresponding components.
 * @return true if the absolute difference between all corresponding components is less than epsilon,
 *         false otherwise.
 */
bool wlf_color_nearly_equal(const struct wlf_color *a, const struct wlf_color *b, double epsilon);

/**
 * @brief Clamps color components to the valid range [0.0, 1.0].
 * @param color Color to clamp.
 * @return Clamped color.
 */
struct wlf_color wlf_color_clamp(const struct wlf_color *color);

/**
 * @brief Linear interpolation between two colors.
 * @param a Start color.
 * @param b End color.
 * @param t Interpolation factor (0.0-1.0).
 * @return Interpolated color.
 */
struct wlf_color wlf_color_lerp(const struct wlf_color *a, const struct wlf_color *b, double t);

/**
 * @brief Multiplies a color by a scalar value.
 * @param color Source color.
 * @param scalar Scalar multiplier.
 * @return Scaled color.
 */
struct wlf_color wlf_color_scale(const struct wlf_color *color, double scalar);

/**
 * @brief Adds two colors component-wise.
 * @param a First color.
 * @param b Second color.
 * @return Sum of the colors.
 */
struct wlf_color wlf_color_add(const struct wlf_color *a, const struct wlf_color *b);

/**
 * @brief Multiplies two colors component-wise.
 * @param a First color.
 * @param b Second color.
 * @return Product of the colors.
 */
struct wlf_color wlf_color_multiply(const struct wlf_color *a, const struct wlf_color *b);

/**
 * @brief Creates a color with modified alpha.
 * @param color Source color.
 * @param alpha New alpha value (0.0-1.0).
 * @return Color with modified alpha.
 */
struct wlf_color wlf_color_with_alpha(const struct wlf_color *color, double alpha);

/**
 * @brief Parses a string representation of a color and creates a wlf_color structure.
 * @param str String representation of color in format "(r,g,b,a)" or "(r, g, b, a)".
 * @param color Pointer to the color structure to fill.
 * @return true if parsing was successful, false otherwise.
 */
bool wlf_color_from_str(const char *str, struct wlf_color *color);

#endif // TYPES_WLF_COLOR_H
