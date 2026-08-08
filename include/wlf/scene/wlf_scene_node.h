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

#include "wlf/math/wlf_frect.h"
#include "wlf/utils/wlf_addon.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_array.h"
#include "wlf/utils/wlf_signal.h"

#include <stdbool.h>
#include <pixman.h>

struct wlf_window;
struct wlf_scene;
struct wlf_scene_node;
struct wlf_render_target_info;
struct wlf_render_list_entry;
struct wlf_render_data;
struct wlf_render_list_constructor_data;

/**
 * @brief Callback function type for scene node bounding box iterations.
 *
 * This iterator is invoked for each scene node or sub-element that intersects with
 * a requested bounding box. It provides the matched node along with its origin coordinates
 * transformed into the query's coordinate space, allowing the caller to calculate local offsets.
 *
 * @param node     The scene node currently being intersected and evaluated.
 * @param sx       The X coordinate of the matched node's origin in the query box space.
 * @param sy       The Y coordinate of the matched node's origin in the query box space.
 * @param data     User-defined context data (e.g., a pointer to `struct wlf_node_at_data`)
 *                 passed through from the initial `in_box` call.
 * @return True to continue the traversal/iteration; false to abort the traversal immediately
 *         (e.g., when the desired node has been found during a hit-test).
 */
typedef bool (*scene_node_box_iterator_func_t)(struct wlf_scene_node *node,
	int sx, int sy, void *data);


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
	void (*set_position)(struct wlf_scene_node *node, int x, int y);

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
	void (*get_size)(struct wlf_scene_node *node,
		uint32_t *width, uint32_t *height);

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
	void (*opaque_region)(struct wlf_scene_node *node, int x, int y,
		pixman_region32_t *opaque);

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
		pixman_region32_t *visible);

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
	bool (*coords)(struct wlf_scene_node *node, int *lx_ptr, int *ly_ptr);

	/**
	 * @brief Updates the scene node with damage region
	 * @param node The scene node
	 * @param damage Damage region
	 */
	void (*update)(struct wlf_scene_node *node, pixman_region32_t *damage);

	/**
	 * @brief Calculates and accumulates the bounding region of a scene node.
	 * @param node Pointer to the scene node to process.
	 * @param x The X coordinate of the node's top-left corner in the
	 * current coordinate space.
	 * @param y The Y coordinate of the node's top-left corner in the
	 * current coordinate space.
	 * @param visible Pointer to an existing region structure.
	 */
	void (*bounds)(struct wlf_scene_node *node,
		int x, int y, pixman_region32_t *visible);

	/**
	 * @brief Iterates over child nodes or sub-elements intersecting a bounding box.
	 * @param node      The parent scene node containing the elements to query.
	 * @param box       The bounding box area to check against, defined in the node's coordinate space.
	 * @param iterator  Callback function invoked for each intersecting element. 
	 *                  The callback receives the element's origin coordinates ($lx, ly$) within the box space.
	 *                  Returning false from the iterator aborts the traversal immediately.
	 * @param user_data User-defined context data (e.g., struct wlf_node_at_data) passed directly to the callback.
	 * @return True if the traversal completed entirely or no elements intersected; false if aborted early by the iterator.
	 */
	bool (*in_box)(struct wlf_scene_node *node, struct wlf_frect *box,
		scene_node_box_iterator_func_t iterator, void *user_data);

	/** Adds this node to a render list when it is eligible for rendering. */
	bool (*construct_render_list_iterator)(struct wlf_scene_node *node,
		int lx, int ly, void *data);

	/**
	 * Submits this leaf node's rendering commands.
	 * @param entry Render-list entry containing stable scene coordinates.
	 * @param data Per-frame scene render context.
	 */
	void (*render)(struct wlf_render_list_entry *entry,
		const struct wlf_render_data *data);
};

/**
 * @brief State information for a scene node.
 *
 * Contains the current state of a scene node including position, size, opacity,
 * and various regions for rendering and input handling.
 */
struct wlf_scene_node_state {
	bool enabled;                           /**< Whether the node is enabled */
	int x, y;                               /**< Position relative to parent */
	uint32_t width, height;                 /**< Non-negative logical size */
	float opacity;                          /**< Opacity level (0.0 to 1.0) */

	enum wlf_focus_policy focus_policy;     /**< Focus policy for the node */
	pixman_region32_t visible;              /**< Visible region */
	pixman_region32_t transparent_region;   /**< Transparent region */
	pixman_region32_t input_passthrough_region; /**< Input passthrough region */
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
	struct wlf_scene *scene;                /**< Owning scene, may be NULL */
	struct wlf_window *window;              /**< Associated window */

	struct wlf_linked_list link;            /**< Link in scene tree children list */

	void *data;                             /**< User data pointer */
	struct wlf_addon_set addons;            /**< Addon extensions */

	struct wlf_scene_node_state state;      /**< Current state of the node */
	struct {
		struct wlf_signal destroy;         /**< Signal emitted when node is destroyed */
	} events;
};

/**
 * @brief Entry for rendering a scene node.
 *
 * Used during the rendering process to specify how a scene node should be rendered,
 * including its position and whether to highlight transparent regions.
 */
