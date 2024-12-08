#ifndef WLF_SCENE_SCENE_RECT_H
#define WLF_SCENE_SCENE_RECT_H

#include "wlf/utils/wlf_color.h"

struct wlf_scene_node;

struct wlf_scene_rect_border {
	struct wlf_color color; // color of the border
	int width; // width of the border
};

struct wlf_scene_rect {
	struct wlf_scene_node *base; // pointer to the scene node
	struct wlf_color color; // color of the rectangle
	float bottom_left_radius; // radius of the bottom left corner
	float bottom_right_radius; // radius of the bottom right corner
	float top_left_radius; // radius of the top left corner
	float top_right_radius; // radius of the top right corner
	struct wlf_scene_rect_border border; // border properties
};

struct wlf_scene_rect *wlf_scene_rect_from_node(struct wlf_scene_node *node);
struct wlf_scene_rect *wlf_scene_rect_create(struct wlf_scene_node *node);
void wlf_scene_rect_destroy(struct wlf_scene_rect *rect);
void wlf_scene_rect_set_color(struct wlf_scene_rect *rect, struct wlf_color color);
void wlf_scene_rect_set_radius(struct wlf_scene_rect *rect, float bottom_left_radius, float bottom_right_radius, float top_left_radius, float top_right_radius);
void wlf_scene_rect_set_border(struct wlf_scene_rect *rect, struct wlf_color color, int width);
void wlf_scene_rect_set_border_width(struct wlf_scene_rect *rect, int width);
void wlf_scene_rect_set_border_color(struct wlf_scene_rect *rect, struct wlf_color color);

#endif
