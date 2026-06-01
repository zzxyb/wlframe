/**
 * @file        wlf_text_button.h
 * @brief       Text button widget interface for wlframe.
 */

#ifndef WIDGETS_WLF_TEXT_BUTTON_H
#define WIDGETS_WLF_TEXT_BUTTON_H

#include "wlf/scene/wlf_scene_radius_rect.h"
#include "wlf/scene/wlf_scene_text.h"
#include "wlf/widgets/wlf_abstract_button.h"

/**
 * @brief Default wlframe text button.
 */
struct wlf_text_button {
	struct wlf_abstract_button base;         /**< Abstract button base */
	char text[256];                          /**< Button label text */
	struct wlf_scene_radius_rect *backend;   /**< Default rounded background */
	struct wlf_scene_text *label;            /**< Default label node */
};

struct wlf_text_button *wlf_text_button_create(struct wlf_scene_tree *parent,
	const struct wlf_frect *frame, const char *text);
bool wlf_abstract_button_is_text(const struct wlf_abstract_button *button);
struct wlf_text_button *wlf_text_button_from_button(
	struct wlf_abstract_button *button);

#endif // WIDGETS_WLF_TEXT_BUTTON_H
