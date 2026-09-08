#include "wlf/buffer/metal/buffer.h"

#include "wlf/renderer/metal/device.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <Metal/Metal.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

static MTLPixelFormat metal_pixel_format(uint32_t format) {
	switch (format) {
	case WLF_FORMAT_ARGB8888:
	case WLF_FORMAT_XRGB8888:
		return MTLPixelFormatBGRA8Unorm;
	case WLF_FORMAT_ABGR8888:
	case WLF_FORMAT_XBGR8888:
		return MTLPixelFormatRGBA8Unorm;
	case WLF_FORMAT_R8:
		return MTLPixelFormatR8Unorm;
	default:
		return MTLPixelFormatInvalid;
	}
}

static void buffer_destroy(struct wlf_buffer *base) {
	struct wlf_mtl_buffer *buffer = wlf_mtl_buffer_from_buffer(base);
	wlf_buffer_finish(base);
	if (buffer->texture != NULL) {
		id<MTLTexture> texture = (__bridge id<MTLTexture>)buffer->texture;
		[texture release];
	}
	free(buffer->mapped_data);
	if (buffer->has_opaque_region) {
		pixman_region32_fini(&buffer->opaque);
	}
	free(buffer);
}

static bool buffer_begin_data_ptr_access(struct wlf_buffer *base,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	struct wlf_mtl_buffer *buffer = wlf_mtl_buffer_from_buffer(base);
	const struct wlf_pixel_format_info *info =
		wlf_get_pixel_format_info(buffer->format);
	if (info == NULL || flags == 0 ||
		(flags & ~(WLF_BUFFER_DATA_PTR_ACCESS_READ |
			WLF_BUFFER_DATA_PTR_ACCESS_WRITE)) != 0) {
		return false;
	}

	int32_t min_stride = pixel_format_info_min_stride(info, (int32_t)base->width);
	if (min_stride <= 0 || base->height > SIZE_MAX / (size_t)min_stride) {
		return false;
	}

	buffer->mapped_stride = (size_t)min_stride;
	buffer->mapped_data = malloc(buffer->mapped_stride * base->height);
	if (buffer->mapped_data == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate Metal buffer staging data");
		return false;
	}
	buffer->mapped_flags = flags;

	if (flags & WLF_BUFFER_DATA_PTR_ACCESS_READ) {
		id<MTLTexture> texture = (__bridge id<MTLTexture>)buffer->texture;
		MTLRegion region = MTLRegionMake2D(0, 0, base->width, base->height);
		[texture getBytes:buffer->mapped_data
			bytesPerRow:buffer->mapped_stride
			fromRegion:region mipmapLevel:0];
	}

	*data = buffer->mapped_data;
	*format = buffer->format;
	*stride = buffer->mapped_stride;
	return true;
}

static void buffer_end_data_ptr_access(struct wlf_buffer *base) {
	struct wlf_mtl_buffer *buffer = wlf_mtl_buffer_from_buffer(base);
	if (buffer->mapped_flags & WLF_BUFFER_DATA_PTR_ACCESS_WRITE) {
		id<MTLTexture> texture = (__bridge id<MTLTexture>)buffer->texture;
		MTLRegion region = MTLRegionMake2D(0, 0, base->width, base->height);
		[texture replaceRegion:region mipmapLevel:0
			withBytes:buffer->mapped_data bytesPerRow:buffer->mapped_stride];
	}
	free(buffer->mapped_data);
	buffer->mapped_data = NULL;
	buffer->mapped_stride = 0;
	buffer->mapped_flags = 0;
}

static const pixman_region32_t *buffer_opaque_region(
		struct wlf_buffer *base) {
	struct wlf_mtl_buffer *buffer = wlf_mtl_buffer_from_buffer(base);
	return buffer->has_opaque_region ? &buffer->opaque : NULL;
}

static const struct wlf_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.begin_data_ptr_access = buffer_begin_data_ptr_access,
	.end_data_ptr_access = buffer_end_data_ptr_access,
	.opaque_region = buffer_opaque_region,
};

struct wlf_mtl_buffer *wlf_mtl_buffer_create(struct wlf_mtl_device *device,
		uint32_t width, uint32_t height, const struct wlf_render_format *format) {
	if (device == NULL || device->device == NULL || format == NULL ||
			width == 0 || height == 0 || width > INT_MAX || height > INT_MAX) {
		return NULL;
	}

	MTLPixelFormat pixel_format = metal_pixel_format(format->format);
	if (pixel_format == MTLPixelFormatInvalid) {
		wlf_log(WLF_ERROR, "unsupported Metal buffer format 0x%08x",
			format->format);
		return NULL;
	}

	@autoreleasepool {
		id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device->device;
		MTLTextureDescriptor *descriptor =
			[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
				width:width height:height mipmapped:NO];
		descriptor.usage = MTLTextureUsageRenderTarget |
			MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
		descriptor.storageMode = MTLStorageModeShared;
		id<MTLTexture> texture = [mtl_device newTextureWithDescriptor:descriptor];
		if (texture == nil) {
			wlf_log(WLF_ERROR, "failed to create Metal buffer texture");
			return NULL;
		}

		struct wlf_mtl_buffer *buffer = calloc(1, sizeof(*buffer));
		if (buffer == NULL) {
			[texture release];
			wlf_log_errno(WLF_ERROR, "failed to allocate wlf_mtl_buffer");
			return NULL;
		}

		wlf_buffer_init(&buffer->base, &buffer_impl, width, height);
		buffer->device = device;
		buffer->texture = (__bridge void *)texture;
		buffer->format = format->format;
		if (!pixel_format_has_alpha(format->format)) {
			pixman_region32_init_rect(&buffer->opaque, 0, 0, width, height);
			buffer->has_opaque_region = true;
		}
		return buffer;
	}
}

bool wlf_buffer_is_mtl(const struct wlf_buffer *buffer) {
	return buffer != NULL && buffer->impl == &buffer_impl;
}

struct wlf_mtl_buffer *wlf_mtl_buffer_from_buffer(
		struct wlf_buffer *buffer) {
	assert(wlf_buffer_is_mtl(buffer));
	struct wlf_mtl_buffer *metal = NULL;
	return wlf_container_of(buffer, metal, base);
}

void *wlf_mtl_buffer_get_texture(const struct wlf_mtl_buffer *buffer) {
	return buffer != NULL ? buffer->texture : NULL;
}

uint32_t wlf_mtl_buffer_get_format(const struct wlf_mtl_buffer *buffer) {
	return buffer != NULL ? buffer->format : WLF_FORMAT_INVALID;
}
