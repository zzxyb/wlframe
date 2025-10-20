#include "wlf/utils/wlf_map.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Create a new red-black tree node.
 */
static struct wlf_map_node *create_node(void *key, void *value) {
	struct wlf_map_node *node = calloc(1, sizeof(struct wlf_map_node));
	if (!node) {
		return NULL;
	}

	node->key = key;
	node->value = value;
	node->color = WLF_MAP_RED;
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;

	return node;
}

/**
 * @brief Get the grandparent of a node.
 */
static struct wlf_map_node *grandparent(struct wlf_map_node *n) {
	if (n && n->parent) {
		return n->parent->parent;
	}
	return NULL;
}

/**
 * @brief Get the uncle of a node.
 */
static struct wlf_map_node *uncle(struct wlf_map_node *n) {
	struct wlf_map_node *g = grandparent(n);
	if (!g) {
		return NULL;
	}
	if (n->parent == g->left) {
		return g->right;
	}
	return g->left;
}

/**
 * @brief Rotate left around a node.
 */
static void rotate_left(struct wlf_map *map, struct wlf_map_node *n) {
	struct wlf_map_node *r = n->right;
	struct wlf_map_node *p = n->parent;

	n->right = r->left;
	if (r->left) {
		r->left->parent = n;
	}
	r->left = n;
	n->parent = r;

	if (p) {
		if (n == p->left) {
			p->left = r;
		} else {
			p->right = r;
		}
	} else {
		map->root = r;
	}
	r->parent = p;
}

/**
 * @brief Rotate right around a node.
 */
static void rotate_right(struct wlf_map *map, struct wlf_map_node *n) {
	struct wlf_map_node *l = n->left;
	struct wlf_map_node *p = n->parent;

	n->left = l->right;
	if (l->right) {
		l->right->parent = n;
	}
	l->right = n;
	n->parent = l;

	if (p) {
		if (n == p->left) {
			p->left = l;
		} else {
			p->right = l;
		}
	} else {
		map->root = l;
	}
	l->parent = p;
}

/**
 * @brief Fix red-black tree violations after insertion.
 */
static void insert_fixup(struct wlf_map *map, struct wlf_map_node *n) {
	while (n->parent && n->parent->color == WLF_MAP_RED) {
		struct wlf_map_node *u = uncle(n);
		struct wlf_map_node *g = grandparent(n);

		if (u && u->color == WLF_MAP_RED) {
			// Case 1: Uncle is red
			n->parent->color = WLF_MAP_BLACK;
			u->color = WLF_MAP_BLACK;
			g->color = WLF_MAP_RED;
			n = g;
		} else {
			// Cases 2 & 3: Uncle is black
			if (n->parent == g->left) {
				if (n == n->parent->right) {
					// Case 2: Node is right child
					n = n->parent;
					rotate_left(map, n);
				}
				// Case 3: Node is left child
				n->parent->color = WLF_MAP_BLACK;
				g->color = WLF_MAP_RED;
				rotate_right(map, g);
			} else {
				if (n == n->parent->left) {
					// Case 2 (mirrored)
					n = n->parent;
					rotate_right(map, n);
				}
				// Case 3 (mirrored)
				n->parent->color = WLF_MAP_BLACK;
				g->color = WLF_MAP_RED;
				rotate_left(map, g);
			}
		}
	}
	map->root->color = WLF_MAP_BLACK;
}

/**
 * @brief Find the minimum node in a subtree.
 */
static struct wlf_map_node *find_min(struct wlf_map_node *n) {
	while (n && n->left) {
		n = n->left;
	}
	return n;
}

/**
 * @brief Replace a node with another in the tree structure.
 */
static void replace_node(struct wlf_map *map, struct wlf_map_node *old_node,
                         struct wlf_map_node *new_node) {
	if (old_node->parent) {
		if (old_node == old_node->parent->left) {
			old_node->parent->left = new_node;
		} else {
			old_node->parent->right = new_node;
		}
	} else {
		map->root = new_node;
	}
	if (new_node) {
		new_node->parent = old_node->parent;
	}
}

/**
 * @brief Fix red-black tree violations after deletion.
 */
