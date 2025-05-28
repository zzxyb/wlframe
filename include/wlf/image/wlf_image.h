#ifndef IMAGE_WLF_DIMAGE_H
#define IMAGE_WLF_DIMAGE_H

#include <stdint.h>

struct wlf_image_impl {
	struct wlf_image *(*create)(uint32_t width, uint32_t height, uint32_t format);
	int (*save)(struct wlf_image *image, const char *filename);
	int (*load)(struct wlf_image *image, const char *filename);
	void (*destroy)(struct wlf_image *image);
};

struct wlf_image {
	const struct wlf_image_impl *impl;

	uint32_t width;
	uint32_t height;
	unsigned char *data;

	uint32_t format;
	uint32_t stride;
};

struct wlf_image *wlf_image_create(uint32_t width, uint32_t height, uint32_t format);
int wlf_image_save(struct wlf_image *image, const char *filename);
int wlf_image_load(struct wlf_image *image, const char *filename);
void wlf_image_destroy(struct wlf_image *image);

#endif // IMAGE_WLF_DIMAGE_H
