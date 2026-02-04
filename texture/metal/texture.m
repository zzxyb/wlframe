#include "wlf/texture/metal/texture.h"
#include "wlf/buffer/metal/render_buffer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/math/wlf_region.h"

#import <Metal/Metal.h>

#include <stdlib.h>
#include <inttypes.h>

static void texture_destroy(struct wlf_texture *wlf_tex) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(wlf_tex);
	wlf_linked_list_remove(&texture->link);

	if (texture->texture != NULL) {
		id<MTLTexture> mtl_tex = (__bridge id<MTLTexture>)texture->texture;
		[mtl_tex release];
		texture->texture = NULL;
	}

	if (texture->buffer != NULL) {
		wlf_buffer_unlock(texture->buffer);
		texture->buffer = NULL;
	}

	free(texture);
}

static bool texture_update_from_buffer(struct wlf_texture *wlf_tex,
		struct wlf_buffer *buffer, const struct wlf_region *damage) {
	@autoreleasepool {
		struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(wlf_tex);

		void *data = NULL;
		uint32_t drm_format;
		size_t stride;
		if (!wlf_buffer_begin_data_ptr_access(buffer,
			    WLF_BUFFER_DATA_PTR_ACCESS_READ,
			    &data, &drm_format, &stride)) {
			wlf_log(WLF_ERROR, "Failed to get buffer data for texture update");
			return false;
		}

		if (drm_format != texture->drm_format) {
			wlf_log(WLF_ERROR, "Buffer format mismatch on texture update");
			wlf_buffer_end_data_ptr_access(buffer);
			return false;
		}

		uint32_t bpp = texture->format_info->bytes_per_block;
		id<MTLTexture> mtl_tex = (__bridge id<MTLTexture>)texture->texture;

		if (damage->data != NULL) {
			for (long i = 0; i < damage->data->numRects; i++) {
				const struct wlf_frect *r = &damage->data->rects[i];
				NSUInteger rx = (NSUInteger)r->x;
				NSUInteger ry = (NSUInteger)r->y;
				NSUInteger rw = (NSUInteger)r->width;
				NSUInteger rh = (NSUInteger)r->height;
				const uint8_t *src = (const uint8_t *)data + ry * stride + rx * bpp;
				[mtl_tex replaceRegion:MTLRegionMake2D(rx, ry, rw, rh)
				           mipmapLevel:0
				            withBytes:src
				          bytesPerRow:stride];
			}
		}

		wlf_buffer_end_data_ptr_access(buffer);
		return true;
	}
}

static bool texture_read_pixels(struct wlf_texture *wlf_tex,
		const struct wlf_texture_read_pixels_options *options) {
	@autoreleasepool {
		struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(wlf_tex);

		uint32_t mtl_fmt = get_mtl_format_from_drm(options->format);
		if (mtl_fmt == 0) {
			wlf_log(WLF_ERROR, "Cannot read pixels: unsupported pixel format");
			return false;
		}

		struct wlf_rect src;
		wlf_texture_read_pixels_options_get_src_box(options, wlf_tex, &src);

		void *p = wlf_texture_read_pixel_options_get_data(options);

		id<MTLTexture> mtl_tex = (__bridge id<MTLTexture>)texture->texture;
		[mtl_tex getBytes:p
		      bytesPerRow:options->stride
		       fromRegion:MTLRegionMake2D(src.x, src.y, src.width, src.height)
		      mipmapLevel:0];
		return true;
	}
}

static uint32_t texture_preferred_read_format(struct wlf_texture *wlf_tex) {
	struct wlf_mtl_texture *texture = wlf_mtl_texture_from_texture(wlf_tex);
	return texture->drm_format;
}

static const struct wlf_texture_impl texture_impl = {
	.update_from_buffer = texture_update_from_buffer,
	.read_pixels = texture_read_pixels,
	.preferred_read_format = texture_preferred_read_format,
	.destroy = texture_destroy,
};

struct wlf_mtl_texture *wlf_mtl_texture_create(
		struct wlf_mtl_renderer *renderer, uint32_t drm_format, uint32_t width,
		uint32_t height) {
	@autoreleasepool {
		struct wlf_mtl_texture *texture = calloc(1, sizeof(*texture));
		if (texture == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mtl_texture");
			return NULL;
		}

		wlf_texture_init(&texture->wlf_texture, &renderer->base,
			&texture_impl, width, height);

		texture->renderer = renderer;
		texture->drm_format = drm_format;

		texture->format_info = wlf_get_pixel_format_info(drm_format);
		if (!texture->format_info) {
			wlf_log(WLF_ERROR, "Unsupported drm format 0x%"PRIX32, drm_format);
			free(texture);
			return NULL;
		}

		uint32_t mtl_fmt = get_mtl_format_from_drm(drm_format);
		if (mtl_fmt == 0) {
			wlf_log(WLF_ERROR, "Unsupported Metal drm format 0x%"PRIX32, drm_format);
			free(texture);
			return NULL;
		}

		id<MTLDevice> device =
			(__bridge id<MTLDevice>)renderer->dev->device;

		MTLTextureDescriptor *desc =
			[MTLTextureDescriptor texture2DDescriptorWithPixelFormat:(MTLPixelFormat)mtl_fmt
			                                                  width:width
			                                                 height:height
			                                              mipmapped:NO];
		desc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
		desc.storageMode = MTLStorageModeShared;

		id<MTLTexture> mtl_tex = [device newTextureWithDescriptor:desc];
		if (mtl_tex == nil) {
			wlf_log(WLF_ERROR, "Failed to create Metal texture");
			free(texture);
			return NULL;
		}

		[mtl_tex retain];
		texture->texture = (__bridge void *)mtl_tex;

		wlf_linked_list_insert(&renderer->textures, &texture->link);

		return texture;
	}
}

bool wlf_texture_is_mtl(const struct wlf_texture *texture) {
	return texture->impl == &texture_impl;
}

struct wlf_mtl_texture *wlf_mtl_texture_from_texture(
		struct wlf_texture *texture) {
	if (!wlf_texture_is_mtl(texture)) {
		return NULL;
	}

	struct wlf_mtl_texture *mtl_texture =
		wlf_container_of(texture, mtl_texture, wlf_texture);
	return mtl_texture;
}
