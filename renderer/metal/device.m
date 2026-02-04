#include "wlf/renderer/metal/device.h"
#include "wlf/utils/wlf_log.h"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

struct wlf_mtl_device *wlf_mtl_device_create(void) {
	@autoreleasepool {
		// Get the default Metal device
		id<MTLDevice> mtl_device = MTLCreateSystemDefaultDevice();
		if (mtl_device == nil) {
			wlf_log(WLF_ERROR, "No Metal-capable device found");
			return NULL;
		}

		struct wlf_mtl_device *device = calloc(1, sizeof(*device));
		if (device == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mtl_device");
			[mtl_device release];
			return NULL;
		}

		// Store device with retained reference
		[mtl_device retain];
		device->device = (__bridge void *)mtl_device;
		
		// Copy device name
		const char *name_cstr = [mtl_device.name UTF8String];
		if (name_cstr != NULL) {
			device->name = strdup(name_cstr);
		}

		// Check if it's a low-power device
		device->is_low_power = mtl_device.lowPower;

		wlf_log(WLF_INFO, "Metal device created: %s (low-power: %s)",
			device->name ? device->name : "Unknown",
			device->is_low_power ? "yes" : "no");

		return device;
	}
}

void wlf_mtl_device_destroy(struct wlf_mtl_device *device) {
	if (device == NULL) {
		return;
	}

	if (device->device != NULL) {
		id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device->device;
		[mtl_device release];
		device->device = NULL;
	}

	free(device->name);
	free(device);
}
