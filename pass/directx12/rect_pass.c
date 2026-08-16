#include "rect_pass.h"
#include "solid.h"
#include <stdlib.h>

struct rect_pass { struct wlf_rect_pass base; struct wlf_dx12_solid_pipeline *pipeline; };
static void destroy(struct wlf_rect_pass *base) {
	struct rect_pass *pass = (struct rect_pass *)base;
	wlf_dx12_solid_pipeline_destroy(pass->pipeline); free(pass);
}
static void render(struct wlf_rect_pass *base,
		struct wlf_render_target_info *target,
		const struct wlf_render_rect_options *options) {
	struct rect_pass *pass = (struct rect_pass *)base;
	float x1 = (float)options->box.x, y1 = (float)options->box.y;
	float x2 = x1 + (float)options->box.width;
	float y2 = y1 + (float)options->box.height;
	struct wlf_vector_vertex v[] = {
		{x1,y1,1},{x2,y1,1},{x2,y2,1},
		{x1,y1,1},{x2,y2,1},{x1,y2,1},
	};
	wlf_dx12_solid_draw(pass->pipeline,target,v,6,options->color,
		options->clip,options->blend_mode);
}
static const struct wlf_rect_pass_impl impl={destroy,render};
struct wlf_rect_pass *wlf_dx12_rect_pass_create(struct wlf_renderer *renderer) {
	struct rect_pass *pass=calloc(1,sizeof(*pass)); if(!pass)return NULL;
	pass->pipeline=wlf_dx12_solid_pipeline_create(
		wlf_dx12_renderer_from_renderer(renderer));
	if(!pass->pipeline){free(pass);return NULL;}
	wlf_render_rect_pass_init(&pass->base,&impl); return &pass->base;
}
