/**
 * @file        wlf_abstract_checkbox.h
 * @brief       Abstract checkbox widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_CHECKBOX_H
#define WIDGETS_WLF_ABSTRACT_CHECKBOX_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_checkbox;

struct wlf_abstract_checkbox_value_event {
	struct wlf_abstract_checkbox *widget;
	bool checked;
	bool indeterminate;
};

struct wlf_abstract_checkbox_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_checkbox *widget);
	void (*set_frame)(struct wlf_abstract_checkbox *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_checkbox *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_checkbox *widget,
		enum wlf_widget_state state);
	void (*set_checked)(struct wlf_abstract_checkbox *widget, bool checked);
	void (*set_indeterminate)(struct wlf_abstract_checkbox *widget,
		bool indeterminate);
	void (*toggle)(struct wlf_abstract_checkbox *widget);
};

struct wlf_abstract_checkbox {
	const struct wlf_abstract_checkbox_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	bool checked;
	bool indeterminate;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal toggled;
	} events;
};

void wlf_abstract_checkbox_init(struct wlf_abstract_checkbox *widget,
	const struct wlf_abstract_checkbox_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_checkbox_destroy(struct wlf_abstract_checkbox *widget);
void wlf_abstract_checkbox_set_frame(struct wlf_abstract_checkbox *widget,
	const struct wlf_frect *frame);
void wlf_abstract_checkbox_set_enabled(struct wlf_abstract_checkbox *widget,
	bool enabled);
void wlf_abstract_checkbox_set_state(struct wlf_abstract_checkbox *widget,
	enum wlf_widget_state state);
void wlf_abstract_checkbox_set_checked(struct wlf_abstract_checkbox *widget,
	bool checked);
void wlf_abstract_checkbox_set_indeterminate(
	struct wlf_abstract_checkbox *widget, bool indeterminate);
void wlf_abstract_checkbox_toggle(struct wlf_abstract_checkbox *widget);

#endif // WIDGETS_WLF_ABSTRACT_CHECKBOX_H
