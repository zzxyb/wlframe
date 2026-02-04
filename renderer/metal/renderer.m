#include "wlf/renderer/metal/renderer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/utils/wlf_env.h"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

static const struct wlf_renderer_impl renderer_impl;

struct wlf_renderer *wlf_mtl_renderer_create_from_backend(
		struct wlf_backend *backend __attribute__((unused))) {
	wlf_log(WLF_INFO, "Creating Metal renderer for macOS");
	
	struct wlf_mtl_device *device = wlf_mtl_device_create();
	if (device == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Metal device");
		return NULL;
	}

	return wlf_mtl_renderer_create_for_device(device);
}

void wlf_mtl_renderer_destroy(struct wlf_mtl_renderer *mtl_render) {
	if (mtl_render == NULL) {
		return;
	}

	if (mtl_render->command_queue != NULL) {
		id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_render->command_queue;
		[queue release];
		mtl_render->command_queue = NULL;
	}

	if (mtl_render->dev != NULL) {
		wlf_mtl_device_destroy(mtl_render->dev);
	}

	free(mtl_render);
}

static void renderer_destroy(struct wlf_renderer *renderer) {
	struct wlf_mtl_renderer *mtl_render = wlf_mtl_renderer_from_render(renderer);
	wlf_mtl_renderer_destroy(mtl_render);
}

static const struct wlf_renderer_impl renderer_impl = {
	.destroy = renderer_destroy,
};

bool wlf_renderer_is_mtl(struct wlf_renderer *wlf_renderer) {
	return wlf_renderer && wlf_renderer->impl == &renderer_impl;
}

struct wlf_mtl_renderer *wlf_mtl_renderer_from_render(struct wlf_renderer *wlf_renderer) {
	if (!wlf_renderer_is_mtl(wlf_renderer)) {
		return NULL;
	}
	return (struct wlf_mtl_renderer *)wlf_renderer;
}

struct wlf_renderer *wlf_mtl_renderer_create_for_device(struct wlf_mtl_device *device) {
	@autoreleasepool {
		struct wlf_mtl_renderer *renderer = calloc(1, sizeof(*renderer));
		if (renderer == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mtl_renderer");
			wlf_mtl_device_destroy(device);
			return NULL;
		}

		renderer->dev = device;
		renderer->base.impl = &renderer_impl;
		
		// Determine renderer type based on device characteristics
		if (device->is_low_power) {
			renderer->base.type = CPU;
		} else {
			renderer->base.type = GPU;
		}

		// Create command queue
		id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device->device;
		id<MTLCommandQueue> queue = [mtl_device newCommandQueue];
		if (queue == nil) {
			wlf_log(WLF_ERROR, "Failed to create Metal command queue");
			free(renderer);
			wlf_mtl_device_destroy(device);
			return NULL;
		}
		
		renderer->command_queue = (__bridge void *)queue;
		renderer->backend = NULL;

		wlf_log(WLF_INFO, "Metal renderer created successfully on device: %s", 
			device->name ? device->name : "Unknown");

		wlf_signal_init(&renderer->base.events.destroy);

		return &renderer->base;
	}
}
