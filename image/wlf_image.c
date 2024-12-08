#include "wlf_image.h"
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>

wlf_image_t *wlf_image_create(int width, int height)
{
    wlf_image_t *image = calloc(1, sizeof(wlf_image_t));
    if (!image)
        return NULL;

    image->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(image->surface) != CAIRO_STATUS_SUCCESS) {
        free(image);
        return NULL;
    }

    image->width = width;
    image->height = height;
    return image;
}

void wlf_image_destroy(wlf_image_t *image)
{
    if (!image)
        return;

    if (image->destroy)
        image->destroy(image);

    if (image->surface)
        cairo_surface_destroy(image->surface);

    free(image);
}

bool wlf_image_resize(wlf_image_t *image, int width, int height)
{
    if (!image || width <= 0 || height <= 0)
        return false;

    cairo_surface_t *new_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status(new_surface) != CAIRO_STATUS_SUCCESS)
        return false;

    cairo_t *cr = cairo_create(new_surface);
    cairo_set_source_surface(cr, image->surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    cairo_surface_destroy(image->surface);
    image->surface = new_surface;
    image->width = width;
    image->height = height;

    return true;
}

cairo_surface_t *wlf_image_get_surface(wlf_image_t *image)
{
    return image ? image->surface : NULL;
}

wlf_image_t *wlf_image_load_from_file(const char *filename)
{
    if (!filename)
        return NULL;

    const char *ext = strrchr(filename, '.');
    if (!ext)
        return NULL;

    ext++; // Skip the dot

    if (strcasecmp(ext, "svg") == 0)
        return wlf_image_load_svg(filename);
    else if (strcasecmp(ext, "png") == 0)
        return wlf_image_load_png(filename);
    else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
        return wlf_image_load_jpeg(filename);

    return NULL;
}

bool wlf_image_save_to_file(wlf_image_t *image, const char *filename)
{
    if (!image || !filename)
        return false;

    const char *ext = strrchr(filename, '.');
    if (!ext)
        return false;

    ext++; // Skip the dot

    if (strcasecmp(ext, "svg") == 0)
        return wlf_image_save_svg(image, filename);
    else if (strcasecmp(ext, "png") == 0)
        return wlf_image_save_png(image, filename);
    else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0)
        return wlf_image_save_jpeg(image, filename, 90); // Default quality

    return false;
}

GLenum wlf_format_to_gl_format(enum wlf_image_format format)
{
    switch (format) {
    case WLF_FORMAT_RGBA8888:
    case WLF_FORMAT_RGBX8888:
        return GL_RGBA;
    case WLF_FORMAT_BGRA8888:
    case WLF_FORMAT_BGRX8888:
        return GL_BGRA;
    case WLF_FORMAT_RGB565:
        return GL_RGB;
    case WLF_FORMAT_BGR565:
        return GL_BGR;
    case WLF_FORMAT_R8:
        return GL_RED;
    case WLF_FORMAT_A8:
        return GL_ALPHA;
    default:
        return GL_INVALID_ENUM;
    }
}

enum wlf_image_format wlf_gl_format_to_wlf_format(GLenum format, GLenum type)
{
    if (type == GL_UNSIGNED_BYTE) {
        switch (format) {
        case GL_RGBA:
            return WLF_FORMAT_RGBA8888;
        case GL_BGRA:
            return WLF_FORMAT_BGRA8888;
        case GL_RGB:
            return WLF_FORMAT_RGBX8888;
        case GL_BGR:
            return WLF_FORMAT_BGRX8888;
        case GL_RED:
            return WLF_FORMAT_R8;
        case GL_ALPHA:
            return WLF_FORMAT_A8;
        }
    } else if (type == GL_UNSIGNED_SHORT_5_6_5) {
        switch (format) {
        case GL_RGB:
            return WLF_FORMAT_RGB565;
        case GL_BGR:
            return WLF_FORMAT_BGR565;
        }
    }
    return WLF_FORMAT_INVALID;
}

struct wlf_image *wlf_image_from_gl_pixels(int width, int height,
                                         GLenum format, GLenum type,
                                         const void *pixels)
{
    enum wlf_image_format wlf_format = wlf_gl_format_to_wlf_format(format, type);
    if (wlf_format == WLF_FORMAT_INVALID)
        return NULL;

    struct wlf_image *image = wlf_image_create(width, height);
    if (!image)
        return NULL;

    cairo_surface_t *surface = wlf_image_get_surface(image);
    if (!surface) {
        wlf_image_destroy(image);
        return NULL;
    }

    unsigned char *data = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    // Lock surface for writing
    cairo_surface_flush(surface);

    // Copy pixel data
    if (format == GL_RGBA || format == GL_BGRA) {
        // Direct copy for compatible formats
        for (int y = 0; y < height; y++) {
            memcpy(data + y * stride, 
                   (const unsigned char *)pixels + y * width * 4, 
                   width * 4);
        }
    } else {
        // Format conversion needed
        for (int y = 0; y < height; y++) {
            const unsigned char *src = (const unsigned char *)pixels + y * width * 
                                     (format == GL_RGB || format == GL_BGR ? 3 : 
                                      format == GL_RED || format == GL_ALPHA ? 1 : 4);
            unsigned char *dst = data + y * stride;
            
            for (int x = 0; x < width; x++) {
                switch (format) {
                case GL_RGB:
                    dst[x * 4 + 0] = src[x * 3 + 0];  // R
                    dst[x * 4 + 1] = src[x * 3 + 1];  // G
                    dst[x * 4 + 2] = src[x * 3 + 2];  // B
                    dst[x * 4 + 3] = 0xFF;            // A
                    break;
                case GL_BGR:
                    dst[x * 4 + 0] = src[x * 3 + 2];  // R
                    dst[x * 4 + 1] = src[x * 3 + 1];  // G
                    dst[x * 4 + 2] = src[x * 3 + 0];  // B
                    dst[x * 4 + 3] = 0xFF;            // A
                    break;
                case GL_RED:
                    dst[x * 4 + 0] = src[x];          // R
                    dst[x * 4 + 1] = 0;               // G
                    dst[x * 4 + 2] = 0;               // B
                    dst[x * 4 + 3] = 0xFF;            // A
                    break;
                case GL_ALPHA:
                    dst[x * 4 + 0] = 0;               // R
                    dst[x * 4 + 1] = 0;               // G
                    dst[x * 4 + 2] = 0;               // B
                    dst[x * 4 + 3] = src[x];          // A
                    break;
                }
            }
        }
    }

    // Mark surface as modified
    cairo_surface_mark_dirty(surface);
    return image;
}