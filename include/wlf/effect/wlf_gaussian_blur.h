/**
 * @file        wlf_gaussian_blur.h
 * @brief       Gaussian blur effect data for wlframe.
 * @details     This file provides a shape-based Gaussian blur primitive.
 *              The primitive stores independent horizontal and vertical
 *              standard deviations together with SVG input and result names.
 * @author      YaoBing Xiao
 * @date        2026-09-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-09-09, initial version\n
 */

#ifndef EFFECT_WLF_GAUSSIAN_BLUR_H
#define EFFECT_WLF_GAUSSIAN_BLUR_H

#include "wlf/shapes/wlf_shape.h"

#include <stdbool.h>

/**
 * @brief A shape-based Gaussian blur effect primitive.
 */
struct wlf_gaussian_blur {
	struct wlf_shape base; /**< Base shape */
	float std_deviation_x; /**< Horizontal Gaussian standard deviation */
	float std_deviation_y; /**< Vertical Gaussian standard deviation */
	char input[64];        /**< Optional input image name */
	char result[64];       /**< Optional result image name */
};

/**
 * @brief Creates a Gaussian blur effect.
 *
 * @param std_deviation_x Horizontal Gaussian standard deviation.
 * @param std_deviation_y Vertical Gaussian standard deviation.
 * @return New Gaussian blur or NULL if allocation fails.
 */
struct wlf_gaussian_blur *wlf_gaussian_blur_create(float std_deviation_x,
	float std_deviation_y);

/**
 * @brief Checks whether a shape is a Gaussian blur.
 *
 * @param shape Shape to test.
 * @return true if the shape is a Gaussian blur, otherwise false.
 */
bool wlf_shape_is_gaussian_blur(struct wlf_shape *shape);

/**
 * @brief Gets a Gaussian blur from its base shape.
 *
 * @param shape Base shape belonging to a Gaussian blur.
 * @return Gaussian blur containing the base shape.
 */
struct wlf_gaussian_blur *wlf_gaussian_blur_from_shape(struct wlf_shape *shape);

#endif // EFFECT_WLF_GAUSSIAN_BLUR_H
