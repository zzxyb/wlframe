#include "wlf/buffer/vulkan/render_buffer.h"
#include "wlf/utils/wlf_linked_list.h"

struct wlf_vk_render_buffer *wlf_vk_render_buffer_create(
		struct wlf_vk_renderer *renderer, struct wlf_buffer *wlf_buffer) {
	struct wlf_vk_render_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_vk_render_buffer");
		return NULL;
	}

	buffer->wlf_buffer = wlf_buffer;
	buffer->renderer = renderer;
	wlf_linked_list_insert(&renderer->render_buffers, &buffer->link);
}

void wlf_vk_render_buffer_destroy(struct wlf_vk_render_buffer *buffer) {

}

struct wlf_vk_render_buffer *wlf_vk_render_buffer_get(
		struct wlf_vk_renderer *renderer, struct wlf_buffer *wlf_buffer) {

}
