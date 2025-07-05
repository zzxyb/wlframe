/**
 * @file        wlf_tree_item.h
 * @brief       Container item system for wlframe UI hierarchies.
 * @details     This file provides the container item structure and functionality for tree nodes.
 *              Tree items extend leaf items with child management capabilities, batch rendering,
 *              and advanced compositing features. They can contain multiple child items and
 *              provide hooks for controlling child rendering, layout, and lifecycle events.
 * @author      YaoBing Xiao
 * @date        2024-12-19
 * @version     v2.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 *      version: v2.0, YaoBing Xiao, 2024-12-19, refactored to new item-only architecture\n
 */

#ifndef WINDOW_WLF_TREE_ITEM_H
#define WINDOW_WLF_TREE_ITEM_H

#include "wlf/item/wlf_item.h"
#include <stddef.h>

struct wlf_item_tree;

/**
 * @brief Extended hook functions for container items.
 *
 * These hooks extend the basic item hooks with container-specific functionality
 * for managing children, batch rendering, and custom compositing operations.
 *
 * @note All hooks are optional and can be NULL.
 *
 * @code
 * wlf_item_tree_hooks hooks = {
 *     .base.on_paint = my_paint_function,
 *     .on_child_added = my_child_added_handler,
 *     .on_children_begin_render = my_batch_render_setup
 * };
 * wlf_item_tree_set_hooks(tree, &hooks);
 * @endcode
 */
struct wlf_item_tree_hooks {
    /** @brief Base item hooks inherited by container */
    struct wlf_item_impl base;

    /**
     * @brief Hook called before rendering all children.
     * @param tree The container item.
     * @param renderer The active renderer.
     * @param context The render context.
     */
    void (*on_children_begin_render)(struct wlf_item_tree *tree, struct wlf_renderer *renderer, struct wlf_render_context *context);

    /**
     * @brief Hook called after rendering all children.
     * @param tree The container item.
     * @param renderer The active renderer.
     * @param context The render context.
     */
    void (*on_children_end_render)(struct wlf_item_tree *tree, struct wlf_renderer *renderer, struct wlf_render_context *context);

    /**
     * @brief Hook called when rendering each child item.
     * @param tree The container item.
     * @param child The child being rendered.
     * @param renderer The active renderer.
     * @param damage The damaged region for the child.
     * @param context The render context.
     */
    void (*on_child_paint)(struct wlf_item_tree *tree, struct wlf_item *child, struct wlf_renderer *renderer,
                          struct wlf_rect *damage, struct wlf_render_context *context);

    /**
     * @brief Hook to determine if a child should render to FBO.
     * @param tree The container item.
     * @param child The child item.
     * @param context The render context.
     * @return true if child should render to FBO, false otherwise.
     */
    bool (*should_render_to_fbo)(struct wlf_item_tree *tree, struct wlf_item *child, struct wlf_render_context *context);

    /**
     * @brief Hook for custom compositing of children.
     * @param tree The container item.
     * @param renderer The active renderer.
     * @param children_fbo The framebuffer containing rendered children.
     * @param context The render context.
     */
    void (*on_composite_children)(struct wlf_item_tree *tree, struct wlf_renderer *renderer,
                                 struct wlf_framebuffer *children_fbo, struct wlf_render_context *context);

    /**
     * @brief Hook called when a child is added to the container.
     * @param tree The container item.
     * @param child The child that was added.
     */
    void (*on_child_added)(struct wlf_item_tree *tree, struct wlf_item *child);

    /**
     * @brief Hook called when a child is removed from the container.
     * @param tree The container item.
     * @param child The child that was removed.
     */
    void (*on_child_removed)(struct wlf_item_tree *tree, struct wlf_item *child);
};

/**
 * @brief Container item structure.
 *
 * Represents a container node in the UI hierarchy. Container items extend
 * leaf items with the ability to contain and manage child items. They provide
 * advanced rendering features including batch rendering, offscreen composition,
 * and custom child management.
 *
 * @note Container items must be created using wlf_item_tree_create() and
 *       destroyed using wlf_item_tree_destroy().
 *
 * @code
 * struct wlf_item_tree *tree = wlf_item_tree_create(window);
 * struct wlf_item *child1 = wlf_item_create(window);
 * struct wlf_item *child2 = wlf_item_create(window);
 * wlf_item_tree_add_child(tree, child1);
 * wlf_item_tree_add_child(tree, child2);
 * @endcode
 */
struct wlf_item_tree {
    struct wlf_item base;

    struct wlf_item **children;             /**< Array of child item pointers */
    size_t children_count;                  /**< Number of children */
    size_t children_capacity;               /**< Allocated capacity for children array */

    struct wlf_framebuffer *children_fbo;   /**< Shared FBO for batch rendering children */
    bool use_children_fbo;                  /**< Whether to use batch FBO rendering */
    bool children_fbo_dirty;                /**< Whether children FBO needs update */
    struct wlf_rect children_bounds;        /**< Bounding rectangle of all children */

    bool force_children_to_fbo;             /**< Force all children to render to FBO */
    bool custom_composite;                  /**< Use custom compositing for children */

    struct wlf_item_tree_hooks tree_hooks;       /**< Container-specific hook functions */
};

/**
 * @brief Create a new container item.
 *
 * Creates a new container item that can hold child items. The container
 * inherits all properties of a leaf item but adds child management capabilities.
 *
 * @param window The window to associate the container with.
 * @return Newly created container pointer, or NULL on failure.
 *
 * @note The returned container must be destroyed using wlf_item_tree_destroy().
 */
struct wlf_item_tree* wlf_item_tree_create(struct wlf_window *window);

