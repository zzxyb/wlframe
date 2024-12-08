#include "wlf_image_svg.h"
#include <librsvg/rsvg.h>

wlf_image_t *wlf_image_load_svg(const char *filename)
{
    RsvgHandle *handle = rsvg_handle_new_from_file(filename, NULL);
    if (!handle)
        return NULL;

    RsvgDimensionData dim;
    rsvg_handle_get_dimensions(handle, &dim);

    wlf_image_t *image = wlf_image_create(dim.width, dim.height);
    if (!image) {
        g_object_unref(handle);
        return NULL;
    }

    cairo_t *cr = cairo_create(image->surface);
    rsvg_handle_render_cairo(handle, cr);
    cairo_destroy(cr);
    g_object_unref(handle);

    return image;
}

bool wlf_image_save_svg(wlf_image_t *image, const char *filename)
{
    // SVG saving is not implemented as it requires vector data
    return false;
}