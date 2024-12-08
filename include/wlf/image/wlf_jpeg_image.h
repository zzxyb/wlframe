#ifndef WLF_IMAGE_JPEG_H
#define WLF_IMAGE_JPEG_H

#include "wlf_image.h"

wlf_image_t *wlf_image_load_jpeg(const char *filename);
bool wlf_image_save_jpeg(wlf_image_t *image, const char *filename, int quality);

#endif