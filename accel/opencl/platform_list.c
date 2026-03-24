#include "wlf/accel/opencl/platform_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

cl_uint alloc_plist(struct platform_list *plist, const struct opt_out *output)
{
	cl_uint num_platforms = plist->num_platforms;
	if (output->null_platform)
		num_platforms += 1;
	plist->platform = calloc(num_platforms, sizeof(*plist->platform));
	if (plist->platform == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to reallocate cl_platform_id");
		abort();
	}
	plist->dev_offset = calloc(num_platforms, sizeof(*plist->dev_offset));
	if (plist->dev_offset == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to reallocate platform device list offset");
		abort();
	}
	plist_devs_reserve(plist, num_platforms);
	plist->pdata = calloc(num_platforms, sizeof(*plist->pdata));
	if (plist->pdata == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to reallocate platform data");
		abort();
	}
	return num_platforms;
}

void platform_list_destroy(struct platform_list *plist) {
	if (plist == NULL) {
		return;
	}

	free(plist->platform);
	free(plist->all_devs);
	free(plist->dev_offset);
	for (cl_uint p = 0 ; p < plist->num_platforms; ++p) {
		free(plist->pdata[p].sname);
		free(plist->pdata[p].pname);
	}
	free(plist->pdata);
}

void plist_devs_reserve(struct platform_list *plist, cl_uint amount) {
	if (amount > plist->alloc_devs) {
		plist->all_devs = realloc(plist->all_devs, amount * sizeof(*plist->all_devs));
		if (plist->all_devs == NULL) {
			wlf_log_errno(WLF_ERROR, "failed to reallocate cl_device_id");
			abort();
		}
		plist->alloc_devs = amount;
	}
}

const cl_device_id *get_platform_devs(const struct platform_list *plist, cl_uint p) {
	return plist->all_devs + plist->dev_offset[p];
}

cl_device_id get_platform_dev(const struct platform_list *plist, cl_uint p, cl_uint d) {
	return get_platform_devs(plist, p)[d];
}
