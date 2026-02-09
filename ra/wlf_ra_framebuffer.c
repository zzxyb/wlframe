#include "wlf/ra/common/wlf_ra_framebuffer.h"

#include <stdlib.h>
#include <string.h>

struct wlf_ra_framebuffer *wlf_ra_framebuffer_create(int width, int height, int depth) {
	struct wlf_ra_framebuffer *fb = calloc(1, sizeof(struct wlf_ra_framebuffer));
	if (!fb) {
		return NULL;
	}

	fb->width = width;
	fb->height = height;
	fb->depth = depth;
	fb->stride = width * (depth / 8);

	/* Allocate framebuffer data */
	size_t data_size = fb->stride * height;
	fb->data = calloc(1, data_size);
	if (!fb->data) {
		free(fb);
		return NULL;
	}

	/* Set default pixel format (RGB32) */
	fb->format.bits_per_pixel = 32;
	fb->format.depth = 24;
	fb->format.big_endian = false;
	fb->format.true_color = true;
	fb->format.red_max = 255;
	fb->format.green_max = 255;
	fb->format.blue_max = 255;
	fb->format.red_shift = 16;
	fb->format.green_shift = 8;
	fb->format.blue_shift = 0;

	/* Initialize signals */
	wlf_signal_init(&fb->events.update);
	wlf_signal_init(&fb->events.destroy);

	return fb;
}

void wlf_ra_framebuffer_destroy(struct wlf_ra_framebuffer *fb) {
	if (fb == NULL) {
		return;
	}

	/* Emit destroy signal */
	wlf_signal_emit(&fb->events.destroy, fb);

	if (fb->data) {
		free(fb->data);
	}

	free(fb);
}

char *wlf_ra_framebuffer_get_data(struct wlf_ra_framebuffer *fb) {
	return fb ? fb->data : NULL;
}

void wlf_ra_framebuffer_get_size(struct wlf_ra_framebuffer *fb, int *width, int *height) {
	if (!fb) {
		return;
	}

	if (width) {
		*width = fb->width;
	}
	if (height) {
		*height = fb->height;
	}
}

void wlf_ra_framebuffer_update(struct wlf_ra_framebuffer *fb, const char *data,
		int width, int height, int stride) {
	if (!fb || !data) {
		return;
	}

	/* Reallocate if size changed */
	if (width != fb->width || height != fb->height) {
		fb->width = width;
		fb->height = height;
		fb->stride = width * (fb->depth / 8);

		size_t new_size = fb->stride * height;
		char *new_data = realloc(fb->data, new_size);
		if (new_data) {
			fb->data = new_data;
		} else {
			return; /* Failed to reallocate */
		}
	}

	/* Copy data */
	if (stride == fb->stride) {
		memcpy(fb->data, data, fb->stride * height);
	} else {
		/* Copy line by line if strides differ */
		int copy_width = (stride < fb->stride) ? stride : fb->stride;
		for (int y = 0; y < height; y++) {
			memcpy(fb->data + y * fb->stride,
				data + y * stride,
				copy_width);
		}
	}

	/* Emit update signal */
	wlf_signal_emit(&fb->events.update, fb);
}
