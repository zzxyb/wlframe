#ifndef ANIMATOR_WLF_ANIMATOR_CURVE_H
#define ANIMATOR_WLF_ANIMATOR_CURVE_H

enum wlf_animator_curve_type {
	WLF_ANIMATOR_CURVE_LINEAR, /**< Linear animation curve */
	WLF_ANIMATOR_CURVE_EASE_IN, /**< Ease-in animation curve */
	WLF_ANIMATOR_CURVE_EASE_OUT, /**< Ease-out animation curve */
	WLF_ANIMATOR_CURVE_EASE_IN_OUT, /**< Ease-in-out animation curve */
	WLF_ANIMATOR_CURVE_BOUNCE, /**< Bounce animation curve */
};

struct wlf_animator_curve {
	enum wlf_animator_curve_type type; /**< Type of the animation curve */
};

#endif // ANIMATOR_WLF_ANIMATOR_CURVE_H