/**
 * @brief Destroy a container item and all its children.
 *
 * Destroys the container item, all its child items, and frees all
 * associated resources including any batch rendering framebuffers.
 *
 * @param tree Container to destroy. Can be NULL (no-op).
 */
void wlf_item_tree_destroy(struct wlf_item_tree *tree);

/* ===== Child Management Functions ===== */

/**
 * @brief Add a child item to a container.
 *
 * Adds a child item to the container. The child will be positioned
 * relative to the container and will inherit the container's transformation
 * and opacity settings.
 *
 * @param parent Container to add the child to.
 * @param child Child item to add.
 *
 * @note The child's parent pointer will be updated to point to the container.
 * @note If the child already has a parent, it will be removed from the old parent first.
 */
void wlf_item_tree_add_child(struct wlf_item_tree *parent, struct wlf_item *child);

/**
 * @brief Remove a child item from a container.
 *
 * Removes a child item from the container. The child will no longer
 * participate in the container's rendering or layout operations.
 *
 * @param parent Container to remove the child from.
 * @param child Child item to remove.
 *
 * @note The child's parent pointer will be set to NULL.
 * @note The child item itself is not destroyed, only removed from the container.
 */
void wlf_item_tree_remove_child(struct wlf_item_tree *parent, struct wlf_item *child);

/**
 * @brief Get the number of children in a container.
 *
 * @param tree Container to query.
 * @return Number of child items in the container.
 */
size_t wlf_item_tree_get_children_count(struct wlf_item_tree *tree);

/**
 * @brief Get a child item by index.
 *
 * @param tree Container to query.
 * @param index Index of the child (0-based).
 * @return Child item at the specified index, or NULL if index is out of bounds.
 */
struct wlf_item* wlf_item_tree_get_child(struct wlf_item_tree *tree, size_t index);

/* ===== Hook Management Functions ===== */

/**
 * @brief Set container-specific hook functions.
 *
 * Installs custom hook functions for the container. This includes both
 * the base item hooks and container-specific hooks for child management.
 *
 * @param tree Container to modify.
 * @param hooks Hook function structure. Can contain NULL function pointers.
 */
void wlf_item_tree_set_hooks(struct wlf_item_tree *tree, struct wlf_item_tree_hooks *hooks);

/* ===== Type Conversion Functions ===== */

/**
 * @brief Convert an item to a container if it's a tree type.
 *
 * Safely converts a generic item pointer to a container pointer,
 * checking the type first.
 *
 * @param item Item to convert.
 * @return Container pointer if item is a tree type, NULL otherwise.
 */
struct wlf_item_tree* wlf_item_to_tree(struct wlf_item *item);

/**
 * @brief Convert a container to its base item.
 *
 * Returns the base item portion of a container, allowing it to be
 * used in functions that expect a generic item pointer.
 *
 * @param tree Container to convert.
 * @return Base item pointer.
 */
struct wlf_item* wlf_item_tree_to_item(struct wlf_item_tree *tree);

/* ===== Batch Rendering Functions ===== */

/**
 * @brief Enable or disable batch FBO rendering for children.
 *
 * When enabled, all children will be rendered to a shared framebuffer
 * first, then composited to the final target. This is useful for
 * applying effects to multiple children or optimizing rendering performance.
 *
 * @param tree Container to modify.
 * @param enable true to enable batch FBO rendering, false to disable.
 */
void wlf_item_tree_enable_children_fbo(struct wlf_item_tree *tree, bool enable);

/**
 * @brief Mark children FBO as needing update.
 *
 * Marks the shared children framebuffer as dirty, forcing it to be
 * re-rendered on the next frame.
 *
 * @param tree Container to mark as dirty.
 */
void wlf_item_tree_mark_children_dirty(struct wlf_item_tree *tree);

/**
 * @brief Update the bounding rectangle of all children.
 *
 * Recalculates the bounding rectangle that encompasses all child items.
 * This is used for optimizing rendering and determining FBO sizes.
 *
 * @param tree Container to update.
 */
void wlf_item_tree_update_children_bounds(struct wlf_item_tree *tree);

/* ===== Rendering Control Functions ===== */

/**
 * @brief Force all children to render to FBO.
 *
 * When enabled, all children will be forced to use offscreen rendering
 * regardless of their individual offscreen settings.
 *
 * @param tree Container to modify.
 * @param force true to force FBO rendering, false to use individual settings.
 */
void wlf_item_tree_set_force_children_to_fbo(struct wlf_item_tree *tree, bool force);

/**
 * @brief Enable custom compositing mode.
 *
 * When enabled, the container will use custom compositing hooks instead
 * of the default blending operations for combining child renderings.
 *
 * @param tree Container to modify.
 * @param custom true to enable custom compositing, false for default.
 */
void wlf_item_tree_set_custom_composite(struct wlf_item_tree *tree, bool custom);

/* ===== Layout Functions ===== */

/**
 * @brief Recursively layout all children.
 *
 * Triggers layout operations for the container and all its children,
 * calculating positions and sizes based on the available space.
 *
 * @param tree Container to layout.
 * @param available Available space for layout.
 */
void wlf_item_tree_layout_children(struct wlf_item_tree *tree, struct wlf_rect *available);

/* ===== Hit Testing Functions ===== */

/**
 * @brief Find the topmost child item at a given point.
 *
 * Performs hit testing to find which child item (if any) is at the
 * specified coordinates. Searches from top to bottom in z-order.
 *
 * @param tree Container to search.
 * @param x X coordinate relative to container.
 * @param y Y coordinate relative to container.
 * @return Child item at the point, or NULL if no child is at that location.
 */
struct wlf_item* wlf_item_tree_hit_test(struct wlf_item_tree *tree, int x, int y);

#endif /* WINDOW_WLF_TREE_ITEM_H */
