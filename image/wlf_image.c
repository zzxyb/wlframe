#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_png_image.h"

#include <assert.h>
#include <string.h>
#include <strings.h>

void wlf_image_init(struct wlf_image *image,
		const struct wlf_image_impl *impl, uint32_t width, uint32_t height, uint32_t format) {
	assert(impl);
	assert(impl->destroy);

	*image = (struct wlf_image) {
		.impl = impl,
		.width = width,
		.height = height,
		.format = format,
	};
	image->has_alpha_channel = false;
	image->is_opaque = false;
}

void wlf_image_finish(struct wlf_image *image) {
	if (image && image->impl && image->impl->destroy) {
		image->impl->destroy(image);
	}
}

enum wlf_image_type wlf_image_type_from_string(const char *str) {
	for (long unsigned int i = 0; i < sizeof(image_type) / sizeof(image_type[0]); i++) {
		if (strcmp(image_type[i].name, str) == 0) {
			return image_type[i].type;
		}
	}

	return WLF_IMAGE_TYPE_UNKNOWN;
}

const char *wlf_image_get_type_string(const struct wlf_image *image) {
	for (long unsigned int i = 0; i < sizeof(image_type) / sizeof(image_type[0]); i++) {
		if (image_type[i].type == image->image_type) {
			return image_type[i].name;
		}
	}

	return "unknown";
}

int wlf_image_get_channels(const struct wlf_image *image) {
	int channels = 0;
	switch (image->format) {
		case WLF_COLOR_TYPE_RGB:
			channels = 3;
			break;
		case WLF_COLOR_TYPE_RGBA:
			channels = 4;
			break;
		case WLF_COLOR_TYPE_GRAY:
			channels = 1;
			break;
		case WLF_COLOR_TYPE_GRAY_ALPHA:
			channels = 2;
			break;
		default:
			return 0;
	}

	return channels;
}

bool wlf_image_save(struct wlf_image *image, const char *filename) {
	struct wlf_png_image *png_image = wlf_png_image_from_image(image);
	if (png_image && png_image->base.impl->save) {
		return png_image->base.impl->save(image, filename);
	}

	return false;
}

struct wlf_image *wlf_image_load(const char *filename) {
	const char *ext = strrchr(filename, '.');
	if (ext && strcasecmp(ext, ".png") == 0) {
		struct wlf_png_image *png_image = wlf_png_image_create();
		if (png_image) {
			png_image->base.image_type = WLF_IMAGE_TYPE_PNG;
			if (png_image->base.impl->load(&png_image->base, filename, false)) {
				return (struct wlf_image *)png_image;
			} else {
				png_image->base.impl->destroy((struct wlf_image *)png_image);
				return NULL;
			}
		} else {
			return NULL;
		}
	}

	return NULL;
}
