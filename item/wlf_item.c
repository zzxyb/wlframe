#include "wlf/item/wlf_item.h"
#include "wlf/item/wlf_tree_item.h"
#include "wlf/framebuffer/wlf_framebuffer.h"

#include <stdlib.h>
#include <string.h>

static void wlf_item_init_base(struct wlf_item *item, struct wlf_window *window, enum wlf_item_type type) {
	static uint32_t next_id = 1;

	memset(item, 0, sizeof(struct wlf_item));

	item->id = next_id++;
	item->type = type;
	item->window = window;
	item->visible = true;
	item->enabled = true;
	item->opacity = 1.0f;
	item->z_order = 0;
	item->use_offscreen = false;
	item->buffer_dirty = true;
}

static void wlf_item_cleanup_base(struct wlf_item *item) {
	if (!item) return;

	if (item->transparent_region) {
		wlf_region_destroy(item->transparent_region);
	}
	if (item->input_region) {
		wlf_region_destroy(item->input_region);
	}
	if (item->damage_region) {
		wlf_region_destroy(item->damage_region);
	}

	if (item->offscreen_buffer) {
		item->offscreen_buffer = NULL;
	}
}

struct wlf_item* wlf_item_create(struct wlf_window *window) {
	if (!window) return NULL;

	struct wlf_item *item = malloc(sizeof(struct wlf_item));
	if (!item) return NULL;

	wlf_item_init_base(item, window, WLF_ITEM_TYPE_LEAF);

	return item;
}

void wlf_item_destroy(struct wlf_item *item) {
	if (!item) return;

	if (item->parent) {
		wlf_item_tree_remove_child(item->parent, item);
	}

	if (item->hooks.on_parent_removed && item->parent) {
		item->hooks.on_parent_removed(item, item->parent);
	}

	wlf_item_cleanup_base(item);
	free(item);
}

void wlf_item_set_geometry(struct wlf_item *item, struct wlf_rect *rect) {
	if (!item || !rect) return;

	item->geometry = *rect;
	item->content_rect = *rect;

	wlf_item_mark_dirty(item, NULL);
}

void wlf_item_set_visible(struct wlf_item *item, bool visible) {
	if (!item || item->visible == visible) return;

	item->visible = visible;
	wlf_item_mark_dirty(item, NULL);
}

void wlf_item_set_opacity(struct wlf_item *item, float opacity) {
	if (!item) return;

	if (opacity < 0.0f) opacity = 0.0f;
	if (opacity > 1.0f) opacity = 1.0f;

	if (item->opacity != opacity) {
		item->opacity = opacity;
		wlf_item_mark_dirty(item, NULL);
	}
}

void wlf_item_set_hooks(struct wlf_item *item, struct wlf_item_impl *hooks) {
	if (!item || !hooks) return;

	item->hooks = *hooks;
}

struct wlf_item_tree* wlf_item_tree_create(struct wlf_window *window) {
	if (!window) return NULL;

	struct wlf_item_tree *tree = malloc(sizeof(struct wlf_item_tree));
	if (!tree) return NULL;

	memset(tree, 0, sizeof(struct wlf_item_tree));

	wlf_item_init_base(&tree->base, window, WLF_ITEM_TYPE_TREE);

	tree->children = NULL;
	tree->children_count = 0;
	tree->children_capacity = 0;
	tree->use_children_fbo = false;
	tree->children_fbo_dirty = true;
	tree->force_children_to_fbo = false;
	tree->custom_composite = false;
	tree->children_fbo = NULL;

	return tree;
}

void wlf_item_tree_destroy(struct wlf_item_tree *tree) {
	if (!tree) return;

	for (size_t i = 0; i < tree->children_count; i++) {
		struct wlf_item *child = tree->children[i];
		child->parent = NULL;
		wlf_item_destroy(child);
	}

	free(tree->children);

	if (tree->children_fbo) {
		tree->children_fbo = NULL;
	}

	wlf_item_cleanup_base(&tree->base);

	free(tree);
}

void wlf_item_tree_add_child(struct wlf_item_tree *parent, struct wlf_item *child) {
	if (!parent || !child) return;

	if (parent->children_count >= parent->children_capacity) {
		size_t new_capacity = parent->children_capacity ? parent->children_capacity * 2 : 4;
		parent->children = realloc(parent->children, new_capacity * sizeof(struct wlf_item*));
		if (!parent->children) return;
		parent->children_capacity = new_capacity;
	}

	if (child->parent) {
		wlf_item_tree_remove_child(child->parent, child);
	}

	parent->children[parent->children_count++] = child;
	child->parent = parent;

	if (parent->tree_hooks.on_child_added) {
		parent->tree_hooks.on_child_added(parent, child);
	}

	if (child->hooks.on_parent_added) {
		child->hooks.on_parent_added(child, parent);
	}

	if (parent->use_children_fbo) {
		wlf_item_tree_mark_children_dirty(parent);
	}

	wlf_item_mark_dirty(&parent->base, NULL);
}

