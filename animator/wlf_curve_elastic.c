#include "wlf/animator/wlf_curve_elastic.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static float in_elastic_value_at(const struct wlf_curve *curve, float t) {
	const struct wlf_curve_elastic *elastic =
		(const struct wlf_curve_elastic *)curve;

	return ease_in_elastic(clamp_t(t), elastic->amplitude, elastic->period);
}

static float out_elastic_value_at(const struct wlf_curve *curve, float t) {
	const struct wlf_curve_elastic *elastic =
		(const struct wlf_curve_elastic *)curve;

	return ease_out_elastic(clamp_t(t), elastic->amplitude, elastic->period);
}

static float in_out_elastic_value_at(const struct wlf_curve *curve, float t) {
	const struct wlf_curve_elastic *elastic =
		(const struct wlf_curve_elastic *)curve;

	return ease_in_out_elastic(clamp_t(t), elastic->amplitude, elastic->period);
}

static float out_in_elastic_value_at(const struct wlf_curve *curve, float t) {
	const struct wlf_curve_elastic *elastic =
		(const struct wlf_curve_elastic *)curve;
	t = clamp_t(t);

	return t < 0.5f ? ease_out_elastic(t * 2.0f, elastic->amplitude, elastic->period) * 0.5f :
		ease_in_elastic(t * 2.0f - 1.0f, elastic->amplitude, elastic->period) * 0.5f + 0.5f;
}

static void elastic_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_elastic *elastic =
		wlf_curve_elastic_from_curve(curve);
	free(elastic);
}

static const struct wlf_curve_impl in_elastic_impl = {
	.value_at = in_elastic_value_at,
	.destroy = elastic_curve_destroy,
};

static const struct wlf_curve_impl out_elastic_impl = {
	.value_at = out_elastic_value_at,
	.destroy = elastic_curve_destroy,
};

static const struct wlf_curve_impl in_out_elastic_impl = {
	.value_at = in_out_elastic_value_at,
	.destroy = elastic_curve_destroy,
};

static const struct wlf_curve_impl out_in_elastic_impl = {
	.value_at = out_in_elastic_value_at,
	.destroy = elastic_curve_destroy,
};

struct wlf_curve *wlf_curve_in_elastic_create(float amplitude, float period) {
	struct wlf_curve_elastic *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_elastic");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_elastic_impl);
	curve->amplitude = amplitude;
	curve->period = period;
	curve->type = WLF_CURVE_IN;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_elastic_create(float amplitude, float period) {
	struct wlf_curve_elastic *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_elastic");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_elastic_impl);
	curve->amplitude = amplitude;
	curve->period = period;
	curve->type = WLF_CURVE_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_in_out_elastic_create(float amplitude, float period) {
	struct wlf_curve_elastic *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_elastic");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_out_elastic_impl);
	curve->amplitude = amplitude;
	curve->period = period;
	curve->type = WLF_CURVE_IN_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_in_elastic_create(float amplitude, float period) {
	struct wlf_curve_elastic *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_elastic");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_in_elastic_impl);
	curve->amplitude = amplitude;
	curve->period = period;
	curve->type = WLF_CURVE_OUT_IN;

	return &curve->base;
}

bool wlf_curve_is_elastic(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &in_elastic_impl ||
		curve->impl == &out_elastic_impl ||
		curve->impl == &in_out_elastic_impl ||
		curve->impl == &out_in_elastic_impl;
}

struct wlf_curve_elastic *wlf_curve_elastic_from_curve(
		struct wlf_curve *curve) {
	if (!wlf_curve_is_elastic(curve)) {
		return NULL;
	}

	return (struct wlf_curve_elastic *)curve;
}
