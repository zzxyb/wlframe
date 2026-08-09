#ifndef PASS_WLF_VECTOR_PASS_H
#define PASS_WLF_VECTOR_PASS_H

#include "wlf/pass/wlf_pass.h"
#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/types/wlf_color.h"
#include "wlf/utils/wlf_signal.h"

#include <pixman.h>
#include <stddef.h>

struct wlf_vector_vertex {
	float x, y;
};

/** Options for drawing a list of independent triangles. */
struct wlf_render_vector_options {
	const struct wlf_vector_vertex *vertices;
	size_t vertex_count; /**< Must be a multiple of three. */
	struct wlf_color color;
	const pixman_region32_t *clip;
	enum wlf_render_blend_mode blend_mode;
};

struct wlf_vector_pass;

struct wlf_vector_pass_impl {
	void (*destroy)(struct wlf_vector_pass *pass);
	void (*render)(struct wlf_vector_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_vector_options *options);
};

struct wlf_vector_pass {
	const struct wlf_vector_pass_impl *impl;
	struct {
		struct wlf_signal destroy;
	} events;
};

void wlf_render_vector_pass_init(struct wlf_vector_pass *pass,
	const struct wlf_vector_pass_impl *impl);
void wlf_render_vector_pass_destroy(struct wlf_vector_pass *pass);
void wlf_render_pass_add_triangles(struct wlf_vector_pass *pass,
	struct wlf_render_target_info *render_target_info,
	const struct wlf_render_vector_options *options);

#endif // PASS_WLF_VECTOR_PASS_H
