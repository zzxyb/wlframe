#include "wlf/swapchain/windows/swapchain.h"

#include "wlf/renderer/pixman/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct wlf_windows_buffer {
	struct wlf_buffer base;
	uint32_t *pixels;
	size_t stride;
};

struct wlf_windows_swapchain {
	struct wlf_swapchain base;
	struct wlf_windows_buffer *buffer;
};

static struct wlf_windows_buffer *windows_buffer_from_buffer(
		struct wlf_buffer *base) {
	struct wlf_windows_buffer *buffer = NULL;
	return wlf_container_of(base, buffer, base);
}

static void windows_buffer_destroy(struct wlf_buffer *base) {
	struct wlf_windows_buffer *buffer = windows_buffer_from_buffer(base);
	wlf_buffer_finish(base);
	free(buffer->pixels);
	free(buffer);
}

static bool windows_buffer_begin_access(struct wlf_buffer *base,
		uint32_t flags, void **data, uint32_t *format, size_t *stride) {
	WLF_UNUSED(flags);
	struct wlf_windows_buffer *buffer = windows_buffer_from_buffer(base);
	*data = buffer->pixels;
	*format = WLF_FORMAT_XRGB8888;
	*stride = buffer->stride;
	return true;
}

static void windows_buffer_end_access(struct wlf_buffer *base) {
	WLF_UNUSED(base);
}

static const struct wlf_buffer_impl windows_buffer_impl = {
	.destroy = windows_buffer_destroy,
	.begin_data_ptr_access = windows_buffer_begin_access,
	.end_data_ptr_access = windows_buffer_end_access,
};

static struct wlf_windows_buffer *windows_buffer_create(int width,
		int height) {
	if (width <= 0 || height <= 0 || width > INT32_MAX / 4) {
		return NULL;
	}
	size_t stride = (size_t)width * 4;
	if ((size_t)height > SIZE_MAX / stride) {
		return NULL;
	}
	struct wlf_windows_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		return NULL;
	}
	buffer->pixels = calloc((size_t)height, stride);
	if (buffer->pixels == NULL) {
		free(buffer);
		return NULL;
	}
	buffer->stride = stride;
	wlf_buffer_init(&buffer->base, &windows_buffer_impl,
		(uint32_t)width, (uint32_t)height);
	return buffer;
}

static struct wlf_windows_swapchain *windows_swapchain_from_base(
		struct wlf_swapchain *base);

static void windows_swapchain_destroy(struct wlf_swapchain *base) {
	struct wlf_windows_swapchain *swapchain =
		windows_swapchain_from_base(base);
	wlf_buffer_drop(&swapchain->buffer->base);
	free(swapchain);
}

static bool windows_swapchain_resize(struct wlf_swapchain *base,
		int width, int height) {
	struct wlf_windows_swapchain *swapchain =
		windows_swapchain_from_base(base);
	struct wlf_windows_buffer *buffer = windows_buffer_create(width, height);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to resize Win32 software buffer");
		return false;
	}
	wlf_buffer_drop(&swapchain->buffer->base);
	swapchain->buffer = buffer;
	base->back = &buffer->base;
	base->width = width;
	base->height = height;
	return true;
}

static HRGN damage_region(const pixman_region32_t *damage) {
	if (damage == NULL) {
		return NULL;
	}
	HRGN region = CreateRectRgn(0, 0, 0, 0);
	int count = 0;
	pixman_box32_t *rectangles = pixman_region32_rectangles(
		(pixman_region32_t *)damage, &count);
	for (int i = 0; i < count; ++i) {
		HRGN rectangle = CreateRectRgn(rectangles[i].x1, rectangles[i].y1,
			rectangles[i].x2, rectangles[i].y2);
		if (rectangle != NULL) {
			CombineRgn(region, region, rectangle, RGN_OR);
			DeleteObject(rectangle);
		}
	}
	return region;
}

static void windows_swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	struct wlf_windows_swapchain *swapchain =
		windows_swapchain_from_base(base);
	HWND hwnd = wlf_window_native_handle(base->window);
	if (hwnd == NULL) {
		return;
	}
	HDC dc = GetDC(hwnd);
	if (dc == NULL) {
		return;
	}
	HRGN clip = damage_region(damage);
	if (clip != NULL) {
		SelectClipRgn(dc, clip);
	}
	RECT client;
	GetClientRect(hwnd, &client);
	BITMAPINFO bitmap = {
		.bmiHeader = {
			.biSize = sizeof(BITMAPINFOHEADER),
			.biWidth = base->width,
			.biHeight = -base->height,
			.biPlanes = 1,
			.biBitCount = 32,
			.biCompression = BI_RGB,
		},
	};
	StretchDIBits(dc, 0, 0, client.right, client.bottom,
		0, 0, base->width, base->height, swapchain->buffer->pixels,
		&bitmap, DIB_RGB_COLORS, SRCCOPY);
	SelectClipRgn(dc, NULL);
	if (clip != NULL) {
		DeleteObject(clip);
	}
	ReleaseDC(hwnd, dc);
}

static const struct wlf_swapchain_impl windows_swapchain_impl = {
	.destroy = windows_swapchain_destroy,
	.resize = windows_swapchain_resize,
	.present = windows_swapchain_present,
};

struct wlf_swapchain *wlf_windows_swapchain_create(
		struct wlf_window *window, int width, int height,
		const struct wlf_render_format *format) {
	if (window == NULL || !wlf_renderer_is_pixman(window->state.renderer) ||
			(format->format != WLF_FORMAT_XRGB8888 &&
			format->format != WLF_FORMAT_ARGB8888)) {
		return NULL;
	}
	struct wlf_windows_swapchain *swapchain = calloc(1,
		sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->buffer = windows_buffer_create(width, height);
	if (swapchain->buffer == NULL) {
		free(swapchain);
		return NULL;
	}
	wlf_swapchain_init(&swapchain->base, NULL, &windows_swapchain_impl,
		width, height);
	swapchain->base.window = window;
	swapchain->base.back = &swapchain->buffer->base;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		wlf_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	return &swapchain->base;
}

bool wlf_swapchain_is_windows(const struct wlf_swapchain *swapchain) {
	return swapchain != NULL && swapchain->impl == &windows_swapchain_impl;
}

static struct wlf_windows_swapchain *windows_swapchain_from_base(
		struct wlf_swapchain *base) {
	assert(wlf_swapchain_is_windows(base));
	struct wlf_windows_swapchain *swapchain = NULL;
	return wlf_container_of(base, swapchain, base);
}
