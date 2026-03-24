#ifndef CL_WLF_OCL_RUNTIME_H
#define CL_WLF_OCL_RUNTIME_H

#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_signal.h"

struct wlf_accel;

struct wlf_accel_impl {
	void (*destroy)(struct wlf_accel *accel);
};

struct wlf_accel {
	const struct wlf_accel_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;

	void *data;
};

struct wlf_accel *wlf_accel_autocreate(struct wlf_backend *backend);
void wlf_accel_destroy(struct wlf_accel *accel);
void wlf_accel_init(struct wlf_accel *accel,
	const struct wlf_accel_impl *impl);

#endif // CL_WLF_OCL_RUNTIME_H
