/**
 * @file        wlf_item.h
 * @brief       Leaf item system for wlframe UI components.
 * @details     This file provides the basic item structure and functionality for leaf nodes.
 *              Leaf items are lightweight UI components suitable for buttons, text, images, etc.
 *              They cannot contain child elements but provide rendering hooks, input handling,
 *              and basic properties for positioning, visibility, and offscreen rendering.
 * @author      YaoBing Xiao
 * @date        2024-12-19
 * @version     v2.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 *      version: v2.0, YaoBing Xiao, 2024-12-19, refactored to new item-only architecture\n
 */

#ifndef ITEM_WLF_ITEM_H
#define ITEM_WLF_ITEM_H

#include "wlf/math/wlf_rect.h"
#include "wlf/math/wlf_region.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct wlf_item;
struct wlf_window;
struct wlf_renderer;
struct wlf_framebuffer;

/**
 * @brief Item type enumeration.
 *
 * Defines whether an item is a leaf node or container node.
 */
enum wlf_item_type{
	WLF_ITEM_TYPE_LEAF,		/**< Leaf item (wlf_item) */
	WLF_ITEM_TYPE_TREE		/**< Container item (wlf_item_tree) */
};

/**
 * @brief Render target type enumeration.
 *
 * Specifies where the item should be rendered to.
 */
enum wlf_render_target_type{
	WLF_RENDER_TARGET_WINDOW,		/**< Render directly to window */
	WLF_RENDER_TARGET_FBO			/**< Render to offscreen framebuffer */
};

/**
 * @brief Render context information passed to hooks.
 *
 * Contains all necessary information for rendering operations,
 * including target type, viewport, transformation matrices, and rendering hints.
 */
struct wlf_render_context{
	enum wlf_render_target_type target_type;	/**< Target type */
	union {
		struct {
			struct wlf_window *window;		/**< Target window */
		} window;
		struct {
			struct wlf_framebuffer *fbo;	/**< Target framebuffer */
			bool is_children_batch;			/**< Whether this is batch rendering for children */
			void *batch_container; /**< Container for batch rendering */
		} fbo;
	} target;

	struct wlf_rect viewport;				/**< Viewport rectangle */
	float opacity_factor;					/**< Opacity multiplication factor */
	bool allow_caching;						/**< Whether caching is allowed */
	bool requires_alpha_blending;			/**< Whether alpha blending is required */
	float transform_matrix[16];				/**< 4x4 transformation matrix */
};

/**
 * @brief Item hook functions for leaf items.
 *
 * These hooks allow customization of rendering, layout, input handling,
 * and lifecycle events for leaf items.
 *
 * @note All hooks are optional and can be NULL.
 *
 * @code
 * wlf_item_impl hooks = {
 *     .on_paint = my_paint_function,
 *     .on_layout = my_layout_function,
 *     .on_mouse_event = my_mouse_handler
 * };
 * wlf_item_set_hooks(item, &hooks);
 * @endcode
 */
struct wlf_item_impl {
	/**
	 * @brief Rendering hook called during paint operations.
	 * @param item The item being rendered.
	 * @param renderer The active renderer.
	 * @param damage The damaged region to be repainted.
	 * @param context The render context information.
	 */
	void (*on_paint)(struct wlf_item *item, struct wlf_renderer *renderer,
					 struct wlf_rect *damage, struct wlf_render_context *context);

	/**
	 * @brief Layout hook called when item needs to be positioned.
	 * @param item The item being laid out.
	 * @param available The available space for layout.
	 */
	void (*on_layout)(struct wlf_item *item, struct wlf_rect *available);

	/**
	 * @brief Mouse event hook for handling mouse button events.
	 * @param item The item receiving the event.
	 * @param button Mouse button code.
	 * @param action Action type (press, release, etc.).
	 * @param x X coordinate relative to item.
	 * @param y Y coordinate relative to item.
	 * @return true if event was handled, false to propagate.
	 */
	bool (*on_mouse_event)(struct wlf_item *item, int button, int action, int x, int y);

	/**
	 * @brief Keyboard event hook for handling key events.
	 * @param item The item receiving the event.
	 * @param key Key code.
	 * @param action Action type (press, release, repeat).
	 * @param mods Modifier keys.
	 * @return true if event was handled, false to propagate.
	 */
	bool (*on_key_event)(struct wlf_item *item, int key, int action, int mods);

