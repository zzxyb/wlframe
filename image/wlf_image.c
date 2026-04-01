#include "wlf/image/wlf_image.h"
#include "wlf/image/wlf_png_image.h"
#include "wlf/image/wlf_jpeg_image.h"
#include "wlf/image/wlf_bmp_image.h"
#include "wlf/image/wlf_ppm_image.h"
#include "wlf/image/wlf_webp_image.h"
#include "wlf/image/wlf_xpm_image.h"

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
	if (image == NULL || filename == NULL) {
		return false;
	}

	// Try PNG first - check if this is a PNG image
	if (wlf_image_is_png(image)) {
		struct wlf_png_image *png_image = wlf_png_image_from_image(image);
		if (png_image && png_image->base.impl->save) {
			return png_image->base.impl->save(image, filename);
		}
	}

	// Try JPEG - check if this is a JPEG image
	if (wlf_image_is_jpeg(image)) {
		struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_from_image(image);
		if (jpeg_image && jpeg_image->base.impl->save) {
			return jpeg_image->base.impl->save(image, filename);
		}
	}

	// Try BMP - check if this is a BMP image
	if (wlf_image_is_bmp(image)) {
		struct wlf_bmp_image *bmp_image = wlf_bmp_image_from_image(image);
		if (bmp_image && bmp_image->base.impl->save) {
			return bmp_image->base.impl->save(image, filename);
		}
	}

	// Try PPM - check if this is a PPM image
	if (wlf_image_is_ppm(image)) {
		struct wlf_ppm_image *ppm_image = wlf_ppm_image_from_image(image);
		if (ppm_image && ppm_image->base.impl->save) {
			return ppm_image->base.impl->save(image, filename);
		}
	}

	// Try WebP - check if this is a WebP image
	if (wlf_image_is_webp(image)) {
		struct wlf_webp_image *webp_image = wlf_webp_image_from_image(image);
		if (webp_image && webp_image->base.impl->save) {
			return webp_image->base.impl->save(image, filename);
		}
	}

	// Try XPM - check if this is a XPM image
	if (wlf_image_is_xpm(image)) {
		struct wlf_xpm_image *xpm_image = wlf_xpm_image_from_image(image);
		if (xpm_image && xpm_image->base.impl->save) {
			return xpm_image->base.impl->save(image, filename);
		}
	}

	return false;
}

struct wlf_image *wlf_image_load(const char *filename) {
	if (filename == NULL) {
		return NULL;
	}

	const char *ext = strrchr(filename, '.');
	if (ext == NULL) {
		// No file extension found
		return NULL;
	}

	// Check for PNG format
	if (strcasecmp(ext, ".png") == 0) {
		struct wlf_png_image *png_image = wlf_png_image_create();
		if (png_image) {
			png_image->base.image_type = WLF_IMAGE_TYPE_PNG;
			if (png_image->base.impl->load(&png_image->base, filename, false)) {
				return &png_image->base;
			} else {
				png_image->base.impl->destroy(&png_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	} else if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) {
		struct wlf_jpeg_image *jpeg_image = wlf_jpeg_image_create();
		if (jpeg_image) {
			jpeg_image->base.image_type = WLF_IMAGE_TYPE_JPEG;
			if (jpeg_image->base.impl->load(&jpeg_image->base, filename, false)) {
				return &jpeg_image->base;
			} else {
				jpeg_image->base.impl->destroy(&jpeg_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	} else if (strcasecmp(ext, ".bmp") == 0) {
		struct wlf_bmp_image *bmp_image = wlf_bmp_image_create();
		if (bmp_image) {
			bmp_image->base.image_type = WLF_IMAGE_TYPE_BMP;
			if (bmp_image->base.impl->load(&bmp_image->base, filename, false)) {
				return &bmp_image->base;
			} else {
				bmp_image->base.impl->destroy(&bmp_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	} else if (strcasecmp(ext, ".ppm") == 0) {
		struct wlf_ppm_image *ppm_image = wlf_ppm_image_create();
		if (ppm_image) {
			ppm_image->base.image_type = WLF_IMAGE_TYPE_PPM;
			if (ppm_image->base.impl->load(&ppm_image->base, filename, false)) {
				return &ppm_image->base;
			} else {
				ppm_image->base.impl->destroy(&ppm_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	} else if (strcasecmp(ext, ".webp") == 0) {
		struct wlf_webp_image *webp_image = wlf_webp_image_create();
		if (webp_image) {
			webp_image->base.image_type = WLF_IMAGE_TYPE_WEBP;
			if (webp_image->base.impl->load(&webp_image->base, filename, false)) {
				return &webp_image->base;
			} else {
				webp_image->base.impl->destroy(&webp_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	} else if (strcasecmp(ext, ".xpm") == 0) {
		struct wlf_xpm_image *xpm_image = wlf_xpm_image_create();
		if (xpm_image) {
			xpm_image->base.image_type = WLF_IMAGE_TYPE_XPM;
			if (xpm_image->base.impl->load(&xpm_image->base, filename, false)) {
				return &xpm_image->base;
			} else {
				xpm_image->base.impl->destroy(&xpm_image->base);
				return NULL;
			}
		} else {
			return NULL;
		}
	}

	return NULL;
}