void wlf_item_tree_remove_child(struct wlf_item_tree *parent, struct wlf_item *child) {
	if (!parent || !child) return;

	for (size_t i = 0; i < parent->children_count; i++) {
		if (parent->children[i] == child) {
			if (child->hooks.on_parent_removed) {
				child->hooks.on_parent_removed(child, parent);
			}

			if (parent->tree_hooks.on_child_removed) {
				parent->tree_hooks.on_child_removed(parent, child);
			}

			memmove(&parent->children[i], &parent->children[i + 1],
				   (parent->children_count - i - 1) * sizeof(struct wlf_item*));
			parent->children_count--;
			child->parent = NULL;

			if (parent->use_children_fbo) {
				wlf_item_tree_mark_children_dirty(parent);
			}

			wlf_item_mark_dirty(&parent->base, NULL);
			break;
		}
	}
}

void wlf_item_tree_set_hooks(struct wlf_item_tree *tree, struct wlf_item_tree_hooks *hooks) {
	if (!tree || !hooks) return;

	tree->tree_hooks = *hooks;
}

bool wlf_item_is_tree(struct wlf_item *item) {
	return item && item->type == WLF_ITEM_TYPE_TREE;
}

struct wlf_item_tree* wlf_item_to_tree(struct wlf_item *item) {
	if (wlf_item_is_tree(item)) {
		return (struct wlf_item_tree*)item;
	}
	return NULL;
}

struct wlf_item* wlf_item_tree_to_item(struct wlf_item_tree *tree) {
	return tree ? &tree->base : NULL;
}

struct wlf_region* wlf_region_create(void) {
	struct wlf_region *region = malloc(sizeof(struct wlf_region));
	if (!region) return NULL;

	memset(region, 0, sizeof(struct wlf_region));

	return region;
}

void wlf_region_destroy(struct wlf_region *region) {
	if (!region) return;

	free(region);
}

void wlf_region_add_item_rect(struct wlf_region *region, struct wlf_rect *rect) {
	if (!region || !rect) return;

}

bool wlf_region_contains_item_point(struct wlf_region *region, int x, int y) {
	if (!region) return false;

	return false;
}

void wlf_item_enable_offscreen(struct wlf_item *item, bool enable) {
	if (!item) return;

	item->use_offscreen = enable;
	if (enable) {
		item->buffer_dirty = true;
	} else if (item->offscreen_buffer) {
		item->offscreen_buffer = NULL;
	}
}

void wlf_item_mark_dirty(struct wlf_item *item, struct wlf_rect *damage) {
	if (!item) return;

	item->buffer_dirty = true;

	if (damage) {
	}

	if (item->parent) {
		wlf_item_mark_dirty((struct wlf_item*)item->parent, damage);
	}
}

void wlf_item_tree_enable_children_fbo(struct wlf_item_tree *tree, bool enable) {
	if (!tree) return;

	if (tree->use_children_fbo == enable) return;

	tree->use_children_fbo = enable;

	if (enable) {
		wlf_item_tree_update_children_bounds(tree);
		tree->children_fbo_dirty = true;
	} else {
		if (tree->children_fbo) {
			tree->children_fbo = NULL;
		}
	}
}

void wlf_item_tree_mark_children_dirty(struct wlf_item_tree *tree) {
	if (!tree) return;

	tree->children_fbo_dirty = true;
	wlf_item_mark_dirty(&tree->base, NULL);
}

void wlf_item_tree_update_children_bounds(struct wlf_item_tree *tree) {
	if (!tree || tree->children_count == 0) {
		if (tree) {
			tree->children_bounds = (struct wlf_rect){0, 0, 0, 0};
		}
		return;
	}

	struct wlf_rect bounds = tree->children[0]->geometry;

	for (size_t i = 1; i < tree->children_count; i++) {
		struct wlf_item *child = tree->children[i];
		if (!child->visible) continue;

		struct wlf_rect child_rect = child->geometry;

		int right = bounds.x + bounds.width;
		int bottom = bounds.y + bounds.height;
		int child_right = child_rect.x + child_rect.width;
		int child_bottom = child_rect.y + child_rect.height;

		bounds.x = (bounds.x < child_rect.x) ? bounds.x : child_rect.x;
		bounds.y = (bounds.y < child_rect.y) ? bounds.y : child_rect.y;
		bounds.width = ((right > child_right) ? right : child_right) - bounds.x;
		bounds.height = ((bottom > child_bottom) ? bottom : child_bottom) - bounds.y;
	}

	tree->children_bounds = bounds;
}

