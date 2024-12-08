#ifndef WLF_RENDER_FRAMEBUFFER_H
#define WLF_RENDER_FRAMEBUFFER_H

#include <stdint.h>

struct wlf_framebuffer;

struct wlf_framebuffer_impl {
	void (*bind)(struct wlf_framebuffer *fb);
    void (*unbind)(struct wlf_framebuffer *fb);
    void (*resize)(struct wlf_framebuffer *fb, int width, int height);
	void (*destroy)(struct wlf_framebuffer *fb);
};

struct wlf_framebuffer {
	const struct wlf_framebuffer_impl *impl; /**< Pointer to the framebuffer implementation */
	uint32_t width; /**< Width of the framebuffer */
	uint32_t height; /**< Height of the framebuffer */
};

#endif
