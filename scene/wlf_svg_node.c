#include "wlf/scene/wlf_svg_node.h"

#include "wlf/scene/wlf_circle_node.h"
#include "wlf/scene/wlf_ellipse_node.h"
#include "wlf/scene/wlf_line_node.h"
#include "wlf/scene/wlf_path_node.h"
#include "wlf/scene/wlf_poly_node.h"
#include "wlf/scene/wlf_rect_shape_node.h"
#include "wlf/scene/wlf_text_node.h"
#include "wlf/shapes/wlf_circle_shape.h"
#include "wlf/shapes/wlf_ellipse_shape.h"
#include "wlf/shapes/wlf_line_shape.h"
#include "wlf/shapes/wlf_path_shape.h"
#include "wlf/shapes/wlf_poly_shape.h"
#include "wlf/shapes/wlf_rect_shape.h"
#include "wlf/shapes/wlf_text_shape.h"
#include "wlf/types/wlf_gradient.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static struct wlf_shape_state *shape_state(struct wlf_shape *shape) {
	if (wlf_shape_is_rect(shape)) {
		return &wlf_rect_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_circle(shape)) {
		return &wlf_circle_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_ellipse(shape)) {
		return &wlf_ellipse_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_line(shape)) {
		return &wlf_line_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_poly(shape)) {
		return &wlf_poly_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_path(shape)) {
		return &wlf_path_shape_from_shape(shape)->state;
	}
	if (wlf_shape_is_text(shape)) {
		return &wlf_text_shape_from_shape(shape)->state;
	}
	return NULL;
}

static double shape_padding(const struct wlf_shape_state *state) {
	double padding = 1.0;
	if (state->has_stroke && state->stroke_width > 0) {
		padding += state->stroke_width / 2.0;
	}
	return padding;
}

static bool geometry_origin(struct wlf_shape *shape, double *x, double *y) {
	struct wlf_shape_state *state = shape_state(shape);
	if (state == NULL) {
		return false;
	}
	double padding = shape_padding(state);
	if (wlf_shape_is_rect(shape)) {
		struct wlf_rect_shape *rect = wlf_rect_shape_from_shape(shape);
		*x = fmin(rect->x, rect->x + rect->width) - padding;
		*y = fmin(rect->y, rect->y + rect->height) - padding;
		return true;
	}
	if (wlf_shape_is_circle(shape)) {
		struct wlf_circle_shape *circle = wlf_circle_shape_from_shape(shape);
		*x = circle->cx - circle->r - padding;
		*y = circle->cy - circle->r - padding;
		return true;
	}
	if (wlf_shape_is_ellipse(shape)) {
		struct wlf_ellipse_shape *ellipse = wlf_ellipse_shape_from_shape(shape);
		*x = ellipse->cx - ellipse->rx - padding;
		*y = ellipse->cy - ellipse->ry - padding;
		return true;
	}
	if (wlf_shape_is_line(shape)) {
		struct wlf_line_shape *line = wlf_line_shape_from_shape(shape);
		*x = fmin(line->x1, line->x2) - padding;
		*y = fmin(line->y1, line->y2) - padding;
		return true;
	}
	if (wlf_shape_is_poly(shape)) {
		struct wlf_poly_shape *poly = wlf_poly_shape_from_shape(shape);
		if (poly->points == NULL || poly->count <= 0) {
			return false;
		}
		*x = INFINITY;
		*y = INFINITY;
		for (int i = 0; i < poly->count; i++) {
			*x = fmin(*x, poly->points[i * 2]);
			*y = fmin(*y, poly->points[i * 2 + 1]);
		}
		*x -= padding;
		*y -= padding;
		return true;
	}
	if (wlf_shape_is_path(shape)) {
		struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(shape);
		*x = INFINITY;
		*y = INFINITY;
		for (struct wlf_path *path = path_shape->paths; path != NULL;
				path = path->next) {
			for (int i = 0; i < path->npts; i++) {
				*x = fmin(*x, path->pts[i * 2]);
				*y = fmin(*y, path->pts[i * 2 + 1]);
			}
		}
		if (!isfinite(*x) || !isfinite(*y)) {
			return false;
		}
		*x -= padding;
		*y -= padding;
		return true;
	}
	return false;
}

