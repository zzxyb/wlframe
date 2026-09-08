#include "wlf/texture/metal/texture.h"

#include "wlf/renderer/metal/device.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <Metal/Metal.h>

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

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

static bool format_is_opaque_32(uint32_t format) {
	return format == WLF_FORMAT_XRGB8888 || format == WLF_FORMAT_XBGR8888;
}

static void texture_destroy(struct wlf_texture *base) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(base);
	wlf_linked_list_remove(&texture->link);
	if (texture->texture != NULL) {
		id<MTLTexture> handle = (__bridge id<MTLTexture>)texture->texture;
		[handle release];
	}
	free(texture);
}

static bool upload_region(struct wlf_mtl_texture *texture, const void *data,
		size_t stride, uint32_t x, uint32_t y, uint32_t width,
		uint32_t height) {
	if (width == 0 || height == 0) {
		return true;
	}

	id<MTLTexture> handle = (__bridge id<MTLTexture>)texture->texture;
	MTLRegion region = MTLRegionMake2D(x, y, width, height);
	const struct wlf_pixel_format_info *info =
		wlf_get_pixel_format_info(texture->format);
	if (info == NULL) {
		return false;
	}
	size_t row_bytes = (size_t)width * info->bytes_per_block;
	const uint8_t *source = data;

	if (!format_is_opaque_32(texture->format)) {
		[handle replaceRegion:region mipmapLevel:0 withBytes:source
			bytesPerRow:stride];
		return true;
	}

	if (row_bytes > SIZE_MAX / height) {
		return false;
	}
	uint8_t *opaque = malloc(row_bytes * height);
	if (opaque == NULL) {
		return false;
	}
	for (uint32_t row = 0; row < height; ++row) {
		memcpy(opaque + row * row_bytes, source + row * stride, row_bytes);
		for (uint32_t column = 0; column < width; ++column) {
			opaque[row * row_bytes + column * 4 + 3] = 0xff;
		}
	}
	[handle replaceRegion:region mipmapLevel:0 withBytes:opaque
		bytesPerRow:row_bytes];
	free(opaque);
	return true;
}

static bool texture_update_from_buffer(struct wlf_texture *base,
		struct wlf_buffer *buffer, const pixman_region32_t *damage) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(base);
	void *data = NULL;
	uint32_t format = WLF_FORMAT_INVALID;
	size_t stride = 0;
	if (!wlf_buffer_begin_data_ptr_access(buffer,
			WLF_BUFFER_DATA_PTR_ACCESS_READ, &data, &format, &stride)) {
		return false;
	}

	bool ok = format == texture->format;
	const struct wlf_pixel_format_info *info = wlf_get_pixel_format_info(format);
	if (ok && info == NULL) {
		ok = false;
	}

	if (ok && damage == NULL) {
		ok = upload_region(texture, data, stride, 0, 0,
			buffer->width, buffer->height);
	} else if (ok) {
		int count = 0;
		pixman_box32_t *rects = pixman_region32_rectangles(
			(pixman_region32_t *)damage, &count);
		for (int i = 0; i < count && ok; ++i) {
			uint32_t x = (uint32_t)rects[i].x1;
			uint32_t y = (uint32_t)rects[i].y1;
			uint32_t width = (uint32_t)(rects[i].x2 - rects[i].x1);
			uint32_t height = (uint32_t)(rects[i].y2 - rects[i].y1);
			const uint8_t *source = data;
			source += (size_t)y * stride +
				(size_t)x * info->bytes_per_block;
			ok = upload_region(texture, source, stride, x, y, width, height);
		}
	}

	wlf_buffer_end_data_ptr_access(buffer);
	return ok;
}

