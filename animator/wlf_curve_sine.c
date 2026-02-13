#include "wlf/animator/wlf_curve_sine.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>

static float in_sine_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_sine(clamp_t(t));
}

static float out_sine_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_out_sine(clamp_t(t));
}

static float in_out_sine_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_out_sine(clamp_t(t));
}

static float out_in_sine_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	t = clamp_t(t);

	return t < 0.5f ? ease_out_sine(t * 2.0f) * 0.5f :
		ease_in_sine(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

static void sine_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_sine *sine =
		wlf_curve_sine_from_curve(curve);
	free(sine);
}

static const struct wlf_curve_impl in_sine_impl = {
	.value_at = in_sine_value_at,
	.destroy = sine_curve_destroy,
};

static const struct wlf_curve_impl out_sine_impl = {
	.value_at = out_sine_value_at,
	.destroy = sine_curve_destroy,
};

static const struct wlf_curve_impl in_out_sine_impl = {
	.value_at = in_out_sine_value_at,
	.destroy = sine_curve_destroy,
};

static const struct wlf_curve_impl out_in_sine_impl = {
	.value_at = out_in_sine_value_at,
	.destroy = sine_curve_destroy,
};

struct wlf_curve *wlf_curve_in_sine_create(void) {
	struct wlf_curve_sine *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_sine");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_sine_impl);
	curve->type = WLF_CURVE_IN;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_sine_create(void) {
	struct wlf_curve_sine *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_sine");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_sine_impl);
	curve->type = WLF_CURVE_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_in_out_sine_create(void) {
	struct wlf_curve_sine *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_sine");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_out_sine_impl);
	curve->type = WLF_CURVE_IN_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_in_sine_create(void) {
	struct wlf_curve_sine *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_sine");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_in_sine_impl);
	curve->type = WLF_CURVE_OUT_IN;

	return &curve->base;
}

bool wlf_curve_is_sine(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &in_sine_impl ||
		curve->impl == &out_sine_impl ||
		curve->impl == &in_out_sine_impl ||
		curve->impl == &out_in_sine_impl;
}

struct wlf_curve_sine *wlf_curve_sine_from_curve(
		struct wlf_curve *curve) {

	if (!wlf_curve_is_sine(curve)) {
		return NULL;
	}

	struct wlf_curve_sine *curve_sine = wlf_container_of(curve, curve_sine, base);

	return curve_sine;
}
