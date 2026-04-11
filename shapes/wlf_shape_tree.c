#include "wlf/shapes/wlf_shape_tree.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static bool shape_tree_contains(const struct wlf_shape_tree *tree,
		const struct wlf_shape *shape) {
	const struct wlf_shape *iter;

	wlf_linked_list_for_each(iter, &tree->children, link) {
		if (iter == shape) {
			return true;
		}
	}

	return false;
}

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_shape_tree *tree = wlf_shape_tree_from_shape(shape);
	struct wlf_shape *iter, *tmp;

	wlf_linked_list_for_each_safe(iter, tmp, &tree->children, link) {
		wlf_linked_list_remove(&iter->link);
		wlf_shape_destroy(iter);
	}

	free(tree);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_shape_tree *src = wlf_shape_tree_from_shape(shape);
	struct wlf_shape *dst_base = wlf_shape_tree_create();
	struct wlf_shape_tree *dst;
	struct wlf_shape *iter;

	if (dst_base == NULL) {
		return NULL;
	}

	dst = wlf_shape_tree_from_shape(dst_base);
	wlf_linked_list_for_each(iter, &src->children, link) {
		struct wlf_shape *child_clone = wlf_shape_clone(iter);
		if (child_clone == NULL) {
			wlf_shape_destroy(&dst->base);
			return NULL;
		}

		wlf_shape_tree_add(dst, child_clone);
	}

	return &dst->base;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_shape_tree_create(void) {
	struct wlf_shape_tree *tree = malloc(sizeof(*tree));
	if (tree == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_shape_tree");
		return NULL;
	}

	wlf_shape_init(&tree->base, &shape_impl);
	wlf_linked_list_init(&tree->children);

	return &tree->base;
}

void wlf_shape_tree_add(struct wlf_shape_tree *tree, struct wlf_shape *shape) {
	assert(tree);
	assert(shape);

	if (shape == &tree->base) {
		return;
	}

	if (shape->link.prev != NULL || shape->link.next != NULL) {
		return;
	}

	wlf_linked_list_insert(tree->children.prev, &shape->link);
}

void wlf_shape_tree_remove(struct wlf_shape_tree *tree, struct wlf_shape *shape) {
	assert(tree);
	assert(shape);

	if (!shape_tree_contains(tree, shape)) {
		return;
	}

	wlf_linked_list_remove(&shape->link);
}

int wlf_shape_tree_child_count(const struct wlf_shape_tree *tree) {
	assert(tree);

	return wlf_linked_list_length(&tree->children);
}

bool wlf_shape_is_tree(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_shape_tree *wlf_shape_tree_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_shape_tree *tree =
		wlf_container_of(shape, tree, base);

	return tree;
}
