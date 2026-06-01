#include "wlf/widgets/wlf_text_button.h"

#include "wlf/scene/wlf_scene_node.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static const struct wlf_color COLOR_TEXT = {0.11, 0.11, 0.12, 1.0};
static const struct wlf_color COLOR_TEXT_DISABLED = {0.55, 0.55, 0.58, 1.0};
static const struct wlf_color COLOR_FILL_NORMAL = {0.95, 0.95, 0.97, 1.0};
static const struct wlf_color COLOR_FILL_HOVERED = {0.92, 0.92, 0.95, 1.0};
static const struct wlf_color COLOR_FILL_PRESSED = {0.86, 0.86, 0.90, 1.0};
static const struct wlf_color COLOR_FILL_DISABLED = {0.96, 0.96, 0.97, 0.75};

static void text_button_apply_style(struct wlf_text_button *button) {
	const struct wlf_color *fill = &COLOR_FILL_NORMAL;
	const struct wlf_color *text_color = &COLOR_TEXT;

	if (!button->base.enabled || button->base.state == WLF_WIDGET_STATE_DISABLED) {
		fill = &COLOR_FILL_DISABLED;
		text_color = &COLOR_TEXT_DISABLED;
	} else if (button->base.state == WLF_WIDGET_STATE_PRESSED) {
		fill = &COLOR_FILL_PRESSED;
	} else if (button->base.state == WLF_WIDGET_STATE_HOVERED) {
		fill = &COLOR_FILL_HOVERED;
	}

	wlf_scene_radius_rect_set_color(button->backend, fill);
	wlf_scene_text_set_color(button->label, text_color);
}

static void text_button_set_frame_impl(struct wlf_abstract_button *base,
		const struct wlf_frect *frame) {
	struct wlf_text_button *button = wlf_container_of(base, button, base);

	wlf_scene_radius_rect_set_size(button->backend, frame->width, frame->height);
	wlf_scene_radius_rect_set_corner_radii(button->backend, 10.0, 10.0, 10.0, 10.0);

	wlf_scene_node_set_position(&button->label->node, 0.0, 0.0);
	wlf_scene_text_set_box(button->label, frame->width, frame->height);
}

static void text_button_destroy_impl(struct wlf_abstract_button *base) {
	struct wlf_text_button *button = wlf_container_of(base, button, base);

	if (button->base.tree != NULL) {
		wlf_scene_node_destroy(&button->base.tree->base);
		button->base.tree = NULL;
	}

	free(button);
}

static void text_button_set_enabled_impl(struct wlf_abstract_button *base, bool enabled) {
	(void)enabled;
	struct wlf_text_button *button = wlf_container_of(base, button, base);
	text_button_apply_style(button);
}

static void text_button_set_state_impl(struct wlf_abstract_button *base,
		enum wlf_widget_state state) {
	(void)state;
	struct wlf_text_button *button = wlf_container_of(base, button, base);
	text_button_apply_style(button);
}

static const struct wlf_abstract_button_impl button_impl = {
	.name = "wlf_text_button",
	.destroy = text_button_destroy_impl,
	.set_frame = text_button_set_frame_impl,
	.set_enabled = text_button_set_enabled_impl,
	.set_state = text_button_set_state_impl,
	.click = NULL,
};

struct wlf_text_button *wlf_text_button_create(struct wlf_scene_tree *parent,
		const struct wlf_frect *frame, const char *text) {
	assert(frame != NULL);

	struct wlf_text_button *button = calloc(1, sizeof(*button));
	if (button == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_text_button");
		return NULL;
	}

	wlf_abstract_button_init(&button->base, &button_impl, parent, frame);
	if (button->base.tree == NULL) {
		free(button);
		return NULL;
	}

	button->backend = wlf_scene_radius_rect_create(button->base.tree,
		frame->width, frame->height, &COLOR_FILL_NORMAL);
	if (button->backend == NULL) {
		wlf_abstract_button_destroy(&button->base);
		return NULL;
	}

	button->label = wlf_scene_text_create(button->base.tree,
		text != NULL ? text : "", "SF Pro Text", 13.0f, &COLOR_TEXT);
	if (button->label == NULL) {
		wlf_abstract_button_destroy(&button->base);
		return NULL;
	}

	wlf_scene_text_set_horizontal_alignment(button->label,
		WLF_TEXT_ALIGN_CENTER);
	wlf_scene_text_set_vertical_alignment(button->label,
		WLF_TEXT_ALIGN_VCENTER);
	wlf_scene_text_set_box(button->label, frame->width, frame->height);

	if (text != NULL) {
		snprintf(button->text, sizeof(button->text), "%s", text);
	}

	text_button_set_frame_impl(&button->base, frame);
	text_button_apply_style(button);
	return button;
}

bool wlf_abstract_button_is_text(const struct wlf_abstract_button *button) {
	return button->impl == &button_impl;
}

struct wlf_text_button *wlf_text_button_from_button(struct wlf_abstract_button *button) {
	assert(button->impl == &button_impl);

	struct wlf_text_button *text_button =
		wlf_container_of(button, text_button, base);

	return text_button;
}
