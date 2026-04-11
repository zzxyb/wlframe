#include "wlf/shapes/wlf_path_shape.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void shape_destroy(struct wlf_shape *shape) {
	struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(shape);
	if (path_shape->owns_paths) {
		wlf_path_destroy_list(path_shape->paths);
	}

	free(path_shape);
}

static struct wlf_shape *shape_clone(struct wlf_shape *shape) {
	struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(shape);
	struct wlf_path *paths = NULL;

	if (path_shape->paths != NULL && path_shape->owns_paths) {
		paths = wlf_path_clone_list(path_shape->paths);
	} else {
		paths = path_shape->paths;
	}

	struct wlf_shape *clone = wlf_path_shape_create(paths, path_shape->owns_paths);
	if (clone != NULL) {
		wlf_path_shape_from_shape(clone)->state = path_shape->state;
	}

	return clone;
}

static const struct wlf_shape_impl shape_impl = {
	.destroy = shape_destroy,
	.clone = shape_clone,
};

struct wlf_shape *wlf_path_shape_create(struct wlf_path *paths, bool owns_paths) {
	struct wlf_path_shape *shape = malloc(sizeof(*shape));
	if (shape == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_path_shape");
		return NULL;
	}

	wlf_shape_init(&shape->base, &shape_impl);
	shape->paths = paths;
	shape->owns_paths = owns_paths;
	wlf_shape_state_init(&shape->state);

	return &shape->base;
}

bool wlf_shape_is_path(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shape_impl;
}

struct wlf_path_shape *wlf_path_shape_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shape_impl);

	struct wlf_path_shape *path_shape =
		wlf_container_of(shape, path_shape, base);

	return path_shape;
}

struct wlf_path *wlf_path_duplicate(const struct wlf_path *path) {
	struct wlf_path *res = NULL;
	res = malloc(sizeof(*res));
	if (res == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_path");
		return NULL;
	}
	memset(res, 0, sizeof(*res));
	res->pts = malloc((size_t)path->npts * 2 * sizeof(float));
	if (res->pts == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_path->pts");
		free(res);
		return NULL;
	}
	memcpy(res->pts, path->pts, (size_t)path->npts * 2 * sizeof(float));
	res->npts = path->npts;
	res->closed = path->closed;
	memcpy(res->bounds, path->bounds, sizeof(path->bounds));
	res->next = NULL;

	return res;
}

struct wlf_path *wlf_path_clone_list(const struct wlf_path *path) {
	struct wlf_path *head = NULL;
	struct wlf_path *tail = NULL;

	while (path != NULL) {
		struct wlf_path *node = wlf_path_duplicate(path);
		if (node == NULL) {
			wlf_path_destroy_list(head);
			return NULL;
		}
		if (head == NULL) {
			head = node;
		} else {
			tail->next = node;
		}
		tail = node;
		path = path->next;
	}
	return head;
}

void wlf_path_destroy_list(struct wlf_path *path) {
	while (path != NULL) {
		struct wlf_path *next = path->next;
		free(path->pts);
		free(path);
		path = next;
	}
}
