#include "wlf/accel/wlf_accel.h"

#include <stdlib.h>
#include <assert.h>

struct wlf_accel *wlf_accel_autocreate(struct wlf_backend *backend) {
	return NULL;
}

void wlf_accel_destroy(struct wlf_accel *accel) {
	if (accel == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&accel->events.destroy, accel);

	assert(wlf_linked_list_empty(&accel->events.destroy.listener_list));

	if (accel->impl && accel->impl->destroy) {
		accel->impl->destroy(accel);
	} else {
		free(accel);
	}
}

void wlf_accel_init(struct wlf_accel *accel, const struct wlf_accel_impl *impl) {
	assert(impl);
	assert(impl->destroy);

	accel->impl = impl;

	wlf_signal_init(&accel->events.destroy);
}