	/**
	 * @brief Mouse movement hook for handling mouse motion.
	 * @param item The item receiving the event.
	 * @param x X coordinate relative to item.
	 * @param y Y coordinate relative to item.
	 * @return true if event was handled, false to propagate.
	 */
	bool (*on_mouse_move)(struct wlf_item *item, int x, int y);

	/**
	 * @brief Lifecycle hook called when item is added to a parent.
	 * @param item The item being added.
	 * @param parent The new parent container.
	 */
	void (*on_parent_added)(struct wlf_item *item, void *parent);

	/**
	 * @brief Lifecycle hook called when item is removed from parent.
	 * @param item The item being removed.
	 * @param parent The previous parent container.
	 */
	void (*on_parent_removed)(struct wlf_item *item, void *parent);
};

/**
 * @brief Leaf item structure.
 *
 * Represents a leaf node in the UI hierarchy. Leaf items are lightweight
 * components that cannot contain children but provide full rendering,
 * input handling, and positioning capabilities.
 *
 * @note Items must be created using wlf_item_create() and destroyed using wlf_item_destroy().
 *
 * @code
 * struct wlf_item *item = wlf_item_create(window);
 * wlf_item_set_geometry(item, &(struct wlf_rect){10, 10, 100, 50});
 * wlf_item_set_visible(item, true);
 * @endcode
 */
struct wlf_item {
	/* Basic properties */
	uint32_t id;						/**< Unique identifier */
	enum wlf_item_type type;			/**< Item type (always WLF_ITEM_TYPE_LEAF) */
	struct wlf_rect geometry;			/**< Position and size relative to parent */
	struct wlf_rect content_rect;		/**< Content area (excluding margins) */

	/* Visibility and interaction */
	bool visible;						/**< Whether item is visible */
	bool enabled;						/**< Whether item accepts input */
	float opacity;						/**< Opacity factor (0.0-1.0) */

	/* Region control */
	struct wlf_region *transparent_region;	/**< Transparent regions for hit testing */
	struct wlf_region *input_region;		/**< Input-sensitive regions */
	struct wlf_region *damage_region;		/**< Damaged regions needing repaint */

	/* Hierarchy relationships */
	void *parent;						/**< Parent container (NULL for root items) */
	int z_order;						/**< Z-axis ordering within parent */

	/* Offscreen rendering */
	struct wlf_framebuffer *offscreen_buffer;	/**< Private offscreen buffer */
	bool use_offscreen;					/**< Whether to use offscreen rendering */
	bool buffer_dirty;					/**< Whether offscreen buffer needs update */

	/* Event and rendering hooks */
	struct wlf_item_impl hooks;			/**< Custom hook functions */
	void *user_data;					/**< User-defined data pointer */

	/* Window association */
	struct wlf_window *window;			/**< Associated window */
};

/**
 * @brief Create a new leaf item.
 *
 * Creates a new leaf item associated with the specified window.
 * The item is initially invisible and positioned at (0,0) with size (0,0).
 *
 * @param window The window to associate the item with.
 * @return Newly created item pointer, or NULL on failure.
 *
 * @note The returned item must be destroyed using wlf_item_destroy().
 */
struct wlf_item* wlf_item_create(struct wlf_window *window);

/**
 * @brief Destroy an item and free its resources.
 *
 * Destroys the item, frees all associated resources, and removes it
 * from its parent if it has one.
 *
 * @param item Item to destroy. Can be NULL (no-op).
 */
void wlf_item_destroy(struct wlf_item *item);

/**
 * @brief Set item geometry (position and size).
 *
 * Sets the item's position and size relative to its parent.
 * This will trigger a layout update and potential repaint.
 *
 * @param item Item to modify.
 * @param rect New geometry rectangle.
 */
void wlf_item_set_geometry(struct wlf_item *item, struct wlf_rect *rect);

/**
 * @brief Set item visibility.
 *
 * Controls whether the item is visible and participates in rendering.
 * Invisible items do not receive input events.
 *
 * @param item Item to modify.
 * @param visible true to make visible, false to hide.
 */
void wlf_item_set_visible(struct wlf_item *item, bool visible);

