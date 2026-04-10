#include "wlf/scene/wlf_scene.h"
#include "wlf/utils/wlf_utils.h"

#include <stdlib.h>

struct wlf_scene *wlf_scene_create(void) {
	struct wlf_scene *scene = calloc(1, sizeof(*scene));
	if (scene == NULL) {
		return NULL;
	}

	wlf_scene_tree_init(&scene->tree, NULL);
	wlf_damage_ring_init(&scene->damage_ring);
	return scene;
}

void wlf_scene_destroy(struct wlf_scene *scene) {
	if (scene == NULL) {
		return;
	}

	wlf_damage_ring_finish(&scene->damage_ring);
	wlf_scene_tree_finish(&scene->tree);
	free(scene);
}

bool wlf_scene_needs_frame(struct wlf_scene *scene) {
	return scene != NULL && !wlf_region_is_nil(&scene->damage_ring.current);
}

bool wlf_scene_commit(struct wlf_scene *scene) {
	if (scene == NULL) {
		return false;
	}

	if (scene->window != NULL) {
		wlf_signal_emit_mutable(&scene->window->events.expose, scene);
	}

	return true;
}

bool wlf_scene_build_state(struct wlf_scene *scene) {
	return scene != NULL;
}

void wlf_scene_send_frame_done(struct wlf_scene *scene,
		const struct timespec *now) {
	WLF_UNUSED(scene);
	WLF_UNUSED(now);
}
