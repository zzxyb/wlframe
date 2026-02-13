#include "wlf/animator/wlf_curve_expo.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>

static float in_expo_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_expo(clamp_t(t));
}

static float out_expo_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_out_expo(clamp_t(t));
}

static float in_out_expo_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_out_expo(clamp_t(t));
}

static float out_in_expo_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	t = clamp_t(t);

	return t < 0.5f ? ease_out_expo(t * 2.0f) * 0.5f :
		ease_in_expo(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

static void expo_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_expo *expo =
		wlf_curve_expo_from_curve(curve);
	free(expo);
}

static const struct wlf_curve_impl in_expo_impl = {
	.value_at = in_expo_value_at,
	.destroy = expo_curve_destroy,
};

static const struct wlf_curve_impl out_expo_impl = {
	.value_at = out_expo_value_at,
	.destroy = expo_curve_destroy,
};

static const struct wlf_curve_impl in_out_expo_impl = {
	.value_at = in_out_expo_value_at,
	.destroy = expo_curve_destroy,
};

static const struct wlf_curve_impl out_in_expo_impl = {
	.value_at = out_in_expo_value_at,
	.destroy = expo_curve_destroy,
};

struct wlf_curve *wlf_curve_in_expo_create(void) {
	struct wlf_curve_expo *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_expo");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_expo_impl);
	curve->type = WLF_CURVE_IN;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_expo_create(void) {
	struct wlf_curve_expo *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_expo");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_expo_impl);
	curve->type = WLF_CURVE_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_in_out_expo_create(void) {
	struct wlf_curve_expo *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_expo");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_out_expo_impl);
	curve->type = WLF_CURVE_IN_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_in_expo_create(void) {
	struct wlf_curve_expo *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_expo");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_in_expo_impl);
	curve->type = WLF_CURVE_OUT_IN;

	return &curve->base;
}

bool wlf_curve_is_expo(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &in_expo_impl ||
		curve->impl == &out_expo_impl ||
		curve->impl == &in_out_expo_impl ||
		curve->impl == &out_in_expo_impl;
}

struct wlf_curve_expo *wlf_curve_expo_from_curve(
		struct wlf_curve *curve) {
	if (!wlf_curve_is_expo(curve)) {
		return NULL;
	}

	struct wlf_curve_expo *curve_expo = wlf_container_of(curve, curve_expo, base);

	return curve_expo;
}
