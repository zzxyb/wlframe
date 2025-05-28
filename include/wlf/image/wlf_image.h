#ifndef IMAGE_WLF_DIMAGE_H
#define IMAGE_WLF_DIMAGE_H

#include "wlf/color/wlf_color.h"

#include <stdint.h>
#include <stdbool.h>

struct wlf_image;

enum wlf_image_type {
	WLF_IMAGE_TYPE_UNKNOWN,
	WLF_IMAGE_TYPE_PNG,
	WLF_IMAGE_TYPE_JPEG,
	WLF_IMAGE_TYPE_SVG,
};

struct wlf_image_type_map {
	enum wlf_image_type type;
	const char *name;
};

struct wlf_image_load_options {

};

struct wlf_image_impl {
	bool (*save)(struct wlf_image *image, const char *filename);
	bool (*load)(struct wlf_image *image, const char *filename, bool enable_16_bit);
	void (*destroy)(struct wlf_image *image);
};

static const struct wlf_image_type_map image_type[] = {
	{ WLF_IMAGE_TYPE_UNKNOWN, "unknown" },
	{ WLF_IMAGE_TYPE_PNG,  "png"  },
	{ WLF_IMAGE_TYPE_JPEG, "jpeg" },
	{ WLF_IMAGE_TYPE_SVG,  "svg"  },
};

enum wlf_image_bit_depth {
	WLF_IMAGE_BIT_DEPTH_1 = 1,
	WLF_IMAGE_BIT_DEPTH_2 = 2,
	WLF_IMAGE_BIT_DEPTH_4 = 4,
	WLF_IMAGE_BIT_DEPTH_8 = 8,
	WLF_IMAGE_BIT_DEPTH_16 = 16,
};

enum wlf_image_format {
	WLF_COLOR_TYPE_UNKNOWN,
	WLF_COLOR_TYPE_RGB,
	WLF_COLOR_TYPE_RGBA,
	WLF_COLOR_TYPE_GRAY,
	WLF_COLOR_TYPE_GRAY_ALPHA, // Grayscale with alpha channel
};

struct wlf_image {
	const struct wlf_image_impl *impl;

	uint32_t width;
	uint32_t height;
	unsigned char *data;
	uint32_t format;
	uint32_t stride;
	enum wlf_image_bit_depth bit_depth;

	bool has_alpha_channel;
	bool is_opaque;
};

void wlf_image_init(struct wlf_image *image,
	const struct wlf_image_impl *impl, uint32_t width, uint32_t height, uint32_t format);
void wlf_image_finish(struct wlf_image *image);

enum wlf_image_type wlf_image_type_from_string(const char *str);
const char *wlf_image_type_to_string(enum wlf_image_type type);

bool wlf_image_save(struct wlf_image *image, const char *filename);
struct wlf_image *wlf_image_load(const char *filename);

int wlf_image_get_channels(const struct wlf_image *image);

#endif // IMAGE_WLF_DIMAGE_H
