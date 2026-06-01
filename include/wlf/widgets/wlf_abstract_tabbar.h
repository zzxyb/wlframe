/**
 * @file        wlf_abstract_tabbar.h
 * @brief       Abstract tab bar widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_TABBAR_H
#define WIDGETS_WLF_ABSTRACT_TABBAR_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stddef.h>

struct wlf_abstract_tabbar;

struct wlf_tabbar_item {
	char text[256];
	void *data;
	bool enabled;
};

struct wlf_abstract_tabbar_selection_event {
	struct wlf_abstract_tabbar *widget;
	int selected_index;
};

struct wlf_abstract_tabbar_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_tabbar *widget);
	void (*set_frame)(struct wlf_abstract_tabbar *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_tabbar *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_tabbar *widget,
		enum wlf_widget_state state);
	void (*set_items)(struct wlf_abstract_tabbar *widget,
		const struct wlf_tabbar_item *items, size_t item_count);
	void (*set_selected_index)(struct wlf_abstract_tabbar *widget,
		int selected_index);
};

struct wlf_abstract_tabbar {
	const struct wlf_abstract_tabbar_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	int selected_index;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal selection_changed;
	} events;
};

void wlf_abstract_tabbar_init(struct wlf_abstract_tabbar *widget,
	const struct wlf_abstract_tabbar_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_tabbar_destroy(struct wlf_abstract_tabbar *widget);
void wlf_abstract_tabbar_set_frame(struct wlf_abstract_tabbar *widget,
	const struct wlf_frect *frame);
void wlf_abstract_tabbar_set_enabled(struct wlf_abstract_tabbar *widget,
	bool enabled);
void wlf_abstract_tabbar_set_state(struct wlf_abstract_tabbar *widget,
	enum wlf_widget_state state);
void wlf_abstract_tabbar_set_items(struct wlf_abstract_tabbar *widget,
	const struct wlf_tabbar_item *items, size_t item_count);
void wlf_abstract_tabbar_set_selected_index(struct wlf_abstract_tabbar *widget,
	int selected_index);

#endif // WIDGETS_WLF_ABSTRACT_TABBAR_H
