/**
 * @file        wlf_shape_geometry.h
 * @brief       Converts vector shapes into covered triangles.
 * @details     These helpers are shared by vector rendering passes and keep
 *              geometry allocation, polygon tessellation, and antialiasing
 *              details out of individual scene-node implementations.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef WLF_SHAPE_GEOMETRY_H
#define WLF_SHAPE_GEOMETRY_H

#include "wlf/pass/wlf_vector_pass.h"
#include "wlf/shapes/wlf_shape.h"

struct wlf_rect_shape;

/**
 * @brief Growable triangle-vertex buffer used during shape tessellation.
 *
 * The buffer stores independent triangles in the format consumed by a vector
 * pass.
 */
struct wlf_shape_vertices {
	struct wlf_vector_vertex *data; /**< Allocated vertex storage. */
	size_t len; /**< Number of valid vertices in @p data. */
	size_t capacity; /**< Allocated vertex capacity. */
	bool failed; /**< Set when an allocation or tessellation step failed. */
};

/**
 * @brief Releases vertex storage and resets @p vertices to an empty state.
 * @param vertices Vertex buffer to finish.
 */
void wlf_shape_vertices_finish(struct wlf_shape_vertices *vertices);

/**
 * @brief Appends an opaque triangle to @p vertices.
 * @param vertices Vertex buffer receiving the triangle.
 * @param ax First vertex x coordinate.
 * @param ay First vertex y coordinate.
 * @param bx Second vertex x coordinate.
 * @param by Second vertex y coordinate.
 * @param cx Third vertex x coordinate.
 * @param cy Third vertex y coordinate.
 */
void wlf_shape_add_triangle(struct wlf_shape_vertices *vertices,
	double ax, double ay, double bx, double by, double cx, double cy);

/**
 * @brief Appends a triangle with per-vertex coverage values.
 * @param vertices Vertex buffer receiving the triangle.
 * @param ax First vertex x coordinate.
 * @param ay First vertex y coordinate.
 * @param ac First vertex coverage.
 * @param bx Second vertex x coordinate.
 * @param by Second vertex y coordinate.
 * @param bc Second vertex coverage.
 * @param cx Third vertex x coordinate.
 * @param cy Third vertex y coordinate.
 * @param cc Third vertex coverage.
 */
void wlf_shape_add_triangle_coverage(struct wlf_shape_vertices *vertices,
		double ax, double ay, float ac, double bx, double by, float bc,
		double cx, double cy, float cc);

/**
 * @brief Appends a rectangle represented by two opaque triangles.
 * @param vertices Vertex buffer receiving the quadrilateral.
 * @param ax First corner x coordinate.
 * @param ay First corner y coordinate.
 * @param bx Second corner x coordinate.
 * @param by Second corner y coordinate.
 * @param cx Third corner x coordinate.
 * @param cy Third corner y coordinate.
 * @param dx Fourth corner x coordinate.
 * @param dy Fourth corner y coordinate.
 */
void wlf_shape_add_quad(struct wlf_shape_vertices *vertices,
	double ax, double ay, double bx, double by,
	double cx, double cy, double dx, double dy);

/**
 * @brief Appends a quadrilateral with per-vertex coverage values.
 * @param vertices Vertex buffer receiving the quadrilateral.
 * @param ax First corner x coordinate.
 * @param ay First corner y coordinate.
 * @param ac First corner coverage.
 * @param bx Second corner x coordinate.
 * @param by Second corner y coordinate.
 * @param bc Second corner coverage.
 * @param cx Third corner x coordinate.
 * @param cy Third corner y coordinate.
 * @param cc Third corner coverage.
 * @param dx Fourth corner x coordinate.
 * @param dy Fourth corner y coordinate.
 * @param dc Fourth corner coverage.
 */
void wlf_shape_add_quad_coverage(struct wlf_shape_vertices *vertices,
	double ax, double ay, float ac, double bx, double by, float bc,
	double cx, double cy, float cc, double dx, double dy, float dc);

/**
 * @brief Appends an antialiased rectangular segment of the requested width.
 * @param vertices Vertex buffer receiving the segment.
 * @param x1 Segment start x coordinate.
 * @param y1 Segment start y coordinate.
 * @param x2 Segment end x coordinate.
 * @param y2 Segment end y coordinate.
 * @param width Segment width.
 */
void wlf_shape_add_segment(struct wlf_shape_vertices *vertices,
	double x1, double y1, double x2, double y2, double width);

/**
 * @brief Triangulates and appends a filled polygon with an origin offset.
 * @param vertices Vertex buffer receiving the polygon.
 * @param points Interleaved x/y polygon points.
 * @param count Number of points in @p points.
 * @param ox x offset applied to every point.
 * @param oy y offset applied to every point.
 */
void wlf_shape_add_polygon_fill(struct wlf_shape_vertices *vertices,
	const float *points, int count, double ox, double oy);

/**
 * @brief Appends an antialiased stroke around a polygon.
 * @param vertices Vertex buffer receiving the stroke.
 * @param points Interleaved x/y polygon points.
 * @param count Number of points in @p points.
 * @param closed Whether to connect the last point to the first point.
 * @param width Stroke width.
 * @param ox x offset applied to every point.
 * @param oy y offset applied to every point.
 */
void wlf_shape_add_polygon_stroke(struct wlf_shape_vertices *vertices,
	const float *points, int count, bool closed,
	double width, double ox, double oy);

/**
 * @brief Appends the antialiased outer fringe of a polygon.
 * @param vertices Vertex buffer receiving the fringe.
 * @param points Interleaved x/y polygon points.
 * @param count Number of points in @p points.
 * @param ox x offset applied to every point.
 * @param oy y offset applied to every point.
 * @param width Fringe width.
 */
void wlf_shape_add_polygon_fringe(struct wlf_shape_vertices *vertices,
	const float *points, int count, double ox, double oy, double width);

/**
 * @brief Generates points approximating a rectangle with rounded corners.
 * @param rect Rectangle geometry to approximate.
 * @param points Output interleaved x/y point array.
 * @param corner_segments Number of line segments per rounded corner.
 * @return Number of points written to @p points.
 */
int wlf_shape_rounded_rect_points(const struct wlf_rect_shape *rect,
	float *points, int corner_segments);

/**
 * @brief Submits generated shape vertices to a vector pass.
 * @param pass Vector pass receiving the geometry.
 * @param target Destination render target.
 * @param vertices Generated vertices to submit.
 * @param color Base shape color.
 * @param alpha Additional shape opacity multiplier.
 * @param clip Optional clip region.
 * @param blend_mode Compositing mode.
 */
void wlf_shape_submit(struct wlf_vector_pass *pass,
	struct wlf_render_target_info *target,
	const struct wlf_shape_vertices *vertices, struct wlf_color color,
	float alpha, const pixman_region32_t *clip,
	enum wlf_render_blend_mode blend_mode);

#endif
