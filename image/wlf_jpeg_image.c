#include "wlf_image_jpeg.h"
#include <jpeglib.h>
#include <stdio.h>

wlf_image_t *wlf_image_load_jpeg(const char *filename)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *file;

    file = fopen(filename, "rb");
    if (!file)
        return NULL;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    wlf_image_t *image = wlf_image_create(cinfo.output_width, cinfo.output_height);
    if (!image) {
        jpeg_destroy_decompress(&cinfo);
        fclose(file);
        return NULL;
    }

    unsigned char *data = cairo_image_surface_get_data(image->surface);
    int stride = cairo_image_surface_get_stride(image->surface);

    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * 3, 1);

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        unsigned char *row = data + cinfo.output_scanline * stride;
        for (unsigned int x = 0; x < cinfo.output_width; x++) {
            row[x * 4 + 0] = buffer[0][x * 3 + 2]; // Blue
            row[x * 4 + 1] = buffer[0][x * 3 + 1]; // Green
            row[x * 4 + 2] = buffer[0][x * 3 + 0]; // Red
            row[x * 4 + 3] = 0xFF;                 // Alpha
        }
    }

    cairo_surface_mark_dirty(image->surface);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file);

    return image;
}

bool wlf_image_save_jpeg(wlf_image_t *image, const char *filename, int quality)
{
    if (!image || !filename)
        return false;

    FILE *file = fopen(filename, "wb");
    if (!file)
        return false;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, file);

    cinfo.image_width = image->width;
    cinfo.image_height = image->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    unsigned char *data = cairo_image_surface_get_data(image->surface);
    int stride = cairo_image_surface_get_stride(image->surface);
    JSAMPROW row_pointer[1];
    unsigned char *row = malloc(image->width * 3);

    while (cinfo.next_scanline < cinfo.image_height) {
        unsigned char *src = data + cinfo.next_scanline * stride;
        for (int x = 0; x < image->width; x++) {
            row[x * 3 + 0] = src[x * 4 + 2]; // Red
            row[x * 3 + 1] = src[x * 4 + 1]; // Green
            row[x * 3 + 2] = src[x * 4 + 0]; // Blue
        }
        row_pointer[0] = row;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    free(row);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(file);

    return true;
}