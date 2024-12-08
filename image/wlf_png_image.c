#include "wlf_image_png.h"

wlf_image_t *wlf_image_load_png(const char *filename)
{
    cairo_surface_t *surface = cairo_image_surface_create_from_png(filename);
    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS)
        return NULL;

    wlf_image_t *image = calloc(1, sizeof(wlf_image_t));
    if (!image) {
        cairo_surface_destroy(surface);
        return NULL;
    }

    image->surface = surface;
    image->width = cairo_image_surface_get_width(surface);
    image->height = cairo_image_surface_get_height(surface);

    return image;
}

bool wlf_image_save_png(wlf_image_t *image, const char *filename)
{
    if (!image || !filename)
        return false;

    cairo_status_t status = cairo_surface_write_to_png(image->surface, filename);
    return status == CAIRO_STATUS_SUCCESS;
}