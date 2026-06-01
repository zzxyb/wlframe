/**
 * @file        wlf_abstract_switch.h
 * @brief       Abstract switch widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_SWITCH_H
#define WIDGETS_WLF_ABSTRACT_SWITCH_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_switch;

struct wlf_abstract_switch_value_event {
	struct wlf_abstract_switch *widget;
	bool checked;
	double position;
};

struct wlf_abstract_switch_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_switch *widget);
	void (*set_frame)(struct wlf_abstract_switch *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_switch *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_switch *widget,
		enum wlf_widget_state state);
	void (*set_checked)(struct wlf_abstract_switch *widget, bool checked);
	void (*set_position)(struct wlf_abstract_switch *widget, double position);
	void (*toggle)(struct wlf_abstract_switch *widget);
};

struct wlf_abstract_switch {
	const struct wlf_abstract_switch_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	bool checked;
	double position;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal toggled;
	} events;
};

void wlf_abstract_switch_init(struct wlf_abstract_switch *widget,
	const struct wlf_abstract_switch_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
void wlf_abstract_switch_destroy(struct wlf_abstract_switch *widget);
void wlf_abstract_switch_set_frame(struct wlf_abstract_switch *widget,
	const struct wlf_frect *frame);
void wlf_abstract_switch_set_enabled(struct wlf_abstract_switch *widget,
	bool enabled);
void wlf_abstract_switch_set_state(struct wlf_abstract_switch *widget,
	enum wlf_widget_state state);
void wlf_abstract_switch_set_checked(struct wlf_abstract_switch *widget,
	bool checked);
void wlf_abstract_switch_set_position(struct wlf_abstract_switch *widget,
	double position);
void wlf_abstract_switch_toggle(struct wlf_abstract_switch *widget);

#endif // WIDGETS_WLF_ABSTRACT_SWITCH_H
