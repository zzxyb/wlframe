/**
 * @file        wlf_scene_node.h
 * @brief       Scene node management for wlframe.
 * @details     This file provides the core scene node structure and operations for building
 *              hierarchical scene graphs in wlframe. Scene nodes represent visual elements
 *              that can be positioned, transformed, and rendered within a window.
 * @author      YaoBing Xiao
 * @date        2026-05-01
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-05-01, initial version\n
 */

#ifndef SCENE_WLF_SCENE_NODE_H
#define SCENE_WLF_SCENE_NODE_H

#include "wlf/math/wlf_region.h"
#include "wlf/utils/wlf_addon.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/math/wlf_frect.h"
#include "wlf/pass/wlf_render_target_info.h"

#include <stdbool.h>

struct wlf_window;
struct wlf_scene_node;

/**
 * @brief Focus policy for scene nodes.
 *
 * Determines how a scene node can receive keyboard focus.
 */
enum wlf_focus_policy {
	NO_FOCUS,     /**< Node cannot receive focus */
	TAB_FOCUS,    /**< Node can receive focus via tab navigation */
	CLICK_FOCUS,  /**< Node can receive focus via mouse click */
};

struct wlr_scene_update_data {
	struct wlf_region *visible;
	const struct wlf_region *update_region;
	struct wlf_frect update_box;
	bool calculate_visibility;
};

struct wlr_render_data {
	float scale;
	struct wlf_frect logical;
	double trans_width, trans_height;

	struct wlf_render_target_info *render_target_info;
	struct wlf_region damage;
};

struct wlr_render_list_constructor_data {
	struct wlf_frect box;
	struct wlf_array *render_list;
	bool calculate_visibility;
	bool highlight_transparent_region;
	bool fractional_scale;
};

struct wlr_render_list_entry {
	struct wlr_scene_node *node;
	bool highlight_transparent_region;
	int x, y;
};

/**
 * @brief Implementation interface for scene node operations.
 *
 * This structure defines the virtual methods that concrete scene node types must implement.
 * It provides the polymorphic interface for different types of scene nodes.
 */
struct wlf_scene_node_impl {
	/**
	 * @brief Destroys the scene node
	 * @param node The scene node to destroy
	 */
	void (*destroy)(struct wlf_scene_node *node);

	/**
	 * @brief Sets whether the scene node is enabled
	 * @param node The scene node
	 * @param enabled Whether the node should be enabled
	 */
	void (*set_enabled)(struct wlf_scene_node *node, bool enabled);

	/**
	 * @brief Sets the position of the scene node
	 * @param node The scene node
	 * @param x X coordinate relative to parent
	 * @param y Y coordinate relative to parent
	 */
	void (*set_position)(struct wlf_scene_node *node, double x, double y);

	/**
	 * @brief Sets the opacity of the scene node
	 * @param node The scene node
	 * @param opacity Opacity level (0.0 to 1.0)
	 */
	void (*set_opacity)(struct wlf_scene_node *node,
		float opacity);

	/**
	 * @brief Gets the size of the scene node
	 * @param node The scene node
	 * @param width Pointer to store width
	 * @param height Pointer to store height
	 */
	void (*get_size)(struct wlf_scene_node *node, double *width, double *height);

	/**
	 * @brief Gets the children list of the scene node
	 * @param node The scene node
	 * @return Linked list of child nodes
	 */
	struct wlf_linked_list *(*get_children)(struct wlf_scene_node *node);

	/**
	 * @brief Gets the opaque region of the scene node
	 * @param node The scene node
	 * @param x X offset
	 * @param y Y offset
	 * @param opaque Region to store opaque area
	 */
	void (*get_opaque_region)(struct wlf_scene_node *node, double x, double y,
		struct wlf_region *opaque);

	/**
	 * @brief Checks if the scene node is invisible
	 * @param node The scene node
	 * @return True if the node is invisible
	 */
	bool (*invisible)(struct wlf_scene_node *node);

