#include "wlf/animator/wlf_animator_curve_back.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static float in_back_value_at(const struct wlf_animator_curve *curve, float t) {
	const struct wlf_animator_curve_back *back =
		(const struct wlf_animator_curve_back *)curve;

	return ease_in_back(clamp_t(t), back->overshoot);
}

static float out_back_value_at(const struct wlf_animator_curve *curve, float t) {
	const struct wlf_animator_curve_back *back =
		(const struct wlf_animator_curve_back *)curve;

	return ease_out_back(clamp_t(t), back->overshoot);
}

static float in_out_back_value_at(const struct wlf_animator_curve *curve, float t) {
	const struct wlf_animator_curve_back *back =
		(const struct wlf_animator_curve_back *)curve;

	return ease_in_out_back(clamp_t(t), back->overshoot);
}

static float out_in_back_value_at(const struct wlf_animator_curve *curve, float t) {
	const struct wlf_animator_curve_back *back =
		(const struct wlf_animator_curve_back *)curve;
	t = clamp_t(t);

	return t < 0.5f ? ease_out_back(t * 2.0f, back->overshoot) * 0.5f :
			ease_in_back(t * 2.0f - 1.0f, back->overshoot) * 0.5f + 0.5f;
}

static void default_destroy(struct wlf_animator_curve *curve) {
	free(curve);
}

static const struct wlf_animator_curve_impl in_back_impl = {
	.value_at = in_back_value_at,
	.destroy = default_destroy,
};

static const struct wlf_animator_curve_impl out_back_impl = {
	.value_at = out_back_value_at,
	.destroy = default_destroy,
};

static const struct wlf_animator_curve_impl in_out_back_impl = {
	.value_at = in_out_back_value_at,
	.destroy = default_destroy,
};

static const struct wlf_animator_curve_impl out_in_back_impl = {
	.value_at = out_in_back_value_at,
	.destroy = default_destroy,
};

struct wlf_animator_curve *wlf_animator_curve_in_back_create(float overshoot) {
	struct wlf_animator_curve_back *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_animator_curve_back");
		return NULL;
	}

	wlf_animator_curve_init(&curve->base, &in_back_impl);
	curve->type = WLF_ANIMATOR_CURVE_BACK_IN;
	curve->overshoot = overshoot;

	return &curve->base;
}

struct wlf_animator_curve *wlf_animator_curve_out_back_create(float overshoot) {
	struct wlf_animator_curve_back *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_animator_curve_back");
		return NULL;
	}

	wlf_animator_curve_init(&curve->base, &out_back_impl);
	curve->type = WLF_ANIMATOR_CURVE_BACK_OUT;
	curve->overshoot = overshoot;

	return &curve->base;
}

struct wlf_animator_curve *wlf_animator_curve_in_out_back_create(float overshoot) {
	struct wlf_animator_curve_back *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_animator_curve_back");
		return NULL;
	}

	wlf_animator_curve_init(&curve->base, &in_out_back_impl);
	curve->type = WLF_ANIMATOR_CURVE_BACK_IN_OUT;
	curve->overshoot = overshoot;

	return &curve->base;
}

struct wlf_animator_curve *wlf_animator_curve_out_in_back_create(float overshoot) {
	struct wlf_animator_curve_back *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_animator_curve_back");
		return NULL;
	}

	wlf_animator_curve_init(&curve->base, &out_in_back_impl);
	curve->type = WLF_ANIMATOR_CURVE_BACK_OUT_IN;
	curve->overshoot = overshoot;

	return &curve->base;
}

bool wlf_animator_curve_is_back(const struct wlf_animator_curve *curve) {
	if (curve == NULL) {
		return false;
	}

	return curve->impl == &in_back_impl ||
			curve->impl == &out_back_impl ||
			curve->impl == &in_out_back_impl ||
			curve->impl == &out_in_back_impl;
}

struct wlf_animator_curve_back *wlf_animator_curve_back_from_curve(
	struct wlf_animator_curve *curve) {
	if (!wlf_animator_curve_is_back(curve)) {
		return NULL;
	}

	return (struct wlf_animator_curve_back *)curve;
}
