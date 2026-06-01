/**
 * @file        wlf_abstract_slider.h
 * @brief       Abstract slider widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_SLIDER_H
#define WIDGETS_WLF_ABSTRACT_SLIDER_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_slider;

struct wlf_abstract_slider_value_event {
	struct wlf_abstract_slider *widget;
	double value;
	double normalized_value;
};

struct wlf_abstract_slider_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_slider *widget);
	void (*set_frame)(struct wlf_abstract_slider *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_slider *widget, bool enabled);
	void (*set_state)(struct wlf_abstract_slider *widget,
		enum wlf_widget_state state);
	void (*set_range)(struct wlf_abstract_slider *widget,
		double minimum, double maximum);
	void (*set_step)(struct wlf_abstract_slider *widget, double step);
	void (*set_value)(struct wlf_abstract_slider *widget, double value);
	void (*set_normalized_value)(struct wlf_abstract_slider *widget,
		double normalized_value);
};

struct wlf_abstract_slider {
	const struct wlf_abstract_slider_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	enum wlf_widget_orientation orientation;
	bool enabled;
	double minimum;
	double maximum;
	double step;
	double value;
	double normalized_value;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal value_changed;
	} events;
};

void wlf_abstract_slider_init(struct wlf_abstract_slider *widget,
	const struct wlf_abstract_slider_impl *impl, struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, enum wlf_widget_orientation orientation);
void wlf_abstract_slider_destroy(struct wlf_abstract_slider *widget);
void wlf_abstract_slider_set_frame(struct wlf_abstract_slider *widget,
	const struct wlf_frect *frame);
void wlf_abstract_slider_set_enabled(struct wlf_abstract_slider *widget,
	bool enabled);
void wlf_abstract_slider_set_state(struct wlf_abstract_slider *widget,
	enum wlf_widget_state state);
void wlf_abstract_slider_set_range(struct wlf_abstract_slider *widget,
	double minimum, double maximum);
void wlf_abstract_slider_set_step(struct wlf_abstract_slider *widget,
	double step);
void wlf_abstract_slider_set_value(struct wlf_abstract_slider *widget,
	double value);
void wlf_abstract_slider_set_normalized_value(
	struct wlf_abstract_slider *widget, double normalized_value);

#endif // WIDGETS_WLF_ABSTRACT_SLIDER_H