static void delete_fixup(struct wlf_map *map, struct wlf_map_node *n,
                         struct wlf_map_node *parent) {
	while (n != map->root && (!n || n->color == WLF_MAP_BLACK)) {
		if (n == parent->left) {
			struct wlf_map_node *w = parent->right;

			if (w->color == WLF_MAP_RED) {
				w->color = WLF_MAP_BLACK;
				parent->color = WLF_MAP_RED;
				rotate_left(map, parent);
				w = parent->right;
			}

			if ((!w->left || w->left->color == WLF_MAP_BLACK) &&
			    (!w->right || w->right->color == WLF_MAP_BLACK)) {
				w->color = WLF_MAP_RED;
				n = parent;
				parent = n->parent;
			} else {
				if (!w->right || w->right->color == WLF_MAP_BLACK) {
					if (w->left) {
						w->left->color = WLF_MAP_BLACK;
					}
					w->color = WLF_MAP_RED;
					rotate_right(map, w);
					w = parent->right;
				}
				w->color = parent->color;
				parent->color = WLF_MAP_BLACK;
				if (w->right) {
					w->right->color = WLF_MAP_BLACK;
				}
				rotate_left(map, parent);
				n = map->root;
				break;
			}
		} else {
			struct wlf_map_node *w = parent->left;

			if (w->color == WLF_MAP_RED) {
				w->color = WLF_MAP_BLACK;
				parent->color = WLF_MAP_RED;
				rotate_right(map, parent);
				w = parent->left;
			}

			if ((!w->left || w->left->color == WLF_MAP_BLACK) &&
			    (!w->right || w->right->color == WLF_MAP_BLACK)) {
				w->color = WLF_MAP_RED;
				n = parent;
				parent = n->parent;
			} else {
				if (!w->left || w->left->color == WLF_MAP_BLACK) {
					if (w->right) {
						w->right->color = WLF_MAP_BLACK;
					}
					w->color = WLF_MAP_RED;
					rotate_left(map, w);
					w = parent->left;
				}
				w->color = parent->color;
				parent->color = WLF_MAP_BLACK;
				if (w->left) {
					w->left->color = WLF_MAP_BLACK;
				}
				rotate_right(map, parent);
				n = map->root;
				break;
			}
		}
	}
	if (n) {
		n->color = WLF_MAP_BLACK;
	}
}

/**
 * @brief Recursively destroy all nodes in a subtree.
 */
static void destroy_subtree(struct wlf_map_node *node,
                            wlf_map_destroy_key_func_t destroy_key,
                            wlf_map_destroy_value_func_t destroy_value) {
	if (!node) {
		return;
	}

	destroy_subtree(node->left, destroy_key, destroy_value);
	destroy_subtree(node->right, destroy_key, destroy_value);

	if (destroy_key) {
		destroy_key(node->key);
	}
	if (destroy_value) {
		destroy_value(node->value);
	}

	free(node);
}

/**
 * @brief In-order traversal for foreach operation.
 */
static bool foreach_helper(struct wlf_map_node *node,
                           wlf_map_foreach_func_t func,
                           void *user_data) {
	if (!node) {
		return true;
	}

	if (!foreach_helper(node->left, func, user_data)) {
		return false;
	}

	if (!func(node->key, node->value, user_data)) {
		return false;
	}

	return foreach_helper(node->right, func, user_data);
}

struct wlf_map *wlf_map_create(wlf_map_compare_func_t compare) {
	if (!compare) {
		wlf_log(WLF_ERROR, "Comparison function cannot be NULL");
		return NULL;
	}

	struct wlf_map *map = calloc(1, sizeof(struct wlf_map));
	if (!map) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate map");
		return NULL;
	}

	map->root = NULL;
	map->compare = compare;
	map->size = 0;

	return map;
}

void wlf_map_destroy(struct wlf_map *map,
                     wlf_map_destroy_key_func_t destroy_key,
                     wlf_map_destroy_value_func_t destroy_value) {
	if (!map) {
		return;
	}

	destroy_subtree(map->root, destroy_key, destroy_value);
	free(map);
}

bool wlf_map_insert(struct wlf_map *map, void *key, void *value) {
	if (!map || !key) {
		return false;
	}

	if (!map->root) {
		map->root = create_node(key, value);
		if (!map->root) {
			return false;
		}
		map->root->color = WLF_MAP_BLACK;
		map->size++;
		return true;
	}

	struct wlf_map_node *current = map->root;
	struct wlf_map_node *parent = NULL;

	while (current) {
		parent = current;
		int cmp = map->compare(key, current->key);

		if (cmp == 0) {
			// Key already exists, update value
			current->value = value;
			return true;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			current = current->right;
		}
	}

	struct wlf_map_node *new_node = create_node(key, value);
	if (!new_node) {
		return false;
	}

	new_node->parent = parent;

	int cmp = map->compare(key, parent->key);
	if (cmp < 0) {
		parent->left = new_node;
	} else {
		parent->right = new_node;
	}

	map->size++;

	insert_fixup(map, new_node);

	return true;
}

