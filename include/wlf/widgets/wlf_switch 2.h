/**
 * @file        wlf_switch.h
 * @brief       Default switch widget for wlframe.
 */

#ifndef WIDGETS_WLF_SWITCH_H
#define WIDGETS_WLF_SWITCH_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/widgets/wlf_abstract_switch.h"

struct wlf_switch {
	struct wlf_abstract_switch base;
	struct wlf_scene_radius_rect *track;
	struct wlf_scene_radius_rect *thumb;
};

struct wlf_switch *wlf_switch_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, bool checked);
bool wlf_abstract_switch_is_default(const struct wlf_abstract_switch *widget);
struct wlf_switch *wlf_switch_from_abstract(struct wlf_abstract_switch *widget);

#endif // WIDGETS_WLF_SWITCH_H
