/**
 * @file        wlf_abstract_combobox.h
 * @brief       Abstract combobox widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_COMBOBOX_H
#define WIDGETS_WLF_ABSTRACT_COMBOBOX_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stddef.h>

struct wlf_abstract_combobox;

struct wlf_combobox_option {
	char text[256];
	void *data;
	bool enabled;
};

struct wlf_abstract_combobox_selection_event {
	struct wlf_abstract_combobox *widget;
	int selected_index;
};

struct wlf_abstract_combobox_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_combobox *widget);
	void (*set_frame)(struct wlf_abstract_combobox *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_combobox *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_combobox *widget,
		enum wlf_widget_state state);
	void (*set_options)(struct wlf_abstract_combobox *widget,
		const struct wlf_combobox_option *options, size_t option_count);
	void (*set_selected_index)(struct wlf_abstract_combobox *widget,
		int selected_index);
	void (*set_open)(struct wlf_abstract_combobox *widget, bool open);
};

struct wlf_abstract_combobox {
	const struct wlf_abstract_combobox_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	bool open;
	int selected_index;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal selection_changed;
		struct wlf_signal open_changed;
	} events;
};

void wlf_abstract_combobox_init(struct wlf_abstract_combobox *widget,
	const struct wlf_abstract_combobox_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_combobox_destroy(struct wlf_abstract_combobox *widget);
void wlf_abstract_combobox_set_frame(struct wlf_abstract_combobox *widget,
	const struct wlf_frect *frame);
void wlf_abstract_combobox_set_enabled(struct wlf_abstract_combobox *widget,
	bool enabled);
void wlf_abstract_combobox_set_state(struct wlf_abstract_combobox *widget,
	enum wlf_widget_state state);
void wlf_abstract_combobox_set_options(struct wlf_abstract_combobox *widget,
	const struct wlf_combobox_option *options, size_t option_count);
void wlf_abstract_combobox_set_selected_index(
	struct wlf_abstract_combobox *widget, int selected_index);
void wlf_abstract_combobox_set_open(struct wlf_abstract_combobox *widget,
	bool open);

#endif // WIDGETS_WLF_ABSTRACT_COMBOBOX_H
