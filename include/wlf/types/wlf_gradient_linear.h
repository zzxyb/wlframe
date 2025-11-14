/**
 * @file        wlf_gradient_linear.h
 * @brief       Linear gradient implementation for wlframe.
 * @details     Provides a linear gradient derived from the base wlf_gradient structure.
 */

#ifndef TYPES_WLF_GRADIENT_LINEAR_H
#define TYPES_WLF_GRADIENT_LINEAR_H

#include "wlf_gradient.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Linear gradient structure.
 */
struct wlf_gradient_linear {
	struct wlf_gradient base; /**< Base gradient interface */
	double x0, y0;            /**< Start point */
	double x1, y1;            /**< End point */
};

/**
 * @brief Creates a new linear gradient.
 * @param x0 Start X
 * @param y0 Start Y
 * @param x1 End X
 * @param y1 End Y
 * @return Newly allocated linear gradient or NULL on failure.
 */
struct wlf_gradient_linear *wlf_gradient_linear_create(double x0, double y0,
                                                       double x1, double y1);

#ifdef __cplusplus
}
#endif

#endif // TYPES_WLF_GRADIENT_LINEAR_H
