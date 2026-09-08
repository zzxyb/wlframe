#include "wlf/swapchain/metal/swapchain.h"

#include "wlf/allocator/metal/allocator.h"
#include "wlf/buffer/metal/buffer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/window/wlf_window.h"

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <assert.h>
#include <stdlib.h>

static CAMetalLayer *layer_from_native_handle(void *native_handle) {
	if (native_handle == NULL) {
		return nil;
	}
	id native = (__bridge id)native_handle;
	if ([native isKindOfClass:[CAMetalLayer class]]) {
		return native;
	}
	NSView *view = nil;
	if ([native isKindOfClass:[NSWindow class]]) {
		view = [(NSWindow *)native contentView];
	} else if ([native isKindOfClass:[NSView class]]) {
		view = native;
	}
	if (view == nil) {
		return nil;
	}
	view.wantsLayer = YES;
	if ([view.layer isKindOfClass:[CAMetalLayer class]]) {
		return (CAMetalLayer *)view.layer;
	}
	CAMetalLayer *layer = [CAMetalLayer layer];
	view.layer = layer;
	return layer;
}

static void swapchain_destroy(struct wlf_swapchain *base) {
	struct wlf_mtl_swapchain *swapchain =
		wlf_mtl_swapchain_from_swapchain(base);
	wlf_buffer_drop(swapchain->back);
	swapchain->back = NULL;
	base->back = NULL;
	[(CAMetalLayer *)swapchain->layer release];
	free(swapchain);
}

static bool swapchain_resize(struct wlf_swapchain *base, int width,
		int height) {
	if (width <= 0 || height <= 0 ||
			(width == base->width && height == base->height)) {
		return width == base->width && height == base->height;
	}
	struct wlf_mtl_swapchain *swapchain =
		wlf_mtl_swapchain_from_swapchain(base);
	struct wlf_buffer *buffer = wlf_allocator_create_buffer(base->allocator,
		(uint32_t)width, (uint32_t)height, &base->format);
	if (buffer == NULL) {
		return false;
	}
	wlf_buffer_drop(swapchain->back);
	swapchain->back = buffer;
	base->back = buffer;
	base->width = width;
	base->height = height;
	CAMetalLayer *layer = (__bridge CAMetalLayer *)swapchain->layer;
	layer.drawableSize = CGSizeMake(width, height);
	return true;
}

static void swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	WLF_UNUSED(damage);
	struct wlf_mtl_swapchain *swapchain =
		wlf_mtl_swapchain_from_swapchain(base);
	struct wlf_mtl_renderer *renderer =
		wlf_mtl_renderer_from_render(base->window->state.renderer);
	struct wlf_mtl_buffer *buffer =
		wlf_mtl_buffer_from_buffer(swapchain->back);
	CAMetalLayer *layer = (__bridge CAMetalLayer *)swapchain->layer;

	@autoreleasepool {
		id<CAMetalDrawable> drawable = [layer nextDrawable];
		if (drawable == nil) {
			wlf_log(WLF_ERROR, "failed to acquire CAMetalLayer drawable");
			return;
		}
		id<MTLCommandQueue> queue =
			(__bridge id<MTLCommandQueue>)renderer->command_queue;
		id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
		id<MTLBlitCommandEncoder> encoder =
			[command_buffer blitCommandEncoder];
		if (command_buffer == nil || encoder == nil) {
			wlf_log(WLF_ERROR, "failed to create Metal presentation command");
			return;
		}
		id<MTLTexture> source =
			(__bridge id<MTLTexture>)wlf_mtl_buffer_get_texture(buffer);
		MTLSize size = MTLSizeMake((NSUInteger)base->width,
			(NSUInteger)base->height, 1);
		[encoder copyFromTexture:source sourceSlice:0 sourceLevel:0
			sourceOrigin:MTLOriginMake(0, 0, 0) sourceSize:size
			toTexture:drawable.texture destinationSlice:0 destinationLevel:0
			destinationOrigin:MTLOriginMake(0, 0, 0)];
		[encoder endEncoding];
		wlf_window_arm_frame(base->window);
		[command_buffer presentDrawable:drawable];
		[command_buffer commit];
	}
}

static const struct wlf_swapchain_impl swapchain_impl = {
	.destroy = swapchain_destroy,
	.resize = swapchain_resize,
	.present = swapchain_present,
};

struct wlf_mtl_swapchain *wlf_mtl_swapchain_create(
		struct wlf_window *window, int width, int height,
		const struct wlf_render_format *format) {
	if (window == NULL || format == NULL || width <= 0 || height <= 0 ||
			!wlf_renderer_is_mtl(window->state.renderer) ||
			(format->format != WLF_FORMAT_ARGB8888 &&
			format->format != WLF_FORMAT_XRGB8888)) {
		return NULL;
	}
	CAMetalLayer *layer = layer_from_native_handle(
		wlf_window_native_handle(window));
	if (layer == nil) {
		wlf_log(WLF_ERROR, "Metal swapchain requires an NSView, NSWindow, "
			"or CAMetalLayer native handle");
		return NULL;
	}
	struct wlf_mtl_renderer *renderer =
		wlf_mtl_renderer_from_render(window->state.renderer);
	struct wlf_mtl_allocator *allocator =
		wlf_mtl_allocator_create(renderer->dev);
	if (allocator == NULL) {
		return NULL;
	}
	struct wlf_mtl_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		wlf_allocator_destroy(&allocator->base);
		return NULL;
	}
	wlf_swapchain_init(&swapchain->base, &allocator->base, &swapchain_impl,
		width, height);
	swapchain->base.window = window;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	swapchain->layer = (__bridge void *)[layer retain];
	layer.device = (__bridge id<MTLDevice>)renderer->dev->device;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	layer.framebufferOnly = NO;
	layer.opaque = !pixel_format_has_alpha(format->format);
	layer.drawableSize = CGSizeMake(width, height);
	swapchain->back = wlf_allocator_create_buffer(&allocator->base,
		(uint32_t)width, (uint32_t)height, format);
	if (swapchain->back == NULL) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	swapchain->base.back = swapchain->back;
	return swapchain;
}

bool wlf_swapchain_is_mtl(const struct wlf_swapchain *swapchain) {
	return swapchain != NULL && swapchain->impl == &swapchain_impl;
}

struct wlf_mtl_swapchain *wlf_mtl_swapchain_from_swapchain(
		struct wlf_swapchain *swapchain) {
	assert(wlf_swapchain_is_mtl(swapchain));
	struct wlf_mtl_swapchain *metal = NULL;
	return wlf_container_of(swapchain, metal, base);
}

void *wlf_mtl_swapchain_get_layer(const struct wlf_mtl_swapchain *swapchain) {
	return swapchain != NULL ? swapchain->layer : NULL;
}
