/**
 * @file        wlf_slider.h
 * @brief       Default slider widget for wlframe.
 */

#ifndef WIDGETS_WLF_SLIDER_H
#define WIDGETS_WLF_SLIDER_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/widgets/wlf_abstract_slider.h"

struct wlf_slider {
	struct wlf_abstract_slider base;
	struct wlf_scene_radius_rect *track;
	struct wlf_scene_radius_rect *fill;
	struct wlf_scene_radius_rect *thumb;
};

struct wlf_slider *wlf_slider_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, enum wlf_widget_orientation orientation);
bool wlf_abstract_slider_is_default(const struct wlf_abstract_slider *widget);
struct wlf_slider *wlf_slider_from_abstract(struct wlf_abstract_slider *widget);

#endif // WIDGETS_WLF_SLIDER_H
