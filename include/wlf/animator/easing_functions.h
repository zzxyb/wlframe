/**
 * @file        easing_functions.h
 * @brief       Easing functions for smooth animation transitions.
 * @details     This file provides a comprehensive collection of easing functions commonly used
 *              in animations and transitions. Each function takes a normalized time parameter
 *              (0.0 to 1.0) and returns a modified value that creates different motion effects.
 *              Includes quadratic, cubic, quartic, quintic, sine, exponential, circular,
 *              elastic, back, and bounce easing variations.
 * @author      YaoBing Xiao
 * @date        2026-02-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-02, initial version\n
 */

#ifndef ANIMATOR_EASING_FUNCTIONS_H
#define ANIMATOR_EASING_FUNCTIONS_H

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
 * @brief Quadratic ease-in function.
 * @details Accelerates from zero velocity with a quadratic curve (t²).
 *          Creates a slow start that gradually accelerates.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_quad(float t) {
	return t * t;
}

/**
 * @brief Quadratic ease-out function.
 * @details Decelerates to zero velocity with a quadratic curve.
 *          Creates a fast start that gradually slows down.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_quad(float t) {
	return -t * (t - 2.0f);
}

/**
 * @brief Quadratic ease-in-out function.
 * @details Accelerates until halfway, then decelerates.
 *          Combines ease-in and ease-out for smooth start and end.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_quad(float t) {
	t *= 2.0f;
	if (t < 1.0f) {
		return 0.5f * t * t;
	}

	t -= 1.0f;

	return -0.5f * (t * (t - 2.0f) - 1.0f);
}

/**
 * @brief Cubic ease-in function.
 * @details Accelerates from zero velocity with a cubic curve (t³).
 *          Creates a slower start than quadratic easing.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_cubic(float t) {
	return t * t * t;
}

/**
 * @brief Cubic ease-out function.
 * @details Decelerates to zero velocity with a cubic curve.
 *          Creates a smoother deceleration than quadratic easing.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_cubic(float t) {
	t -= 1.0f;

	return t * t * t + 1.0f;
}

/**
 * @brief Cubic ease-in-out function.
 * @details Accelerates until halfway, then decelerates with cubic curves.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_cubic(float t) {
	t *= 2.0f;
	if (t < 1.0f) {
		return 0.5f * t * t * t;
	}

	t -= 2.0f;

	return 0.5f * (t * t * t + 2.0f);
}

/**
 * @brief Quartic ease-in function.
 * @details Accelerates from zero velocity with a quartic curve (t⁴).
 *          Creates an even slower start than cubic easing.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_quart(float t) {
	return t * t * t * t;
}

/**
 * @brief Quartic ease-out function.
 * @details Decelerates to zero velocity with a quartic curve.
 *          Creates a very smooth deceleration.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_quart(float t) {
	t -= 1.0f;

	return -(t * t * t * t - 1.0f);
}

/**
 * @brief Quartic ease-in-out function.
 * @details Accelerates until halfway, then decelerates with quartic curves.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_quart(float t) {
	t *= 2.0f;
	if (t < 1.0f) {
		return 0.5f * t * t * t * t;
	}

	t -= 2.0f;

	return -0.5f * (t * t * t * t - 2.0f);
}

/**
 * @brief Quintic ease-in function.
 * @details Accelerates from zero velocity with a quintic curve (t⁵).
 *          Creates the slowest start among polynomial easings.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_quint(float t) {
	return t * t * t * t * t;
}

/**
 * @brief Quintic ease-out function.
 * @details Decelerates to zero velocity with a quintic curve.
 *          Creates an extremely smooth deceleration.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_quint(float t) {
	t -= 1.0f;
	return t * t * t * t * t + 1.0f;
}

/**
 * @brief Quintic ease-in-out function.
 * @details Accelerates until halfway, then decelerates with quintic curves.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_quint(float t) {
	t *= 2.0f;
	if (t < 1.0f)
		return 0.5f * t * t * t * t * t;
	t -= 2.0f;
	return 0.5f * (t * t * t * t * t + 2.0f);
}

/**
 * @brief Sinusoidal ease-in function.
 * @details Accelerates using a sine wave curve.
 *          Creates a gentle, natural-feeling acceleration.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_sine(float t) {
	return -cosf(t * M_PI / 2.0f) + 1.0f;
}

/**
 * @brief Sinusoidal ease-out function.
 * @details Decelerates using a sine wave curve.
 *          Creates a gentle, natural-feeling deceleration.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_sine(float t) {
	return sinf(t * M_PI / 2.0f);
}

/**
 * @brief Sinusoidal ease-in-out function.
 * @details Accelerates and decelerates using a sine wave.
 *          Creates a very smooth, natural motion throughout.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_sine(float t) {
	return -0.5f * (cosf(M_PI * t) - 1.0f);
}

/**
 * @brief Exponential ease-in function.
 * @details Accelerates exponentially from zero velocity.
 *          Creates a very slow start with rapid acceleration near the end.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value. Returns 0.0 at t=0.0.
 */
