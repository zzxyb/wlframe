#include "wlf/renderer/metal/renderer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/buffer/metal/buffer.h"
#include "wlf/pass/metal/render_target_info.h"
#include "wlf/texture/metal/texture.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_linked_list.h"

#import <Metal/Metal.h>
#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

static const struct wlf_renderer_impl renderer_impl;

struct wlf_mtl_renderer *wlf_mtl_renderer_create_from_backend(
		struct wlf_backend *backend) {
	wlf_log(WLF_INFO, "Creating Metal renderer for macOS");
	
	struct wlf_mtl_device *device = wlf_mtl_device_create();
	if (device == NULL) {
		wlf_log(WLF_ERROR, "Failed to create Metal device");
		return NULL;
	}

	struct wlf_mtl_renderer *renderer =
		wlf_mtl_renderer_create_for_device(device);
	if (renderer != NULL) {
		renderer->backend = backend;
	}
	return renderer;
}

static void renderer_destroy(struct wlf_renderer *renderer) {
	struct wlf_mtl_renderer *mtl_render = wlf_mtl_renderer_from_render(renderer);
	struct wlf_mtl_texture *texture, *tmp;
	wlf_linked_list_for_each_safe(texture, tmp, &mtl_render->textures, link) {
		wlf_texture_destroy(&texture->base);
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

static struct wlf_texture *renderer_texture_from_buffer(
		struct wlf_renderer *base, struct wlf_buffer *buffer) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_buffer(
		wlf_mtl_renderer_from_render(base), buffer);
	return texture != NULL ? &texture->base : NULL;
}

static struct wlf_render_target_info *renderer_begin_buffer_pass(
		struct wlf_renderer *base, struct wlf_buffer *buffer,
		const struct wlf_buffer_pass_options *options) {
	(void)options;
	if (!wlf_buffer_is_mtl(buffer)) {
		return NULL;
	}
	struct wlf_mtl_render_target_info *target =
		wlf_mtl_begin_buffer_render_pass(wlf_mtl_buffer_from_buffer(buffer),
			wlf_mtl_renderer_from_render(base));
	return target != NULL ? &target->base : NULL;
}

static const struct wlf_renderer_impl renderer_impl = {
	.destroy = renderer_destroy,
	.begin_buffer_pass = renderer_begin_buffer_pass,
	.texture_from_buffer = renderer_texture_from_buffer,
};

bool wlf_renderer_is_mtl(struct wlf_renderer *wlf_renderer) {
	return wlf_renderer != NULL && wlf_renderer->impl == &renderer_impl;
}

struct wlf_mtl_renderer *wlf_mtl_renderer_from_render(struct wlf_renderer *wlf_renderer) {
	assert(wlf_renderer->impl == &renderer_impl);

	struct wlf_mtl_renderer *mtl_renderer =
		wlf_container_of(wlf_renderer, mtl_renderer, base);

	return mtl_renderer;
}

struct wlf_mtl_renderer *wlf_mtl_renderer_create_for_device(struct wlf_mtl_device *device) {
	@autoreleasepool {
		struct wlf_mtl_renderer *renderer = calloc(1, sizeof(*renderer));
		if (renderer == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mtl_renderer");
			wlf_mtl_device_destroy(device);
			return NULL;
		}

		renderer->dev = device;
		wlf_renderer_init(&renderer->base, &renderer_impl);

		renderer->base.type = GPU;
		renderer->base.features.damage = true;

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

		wlf_linked_list_init(&renderer->buffers);
		wlf_linked_list_init(&renderer->textures);

		wlf_log(WLF_INFO, "Metal renderer created successfully on device: %s", 
			device->name ? device->name : "Unknown");

		return renderer;
	}
}