static bool texture_read_pixels(struct wlf_texture *base,
		const struct wlf_texture_read_pixels_options *options) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(base);
	if (options == NULL || options->data == NULL ||
			options->format != texture->format) {
		return false;
	}

	struct wlf_rect box;
	wlf_texture_read_pixels_options_get_src_box(options, base, &box);
	if (box.x < 0 || box.y < 0 || box.width <= 0 || box.height <= 0 ||
			(uint32_t)(box.x + box.width) > base->width ||
			(uint32_t)(box.y + box.height) > base->height) {
		return false;
	}
	const struct wlf_pixel_format_info *info =
		wlf_get_pixel_format_info(options->format);
	if (info == NULL || !pixel_format_info_check_stride(info,
			(int32_t)options->stride, options->dst_x + (uint32_t)box.width)) {
		return false;
	}

	id<MTLTexture> handle = (__bridge id<MTLTexture>)texture->texture;
	MTLRegion region = MTLRegionMake2D(box.x, box.y, box.width, box.height);
	[handle getBytes:wlf_texture_read_pixel_options_get_data(options)
		bytesPerRow:options->stride fromRegion:region mipmapLevel:0];
	return true;
}

static uint32_t texture_preferred_read_format(struct wlf_texture *base) {
	return wlf_mtl_texture_from_texture(base)->format;
}

static const struct wlf_texture_impl texture_impl = {
	.update_from_buffer = texture_update_from_buffer,
	.read_pixels = texture_read_pixels,
	.preferred_read_format = texture_preferred_read_format,
	.destroy = texture_destroy,
};

struct wlf_mtl_texture *wlf_mtl_texture_from_buffer(
		struct wlf_mtl_renderer *renderer, struct wlf_buffer *buffer) {
	if (renderer == NULL || buffer == NULL) {
		return NULL;
	}
	void *data = NULL;
	uint32_t format = WLF_FORMAT_INVALID;
	size_t stride = 0;
	if (!wlf_buffer_begin_data_ptr_access(buffer,
			WLF_BUFFER_DATA_PTR_ACCESS_READ, &data, &format, &stride)) {
		return NULL;
	}

	MTLPixelFormat pixel_format = metal_pixel_format(format);
	const struct wlf_pixel_format_info *info = wlf_get_pixel_format_info(format);
	int32_t min_stride = info != NULL ?
		pixel_format_info_min_stride(info, (int32_t)buffer->width) : 0;
	if (pixel_format == MTLPixelFormatInvalid || min_stride <= 0 ||
			stride < (size_t)min_stride) {
		wlf_buffer_end_data_ptr_access(buffer);
		wlf_log(WLF_ERROR, "unsupported Metal texture format or row stride");
		return NULL;
	}

	@autoreleasepool {
		id<MTLDevice> device =
			(__bridge id<MTLDevice>)renderer->dev->device;
		MTLTextureDescriptor *descriptor =
			[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixel_format
				width:buffer->width height:buffer->height mipmapped:NO];
		descriptor.usage = MTLTextureUsageShaderRead;
		descriptor.storageMode = MTLStorageModeShared;
		id<MTLTexture> handle = [device newTextureWithDescriptor:descriptor];
		if (handle == nil) {
			wlf_buffer_end_data_ptr_access(buffer);
			return NULL;
		}

		struct wlf_mtl_texture *texture = calloc(1, sizeof(*texture));
		if (texture == NULL) {
			[handle release];
			wlf_buffer_end_data_ptr_access(buffer);
			return NULL;
		}
		wlf_texture_init(&texture->base, &renderer->base, &texture_impl,
			buffer->width, buffer->height);
		texture->renderer = renderer;
		texture->texture = (__bridge void *)handle;
		texture->format = format;
		bool uploaded = upload_region(texture, data, stride, 0, 0,
			buffer->width, buffer->height);
		wlf_buffer_end_data_ptr_access(buffer);
		if (!uploaded) {
			[handle release];
			free(texture);
			return NULL;
		}
		wlf_linked_list_insert(&renderer->textures, &texture->link);
		return texture;
	}
}

bool wlf_texture_is_mtl(const struct wlf_texture *texture) {
	return texture != NULL && texture->impl == &texture_impl;
}

struct wlf_mtl_texture *wlf_mtl_texture_from_texture(
		struct wlf_texture *texture) {
	assert(wlf_texture_is_mtl(texture));
	struct wlf_mtl_texture *metal = NULL;
	return wlf_container_of(texture, metal, base);
}

void *wlf_mtl_texture_get_handle(const struct wlf_mtl_texture *texture) {
	return texture != NULL ? texture->texture : NULL;
}