static inline float ease_in_expo(float t) {
	return (t == 0.0f) ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}

/**
 * @brief Exponential ease-out function.
 * @details Decelerates exponentially to zero velocity.
 *          Creates a rapid start with very slow deceleration.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value. Returns 1.0 at t=1.0.
 */
static inline float ease_out_expo(float t) {
	return (t == 1.0f) ? 1.0f : (-powf(2.0f, -10.0f * t) + 1.0f);
}

/**
 * @brief Exponential ease-in-out function.
 * @details Accelerates and decelerates exponentially.
 *          Creates a dramatic motion with extreme slowness at both ends.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value. Returns 0.0 at t=0.0 and 1.0 at t=1.0.
 */
static inline float ease_in_out_expo(float t) {
	if (t == 0.0f) {
		return 0.0f;
	}

	if (t == 1.0f) {
		return 1.0f;
	}

	t *= 2.0f;
	if (t < 1.0f) {
		return 0.5f * powf(2.0f, 10.0f * (t - 1.0f));
	}

	t -= 1.0f;

	return 0.5f * (-powf(2.0f, -10.0f * t) + 2.0f);
}

/**
 * @brief Circular ease-in function.
 * @details Accelerates using a circular curve (quarter circle).
 *          Creates a smooth, accelerating curve based on circle geometry.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_circ(float t) {
	return -(sqrtf(1.0f - t * t) - 1.0f);
}

/**
 * @brief Circular ease-out function.
 * @details Decelerates using a circular curve.
 *          Creates a smooth, decelerating curve based on circle geometry.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_circ(float t) {
	t -= 1.0f;
	return sqrtf(1.0f - t * t);
}

/**
 * @brief Circular ease-in-out function.
 * @details Accelerates and decelerates using circular curves.
 *          Creates smooth motion based on circle geometry throughout.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_circ(float t) {
	t *= 2.0f;
	if (t < 1.0f) {
		return -0.5f * (sqrtf(1.0f - t * t) - 1.0f);
	}

	t -= 2.0f;

	return 0.5f * (sqrtf(1.0f - t * t) + 1.0f);
}

/**
 * @brief Elastic ease-in function.
 * @details Creates an oscillating effect like a spring being compressed then released.
 *          The motion starts slowly, then oscillates with increasing amplitude before reaching the target.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param amplitude The amplitude of the oscillation. Values >= 1.0 recommended.
 * @param period The period of the oscillation. Smaller values = faster oscillation.
 * @return Eased value. Returns 0.0 at t=0.0 and 1.0 at t=1.0.
 */
static inline float ease_in_elastic(float t, float amplitude, float period) {
	if (t == 0.0f) {
		return 0.0f;
	}

	if (t == 1.0f) {
		return 1.0f;
	}

	float s;
	if (amplitude < 1.0f) {
		amplitude = 1.0f;
		s = period / 4.0f;
	} else {
		s = period / (2.0f * M_PI) * asinf(1.0f / amplitude);
	}

	t -= 1.0f;

	return -(amplitude * powf(2.0f, 10.0f * t) * sinf((t - s) * (2.0f * M_PI) / period));
}

/**
 * @brief Elastic ease-out function.
 * @details Creates an oscillating effect like a spring overshooting and settling.
 *          The motion reaches the target quickly then oscillates with decreasing amplitude.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param amplitude The amplitude of the oscillation. Values >= 1.0 recommended.
 * @param period The period of the oscillation. Smaller values = faster oscillation.
 * @return Eased value. Returns 0.0 at t=0.0 and 1.0 at t=1.0.
 */
static inline float ease_out_elastic(float t, float amplitude, float period) {
	if (t == 0.0f) {
		return 0.0f;
	}

	if (t == 1.0f) {
		return 1.0f;
	}

	float s;
	if (amplitude < 1.0f) {
		amplitude = 1.0f;
		s = period / 4.0f;
	} else {
		s = period / (2.0f * M_PI) * asinf(1.0f / amplitude);
	}

	return amplitude * powf(2.0f, -10.0f * t) * sinf((t - s) * (2.0f * M_PI) / period) + 1.0f;
}

