#ifndef WLF_PASS_DIRECTX12_SOLID_H
#define WLF_PASS_DIRECTX12_SOLID_H

#include "render_target_info.h"
#include "wlf/pass/wlf_vector_pass.h"

struct wlf_dx12_solid_pipeline;

struct wlf_dx12_solid_pipeline *wlf_dx12_solid_pipeline_create(
	struct wlf_dx12_renderer *renderer);
void wlf_dx12_solid_pipeline_destroy(
	struct wlf_dx12_solid_pipeline *pipeline);
void wlf_dx12_solid_draw(struct wlf_dx12_solid_pipeline *pipeline,
	struct wlf_render_target_info *target,
	const struct wlf_vector_vertex *vertices, size_t vertex_count,
	struct wlf_color color, const pixman_region32_t *clip,
	enum wlf_render_blend_mode blend_mode);

#endif
