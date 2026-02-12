/**
 * @file        curve_helpers.h
 * @brief       Helper utilities for curve calculations in wlframe.
 * @details     This file provides common mathematical utilities for curves,
 *              including clamping functions and mathematical constants.
 * @author      YaoBing Xiao
 * @date        2026-02-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-02, initial version\n
 */

#ifndef ANIMATOR_CURVE_HELPERS_H
#define ANIMATOR_CURVE_HELPERS_H

#include <math.h>

/**
 * @def M_PI
 * @brief Mathematical constant PI.
 * @details Defines PI value if not already defined by the math library.
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Clamps a time parameter to the range [0.0, 1.0].
 *
 * This function restricts the input value to be within the normalized
 * time range, where 0.0 represents the start and 1.0 represents the end.
 *
 * @param t The time parameter to clamp.
 * @return The clamped value, guaranteed to be in the range [0.0, 1.0].
 *
 * @code
 * float t1 = clamp_t(-0.5f);  // Returns 0.0f
 * float t2 = clamp_t(0.5f);   // Returns 0.5f
 * float t3 = clamp_t(1.5f);   // Returns 1.0f
 * @endcode
 */
static inline float clamp_t(float t) {
	if (t < 0.0f) {
		return 0.0f;
	}

	if (t > 1.0f) {
		return 1.0f;
	}

	return t;
}

#endif // ANIMATOR_CURVE_HELPERS_H
