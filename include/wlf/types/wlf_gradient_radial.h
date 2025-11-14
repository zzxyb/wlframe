/**
 * @file        wlf_gradient_radial.h
 * @brief       Radial gradient implementation for wlframe.
 */

#ifndef TYPES_WLF_GRADIENT_RADIAL_H
#define TYPES_WLF_GRADIENT_RADIAL_H

#include "wlf_gradient.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Radial gradient structure.
 */
struct wlf_gradient_radial {
	struct wlf_gradient base; /**< Base gradient interface */
	double cx0, cy0, r0;      /**< Inner circle */
	double cx1, cy1, r1;      /**< Outer circle */
};

/**
 * @brief Creates a new radial gradient.
 */
struct wlf_gradient_radial *wlf_gradient_radial_create(double cx0, double cy0, double r0,
                                                       double cx1, double cy1, double r1);

#ifdef __cplusplus
}
#endif

#endif // TYPES_WLF_GRADIENT_RADIAL_H
