#include "wlf/animator/wlf_curve.h"

#include <assert.h>
#include <stdlib.h>

void wlf_curve_init(struct wlf_curve *curve,
		const struct wlf_curve_impl *impl) {
	assert(impl);
	assert(impl->value_at);
	assert(impl->destroy);

	curve->impl = impl;
}

float wlf_curve_value_at(const struct wlf_curve *curve, float t) {
	return curve->impl->value_at(curve, t);
}

void wlf_curve_destroy(struct wlf_curve *curve) {
	if (curve == NULL) {
		return;
	}

	curve->impl->destroy(curve);
}
