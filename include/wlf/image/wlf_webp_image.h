#ifndef IMAGE_WLF_WEBP_IMAGE_H
#define IMAGE_WLF_WEBP_IMAGE_H

#include "wlf/image/wlf_image.h"

struct wlf_webp_image {
	struct wlf_image base;
	struct wlf_webp_animation_info *ani_info;
};

struct wlf_webp_frame;

struct wlf_webp_animation_info {
	int frame_count;
	int canvas_w;
	int canvas_h;
	int loop_count;
	uint32_t bgcolor;
	uint8_t *_pixbuf;
	struct wlf_webp_frame *frames;
};

struct wlf_webp_frame {
	uint8_t *pixels;
	int timestamp; /* milliseconds, start time of frame */
	int width;
	int height;
};

struct wlf_webp_image *wlf_webp_image_create(void);
bool wlf_image_is_webp(const struct wlf_image *image);
struct wlf_webp_image *wlf_webp_image_from_image(struct wlf_image *wlf_image);
bool file_is_webp(const char *file_name);

#endif // IMAGE_WLF_WEBP_IMAGE_H