bool wlf_map_remove(struct wlf_map *map, void *key,
                    wlf_map_destroy_key_func_t destroy_key,
                    wlf_map_destroy_value_func_t destroy_value) {
	if (!map || !key || !map->root) {
		return false;
	}

	struct wlf_map_node *node = map->root;
	while (node) {
		int cmp = map->compare(key, node->key);
		if (cmp == 0) {
			break;
		} else if (cmp < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}

	if (!node) {
		return false; // Key not found
	}

	struct wlf_map_node *to_delete = node;
	struct wlf_map_node *child;
	enum wlf_map_color original_color = to_delete->color;

	if (!node->left) {
		child = node->right;
		replace_node(map, node, node->right);
	} else if (!node->right) {
		child = node->left;
		replace_node(map, node, node->left);
	} else {
		to_delete = find_min(node->right);
		original_color = to_delete->color;
		child = to_delete->right;

		if (to_delete->parent == node) {
			if (child) {
				child->parent = to_delete;
			}
		} else {
			replace_node(map, to_delete, to_delete->right);
			to_delete->right = node->right;
			to_delete->right->parent = to_delete;
		}

		replace_node(map, node, to_delete);
		to_delete->left = node->left;
		to_delete->left->parent = to_delete;
		to_delete->color = node->color;
	}

	if (destroy_key) {
		destroy_key(node->key);
	}
	if (destroy_value) {
		destroy_value(node->value);
	}

	struct wlf_map_node *fixup_parent = to_delete->parent;
	free(node);
	map->size--;

	if (original_color == WLF_MAP_BLACK && child) {
		delete_fixup(map, child, fixup_parent);
	}

	return true;
}

void *wlf_map_find(struct wlf_map *map, const void *key) {
	if (!map || !key) {
		return NULL;
	}

	struct wlf_map_node *current = map->root;

	while (current) {
		int cmp = map->compare(key, current->key);
		if (cmp == 0) {
			return current->value;
		} else if (cmp < 0) {
			current = current->left;
		} else {
			current = current->right;
		}
	}

	return NULL;
}

bool wlf_map_contains(struct wlf_map *map, const void *key) {
	return wlf_map_find(map, key) != NULL;
}

size_t wlf_map_size(struct wlf_map *map) {
	return map ? map->size : 0;
}

bool wlf_map_is_empty(struct wlf_map *map) {
	return !map || map->size == 0;
}

void wlf_map_clear(struct wlf_map *map,
                   wlf_map_destroy_key_func_t destroy_key,
                   wlf_map_destroy_value_func_t destroy_value) {
	if (!map) {
		return;
	}

	destroy_subtree(map->root, destroy_key, destroy_value);
	map->root = NULL;
	map->size = 0;
}

void wlf_map_foreach(struct wlf_map *map,
                     wlf_map_foreach_func_t func,
                     void *user_data) {
	if (!map || !func) {
		return;
	}

	foreach_helper(map->root, func, user_data);
}

struct wlf_map_iterator wlf_map_iterator_create(struct wlf_map *map) {
	struct wlf_map_iterator it = {
		.map = map,
		.current = NULL
	};

	if (map && map->root) {
		it.current = find_min(map->root);
	}

	return it;
}

bool wlf_map_iterator_has_next(struct wlf_map_iterator *it) {
	return it && it->current != NULL;
}

void wlf_map_iterator_next(struct wlf_map_iterator *it) {
	if (!it || !it->current) {
		return;
	}

	if (it->current->right) {
		it->current = find_min(it->current->right);
		return;
	}

	struct wlf_map_node *parent = it->current->parent;
	while (parent && it->current == parent->right) {
		it->current = parent;
		parent = parent->parent;
	}

	it->current = parent;
}

void *wlf_map_iterator_key(struct wlf_map_iterator *it) {
	return (it && it->current) ? it->current->key : NULL;
}

void *wlf_map_iterator_value(struct wlf_map_iterator *it) {
	return (it && it->current) ? it->current->value : NULL;
}
