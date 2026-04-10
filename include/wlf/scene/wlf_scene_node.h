#ifndef SCENE_WLF_SCENE_NODE_H
#define SCENE_WLF_SCENE_NODE_H

#include "wlf/math/wlf_region.h"
#include "wlf/scene/wlf_scene_capture.h"
#include "wlf/utils/wlf_addon.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>

struct wlf_scene;
struct wlf_scene_node;

enum wlf_focus_policy {
	NO_FOCUS,
	TAB_FOCUS,
	CLICK_FOCUS,
};

struct wlf_scene_node_impl {
	void (*destroy)(struct wlf_scene_node *node);
	void (*set_enabled)(struct wlf_scene_node *node, bool enabled);
	void (*set_position)(struct wlf_scene_node *node, double x, double y);
	void (*place_above)(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling);
	void (*place_below)(struct wlf_scene_node *node,
		struct wlf_scene_node *sibling);
	void (*raise_to_top)(struct wlf_scene_node *node);
	void (*lower_to_bottom)(struct wlf_scene_node *node);
	void (*reparent)(struct wlf_scene_node *node,
		struct wlf_scene_tree *new_parent);
	struct wlf_linked_list *(*get_children)(struct wlf_scene_node *node);
	struct wlf_scene_node *(*get_base)(struct wlf_scene_node *node);
	void (*get_size)(struct wlf_scene_node *node,
		double *width, double *height);
	void (*set_opacity)(struct wlf_scene_node *node,
		float opacity);
	void (*set_transparent_region)(struct wlf_scene_node *node,
		struct wlf_region *region);
	void (*set_input_passthrough_region)(struct wlf_scene_node *node,
		struct wlf_region *region);
	struct wlf_scene_node *(*at)(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny);
	bool (*coords)(struct wlf_scene_node *node,
		double *lx_ptr, double *ly_ptr);
	void (*update)(struct wlf_scene_node *node, struct wlf_region *damage);
	void (*visibility)(struct wlf_scene_node *node, struct wlf_region *visible);
};

struct wlf_scene_node_state {
	bool enabled;
	double x, y, width, height; // pos relative to parent
	float opacity;

	enum wlf_focus_policy focus_policy;
	struct wlf_region visible;
	struct wlf_region transparent_region;
	struct wlf_region input_passthrough_region;
};

struct wlf_scene_node {
	const struct wlf_scene_node_impl *impl;

	struct wlf_scene_node *parent;

	struct wlf_linked_list link; // wlf_scene_tree.children

	struct wlf_scene *scene;
	struct {
		struct wlf_signal destroy;
	} events;

	struct wlf_scene_capture capture;
	struct wlf_scene_node_state state;
	void *data;
	struct wlf_addon_set addons;
};

struct wlf_scene_node_render_entry {
	struct wlf_scene_node *node;
	bool highlight_transparent_region;
	double x, y;
};

void wlf_scene_node_init(struct wlf_scene_node *node,
	const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent);
void wlf_scene_node_destroy(struct wlf_scene_node *node);
void wlf_scene_node_set_enabled(struct wlf_scene_node *node, bool enabled);
void wlf_scene_node_set_position(struct wlf_scene_node *node, double x, double y);
void wlf_scene_node_place_above(struct wlf_scene_node *node,
	struct wlf_scene_node *sibling);
void wlf_scene_node_place_below(struct wlf_scene_node *node,
	struct wlf_scene_node *sibling);
void wlf_scene_node_raise_to_top(struct wlf_scene_node *node);
void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node);
void wlf_scene_node_reparent(struct wlf_scene_node *node,
	struct wlf_scene_node *new_parent);
struct wlf_linked_list *wlf_scene_node_get_children(struct wlf_scene_node *node);
struct wlf_scene_node *wlf_scene_node_get_base(struct wlf_scene_node *node);
void wlf_scene_node_get_size(struct wlf_scene_node *node,
	double *width, double *height);
void wlf_scene_node_set_opacity(struct wlf_scene_node *node,
	float opacity);
void wlf_scene_node_set_transparent_region(struct wlf_scene_node *node,
	struct wlf_region *region);
void wlf_scene_node_set_input_passthrough_region(struct wlf_scene_node *node,
	struct wlf_region *region);
struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene_node *node,
	double lx, double ly, double *nx, double *ny);
bool wlf_scene_node_coords(struct wlf_scene_node *node,
	double *lx_ptr, double *ly_ptr);
void wlf_scene_node_update(struct wlf_scene_node *node, struct wlf_region *damage);
void wlf_scene_node_visibility(struct wlf_scene_node *node, struct wlf_region *visible);

#endif // SCENE_WLF_SCENE_NODE_H