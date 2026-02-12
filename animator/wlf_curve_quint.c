#include "wlf/animator/wlf_curve_quint.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static float in_quint_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_quint(clamp_t(t));
}

static float out_quint_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_out_quint(clamp_t(t));
}

static float in_out_quint_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_out_quint(clamp_t(t));
}

static float out_in_quint_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	t = clamp_t(t);

	return t < 0.5f ? ease_out_quint(t * 2.0f) * 0.5f :
		ease_in_quint(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

static void quint_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_quint *quint =
		wlf_curve_quint_from_curve(curve);
	free(quint);
}

static const struct wlf_curve_impl in_quint_impl = {
	.value_at = in_quint_value_at,
	.destroy = quint_curve_destroy,
};

static const struct wlf_curve_impl out_quint_impl = {
	.value_at = out_quint_value_at,
	.destroy = quint_curve_destroy,
};

static const struct wlf_curve_impl in_out_quint_impl = {
	.value_at = in_out_quint_value_at,
	.destroy = quint_curve_destroy,
};

static const struct wlf_curve_impl out_in_quint_impl = {
	.value_at = out_in_quint_value_at,
	.destroy = quint_curve_destroy,
};

struct wlf_curve *wlf_curve_in_quint_create(void) {
	struct wlf_curve_quint *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_quint");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_quint_impl);
	curve->type = WLF_CURVE_IN;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_quint_create(void) {
	struct wlf_curve_quint *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_quint");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_quint_impl);
	curve->type = WLF_CURVE_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_in_out_quint_create(void) {
	struct wlf_curve_quint *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_quint");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_out_quint_impl);
	curve->type = WLF_CURVE_IN_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_in_quint_create(void) {
	struct wlf_curve_quint *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_quint");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_in_quint_impl);
	curve->type = WLF_CURVE_OUT_IN;

	return &curve->base;
}

bool wlf_curve_is_quint(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &in_quint_impl || curve->impl == &out_quint_impl ||
		curve->impl == &in_out_quint_impl || curve->impl == &out_in_quint_impl;
}

struct wlf_curve_quint *wlf_curve_quint_from_curve(
		struct wlf_curve *curve) {
	if (!wlf_curve_is_quint(curve)) {
		return NULL;
	}

	return (struct wlf_curve_quint *)curve;
}
