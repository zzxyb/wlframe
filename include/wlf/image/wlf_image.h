#ifndef WLF_IMAGE_H
#define WLF_IMAGE_H

#include <stdbool.h>
#include <stdint.h>

#include <cairo/cairo.h>
// #include <GL/gl.h>

// /* OpenGL format conversion */
// GLenum wlf_format_to_gl_format(enum wlf_image_format format);

// enum wlf_image_format wlf_gl_format_to_wlf_format(GLenum format, GLenum type);

// /* Create image from OpenGL pixels */
// struct wlf_image *wlf_image_from_gl_pixels(int width, int height,
//                                          GLenum format, GLenum type,
//                                          const void *pixels);

enum wlf_image_format {
    /* 32-bit formats */
    WLF_FORMAT_ARGB8888 = 0,    /* [31:0] A:R:G:B 8:8:8:8 little endian */
    WLF_FORMAT_XRGB8888 = 1,    /* [31:0] x:R:G:B 8:8:8:8 little endian */
    WLF_FORMAT_ABGR8888 = 2,    /* [31:0] A:B:G:R 8:8:8:8 little endian */
    WLF_FORMAT_XBGR8888 = 3,    /* [31:0] x:B:G:R 8:8:8:8 little endian */
    WLF_FORMAT_RGBA8888 = 4,    /* [31:0] R:G:B:A 8:8:8:8 little endian */
    WLF_FORMAT_RGBX8888 = 5,    /* [31:0] R:G:B:x 8:8:8:8 little endian */
    WLF_FORMAT_BGRA8888 = 6,    /* [31:0] B:G:R:A 8:8:8:8 little endian */
    WLF_FORMAT_BGRX8888 = 7,    /* [31:0] B:G:R:x 8:8:8:8 little endian */

    /* Premultiplied alpha formats */
    WLF_FORMAT_PARGB8888 = 8,   /* [31:0] A:R:G:B 8:8:8:8 premultiplied alpha */
    WLF_FORMAT_PABGR8888 = 9,   /* [31:0] A:B:G:R 8:8:8:8 premultiplied alpha */
    WLF_FORMAT_PRGBA8888 = 10,  /* [31:0] R:G:B:A 8:8:8:8 premultiplied alpha */
    WLF_FORMAT_PBGRA8888 = 11,  /* [31:0] B:G:R:A 8:8:8:8 premultiplied alpha */

    /* 16-bit formats */
    WLF_FORMAT_RGB565   = 12,   /* [15:0] R:G:B 5:6:5 little endian */
    WLF_FORMAT_BGR565   = 13,   /* [15:0] B:G:R 5:6:5 little endian */
    WLF_FORMAT_RGBA5551 = 14,   /* [15:0] R:G:B:A 5:5:5:1 little endian */
    WLF_FORMAT_BGRA5551 = 15,   /* [15:0] B:G:R:A 5:5:5:1 little endian */
    WLF_FORMAT_RGBA4444 = 16,   /* [15:0] R:G:B:A 4:4:4:4 little endian */
    WLF_FORMAT_BGRA4444 = 17,   /* [15:0] B:G:R:A 4:4:4:4 little endian */
    
    /* 8-bit formats */
    WLF_FORMAT_R8       = 18,   /* [7:0] R */
    WLF_FORMAT_A8       = 19,   /* [7:0] A */
    WLF_FORMAT_G8       = 20,   /* [7:0] G - grayscale */

    /* YUV formats */
    WLF_FORMAT_YUV420   = 21,   /* 3 plane YCbCr format, 2x2 subsampled Cb (1) and Cr (2) planes */
    WLF_FORMAT_NV12     = 22,   /* 2 plane YCbCr Cr:Cb format, 2x2 subsampled Cr:Cb plane */

    WLF_FORMAT_INVALID  = -1
};

enum wlf_image_source_formats {
	UNKNOWN = 0,
	BMP = 1,
	PNG = 2,
	JPEG = 3,
	XBM = 4,
	XPM = 5,
	SVG = 6,
};

struct wlf_image_impl {
	int (*save)(struct wlf_image *image, const char *filename);
	int (*load)(struct wlf_image *image, const char *filename);
};

struct wlf_image {
	const struct wlf_image_impl *impl;

	void *user_data;
	cairo_surface_t *surface;
	int width;
	int height;
	enum wlf_image_format format;
	enum wlf_image_source_formats source_format;
};

wlf_image *wlf_image_create(int width, int height, wlf_image_format format);
void wlf_image_destroy(wlf_image *image);
bool wlf_image_resize(wlf_image *image, int width, int height);

#endif