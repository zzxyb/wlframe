#ifndef SCENE_WLF_SHAPE_NODE_H
#define SCENE_WLF_SHAPE_NODE_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/scene/wlf_scene_node.h"
#include "wlf/shapes/wlf_circle_shape.h"
#include "wlf/shapes/wlf_ellipse_shape.h"
#include "wlf/shapes/wlf_line_shape.h"
#include "wlf/shapes/wlf_path_shape.h"
#include "wlf/shapes/wlf_poly_shape.h"
#include "wlf/shapes/wlf_rect_shape.h"

enum wlf_shape_node_type {
	WLF_SHAPE_NODE_RECT,
	WLF_SHAPE_NODE_CIRCLE,
	WLF_SHAPE_NODE_ELLIPSE,
	WLF_SHAPE_NODE_LINE,
	WLF_SHAPE_NODE_POLY,
	WLF_SHAPE_NODE_PATH,
};

/** Common state shared by all directly rendered geometric scene nodes. */
struct wlf_shape_node {
	struct wlf_scene_node base;
	struct wlf_shape *shape; /**< Owned by this node. */
	enum wlf_shape_node_type type;
	double geometry_x, geometry_y;
};

struct wlf_rect_shape_node {
	struct wlf_shape_node shape_node;
};

struct wlf_circle_node {
	struct wlf_shape_node shape_node;
};

struct wlf_ellipse_node {
	struct wlf_shape_node shape_node;
};

struct wlf_line_node {
	struct wlf_shape_node shape_node;
};

struct wlf_poly_node {
	struct wlf_shape_node shape_node;
};

struct wlf_path_node {
	struct wlf_shape_node shape_node;
};

/** The node takes ownership of shape when creation succeeds. */
struct wlf_rect_shape_node *wlf_rect_shape_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_rect_shape *shape);
struct wlf_circle_node *wlf_circle_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_circle_shape *shape);
struct wlf_ellipse_node *wlf_ellipse_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_ellipse_shape *shape);
struct wlf_line_node *wlf_line_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_line_shape *shape);
struct wlf_poly_node *wlf_poly_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_poly_shape *shape);
struct wlf_path_node *wlf_path_node_create(
	struct wlf_scene_node *parent, double x, double y,
	struct wlf_path_shape *shape);

bool wlf_scene_node_is_shape(const struct wlf_scene_node *node);
struct wlf_shape_node *wlf_shape_node_from_node(struct wlf_scene_node *node);

bool wlf_scene_node_is_rect_shape(const struct wlf_scene_node *node);
bool wlf_scene_node_is_circle(const struct wlf_scene_node *node);
bool wlf_scene_node_is_ellipse(const struct wlf_scene_node *node);
bool wlf_scene_node_is_line(const struct wlf_scene_node *node);
bool wlf_scene_node_is_poly(const struct wlf_scene_node *node);
bool wlf_scene_node_is_path(const struct wlf_scene_node *node);

struct wlf_rect_shape_node *wlf_rect_shape_node_from_node(
	struct wlf_scene_node *node);
struct wlf_circle_node *wlf_circle_node_from_node(struct wlf_scene_node *node);
struct wlf_ellipse_node *wlf_ellipse_node_from_node(struct wlf_scene_node *node);
struct wlf_line_node *wlf_line_node_from_node(struct wlf_scene_node *node);
struct wlf_poly_node *wlf_poly_node_from_node(struct wlf_scene_node *node);
struct wlf_path_node *wlf_path_node_from_node(struct wlf_scene_node *node);

/**
 * Tessellates the current shape parameters and draws them directly.
 * No texture or intermediate raster image is created for shape nodes.
 */
void wlf_shape_node_render(struct wlf_shape_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

void wlf_rect_shape_node_render(struct wlf_rect_shape_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);
void wlf_circle_node_render(struct wlf_circle_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);
void wlf_ellipse_node_render(struct wlf_ellipse_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);
void wlf_line_node_render(struct wlf_line_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);
void wlf_poly_node_render(struct wlf_poly_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);
void wlf_path_node_render(struct wlf_path_node *node,
	struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const pixman_region32_t *clip);

#endif // SCENE_WLF_SHAPE_NODE_H
