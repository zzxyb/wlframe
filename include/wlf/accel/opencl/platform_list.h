#ifndef OPENCL_PLATFORM_LIST_H
#define OPENCL_PLATFORM_LIST_H

#include <CL/cl.h>

enum output_modes {
	CLINFO_HUMAN = 1, /* more human readable */
	CLINFO_RAW = 2, /* property-by-property */
	CLINFO_BOTH = CLINFO_HUMAN | CLINFO_RAW
};

/* Specify how we should handle conditional properties. */
enum cond_prop_modes {
	COND_PROP_CHECK = 0, /* default: check, skip if invalid */
	COND_PROP_TRY = 1, /* try, don't print an error if invalid */
	COND_PROP_SHOW = 2 /* try, print an error if invalid */
};

struct opt_out {
	enum output_modes mode;
	enum cond_prop_modes cond;
/* Specify that we should only print information about specific devices */
/* TODO proper memory management */
#define MAX_SELECTED_DEVICES 256
	cl_uint2 selected_devices[MAX_SELECTED_DEVICES];
	size_t num_selected_devices;

/* Specify that we should only print information about a specific property */
/* TODO proper memory management */
#define MAX_SELECTED_PROPS 256
	const char *selected_props[MAX_SELECTED_PROPS];
	size_t num_selected_props;

/* Specify if we should only be listing the platform and devices;
 * can be done in both human and raw mode, and only the platform
 * and device names (and number) will be shown
 * TODO check if terminal supports UTF-8 and use Unicode line-drawing
 * for the tree in list mode
 */
	cl_bool brief;
	cl_bool detailed; // !brief
	cl_bool offline;
	cl_bool null_platform;

/* JSON output for RAW */
	cl_bool json;

/* clGetDeviceInfo returns CL_INVALID_VALUE both for unknown properties
 * and when the destination variable is too small. Set the following to CL_TRUE
 * to check which one is the case
 */
	cl_bool check_size;
};

struct platform_data {
	char *pname; /* CL_PLATFORM_NAME */
	char *sname; /* CL_PLATFORM_ICD_SUFFIX_KHR or surrogate */
	cl_uint ndevs; /* number of devices */
	cl_bool has_amd_offline; /* has cl_amd_offline_devices extension */
};

struct platform_list {
	/* Number of platforms in the system */
	cl_uint num_platforms;
	/* Total number of devices across all platforms */
	cl_uint ndevs_total;
	/* Number of devices allocated in all_devs array */
	cl_uint alloc_devs;
	/* Highest OpenCL version supported by any platform.
	 * If the OpenCL library / ICD loader only supports
	 * a lower version, problems may arise (such as
	 * API calls causing segfaults or any other unexpected
	 * behavior
	 */
	cl_uint max_plat_version;
	/* Largest number of devices on any platform */
	cl_uint max_devs;
	/* Length of the longest platform sname */
	size_t max_sname_len;
	/* Array of platform IDs */
	cl_platform_id *platform;
	/* Array of device IDs (across all platforms) */
	cl_device_id *all_devs;
	/* Array of offsets in all_devs where the devices
	 * of each platform begin */
	cl_uint *dev_offset;
	/* Array of clinfo-specific platform data */
	struct platform_data *pdata;
};

cl_uint
alloc_plist(struct platform_list *plist, const struct opt_out *output);
void platform_list_destroy(struct platform_list *plist);
void plist_devs_reserve(struct platform_list *plist, cl_uint amount);
const cl_device_id *get_platform_devs(const struct platform_list *plist, cl_uint p);
cl_device_id get_platform_dev(const struct platform_list *plist, cl_uint p, cl_uint d);

#endif // OPENCL_PLATFORM_LIST_H
