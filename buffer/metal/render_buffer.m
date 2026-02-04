#include "wlf/buffer/metal/render_buffer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/types/wlf_pixel_format.h"

#import <Metal/Metal.h>

#include <inttypes.h>
#include <stdlib.h>

/*
 * DRM format to MTLPixelFormat mapping table.
 *
 * DRM format names describe component order from MSB to LSB in a native-endian
 * 32-bit word. On little-endian hosts the byte order in memory is reversed, so
 * DRM_FORMAT_ARGB8888 (0xAARRGGBB) lays out as B, G, R, A in memory — which
 * matches MTLPixelFormatBGRA8Unorm.
 */
static const struct wlf_mtl_pixel_format formats[] = {
	/* 8-bit-per-channel BGRA / RGBA variants */
	{
		.drm_format = WLF_FORMAT_ARGB8888,
		.mtl_format = MTLPixelFormatBGRA8Unorm,
	},
	{
		.drm_format = WLF_FORMAT_XRGB8888,
		.mtl_format = MTLPixelFormatBGRA8Unorm,
	},
	{
		.drm_format = WLF_FORMAT_ABGR8888,
		.mtl_format = MTLPixelFormatRGBA8Unorm,
	},
	{
		.drm_format = WLF_FORMAT_XBGR8888,
		.mtl_format = MTLPixelFormatRGBA8Unorm,
	},
};

static void destroy_buffer(struct wlf_mtl_render_buffer *buffer) {
	wlf_linked_list_remove(&buffer->link);
	wlf_linked_list_remove(&buffer->buffer_destroy.link);

	if (buffer->texture != NULL) {
		id<MTLTexture> tex = (__bridge id<MTLTexture>)buffer->texture;
		[tex release];
		buffer->texture = NULL;
	}

	free(buffer);
}

static void handle_destroy_buffer(struct wlf_listener *listener, void *data __attribute__((unused))) {
	struct wlf_mtl_render_buffer *buffer =
		wlf_container_of(listener, buffer, buffer_destroy);
	destroy_buffer(buffer);
}

struct wlf_mtl_render_buffer *wlf_mtl_render_buffer_create(
		struct wlf_mtl_renderer *renderer, struct wlf_buffer *wlf_buffer) {
	@autoreleasepool {
		struct wlf_mtl_render_buffer *buffer = calloc(1, sizeof(*buffer));
		if (buffer == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mtl_render_buffer");
			return NULL;
		}

		buffer->buffer = wlf_buffer;
		buffer->renderer = renderer;

		void *data = NULL;
		uint32_t drm_format;
		size_t stride;
		if (!wlf_buffer_begin_data_ptr_access(wlf_buffer,
			    WLF_BUFFER_DATA_PTR_ACCESS_READ,
			    &data, &drm_format, &stride)) {
			wlf_log(WLF_ERROR, "Failed to get buffer data");
			goto failed;
		}

		uint32_t mtl_fmt = get_mtl_format_from_drm(drm_format);
		if (mtl_fmt == 0) {
			wlf_log(WLF_ERROR, "Unsupported Metal drm format 0x%"PRIX32, drm_format);
			wlf_buffer_end_data_ptr_access(wlf_buffer);
			goto failed;
		}

		id<MTLDevice> device =
			(__bridge id<MTLDevice>)renderer->dev->device;

		MTLTextureDescriptor *desc =
			[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:(MTLPixelFormat)mtl_fmt
			                                                  width:wlf_buffer->width
			                                                 height:wlf_buffer->height
			                                              mipmapped:NO];
		desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
		desc.storageMode = MTLStorageModeShared;

		id<MTLTexture> texture = [device newTextureWithDescriptor:desc];
		if (texture == nil) {
			wlf_log(WLF_ERROR, "Failed to create Metal texture");
			wlf_buffer_end_data_ptr_access(wlf_buffer);
			goto failed;
		}

		[texture replaceRegion:MTLRegionMake2D(0, 0, wlf_buffer->width, wlf_buffer->height)
		           mipmapLevel:0
		            withBytes:data
		          bytesPerRow:stride];

		wlf_buffer_end_data_ptr_access(wlf_buffer);

		[texture retain];
		buffer->texture = (__bridge void *)texture;

		buffer->buffer_destroy.notify = handle_destroy_buffer;
		wlf_signal_add(&wlf_buffer->events.destroy, &buffer->buffer_destroy);

		wlf_linked_list_insert(&renderer->buffers, &buffer->link);

		wlf_log(WLF_DEBUG, "Created Metal render buffer %dx%d",
			wlf_buffer->width, wlf_buffer->height);

		return buffer;

	failed:
		free(buffer);
		return NULL;
	}
}

void wlf_mtl_render_buffer_destroy(struct wlf_mtl_render_buffer *buffer) {
	destroy_buffer(buffer);
}

struct wlf_mtl_render_buffer *wlf_mtl_render_buffer_get(
		struct wlf_mtl_renderer *renderer, struct wlf_buffer *wlf_buffer) {
	struct wlf_mtl_render_buffer *buf;
	wlf_linked_list_for_each(buf, &renderer->buffers, link) {
		if (buf->buffer == wlf_buffer) {
			return buf;
		}
	}

	return NULL;
}

uint32_t get_mtl_format_from_drm(uint32_t fmt) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
		if (formats[i].drm_format == fmt) {
			return formats[i].mtl_format;
		}
	}

	wlf_log(WLF_ERROR, "DRM format 0x%"PRIX32" has no Metal equivalent", fmt);
	return 0;
}

const uint32_t *get_mtl_drm_formats(size_t *len) {
	static uint32_t drm_fmts[sizeof(formats) / sizeof(*formats)];
	static bool built = false;

	if (!built) {
		for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); ++i) {
			drm_fmts[i] = formats[i].drm_format;
		}
		built = true;
	}

	*len = sizeof(formats) / sizeof(*formats);
	return drm_fmts;
}
