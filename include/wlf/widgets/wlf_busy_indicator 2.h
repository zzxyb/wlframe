/**
 * @file        wlf_busy_indicator.h
 * @brief       Default busy indicator widget for wlframe.
 */

#ifndef WIDGETS_WLF_BUSY_INDICATOR_H
#define WIDGETS_WLF_BUSY_INDICATOR_H

#include "wlf/scene/wlf_scene_line.h"
#include "wlf/widgets/wlf_abstract_busy_indicator.h"

#define WLF_BUSY_INDICATOR_SPOKE_COUNT 8

struct wlf_busy_indicator {
	struct wlf_abstract_busy_indicator base;
	struct wlf_scene_line *spokes[WLF_BUSY_INDICATOR_SPOKE_COUNT];
};

struct wlf_busy_indicator *wlf_busy_indicator_create(
	struct wlf_scene_tree *parent, const struct wlf_frect *frame,
	bool running);
bool wlf_abstract_busy_indicator_is_default(
	const struct wlf_abstract_busy_indicator *widget);
struct wlf_busy_indicator *wlf_busy_indicator_from_abstract(
	struct wlf_abstract_busy_indicator *widget);

#endif // WIDGETS_WLF_BUSY_INDICATOR_H