/**
 * @brief Elastic ease-in-out function.
 * @details Combines elastic ease-in and ease-out effects.
 *          Creates oscillating motion at both the start and end of the animation.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param amplitude The amplitude of the oscillation. Values >= 1.0 recommended.
 * @param period The period of the oscillation. Smaller values = faster oscillation.
 * @return Eased value. Returns 0.0 at t=0.0 and 1.0 at t=2.0 (normalized).
 */
static inline float ease_in_out_elastic(float t, float amplitude, float period) {
	if (t == 0.0f) {
		return 0.0f;
	}

	t *= 2.0f;
	if (t == 2.0f) {
		return 1.0f;
	}

	float s;
	if (amplitude < 1.0f) {
		amplitude = 1.0f;
		s = period / 4.0f;
	} else {
		s = period / (2.0f * M_PI) * asinf(1.0f / amplitude);
	}

	if (t < 1.0f) {
		t -= 1.0f;
		return -0.5f * (amplitude * powf(2.0f, 10.0f * t) *
			sinf((t - s) * (2.0f * M_PI) / period));
	}
	t -= 1.0f;
	return amplitude * powf(2.0f, -10.0f * t) *
			sinf((t - s) * (2.0f * M_PI) / period) * 0.5f + 1.0f;
}

/**
 * @brief Back ease-in function.
 * @details Creates a "wind-up" effect by pulling back slightly before moving forward.
 *          The motion goes backward initially, then accelerates past the start point.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param overshoot Controls how far back the motion pulls. Typical value: 1.70158.
 * @return Eased value.
 */
static inline float ease_in_back(float t, float overshoot) {
	return t * t * ((overshoot + 1.0f) * t - overshoot);
}

/**
 * @brief Back ease-out function.
 * @details Creates an overshoot effect by going past the target then settling back.
 *          The motion overshoots the target, then pulls back to the final position.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param overshoot Controls how far past the target the motion goes. Typical value: 1.70158.
 * @return Eased value.
 */
static inline float ease_out_back(float t, float overshoot) {
	t -= 1.0f;

	return t * t * ((overshoot + 1.0f) * t + overshoot) + 1.0f;
}

/**
 * @brief Back ease-in-out function.
 * @details Combines back ease-in and ease-out effects.
 *          Pulls back at the start, overshoots in the middle, then settles at the end.
 * @param t Normalized time parameter [0.0, 1.0].
 * @param overshoot Controls the amount of back/overshoot. Typical value: 1.70158.
 * @return Eased value.
 */
static inline float ease_in_out_back(float t, float overshoot) {
	t *= 2.0f;
	overshoot *= 1.525f;
	if (t < 1.0f) {
		return 0.5f * (t * t * ((overshoot + 1.0f) * t - overshoot));
	}

	t -= 2.0f;

	return 0.5f * (t * t * ((overshoot + 1.0f) * t + overshoot) + 2.0f);
}

/**
 * @brief Bounce ease-out function.
 * @details Creates a bouncing ball effect when reaching the target.
 *          The motion hits the target and bounces several times with decreasing amplitude.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_out_bounce(float t) {
	if (t < 1.0f / 2.75f) {
		return 7.5625f * t * t;
	} else if (t < 2.0f / 2.75f) {
		t -= 1.5f / 2.75f;
		return 7.5625f * t * t + 0.75f;
	} else if (t < 2.5f / 2.75f) {
		t -= 2.25f / 2.75f;
		return 7.5625f * t * t + 0.9375f;
	} else {
		t -= 2.625f / 2.75f;
		return 7.5625f * t * t + 0.984375f;
	}
}

/**
 * @brief Bounce ease-in function.
 * @details Creates a bouncing ball effect at the start of the motion.
 *          The motion bounces several times with increasing amplitude before reaching full speed.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_bounce(float t) {
	return 1.0f - ease_out_bounce(1.0f - t);
}

/**
 * @brief Bounce ease-in-out function.
 * @details Combines bounce effects at both the start and end.
 *          The motion bounces at the beginning, moves smoothly in the middle, then bounces at the end.
 * @param t Normalized time parameter [0.0, 1.0].
 * @return Eased value.
 */
static inline float ease_in_out_bounce(float t) {
	if (t < 0.5f) {
		return ease_in_bounce(t * 2.0f) * 0.5f;
	}

	return ease_out_bounce(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

#endif // ANIMATOR_EASING_FUNCTIONS_H
