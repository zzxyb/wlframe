/**
 * @file        wlf_menu.h
 * @brief       Default menu widget for wlframe.
 */

#ifndef WIDGETS_WLF_MENU_H
#define WIDGETS_WLF_MENU_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/scene/wlf_scene_text.h"
#include "wlf/widgets/wlf_abstract_menu.h"

struct wlf_menu {
	struct wlf_abstract_menu base;
	struct wlf_menu_item *items;
	size_t item_count;

	struct wlf_scene_radius_rect *background;
	struct wlf_scene_radius_rect **item_backgrounds;
	struct wlf_scene_text **labels;
};

struct wlf_menu *wlf_menu_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame);
bool wlf_abstract_menu_is_default(const struct wlf_abstract_menu *widget);
struct wlf_menu *wlf_menu_from_abstract(struct wlf_abstract_menu *widget);

#endif // WIDGETS_WLF_MENU_H