/**
 * @brief Set item opacity.
 *
 * Sets the item's opacity factor. This is multiplied with the parent's
 * opacity to determine the final rendering opacity.
 *
 * @param item Item to modify.
 * @param opacity Opacity value (0.0 = transparent, 1.0 = opaque).
 */
void wlf_item_set_opacity(struct wlf_item *item, float opacity);

/**
 * @brief Set item hook functions.
 *
 * Installs custom hook functions for the item. All hooks are optional
 * and can be NULL to use default behavior.
 *
 * @param item Item to modify.
 * @param hooks Hook function structure. Can contain NULL function pointers.
 */
void wlf_item_set_hooks(struct wlf_item *item, struct wlf_item_impl *hooks);

/**
 * @brief Check if an item is a container type.
 *
 * @param item Item to check.
 * @return true if item is a container (wlf_item_tree), false if leaf item.
 */
bool wlf_item_is_tree(struct wlf_item *item);

/**
 * @brief Create a new region.
 *
 * Creates an empty region that can be used for transparent areas,
 * input regions, or damage tracking.
 *
 * @return New region pointer, or NULL on failure.
 *
 * @note The returned region must be destroyed using wlf_region_destroy().
 */
struct wlf_region* wlf_region_create(void);

/**
 * @brief Destroy a region and free its resources.
 *
 * @param region Region to destroy. Can be NULL (no-op).
 */
void wlf_region_destroy(struct wlf_region *region);

/**
 * @brief Add a rectangle to a region.
 *
 * @param region Region to modify.
 * @param rect Rectangle to add to the region.
 */
void wlf_region_add_item_rect(struct wlf_region *region, struct wlf_rect *rect);

/**
 * @brief Check if a region contains a point.
 *
 * @param region Region to test.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return true if point is within the region, false otherwise.
 */
bool wlf_region_contains_item_point(struct wlf_region *region, int x, int y);

/**
 * @brief Enable or disable offscreen rendering for an item.
 *
 * When enabled, the item will be rendered to a private framebuffer
 * first, then composited to the final target. This is useful for
 * complex items that need special effects or caching.
 *
 * @param item Item to modify.
 * @param enable true to enable offscreen rendering, false to disable.
 */
void wlf_item_enable_offscreen(struct wlf_item *item, bool enable);

/**
 * @brief Mark an item as needing repaint.
 *
 * Marks the item (or a specific region of it) as damaged and needing
 * a repaint operation. This will trigger rendering on the next frame.
 *
 * @param item Item to mark as dirty.
 * @param damage Damaged region, or NULL to mark entire item as dirty.
 */
void wlf_item_mark_dirty(struct wlf_item *item, struct wlf_rect *damage);

/**
 * @brief Recursively render an item.
 *
 * Renders the item using its associated hooks and render context.
 * For leaf items, this calls the on_paint hook if available.
 *
 * @param item Item to render.
 * @param renderer Active renderer instance.
 * @param clip Clipping rectangle for rendering optimization.
 */
void wlf_item_render_recursive(struct wlf_item *item, struct wlf_renderer *renderer,
	struct wlf_rect *clip);

/**
 * @brief Initialize a render context for window rendering.
 *
 * Sets up a render context for rendering directly to a window.
 *
 * @param ctx Context structure to initialize.
 * @param window Target window.
 * @param viewport Viewport rectangle.
 * @param opacity Opacity multiplication factor.
 */
void wlf_render_context_init_window(struct wlf_render_context *ctx,
	struct wlf_window *window, struct wlf_rect *viewport, float opacity);

/**
 * @brief Initialize a render context for FBO rendering.
 *
 * Sets up a render context for rendering to a framebuffer object.
 *
 * @param ctx Context structure to initialize.
 * @param fbo Target framebuffer.
 * @param viewport Viewport rectangle.
 * @param opacity Opacity multiplication factor.
 * @param is_batch Whether this is batch rendering for multiple items.
 * @param container Container item for batch rendering (can be NULL).
 */
void wlf_render_context_init_fbo(struct wlf_render_context *ctx,
	struct wlf_framebuffer *fbo, struct wlf_rect *viewport,
	float opacity, bool is_batch, void *container);

#endif // ITEM_WLF_ITEM_H