static struct wlf_scene_node *create_geometry_node(
		struct wlf_svg_node *svg_node, struct wlf_svg_shape *svg_shape) {
	struct wlf_shape *geometry = wlf_shape_clone(svg_shape->geometry);
	if (geometry == NULL) {
		return NULL;
	}

	struct wlf_shape_state *state = shape_state(geometry);
	if (state != NULL) {
		state->fill_gradient = svg_shape->fill;
		state->stroke_gradient = svg_shape->stroke;
		const struct wlf_fpoint center = {
			.x = (svg_shape->bounds[0] + svg_shape->bounds[2]) / 2.0,
			.y = (svg_shape->bounds[1] + svg_shape->bounds[3]) / 2.0,
		};
		if (svg_shape->fill != NULL) {
			state->fill_color = wlf_gradient_sample(svg_shape->fill, &center);
		}
		if (svg_shape->stroke != NULL) {
			state->stroke_color = wlf_gradient_sample(svg_shape->stroke, &center);
		}
		/* SVG paint gradients already contain the effective paint alpha. */
		state->opacity = 1.0f;
		state->fill_opacity = 1.0f;
		state->stroke_opacity = 1.0f;
	}

	double x, y;
	if (!geometry_origin(geometry, &x, &y)) {
		wlf_shape_destroy(geometry);
		return NULL;
	}
	struct wlf_scene_node *node = NULL;
	if (wlf_shape_is_rect(geometry)) {
		struct wlf_rect_shape_node *rect = wlf_rect_shape_node_create(
			&svg_node->base, x, y, wlf_rect_shape_from_shape(geometry));
		node = rect != NULL ? &rect->base : NULL;
	} else if (wlf_shape_is_circle(geometry)) {
		struct wlf_circle_node *circle = wlf_circle_node_create(
			&svg_node->base, x, y, wlf_circle_shape_from_shape(geometry));
		node = circle != NULL ? &circle->base : NULL;
	} else if (wlf_shape_is_ellipse(geometry)) {
		struct wlf_ellipse_node *ellipse = wlf_ellipse_node_create(
			&svg_node->base, x, y, wlf_ellipse_shape_from_shape(geometry));
		node = ellipse != NULL ? &ellipse->base : NULL;
	} else if (wlf_shape_is_line(geometry)) {
		struct wlf_line_node *line = wlf_line_node_create(
			&svg_node->base, x, y, wlf_line_shape_from_shape(geometry));
		node = line != NULL ? &line->base : NULL;
	} else if (wlf_shape_is_poly(geometry)) {
		struct wlf_poly_node *poly = wlf_poly_node_create(
			&svg_node->base, x, y, wlf_poly_shape_from_shape(geometry));
		node = poly != NULL ? &poly->base : NULL;
	} else if (wlf_shape_is_path(geometry)) {
		struct wlf_path_node *path = wlf_path_node_create(
			&svg_node->base, x, y, wlf_path_shape_from_shape(geometry));
		node = path != NULL ? &path->base : NULL;
	}

	if (node == NULL) {
		wlf_shape_destroy(geometry);
	}
	return node;
}

static struct wlf_scene_node *create_text_node(struct wlf_svg_node *svg_node,
		struct wlf_svg_shape *svg_shape) {
	struct wlf_text_shape *shape =
		wlf_text_shape_from_shape(svg_shape->geometry);
	struct wlf_color color = WLF_COLOR_TRANSPARENT;
	if (svg_shape->fill != NULL) {
		const struct wlf_fpoint point = {
			.x = shape->x,
			.y = shape->y,
		};
		color = wlf_gradient_sample(svg_shape->fill, &point);
	}

	struct wlf_text_node *text = wlf_text_node_create(&svg_node->base,
		shape->x, shape->y, shape->text, shape->font_family,
		shape->font_size, &color);
	if (text == NULL) {
		return NULL;
	}

	double x = shape->x;
	if (shape->text_anchor == WLF_TEXT_ANCHOR_MIDDLE) {
		x -= text->natural_width / 2.0;
	} else if (shape->text_anchor == WLF_TEXT_ANCHOR_END) {
		x -= text->natural_width;
	}
	wlf_scene_node_set_position(&text->base, x, shape->y - text->baseline);
	return &text->base;
}

static bool create_children(struct wlf_svg_node *node) {
	for (struct wlf_shape *base = node->image->shapes; base != NULL;) {
		struct wlf_svg_shape *svg_shape = wlf_svg_shape_from_shape(base);
		base = (struct wlf_shape *)svg_shape->next;
		if (!(svg_shape->flags & WLF_SVG_FLAGS_VISIBLE) ||
				svg_shape->geometry == NULL) {
			continue;
		}

		struct wlf_scene_node *child;
		if (wlf_shape_is_text(svg_shape->geometry)) {
			child = create_text_node(node, svg_shape);
		} else {
			child = create_geometry_node(node, svg_shape);
		}
		if (child == NULL) {
			wlf_log(WLF_ERROR, "failed to create SVG scene child");
			return false;
		}
	}
	return true;
}

static void scene_node_destroy(struct wlf_scene_node *base) {
	struct wlf_svg_node *node = wlf_svg_node_from_node(base);
	struct wlf_scene_node *child, *tmp;
	wlf_linked_list_for_each_safe(child, tmp, &node->children, link) {
		wlf_scene_node_destroy(child);
	}
	wlf_svg_destroy(node->image);
	free(node);
}

