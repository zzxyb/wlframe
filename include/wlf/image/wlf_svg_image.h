#ifndef WLF_IMAGE_SVG_H
#define WLF_IMAGE_SVG_H

#include "wlf_image.h"

wlf_image_t *wlf_image_load_svg(const char *filename);
bool wlf_image_save_svg(wlf_image_t *image, const char *filename);

#endif