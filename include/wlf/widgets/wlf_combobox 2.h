/**
 * @file        wlf_combobox.h
 * @brief       Default combobox widget for wlframe.
 */

#ifndef WIDGETS_WLF_COMBOBOX_H
#define WIDGETS_WLF_COMBOBOX_H

#include "wlf/scene/wlf_scene_line.h"
#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/scene/wlf_scene_text.h"
#include "wlf/widgets/wlf_abstract_combobox.h"

struct wlf_combobox {
	struct wlf_abstract_combobox base;
	struct wlf_combobox_option *options;
	size_t option_count;

	struct wlf_scene_radius_rect *backend;
	struct wlf_scene_text *label;
	struct wlf_scene_line *arrow_a;
	struct wlf_scene_line *arrow_b;
	struct wlf_scene_tree *popup_tree;
	struct wlf_scene_radius_rect *popup_bg;
	struct wlf_scene_text **popup_labels;
};

struct wlf_combobox *wlf_combobox_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
bool wlf_abstract_combobox_is_default(
	const struct wlf_abstract_combobox *widget);
struct wlf_combobox *wlf_combobox_from_abstract(
	struct wlf_abstract_combobox *widget);

#endif // WIDGETS_WLF_COMBOBOX_H
