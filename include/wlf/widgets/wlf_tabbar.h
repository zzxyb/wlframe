/**
 * @file        wlf_tabbar.h
 * @brief       Default tab bar widget for wlframe.
 */

#ifndef WIDGETS_WLF_TABBAR_H
#define WIDGETS_WLF_TABBAR_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/scene/wlf_scene_text.h"
#include "wlf/widgets/wlf_abstract_tabbar.h"

struct wlf_tabbar {
	struct wlf_abstract_tabbar base;
	struct wlf_tabbar_item *items;
	size_t item_count;

	struct wlf_scene_radius_rect *background;
	struct wlf_scene_radius_rect **segments;
	struct wlf_scene_text **labels;
};

struct wlf_tabbar *wlf_tabbar_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
bool wlf_abstract_tabbar_is_default(
	const struct wlf_abstract_tabbar *widget);
struct wlf_tabbar *wlf_tabbar_from_abstract(
	struct wlf_abstract_tabbar *widget);

#endif // WIDGETS_WLF_TABBAR_H
