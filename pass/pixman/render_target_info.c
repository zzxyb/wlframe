#include "wlf/pass/pixman/render_target_info.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>

static void render_target_info_destroy(struct wlf_render_target_info *info) {
	struct wlf_pixman_render_target_info *pixman_render_target_info =
		wlf_pixman_render_target_info_from_render_target_info(info);
	free(pixman_render_target_info);
}

const struct wlf_render_target_info_impl render_target_info_impl = {
	.destroy = render_target_info_destroy,
};

struct wlf_pixman_render_target_info *begin_pixman_render_pass(
		struct wlf_pixman_render_buffer *buffer) {
	struct wlf_pixman_render_target_info *render_target_info = malloc(sizeof(*render_target_info));
	if (render_target_info == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_pixman_render_target_info");
		return NULL;
	}

	wlf_render_target_info_init(&render_target_info->base, &render_target_info_impl);

	if (!begin_pixman_data_ptr_access(buffer->buffer, &buffer->image,
			WLF_BUFFER_DATA_PTR_ACCESS_READ | WLF_BUFFER_DATA_PTR_ACCESS_WRITE)) {
		free(render_target_info);
		return NULL;
	}

	wlf_buffer_lock(buffer->buffer);
	render_target_info->buffer = buffer;

	return render_target_info;
}

bool wlf_pixman_render_target_info_is_pixman(const struct wlf_render_target_info *info) {
	return info->impl == &render_target_info_impl;
}

struct wlf_pixman_render_target_info *wlf_pixman_render_target_info_from_render_target_info(
		struct wlf_render_target_info *base) {
	assert(wlf_pixman_render_target_info_is_pixman(base));
	struct wlf_pixman_render_target_info *info =
		wlf_container_of(base, info, base);

	return info;
}
