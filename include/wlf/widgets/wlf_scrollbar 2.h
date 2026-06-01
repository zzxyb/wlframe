/**
 * @file        wlf_scrollbar.h
 * @brief       Default scrollbar widget for wlframe.
 */

#ifndef WIDGETS_WLF_SCROLLBAR_H
#define WIDGETS_WLF_SCROLLBAR_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/widgets/wlf_abstract_scrollbar.h"

struct wlf_scrollbar {
	struct wlf_abstract_scrollbar base;
	struct wlf_scene_radius_rect *track;
	struct wlf_scene_radius_rect *thumb;
};

struct wlf_scrollbar *wlf_scrollbar_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, enum wlf_widget_orientation orientation);
bool wlf_abstract_scrollbar_is_default(
	const struct wlf_abstract_scrollbar *widget);
struct wlf_scrollbar *wlf_scrollbar_from_abstract(
	struct wlf_abstract_scrollbar *widget);

#endif // WIDGETS_WLF_SCROLLBAR_H
