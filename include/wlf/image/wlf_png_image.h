#ifndef WLF_IMAGE_PNG_H
#define WLF_IMAGE_PNG_H

#include "wlf_image.h"

wlf_image_t *wlf_image_load_png(const char *filename);
bool wlf_image_save_png(wlf_image_t *image, const char *filename);

#endif