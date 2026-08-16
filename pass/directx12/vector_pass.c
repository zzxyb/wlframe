#include "vector_pass.h"
#include "solid.h"
#include <stdlib.h>

struct vector_pass { struct wlf_vector_pass base; struct wlf_dx12_solid_pipeline *pipeline; };
static void destroy(struct wlf_vector_pass *base) {
	struct vector_pass *pass=(struct vector_pass *)base;
	wlf_dx12_solid_pipeline_destroy(pass->pipeline); free(pass);
}
static void render(struct wlf_vector_pass *base,
		struct wlf_render_target_info *target,
		const struct wlf_render_vector_options *options) {
	struct vector_pass *pass=(struct vector_pass *)base;
	wlf_dx12_solid_draw(pass->pipeline,target,options->vertices,
		options->vertex_count,options->color,options->clip,options->blend_mode);
}
static const struct wlf_vector_pass_impl impl={destroy,render};
struct wlf_vector_pass *wlf_dx12_vector_pass_create(struct wlf_renderer *renderer) {
	struct vector_pass *pass=calloc(1,sizeof(*pass));if(!pass)return NULL;
	pass->pipeline=wlf_dx12_solid_pipeline_create(
		wlf_dx12_renderer_from_renderer(renderer));
	if(!pass->pipeline){free(pass);return NULL;}
	wlf_render_vector_pass_init(&pass->base,&impl);return &pass->base;
}