	/**
	 * @brief Sets the visible region of the scene node
	 * @param node The scene node
	 * @param visible Visible region
	 */
	void (*visibility)(struct wlf_scene_node *node,
		struct wlf_region *visible);

	/**
	 * @brief Finds the scene node at a given position
	 * @param node Root node to search from
	 * @param lx Local X coordinate
	 * @param ly Local Y coordinate
	 * @param nx Pointer to store node X coordinate
	 * @param ny Pointer to store node Y coordinate
	 * @return Node at the position, or NULL
	 */
	struct wlf_scene_node *(*at)(struct wlf_scene_node *node,
		double lx, double ly, double *nx, double *ny);

	/**
	 * @brief Gets the coordinates of the scene node
	 * @param node The scene node
	 * @param lx_ptr Pointer to store local X
	 * @param ly_ptr Pointer to store local Y
	 * @return True if coordinates are valid
	 */
	bool (*coords)(struct wlf_scene_node *node,
		double *lx_ptr, double *ly_ptr);

	/**
	 * @brief Updates the scene node with damage region
	 * @param node The scene node
	 * @param damage Damage region
	 */
	void (*update)(struct wlf_scene_node *node, struct wlf_region *damage);

	void (*bounds)(struct wlf_scene_node *node,
		int x, int y, struct wlf_region *visible);
};

/**
 * @brief State information for a scene node.
 *
 * Contains the current state of a scene node including position, size, opacity,
 * and various regions for rendering and input handling.
 */
struct wlf_scene_node_state {
	bool enabled;                           /**< Whether the node is enabled */
	double x, y, width, height;             /**< Position and size relative to parent */
	float opacity;                          /**< Opacity level (0.0 to 1.0) */

	enum wlf_focus_policy focus_policy;     /**< Focus policy for the node */
	struct wlf_region visible;              /**< Visible region */
	struct wlf_region transparent_region;   /**< Transparent region */
	struct wlf_region input_passthrough_region; /**< Input passthrough region */
};

/**
 * @brief Core scene node structure.
 *
 * Represents a node in the scene graph hierarchy. Scene nodes can have children,
 * handle events, and participate in rendering. Each node has an implementation
 * that defines its behavior.
 */
struct wlf_scene_node {
	const struct wlf_scene_node_impl *impl; /**< Implementation interface */

	struct wlf_scene_node *parent;          /**< Parent node in the hierarchy */
	struct wlf_window *window;              /**< Associated window */

	struct wlf_linked_list link;            /**< Link in scene tree children list */

	struct {
		struct wlf_signal destroy;         /**< Signal emitted when node is destroyed */
	} events;

	struct wlf_scene_node_state state;      /**< Current state of the node */
	void *data;                             /**< User data pointer */
	struct wlf_addon_set addons;            /**< Addon extensions */
};

/**
 * @brief Entry for rendering a scene node.
 *
 * Used during the rendering process to specify how a scene node should be rendered,
 * including its position and whether to highlight transparent regions.
 */
struct wlf_scene_node_render_entry {
	struct wlf_scene_node *node;             /**< Node to render */
	bool highlight_transparent_region;       /**< Whether to highlight transparent regions */
	double x, y;                             /**< Rendering position */
};

/**
 * @brief Places a scene node above another sibling.
 *
 * @param node Node to move.
 * @param sibling Sibling node to place above.
 */
void wlf_scene_node_place_above(struct wlf_scene_node *node,
	struct wlf_scene_node *sibling);

/**
 * @brief Places a scene node below another sibling.
 *
 * @param node Node to move.
 * @param sibling Sibling node to place below.
 */
void wlf_scene_node_place_below(struct wlf_scene_node *node,
	struct wlf_scene_node *sibling);

/**
 * @brief Raises a scene node to the top of its siblings.
 *
 * @param node Node to raise.
 */
void wlf_scene_node_raise_to_top(struct wlf_scene_node *node);

/**
 * @brief Lowers a scene node to the bottom of its siblings.
 *
 * @param node Node to lower.
 */
void wlf_scene_node_lower_to_bottom(struct wlf_scene_node *node);

