#include "wlf/curve/wlf_curve.h"

#include <assert.h>
#include <stdlib.h>

void wlf_curve_init(struct wlf_curve *curve,
		const struct wlf_curve_impl *impl) {
	assert(impl);
	assert(impl->value_at);
	assert(impl->destroy);

	*curve = (struct wlf_curve) {
		.impl = impl,
		.listener = NULL,
		.user_data = NULL,
	};
}

float wlf_curve_value_at(const struct wlf_curve *curve, float t) {
	return curve->impl->value_at(curve, t);
}

void wlf_curve_destroy(struct wlf_curve *curve) {
	if (curve == NULL) {
		return;
	}

	if (curve->listener != NULL && curve->listener->destroy != NULL) {
		curve->listener->destroy(curve->user_data, curve);
	}

	if (curve->impl && curve->impl->destroy) {
		curve->impl->destroy(curve);
	} else {
		free(curve);
	}
}

void wlf_curve_add_listener(struct wlf_curve *curve,
		const struct wlf_curve_listener *listener, void *data) {
	curve->listener = listener;
	curve->user_data = data;
}
