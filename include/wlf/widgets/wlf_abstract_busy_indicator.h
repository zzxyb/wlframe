/**
 * @file        wlf_abstract_busy_indicator.h
 * @brief       Abstract busy indicator widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ABSTRACT_BUSY_INDICATOR_H
#define WIDGETS_WLF_ABSTRACT_BUSY_INDICATOR_H

#include "wlf/math/wlf_frect.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/widgets/wlf_palette.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_abstract_busy_indicator;

struct wlf_abstract_busy_indicator_impl {
	const char *name;

	void (*destroy)(struct wlf_abstract_busy_indicator *widget);
	void (*set_frame)(struct wlf_abstract_busy_indicator *widget,
		const struct wlf_frect *frame);
	void (*set_enabled)(struct wlf_abstract_busy_indicator *widget,
		bool enabled);
	void (*set_state)(struct wlf_abstract_busy_indicator *widget,
		enum wlf_widget_state state);
	void (*set_running)(struct wlf_abstract_busy_indicator *widget,
		bool running);
	void (*set_progress)(struct wlf_abstract_busy_indicator *widget,
		double progress);
};

struct wlf_abstract_busy_indicator {
	const struct wlf_abstract_busy_indicator_impl *impl;
	struct wlf_scene_tree *tree;

	struct wlf_frect frame;
	bool enabled;
	bool running;
	double progress;
	enum wlf_widget_state state;
	void *data;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal running_changed;
		struct wlf_signal progress_changed;
	} events;
};

void wlf_abstract_busy_indicator_init(
	struct wlf_abstract_busy_indicator *widget,
	const struct wlf_abstract_busy_indicator_impl *impl,
	struct wlf_scene_tree *parent, const struct wlf_frect *frame);
void wlf_abstract_busy_indicator_destroy(
	struct wlf_abstract_busy_indicator *widget);
void wlf_abstract_busy_indicator_set_frame(
	struct wlf_abstract_busy_indicator *widget, const struct wlf_frect *frame);
void wlf_abstract_busy_indicator_set_enabled(
	struct wlf_abstract_busy_indicator *widget, bool enabled);
void wlf_abstract_busy_indicator_set_state(
	struct wlf_abstract_busy_indicator *widget, enum wlf_widget_state state);
void wlf_abstract_busy_indicator_set_running(
	struct wlf_abstract_busy_indicator *widget, bool running);
void wlf_abstract_busy_indicator_set_progress(
	struct wlf_abstract_busy_indicator *widget, double progress);

#endif // WIDGETS_WLF_ABSTRACT_BUSY_INDICATOR_H
