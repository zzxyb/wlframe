/**
 * @file        wlf_abstract_progress_bar.h
 * @brief       Abstract progress bar widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_PROGRESS_BAR_H
#define WIDGETS_WLF_ABSTRACT_PROGRESS_BAR_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_progress_bar;

struct wlf_abstract_progress_bar_value_event {
	struct wlf_abstract_progress_bar *widget;
	double value;
	double normalized_value;
};

struct wlf_abstract_progress_bar_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_progress_bar *widget);
	void (*set_frame)(struct wlf_abstract_progress_bar *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_progress_bar *widget,
		bool enabled);
	void (*set_state)(struct wlf_abstract_progress_bar *widget,
		enum wlf_widget_state state);
	void (*set_range)(struct wlf_abstract_progress_bar *widget,
		double minimum, double maximum);
	void (*set_value)(struct wlf_abstract_progress_bar *widget,
		double value);
};

struct wlf_abstract_progress_bar {
	const struct wlf_abstract_progress_bar_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	enum wlf_widget_orientation orientation;
	bool enabled;
	double minimum;
	double maximum;
	double value;
	double normalized_value;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal value_changed;
	} events;
};

void wlf_abstract_progress_bar_init(struct wlf_abstract_progress_bar *widget,
	const struct wlf_abstract_progress_bar_impl *impl,
	struct wlf_scene_tree *parent, const struct wlf_frect *frame,
	enum wlf_widget_orientation orientation);
void wlf_abstract_progress_bar_destroy(struct wlf_abstract_progress_bar *widget);
void wlf_abstract_progress_bar_set_frame(
	struct wlf_abstract_progress_bar *widget, const struct wlf_frect *frame);
void wlf_abstract_progress_bar_set_enabled(
	struct wlf_abstract_progress_bar *widget, bool enabled);
void wlf_abstract_progress_bar_set_state(
	struct wlf_abstract_progress_bar *widget, enum wlf_widget_state state);
void wlf_abstract_progress_bar_set_range(
	struct wlf_abstract_progress_bar *widget, double minimum, double maximum);
void wlf_abstract_progress_bar_set_value(
	struct wlf_abstract_progress_bar *widget, double value);

#endif // WIDGETS_WLF_ABSTRACT_PROGRESS_BAR_H