struct wlf_render_list_entry {
	struct wlf_scene_node *node;             /**< Node to render */
	bool highlight_transparent_region;       /**< Whether to highlight transparent regions */
	int x, y;                                /**< Logical rendering position */
};

/** Data used while scene nodes construct a render list. */
struct wlf_render_list_constructor_data {
	struct wlf_frect box;       /**< Logical area being rendered */
	struct wlf_array *render_list; /**< Array of wlf_render_list_entry */
	bool calculate_visibility;  /**< Whether opaque occlusion is enabled */
	bool highlight_transparent_region; /**< Highlight translucent regions */
	bool failed;                /**< Set when growing render_list fails */
};

/** Per-frame context passed to scene-node render callbacks. */
struct wlf_render_data {
	struct wlf_scene *scene;              /**< Scene being rendered */
	struct wlf_render_target_info *target; /**< Active render target */
	struct wlf_frect logical;             /**< Logical render area */
	pixman_region32_t damage;             /**< Buffer damage to render */
};

/**
 * @brief Data structure for node querying or coordinate transformation at a specific point.
 *
 * Used to store localized coordinates, relative coordinates, and the target scene node,
 * typically during hit-testing or input routing (e.g., finding which node is at a given coordinate).
 */
struct wlf_node_at_data {
	double lx, ly;               /**< Local coordinates relative to the node */
	double rx, ry;               /**< Relative or requested input coordinates */
	struct wlf_scene_node *node; /**< The target scene node found at the coordinates */
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
void wlf_scene_node_set_position(struct wlf_scene_node *node, int x, int y);

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
	uint32_t *width, uint32_t *height);

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
void wlf_scene_node_opaque_region(struct wlf_scene_node *node, int x,
	int y, pixman_region32_t *opaque);

/** Compatibility name for wlf_scene_node_opaque_region(). */
void wlf_scene_node_get_opaque_region(struct wlf_scene_node *node, int x,
	int y, pixman_region32_t *opaque);

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
	pixman_region32_t *visible);

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
	int *lx_ptr, int *ly_ptr);

/**
 * @brief Updates a scene node with damage region.
 *
 * @param node Node to update.
 * @param damage Damage region.
 */
void wlf_scene_node_update(struct wlf_scene_node *node, pixman_region32_t *damage);

/**
 * @brief Calculates and accumulates the bounding region of a scene node.
 *
 * Computes the bounding rectangle for the given @p node at the specified
 * coordinates (@p x, @p y) and merges it into the @p visible region using
 * a union operation. If a custom bounds implementation is provided via
 * @c node->impl->bounds, it is invoked directly. Otherwise, the default
 * fallback logic is used: the function verifies that the node is enabled,
 * retrieves its dimensions, constructs a rectangle at the given position,
 * and unions it with the existing @p visible region.
 *
 * @param node     Pointer to the scene node to process.
 * @param x        The X coordinate of the node's top-left corner in the
 *                 current coordinate space.
 * @param y        The Y coordinate of the node's top-left corner in the
 *                 current coordinate space.
 * @param visible  Pointer to an existing region structure. The computed
 *                 bounds will be unioned into this region in-place. If
 *                 the node is disabled and lacks a custom implementation,
 *                 this region remains unmodified.
 */
void wlf_scene_node_bounds(struct wlf_scene_node *node,
	int x, int y, pixman_region32_t *visible);

/**
 * @brief Traverses and evaluates scene graph elements within a specified bounding box.
 *
 * Dispatches a bounding box query onto the given @p node. It searches for child nodes
 * or visual regions intersecting @p box. When a match is found, @p iterator is executed
 * with the matched element's origin ($lx, ly$) inside the query coordinate system, enabling
 * downstream logic to calculate local relative positions (e.g., $rx = target\_x - lx$).
 *
 * @param node      Pointer to the scene node serving as the root of this box query.
 * @param box       Pointer to the rectangle defining the geometric search boundaries.
 * @param iterator  The callback function triggered for every intersecting child or region.
 * @param user_data Opaque pointer passed to the iterator for state tracking or data collection.
 * @return True if the traversal successfully finished without interruption; 
 *         false if a callback returned false to prematurely stop the search (e.g., upon a successful hit-test).
 */
bool wlf_scene_node_nodes_in_box(struct wlf_scene_node *node,
	struct wlf_frect *box,
	scene_node_box_iterator_func_t iterator, void *user_data);

/** Compatibility name for wlf_scene_node_nodes_in_box(). */
bool wlf_scene_node_in_box(struct wlf_scene_node *node, struct wlf_frect *box,
	scene_node_box_iterator_func_t iterator, void *user_data);

bool wlf_scene_node_construct_render_list_iterator(
	struct wlf_scene_node *node, int lx, int ly, void *data);

/** Adds an eligible node to the render list in data. */
bool wlf_scene_node_add_render_list_entry(struct wlf_scene_node *node,
	int lx, int ly, void *data);

/** Initializes render_region with the node-visible portion of frame damage. */
bool wlf_scene_node_init_render_region(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data, pixman_region32_t *render_region);

/** Dispatches a render-list entry through its node implementation. */
void wlf_scene_node_render(struct wlf_render_list_entry *entry,
	const struct wlf_render_data *data);

#endif // SCENE_WLF_SCENE_NODE_H
