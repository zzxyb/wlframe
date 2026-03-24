#ifndef OPENCL_ACCEL_H
#define OPENCL_ACCEL_H

#include "wlf/accel/wlf_accel.h"

#include <CL/cl.h>

struct opencl_accel {
	struct wlf_accel base;
	struct {
		bool cl_khr_icd;
		bool cl_amd_object_metadata;
		bool cl_khr_external_memory;
		bool cl_khr_semaphore;
		bool cl_khr_external_semaphore;
		bool cl_amd_offline_devices;
	} exts;
};

struct wlf_accel *wlf_opencl_accel_create(void);
bool wlf_accel_is_opencl(const struct wlf_accel *accel);
struct opencl_accel *opencl_accel_from_accel(struct wlf_accel *accel);

#endif // OPENCL_ACCEL_H
