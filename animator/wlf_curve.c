#include "wlf/animator/wlf_curve.h"

#include <assert.h>
#include <stdlib.h>

void wlf_curve_init(struct wlf_curve *curve,
		const struct wlf_curve_impl *impl) {
	assert(impl);
	assert(impl->value_at);
	assert(impl->destroy);

	curve->impl = impl;

	wlf_signal_init(&curve->events.destroy);
}

float wlf_curve_value_at(const struct wlf_curve *curve, float t) {
	return curve->impl->value_at(curve, t);
}

void wlf_curve_destroy(struct wlf_curve *curve) {
	if (curve == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&curve->events.destroy, curve);

	assert(wlf_linked_list_empty(&curve->events.destroy.listener_list));

	if (curve->impl && curve->impl->destroy) {
		curve->impl->destroy(curve);
	} else {
		free(curve);
	}
}
