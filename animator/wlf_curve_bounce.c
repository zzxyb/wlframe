#include "wlf/animator/wlf_curve_bounce.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/animator/easing_functions.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <stdlib.h>
#include <string.h>

static float in_bounce_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_bounce(clamp_t(t));
}

static float out_bounce_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_out_bounce(clamp_t(t));
}

static float in_out_bounce_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return ease_in_out_bounce(clamp_t(t));
}

static float out_in_bounce_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	t = clamp_t(t);

	return t < 0.5f ? ease_out_bounce(t * 2.0f) * 0.5f :
		ease_in_bounce(t * 2.0f - 1.0f) * 0.5f + 0.5f;
}

static void bounce_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_bounce *bounce =
		wlf_curve_bounce_from_curve(curve);
	free(bounce);
}

static const struct wlf_curve_impl in_bounce_impl = {
	.value_at = in_bounce_value_at,
	.destroy = bounce_curve_destroy,
};

static const struct wlf_curve_impl out_bounce_impl = {
	.value_at = out_bounce_value_at,
	.destroy = bounce_curve_destroy,
};

static const struct wlf_curve_impl in_out_bounce_impl = {
	.value_at = in_out_bounce_value_at,
	.destroy = bounce_curve_destroy,
};

static const struct wlf_curve_impl out_in_bounce_impl = {
	.value_at = out_in_bounce_value_at,
	.destroy = bounce_curve_destroy,
};

struct wlf_curve *wlf_curve_in_bounce_create(void) {
	struct wlf_curve_bounce *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_bounce");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_bounce_impl);
	curve->type = WLF_CURVE_IN;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_bounce_create(void) {
	struct wlf_curve_bounce *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_bounce");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_bounce_impl);
	curve->type = WLF_CURVE_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_in_out_bounce_create(void) {
	struct wlf_curve_bounce *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_bounce");
		return NULL;
	}

	wlf_curve_init(&curve->base, &in_out_bounce_impl);
	curve->type = WLF_CURVE_IN_OUT;

	return &curve->base;
}

struct wlf_curve *wlf_curve_out_in_bounce_create(void) {
	struct wlf_curve_bounce *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_bounce");
		return NULL;
	}

	wlf_curve_init(&curve->base, &out_in_bounce_impl);
	curve->type = WLF_CURVE_OUT_IN;

	return &curve->base;
}

bool wlf_curve_is_bounce(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &in_bounce_impl ||
		curve->impl == &out_bounce_impl ||
		curve->impl == &in_out_bounce_impl ||
		curve->impl == &out_in_bounce_impl;
}

struct wlf_curve_bounce *wlf_curve_bounce_from_curve(
		struct wlf_curve *curve) {
	if (!wlf_curve_is_bounce(curve)) {
		return NULL;
	}

	struct wlf_curve_bounce *curve_bounce = wlf_container_of(curve, curve_bounce, base);

	return curve_bounce;
}

