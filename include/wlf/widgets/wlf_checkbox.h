/**
 * @file        wlf_checkbox.h
 * @brief       Default checkbox widget for wlframe.
 */

#ifndef WIDGETS_WLF_CHECKBOX_H
#define WIDGETS_WLF_CHECKBOX_H

#include "wlf/scene/wlf_scene_line.h"
#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/widgets/wlf_abstract_checkbox.h"

struct wlf_checkbox {
	struct wlf_abstract_checkbox base;
	struct wlf_scene_radius_rect *backend;
	struct wlf_scene_line *mark_a;
	struct wlf_scene_line *mark_b;
};

struct wlf_checkbox *wlf_checkbox_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, bool checked);
bool wlf_abstract_checkbox_is_default(
	const struct wlf_abstract_checkbox *widget);
struct wlf_checkbox *wlf_checkbox_from_abstract(
	struct wlf_abstract_checkbox *widget);

#endif // WIDGETS_WLF_CHECKBOX_H
