#ifndef IMAGE_WLF_PNG_DIMAGE_H
#define IMAGE_WLF_PNG_DIMAGE_H

#include "wlf/image/wlf_image.h"

enum wlf_image_interlace_type {
	WLF_IMAGE_INTERLACE_NONE = 0,
	WLF_IMAGE_INTERLACE_ADAM7,
};

struct wlf_png_image {
	struct wlf_image base;
	enum wlf_image_interlace_type interlace_type;
};

struct wlf_png_image *wlf_png_image_create();
struct wlf_png_image *wlf_png_image_from_image(struct wlf_image *wlf_image);
void wlf_png_image_print_data(const struct wlf_image *png_image);
void wlf_png_image_print_data_gimp_style(const struct wlf_image *image);
int wlf_color_type_to_png(struct wlf_image *image);

#endif