static void scene_node_get_size(struct wlf_scene_node *base,
		double *width, double *height) {
	struct wlf_svg_node *node = wlf_svg_node_from_node(base);
	*width = node->image->width;
	*height = node->image->height;
}

static struct wlf_linked_list *scene_node_get_children(
		struct wlf_scene_node *base) {
	return &wlf_svg_node_from_node(base)->children;
}

static bool scene_node_invisible(struct wlf_scene_node *base) {
	(void)base;
	return true;
}

static void scene_node_visibility(struct wlf_scene_node *base,
		pixman_region32_t *visible) {
	if (!base->state.enabled) {
		return;
	}
	struct wlf_svg_node *node = wlf_svg_node_from_node(base);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, &node->children, link) {
		wlf_scene_node_visibility(child, visible);
	}
}

static bool node_at_iterator(struct wlf_scene_node *node,
		double lx, double ly, void *data) {
	struct wlf_node_at_data *at = data;
	at->rx = at->lx - lx;
	at->ry = at->ly - ly;
	at->node = node;
	return true;
}

static struct wlf_scene_node *scene_node_at(struct wlf_scene_node *base,
		double lx, double ly, double *nx, double *ny) {
	struct wlf_frect box = {
		.x = lx,
		.y = ly,
		.width = 1,
		.height = 1,
	};
	struct wlf_node_at_data data = {
		.lx = lx,
		.ly = ly,
	};
	if (!wlf_scene_node_nodes_in_box(base, &box, node_at_iterator, &data)) {
		return NULL;
	}
	if (nx != NULL) {
		*nx = data.rx;
	}
	if (ny != NULL) {
		*ny = data.ry;
	}
	return data.node;
}

static void scene_node_bounds(struct wlf_scene_node *base,
		double x, double y, pixman_region32_t *visible) {
	if (!base->state.enabled) {
		return;
	}
	struct wlf_svg_node *node = wlf_svg_node_from_node(base);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each(child, &node->children, link) {
		wlf_scene_node_bounds(child, x + child->state.x,
			y + child->state.y, visible);
	}
}

static bool scene_nodes_in_box(struct wlf_scene_node *base,
		struct wlf_frect *box, scene_node_box_iterator_func_t iterator,
		void *user_data) {
	if (!base->state.enabled) {
		return false;
	}
	struct wlf_svg_node *node = wlf_svg_node_from_node(base);
	struct wlf_scene_node *child;
	wlf_linked_list_for_each_reverse(child, &node->children, link) {
		if (wlf_scene_node_nodes_in_box(child, box, iterator, user_data)) {
			return true;
		}
	}
	return false;
}

static const struct wlf_scene_node_impl scene_node_impl = {
	.destroy = scene_node_destroy,
	.get_size = scene_node_get_size,
	.get_children = scene_node_get_children,
	.invisible = scene_node_invisible,
	.visibility = scene_node_visibility,
	.at = scene_node_at,
	.bounds = scene_node_bounds,
	.in_box = scene_nodes_in_box,
};

struct wlf_svg_node *wlf_svg_node_create(struct wlf_scene_node *parent,
		double x, double y, struct wlf_svg_image *image) {
	if (parent == NULL || image == NULL ||
			!isfinite(image->width) || !isfinite(image->height) ||
			image->width < 0 || image->height < 0) {
		return NULL;
	}

	struct wlf_svg_node *node = calloc(1, sizeof(*node));
	if (node == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_svg_node");
		return NULL;
	}
	wlf_scene_node_init(&node->base, &scene_node_impl, parent);
	wlf_linked_list_init(&node->children);
	node->base.state.x = x;
	node->base.state.y = y;
	node->base.state.width = image->width;
	node->base.state.height = image->height;
	node->image = image;

	if (!create_children(node)) {
		node->image = NULL;
		wlf_scene_node_destroy(&node->base);
		return NULL;
	}
	wlf_scene_node_update(&node->base, NULL);
	return node;
}

struct wlf_svg_node *wlf_svg_node_create_from_file(
		struct wlf_scene_node *parent, double x, double y,
		const char *filename, const char *units, float dpi) {
	if (filename == NULL) {
		return NULL;
	}
	struct wlf_svg_image *image = wlf_svg_parse_from_file(filename, units, dpi);
	if (image == NULL) {
		return NULL;
	}
	struct wlf_svg_node *node = wlf_svg_node_create(parent, x, y, image);
	if (node == NULL) {
		wlf_svg_destroy(image);
	}
	return node;
}

bool wlf_scene_node_is_svg(const struct wlf_scene_node *node) {
	return node != NULL && node->impl == &scene_node_impl;
}

struct wlf_svg_node *wlf_svg_node_from_node(struct wlf_scene_node *node) {
	assert(wlf_scene_node_is_svg(node));
	struct wlf_svg_node *svg_node = wlf_container_of(node, svg_node, base);
	return svg_node;
}
