#ifndef WLF_RENDER_SCENE_H
#define WLF_RENDER_SCENE_H

#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_region.h"

struct wlf_scene_tree;

enum wlf_scene_debug_damage_option {
	WLF_SCENE_DEBUG_DAMAGE_NONE = 0,
	WLF_SCENE_DEBUG_DAMAGE_HIGHLIGHT = 1,
};

struct wlf_scene_node {
	struct wlf_scene_tree *parent; // wlf_scene_tree.parent
	struct wlf_double_list link; // wlf_scene_tree.children

	bool enabled;
	bool visible;
	float opacity;
	int x, y; // relative to parent
	int width, height; // size of the node

	struct {
		struct wlf_signal destroy;
	} events;

	void *data;
	struct wlf_region visible_region; // visible region of the node
	struct wlf_region opaque_region; // opaque region of the node
	struct wlf_region input_region; // input region of the node
};

struct wlf_scene_tree {
	struct wlf_scene_node *node; // pointer to the scene node

	struct wlf_double_list children; // list of child nodes
};

struct wlf_scene {
	struct wlf_scene_tree *tree;

	enum wlf_scene_debug_damage_option debug_damage_option;
	bool highlight_transparent_regions;
};

struct wlf_scene *wlf_scene_create(void);
void wlf_scene_destroy(struct wlf_scene *scene);

struct wlf_scene_node *wlf_scene_node_create(struct wlf_scene *scene);
void wlf_scene_node_destroy(struct wlf_scene_node *node);
void wlf_scene_node_set_position(struct wlf_scene_node *node, int x, int y);
void wlf_scene_node_set_enabled(struct wlf_scene_node *node, bool enabled);
void wlf_scene_node_set_visible(struct wlf_scene_node *node, bool visible);
void wlf_scene_node_set_size(struct wlf_scene_node *node, int width, int height);
void wlf_scene_node_set_opacity(struct wlf_scene_node *node, float opacity);

void wlf_scene_node_place_above(struct wlf_scene_node *node, struct wlf_scene_node *above);
void wlf_scene_node_place_below(struct wlf_scene_node *node, struct wlf_scene_node *below);
void wlf_scene_node_raise_to_top(struct wlf_scene_node *node);
void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node);
void wlf_scene_node_reparent(struct wlf_scene_node *node, struct wlf_scene_node *new_parent);

struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene *scene, double x, double y, double *nx, double *ny);

struct wlf_scene_tree *wlf_scene_tree_create(struct wlf_scene *scene);
void wlf_scene_tree_destroy(struct wlf_scene_tree *tree);
struct wlf_scene_tree *wlf_scene_tree_from_node(struct wlf_scene_node *node);

#endif
