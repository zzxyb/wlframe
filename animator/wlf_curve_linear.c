#include "wlf/animator/wlf_curve_linear.h"
#include "wlf/animator/curve_helpers.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

static float linear_value_at(const struct wlf_curve *curve, float t) {
	WLF_UNUSED(curve);

	return clamp_t(t);
}

static void linear_curve_destroy(struct wlf_curve *curve) {
	struct wlf_curve_linear *linear =
		wlf_curve_linear_from_curve(curve);
	free(linear);
}

static const struct wlf_curve_impl linear_impl = {
	.value_at = linear_value_at,
	.destroy = linear_curve_destroy,
};

struct wlf_curve *wlf_curve_linear_create(void) {
	struct wlf_curve_linear *curve = malloc(sizeof(*curve));
	if (curve == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_linear");
		return NULL;
	}

	wlf_curve_init(&curve->base, &linear_impl);

	return &curve->base;
}

bool wlf_curve_is_linear(const struct wlf_curve *curve) {
	if (curve == NULL || curve->impl == NULL) {
		return false;
	}

	return curve->impl == &linear_impl;
}

struct wlf_curve_linear *wlf_curve_linear_from_curve(
		struct wlf_curve *curve) {
	if (!wlf_curve_is_linear(curve)) {
		return NULL;
	}

	return (struct wlf_curve_linear *)curve;
}
