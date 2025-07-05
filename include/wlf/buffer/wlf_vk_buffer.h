#ifndef BUFFER_WLF_VK_BUFFER_H
#define BUFFER_WLF_VK_BUFFER_H



struct wlf_vk_renderer;
struct wlf_buffer;

struct wlf_vk_buffer {
	struct wlf_buffer *buffer;
	struct wlf_vk_renderer *renderer;
};

#endif
