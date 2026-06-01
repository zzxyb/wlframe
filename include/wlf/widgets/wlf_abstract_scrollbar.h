/**
 * @file        wlf_abstract_scrollbar.h
 * @brief       Abstract scrollbar widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_SCROLLBAR_H
#define WIDGETS_WLF_ABSTRACT_SCROLLBAR_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_scrollbar;

struct wlf_abstract_scrollbar_value_event {
	struct wlf_abstract_scrollbar *widget;
	double value;
	double normalized_value;
};

struct wlf_abstract_scrollbar_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_scrollbar *widget);
	void (*set_frame)(struct wlf_abstract_scrollbar *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_scrollbar *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_scrollbar *widget,
		enum wlf_widget_state state);
	void (*set_range)(struct wlf_abstract_scrollbar *widget,
		double minimum, double maximum);
	void (*set_page_size)(struct wlf_abstract_scrollbar *widget,
		double page_size);
	void (*set_value)(struct wlf_abstract_scrollbar *widget, double value);
	void (*set_normalized_value)(struct wlf_abstract_scrollbar *widget,
		double normalized_value);
};

struct wlf_abstract_scrollbar {
	const struct wlf_abstract_scrollbar_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	enum wlf_widget_orientation orientation;
	bool enabled;
	double minimum;
	double maximum;
	double page_size;
	double value;
	double normalized_value;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal value_changed;
	} events;
};

void wlf_abstract_scrollbar_init(struct wlf_abstract_scrollbar *widget,
	const struct wlf_abstract_scrollbar_impl *impl,
	struct wlf_scene_tree *parent, const struct wlf_frect *frame,
	enum wlf_widget_orientation orientation);
void wlf_abstract_scrollbar_destroy(struct wlf_abstract_scrollbar *widget);
void wlf_abstract_scrollbar_set_frame(struct wlf_abstract_scrollbar *widget,
	const struct wlf_frect *frame);
void wlf_abstract_scrollbar_set_enabled(struct wlf_abstract_scrollbar *widget,
	bool enabled);
void wlf_abstract_scrollbar_set_state(struct wlf_abstract_scrollbar *widget,
	enum wlf_widget_state state);
void wlf_abstract_scrollbar_set_range(struct wlf_abstract_scrollbar *widget,
	double minimum, double maximum);
void wlf_abstract_scrollbar_set_page_size(struct wlf_abstract_scrollbar *widget,
	double page_size);
void wlf_abstract_scrollbar_set_value(struct wlf_abstract_scrollbar *widget,
	double value);
void wlf_abstract_scrollbar_set_normalized_value(
	struct wlf_abstract_scrollbar *widget, double normalized_value);

#endif // WIDGETS_WLF_ABSTRACT_SCROLLBAR_H
