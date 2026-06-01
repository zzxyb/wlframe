#include "wlf/widgets/wlf_icon_button.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>

static void icon_button_set_frame_impl(struct wlf_abstract_button *base,
		const struct wlf_frect *frame) {
	struct wlf_icon_button *button = wlf_container_of(base, button, base);

	if (button->backend != NULL) {
		wlf_scene_radius_rect_set_size(button->backend, frame->width, frame->height);
		wlf_scene_radius_rect_set_corner_radii(button->backend, 10.0, 10.0, 10.0, 10.0);
	}

	if (button->image != NULL) {
		wlf_scene_node_set_position(&button->image->node, 0.0, 0.0);
		wlr_scene_buffer_set_dest_size(button->image,
			(int)frame->width, (int)frame->height);
	}
}

static void icon_button_destroy_impl(struct wlf_abstract_button *base) {
	struct wlf_icon_button *button = wlf_container_of(base, button, base);

	if (button->base.tree != NULL) {
		wlf_scene_node_destroy(&button->base.tree->base);
		button->base.tree = NULL;
	}

	free(button);
}

static const struct wlf_abstract_button_impl button_impl = {
	.name = "wlf_icon_button",
	.destroy = icon_button_destroy_impl,
	.set_frame = icon_button_set_frame_impl,
	.set_enabled = NULL,
	.set_state = NULL,
	.click = NULL,
};

struct wlf_icon_button *wlf_icon_button_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, struct wlf_texture *icon_texture,
		bool own_texture) {
	assert(frame != NULL);

	struct wlf_icon_button *button = calloc(1, sizeof(*button));
	if (button == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_icon_button");
		return NULL;
	}

	wlf_abstract_button_init(&button->base, &button_impl, parent, frame);
	if (button->base.tree == NULL) {
		free(button);
		return NULL;
	}

	button->backend = wlf_scene_radius_rect_create(button->base.tree,
		frame->width, frame->height, &WLF_COLOR_TRANSPARENT);
	if (button->backend == NULL) {
		wlf_abstract_button_destroy(&button->base);
		return NULL;
	}

	button->image = wlf_scene_texture_create(button->base.tree,
		icon_texture, own_texture);
	if (button->image == NULL) {
		wlf_abstract_button_destroy(&button->base);
		return NULL;
	}

	icon_button_set_frame_impl(&button->base, frame);
	return button;
}

bool wlf_abstract_button_is_icon(const struct wlf_abstract_button *button) {
	return button->impl == &button_impl;
}

struct wlf_icon_button *wlf_icon_button_from_button(struct wlf_abstract_button *button) {
	assert(button->impl == &button_impl);

	struct wlf_icon_button *icon_button =
		wlf_container_of(button, icon_button, base);

	return icon_button;
}
