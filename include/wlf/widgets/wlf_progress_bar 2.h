/**
 * @file        wlf_progress_bar.h
 * @brief       Default progress bar widget for wlframe.
 */

#ifndef WIDGETS_WLF_PROGRESS_BAR_H
#define WIDGETS_WLF_PROGRESS_BAR_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/widgets/wlf_abstract_progress_bar.h"

struct wlf_progress_bar {
	struct wlf_abstract_progress_bar base;
	struct wlf_scene_radius_rect *track;
	struct wlf_scene_radius_rect *fill;
};

struct wlf_progress_bar *wlf_progress_bar_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, enum wlf_widget_orientation orientation);
bool wlf_abstract_progress_bar_is_default(
	const struct wlf_abstract_progress_bar *widget);
struct wlf_progress_bar *wlf_progress_bar_from_abstract(
	struct wlf_abstract_progress_bar *widget);

#endif // WIDGETS_WLF_PROGRESS_BAR_H
