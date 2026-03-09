#include "wlf/texture/pixman/texture.h"
#include "wlf/buffer/pixman/render_buffer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/math/wlf_rect.h"

#include <inttypes.h>

static void texture_destroy(struct wlf_texture *wlf_texture) {
	struct wlf_pixman_texture *texture = wlf_pixman_texture_from_texture(wlf_texture);
	wlf_linked_list_remove(&texture->link);
	pixman_image_unref(texture->image);
	wlf_buffer_unlock(texture->buffer);
	free(texture->data);
	free(texture);
}

static bool texture_read_pixels(struct wlf_texture *wlf_texture,
		const struct wlf_texture_read_pixels_options *options) {
	struct wlf_pixman_texture *texture = wlf_pixman_texture_from_texture(wlf_texture);

	struct wlf_rect src;
	wlf_texture_read_pixels_options_get_src_box(options, wlf_texture, &src);

	pixman_format_code_t fmt = get_pixman_format_from_drm(options->format);
	if (fmt == 0) {
		wlf_log(WLF_ERROR, "Cannot read pixels: unsupported pixel format");
		return false;
	}

	void *p = wlf_texture_read_pixel_options_get_data(options);

	pixman_image_t *dst = pixman_image_create_bits_no_clear(fmt,
			src.width, src.height, p, options->stride);

	pixman_image_composite32(PIXMAN_OP_SRC, texture->image, NULL, dst,
			src.x, src.y, 0, 0, 0, 0, src.width, src.height);

	pixman_image_unref(dst);

	return true;
}

static uint32_t pixman_texture_preferred_read_format(struct wlf_texture *wlf_texture) {
	struct wlf_pixman_texture *texture = wlf_pixman_texture_from_texture(wlf_texture);

	pixman_format_code_t pixman_format = pixman_image_get_format(texture->image);

	return get_drm_format_from_pixman(pixman_format);
}

static const struct wlf_texture_impl texture_impl = {
	.read_pixels = texture_read_pixels,
	.preferred_read_format = pixman_texture_preferred_read_format,
	.destroy = texture_destroy,
};

struct wlf_pixman_texture *wlf_pixman_texture_create(
		struct wlf_pixman_renderer *renderer, uint32_t drm_format, uint32_t width,
		uint32_t height) {
	struct wlf_pixman_texture *texture = calloc(1, sizeof(*texture));
	if (texture == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate pixman texture");
		return NULL;
	}

	wlf_texture_init(&texture->wlf_texture, &renderer->base,
		&texture_impl, width, height);

	texture->format_info = wlf_get_pixel_format_info(drm_format);
	if (!texture->format_info) {
		wlf_log(WLF_ERROR, "Unsupported drm format 0x%"PRIX32, drm_format);
		free(texture);
		return NULL;
	}

	texture->format = get_pixman_format_from_drm(drm_format);
	if (texture->format == 0) {
		wlf_log(WLF_ERROR, "Unsupported pixman drm format 0x%"PRIX32,
				drm_format);
		free(texture);
		return NULL;
	}

	wlf_linked_list_insert(&renderer->textures, &texture->link);

	return texture;
}

bool wlf_texture_is_pixman(const struct wlf_texture *texture) {
	return texture->impl == &texture_impl;
}

struct wlf_pixman_texture *wlf_pixman_texture_from_texture(
		struct wlf_texture *texture) {
	if (!wlf_texture_is_pixman(texture)) {
		return NULL;
	}

	struct wlf_pixman_texture *pixman_texture =
		wlf_container_of(texture, pixman_texture, wlf_texture);

	return pixman_texture;
}