/**
 * @brief Reparents a scene node.
 *
 * @param node Node to reparent.
 * @param new_parent New parent node.
 */
void wlf_scene_node_reparent(struct wlf_scene_node *node,
	struct wlf_scene_node *new_parent);

/**
 * @brief Initializes a scene node.
 *
 * @param node Node to initialize.
 * @param impl Implementation interface for the node.
 * @param parent Parent node in the hierarchy.
 */
void wlf_scene_node_init(struct wlf_scene_node *node,
	const struct wlf_scene_node_impl *impl, struct wlf_scene_node *parent);

/**
 * @brief Destroys a scene node.
 *
 * @param node Node to destroy.
 */
void wlf_scene_node_destroy(struct wlf_scene_node *node);

/**
 * @brief Sets whether a scene node is enabled.
 *
 * @param node Node to modify.
 * @param enabled Whether the node should be enabled.
 */
void wlf_scene_node_set_enabled(struct wlf_scene_node *node, bool enabled);

/**
 * @brief Sets the position of a scene node.
 *
 * @param node Node to modify.
 * @param x X coordinate relative to parent.
 * @param y Y coordinate relative to parent.
 */
void wlf_scene_node_set_position(struct wlf_scene_node *node, double x, double y);

/**
 * @brief Sets the opacity of a scene node.
 *
 * @param node Node to modify.
 * @param opacity Opacity level (0.0 to 1.0).
 */
void wlf_scene_node_set_opacity(struct wlf_scene_node *node,
	float opacity);

/**
 * @brief Gets the size of a scene node.
 *
 * @param node Node to query.
 * @param width Pointer to store width.
 * @param height Pointer to store height.
 */
void wlf_scene_node_get_size(struct wlf_scene_node *node,
	double *width, double *height);

/**
 * @brief Gets the children list of a scene node.
 *
 * @param node Node to query.
 * @return Linked list of child nodes.
 */
struct wlf_linked_list *wlf_scene_node_get_children(struct wlf_scene_node *node);

/**
 * @brief Gets the opaque region of a scene node.
 *
 * @param node Node to query.
 * @param x X offset.
 * @param y Y offset.
 * @param opaque Region to store opaque area.
 */
void wlf_scene_node_get_opaque_region(struct wlf_scene_node *node, double x,
	double y, struct wlf_region *opaque);

/**
 * @brief Checks if a scene node is invisible.
 *
 * @param node Node to check.
 * @return True if the node is invisible.
 */
bool wlf_scene_node_invisible(struct wlf_scene_node *node);

/**
 * @brief Sets the visible region of a scene node.
 *
 * @param node Node to modify.
 * @param visible Visible region.
 */
void wlf_scene_node_visibility(struct wlf_scene_node *node,
	struct wlf_region *visible);

/**
 * @brief Finds the scene node at a given position.
 *
 * @param node Root node to search from.
 * @param lx Local X coordinate.
 * @param ly Local Y coordinate.
 * @param nx Pointer to store node X coordinate.
 * @param ny Pointer to store node Y coordinate.
 * @return Node at the position, or NULL.
 */
struct wlf_scene_node *wlf_scene_node_at(struct wlf_scene_node *node,
	double lx, double ly, double *nx, double *ny);

/**
 * @brief Gets the coordinates of a scene node.
 *
 * @param node Node to query.
 * @param lx_ptr Pointer to store local X.
 * @param ly_ptr Pointer to store local Y.
 * @return True if coordinates are valid.
 */
bool wlf_scene_node_coords(struct wlf_scene_node *node,
	double *lx_ptr, double *ly_ptr);

/**
 * @brief Updates a scene node with damage region.
 *
 * @param node Node to update.
 * @param damage Damage region.
 */
void wlf_scene_node_update(struct wlf_scene_node *node, struct wlf_region *damage);

void wlf_scene_node_bounds(struct wlf_scene_node *node,
	int x, int y, struct wlf_region *visible);

#endif // SCENE_WLF_SCENE_NODE_H