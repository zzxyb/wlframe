/**
 * @file        wlf_abstract_button.h
 * @brief       Abstract button widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_BUTTON_H
#define WIDGETS_WLF_ABSTRACT_BUTTON_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_button;

struct wlf_abstract_button_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_button *button);
	void (*set_frame)(struct wlf_abstract_button *button,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_button *button, bool enabled);
	void (*set_state)(struct wlf_abstract_button *button,
		enum wlf_widget_state state);
	void (*click)(struct wlf_abstract_button *button);
};

struct wlf_abstract_button {
	const struct wlf_abstract_button_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal clicked;
	} events;
};

void wlf_abstract_button_init(struct wlf_abstract_button *button,
	const struct wlf_abstract_button_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_button_destroy(struct wlf_abstract_button *button);
void wlf_abstract_button_set_frame(struct wlf_abstract_button *button,
	const struct wlf_frect *frame);
void wlf_abstract_button_set_enabled(struct wlf_abstract_button *button,
	bool enabled);
void wlf_abstract_button_set_state(struct wlf_abstract_button *button,
	enum wlf_widget_state state);
void wlf_abstract_button_click(struct wlf_abstract_button *button);

#endif // WIDGETS_WLF_ABSTRACT_BUTTON_H
