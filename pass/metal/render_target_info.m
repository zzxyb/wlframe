#include "wlf/pass/metal/render_target_info.h"

#include "wlf/buffer/metal/buffer.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <Metal/Metal.h>

#include <assert.h>
#include <stdlib.h>

static void target_destroy(struct wlf_render_target_info *base) {
	struct wlf_mtl_render_target_info *target =
		wlf_mtl_render_target_info_from_info(base);
	id<MTLRenderCommandEncoder> encoder =
		(__bridge id<MTLRenderCommandEncoder>)target->encoder;
	id<MTLCommandBuffer> command_buffer =
		(__bridge id<MTLCommandBuffer>)target->command_buffer;
	[encoder endEncoding];
	[command_buffer commit];
	[encoder release];
	[command_buffer release];
	free(target);
}

static struct wlf_renderer *target_get_renderer(
		struct wlf_render_target_info *base) {
	return &wlf_mtl_render_target_info_from_info(base)->renderer->base;
}

static const struct wlf_render_target_info_impl target_impl = {
	.destroy = target_destroy,
	.get_renderer = target_get_renderer,
};

struct wlf_mtl_render_target_info *wlf_mtl_begin_buffer_render_pass(
		struct wlf_mtl_buffer *buffer, struct wlf_mtl_renderer *renderer) {
	if (buffer == NULL || renderer == NULL || buffer->device != renderer->dev) {
		return NULL;
	}

	@autoreleasepool {
		id<MTLCommandQueue> queue =
			(__bridge id<MTLCommandQueue>)renderer->command_queue;
		id<MTLTexture> texture =
			(__bridge id<MTLTexture>)wlf_mtl_buffer_get_texture(buffer);
		id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
		if (command_buffer == nil) {
			return NULL;
		}

		MTLRenderPassDescriptor *descriptor = [MTLRenderPassDescriptor
			renderPassDescriptor];
		descriptor.colorAttachments[0].texture = texture;
		descriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		id<MTLRenderCommandEncoder> encoder =
			[command_buffer renderCommandEncoderWithDescriptor:descriptor];
		if (encoder == nil) {
			wlf_log(WLF_ERROR, "failed to create Metal render command encoder");
			return NULL;
		}

		struct wlf_mtl_render_target_info *target =
			calloc(1, sizeof(*target));
		if (target == NULL) {
			[encoder endEncoding];
			return NULL;
		}
		wlf_render_target_info_init(&target->base, &target_impl);
		target->base.logical_width = (int)buffer->base.width;
		target->base.logical_height = (int)buffer->base.height;
		target->base.buffer_width = (int)buffer->base.width;
		target->base.buffer_height = (int)buffer->base.height;
		target->buffer = buffer;
		target->renderer = renderer;
		target->command_buffer = (__bridge void *)[command_buffer retain];
		target->encoder = (__bridge void *)[encoder retain];
		return target;
	}
}

bool wlf_render_target_info_is_mtl(
		const struct wlf_render_target_info *render_target) {
	return render_target != NULL && render_target->impl == &target_impl;
}

struct wlf_mtl_render_target_info *wlf_mtl_render_target_info_from_info(
		struct wlf_render_target_info *render_target) {
	assert(wlf_render_target_info_is_mtl(render_target));
	struct wlf_mtl_render_target_info *target = NULL;
	return wlf_container_of(render_target, target, base);
}

void *wlf_mtl_render_target_get_encoder(
		const struct wlf_mtl_render_target_info *target) {
	return target != NULL ? target->encoder : NULL;
}
