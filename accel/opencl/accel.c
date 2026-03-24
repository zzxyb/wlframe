#include "wlf/accel/opencl/accel.h"
#include "wlf/utils/wlf_log.h"

#include <CL/cl_ext.h>
#include <stdlib.h>
#include <string.h>

struct wlf_accel *wlf_opencl_accel_create(void) {
	struct opencl_accel *accel = calloc(1, sizeof(*accel));
	if (accel == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate opencl_accel");
		return NULL;
	}
	// cl_uint num_platforms = 0;
	// cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
	// if (err != CL_PLATFORM_NOT_FOUND_KHR) {
	// 	wlf_log(WLF_ERROR, "find OpenCl platform failed");
	// 	goto failed;
	// }

	return &accel->base;

// failed:
// 	wlf_accel_destroy(&accel->base);
// 	return NULL;
}

// bool wlf_accel_is_opencl(const struct wlf_accel *accel) {

// }

// struct opencl_accel *opencl_accel_from_accel(struct wlf_accel *accel) {

// }