void wlf_item_tree_set_force_children_to_fbo(struct wlf_item_tree *tree, bool force) {
	if (!tree) return;

	tree->force_children_to_fbo = force;
	if (force && !tree->use_children_fbo) {
		wlf_item_tree_enable_children_fbo(tree, true);
	}
}

void wlf_item_tree_set_custom_composite(struct wlf_item_tree *tree, bool custom) {
	if (!tree) return;

	tree->custom_composite = custom;
}

void wlf_item_render_recursive(struct wlf_item *item, struct wlf_renderer *renderer, struct wlf_rect *clip) {
	if (!item || !renderer || !item->visible || item->opacity <= 0.0f) return;

	struct wlf_rect item_rect = item->geometry;
	struct wlf_rect draw_rect;

	draw_rect.x = (item_rect.x > clip->x) ? item_rect.x : clip->x;
	draw_rect.y = (item_rect.y > clip->y) ? item_rect.y : clip->y;
	int right = ((item_rect.x + item_rect.width) < (clip->x + clip->width)) ?
				(item_rect.x + item_rect.width) : (clip->x + clip->width);
	int bottom = ((item_rect.y + item_rect.height) < (clip->y + clip->height)) ?
				 (item_rect.y + item_rect.height) : (clip->y + clip->height);
	draw_rect.width = right - draw_rect.x;
	draw_rect.height = bottom - draw_rect.y;

	if (draw_rect.width <= 0 || draw_rect.height <= 0) return;

	if (item->use_offscreen) {
		return;
	}

	if (item->hooks.on_paint) {
		struct wlf_render_context context;
		wlf_render_context_init_window(&context, item->window, &draw_rect, item->opacity);

		item->hooks.on_paint(item, renderer, &draw_rect, &context);
	}

	if (wlf_item_is_tree(item)) {
		struct wlf_item_tree *tree = wlf_item_to_tree(item);

		if (tree->use_children_fbo && tree->children_count > 0) {
		} else {
			for (size_t i = 0; i < tree->children_count; i++) {
				wlf_item_render_recursive(tree->children[i], renderer, &draw_rect);
			}
		}
	}
}

void wlf_render_context_init_window(struct wlf_render_context *ctx, struct wlf_window *window,
									struct wlf_rect *viewport, float opacity) {
	if (!ctx) return;

	memset(ctx, 0, sizeof(struct wlf_render_context));

	ctx->target_type = WLF_RENDER_TARGET_WINDOW;
	ctx->target.window.window = window;
	ctx->viewport = viewport ? *viewport : (struct wlf_rect){0, 0, 0, 0};
	ctx->opacity_factor = opacity;
	ctx->allow_caching = true;
	ctx->requires_alpha_blending = (opacity < 1.0f);

	memset(ctx->transform_matrix, 0, sizeof(ctx->transform_matrix));
	ctx->transform_matrix[0] = ctx->transform_matrix[5] = ctx->transform_matrix[10] = ctx->transform_matrix[15] = 1.0f;
}

void wlf_render_context_init_fbo(struct wlf_render_context *ctx, struct wlf_framebuffer *fbo,
								struct wlf_rect *viewport, float opacity,
								bool is_batch, void *container) {
	if (!ctx) return;

	memset(ctx, 0, sizeof(struct wlf_render_context));

	ctx->target_type = WLF_RENDER_TARGET_FBO;
	ctx->target.fbo.fbo = fbo;
	ctx->target.fbo.is_children_batch = is_batch;
	ctx->target.fbo.batch_container = container;
	ctx->viewport = viewport ? *viewport : (struct wlf_rect){0, 0, 0, 0};
	ctx->opacity_factor = opacity;
	ctx->allow_caching = !is_batch;
	ctx->requires_alpha_blending = true;

	memset(ctx->transform_matrix, 0, sizeof(ctx->transform_matrix));
	ctx->transform_matrix[0] = ctx->transform_matrix[5] = ctx->transform_matrix[10] = ctx->transform_matrix[15] = 1.0f;
}
