#ifndef RENDER_WLF_RENDER_H
#define RENDER_WLF_RENDER_H

struct wlf_backend;
struct wlf_buffer;
struct wlf_texture;
struct wlf_render;

enum wlf_render_type {
	WLF_RENDER_VULKAN = 0,
};

struct wlf_renderer_impl {
	enum wlf_render_type type; /**< Renderer type */
	void (*destroy)(struct wlf_render *render);
	struct wlf_texture *(*texture_from_buffer)(struct wlf_render *render,
		struct wlf_buffer *buffer);
};

struct wlf_render {
	const struct wlf_renderer_impl *impl;
};

struct wlf_render *wlf_renderer_autocreate(struct wlf_backend *backend);

#endif // RENDER_WLF_RENDER_H
