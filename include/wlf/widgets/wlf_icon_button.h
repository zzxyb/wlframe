/**
 * @file        wlf_icon_button.h
 * @brief       Icon button widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_ICON_BUTTON_H
#define WIDGETS_WLF_ICON_BUTTON_H

#include "wlf/widgets/wlf_abstract_button.h"
#include "wlf/scene/wlf_scene_texture.h"
#include "wlf/scene/wlf_scene_radius_rect.h"

#include <stdbool.h>

struct wlf_texture;

/**
 * @brief Concrete icon button base.
 */
struct wlf_icon_button {
	struct wlf_abstract_button base;         /**< Abstract button base */
	struct wlf_scene_texture *image;
	struct wlf_scene_radius_rect *backend;
};

struct wlf_icon_button *wlf_icon_button_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, struct wlf_texture *icon_texture,
	bool own_texture);
bool wlf_abstract_button_is_icon(const struct wlf_abstract_button *button);
struct wlf_icon_button *wlf_icon_button_from_button(
	struct wlf_abstract_button *button);

#endif // WIDGETS_WLF_ICON_BUTTON_H
