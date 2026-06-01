/**
 * @file        wlf_abstract_menu.h
 * @brief       Abstract menu widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_MENU_H
#define WIDGETS_WLF_ABSTRACT_MENU_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <stddef.h>

struct wlf_abstract_menu;

struct wlf_menu_item {
	char text[256];
	void *data;
	bool enabled;
};

struct wlf_abstract_menu_selection_event {
	struct wlf_abstract_menu *widget;
	int selected_index;
};

struct wlf_abstract_menu_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_menu *widget);
	void (*set_frame)(struct wlf_abstract_menu *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_menu *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_menu *widget,
		enum wlf_widget_state state);
	void (*set_items)(struct wlf_abstract_menu *widget,
		const struct wlf_menu_item *items, size_t item_count);
	void (*set_selected_index)(struct wlf_abstract_menu *widget,
		int selected_index);
	void (*set_open)(struct wlf_abstract_menu *widget, bool open);
};

struct wlf_abstract_menu {
	const struct wlf_abstract_menu_impl *impl;
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

void wlf_abstract_menu_init(struct wlf_abstract_menu *widget,
	const struct wlf_abstract_menu_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_menu_destroy(struct wlf_abstract_menu *widget);
void wlf_abstract_menu_set_frame(struct wlf_abstract_menu *widget,
	const struct wlf_frect *frame);
void wlf_abstract_menu_set_enabled(struct wlf_abstract_menu *widget,
	bool enabled);
void wlf_abstract_menu_set_state(struct wlf_abstract_menu *widget,
	enum wlf_widget_state state);
void wlf_abstract_menu_set_items(struct wlf_abstract_menu *widget,
	const struct wlf_menu_item *items, size_t item_count);
void wlf_abstract_menu_set_selected_index(struct wlf_abstract_menu *widget,
	int selected_index);
void wlf_abstract_menu_set_open(struct wlf_abstract_menu *widget, bool open);

#endif // WIDGETS_WLF_ABSTRACT_MENU_H
