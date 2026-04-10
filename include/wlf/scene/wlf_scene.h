#ifndef SCENE_WLF_SCENE_H
#define SCENE_WLF_SCENE_H

#include "wlf/math/wlf_region.h"
#include "wlf/scene/wlf_scene_tree.h"
#include "wlf/types/wlf_damage_ring.h"
#include "wlf/window/wlf_window.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <time.h>

enum wlf_scene_debug_damage_option {
	WLF_SCENE_DEBUG_DAMAGE_NONE,
	WLF_SCENE_DEBUG_DAMAGE_RERENDER,
	WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT
};

struct wlf_scene {
	struct wlf_scene_tree tree;

	struct wlf_window *window;

	struct wlf_damage_ring damage_ring;
	enum wlf_scene_debug_damage_option debug_damage_option;
	bool direct_scanout;
	bool calculate_visibility;
	bool highlight_transparent_region;

	struct {
		struct wlf_signal destroy;
	} events;
};

struct wlf_scene *wlf_scene_create(void);
void wlf_scene_destroy(struct wlf_scene *scene);

#endif // SCENE_WLF_SCENE_H