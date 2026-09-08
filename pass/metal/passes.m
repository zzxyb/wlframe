#include "wlf/pass/metal/rect_pass.h"
#include "wlf/pass/metal/render_target_info.h"
#include "wlf/pass/metal/texture_pass.h"
#include "wlf/pass/metal/vector_pass.h"

#include "wlf/buffer/metal/buffer.h"
#include "wlf/renderer/metal/device.h"
#include "wlf/renderer/metal/renderer.h"
#include "wlf/texture/metal/texture.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <Metal/Metal.h>

#include <math.h>
#include <stdlib.h>

static NSString *const shader_source = @
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct SolidVertex { packed_float2 position; float coverage; };\n"
"struct TextureVertex { packed_float2 position; packed_float2 texcoord; };\n"
"struct SolidOut { float4 position [[position]]; float coverage; };\n"
"struct TextureOut { float4 position [[position]]; float2 texcoord; };\n"
"vertex SolidOut solid_vertex(const device SolidVertex *v [[buffer(0)]], "
"constant packed_float2 &viewport [[buffer(1)]], uint id [[vertex_id]]) { "
"SolidOut o; float2 p = v[id].position / float2(viewport); "
"o.position=float4(p.x*2.0-1.0, 1.0-p.y*2.0, 0, 1); "
"o.coverage=v[id].coverage; return o; }\n"
"fragment float4 solid_fragment(SolidOut in [[stage_in]], "
"constant float4 &color [[buffer(0)]]) { return color * in.coverage; }\n"
"vertex TextureOut texture_vertex(const device TextureVertex *v [[buffer(0)]], "
"constant packed_float2 &viewport [[buffer(1)]], uint id [[vertex_id]]) { "
"TextureOut o; float2 p=v[id].position / float2(viewport); "
"o.position=float4(p.x*2.0-1.0, 1.0-p.y*2.0, 0, 1); "
"o.texcoord=v[id].texcoord; return o; }\n"
"fragment float4 texture_fragment(TextureOut in [[stage_in]], "
"texture2d<float> tex [[texture(0)]], sampler smp [[sampler(0)]], "
"constant float &opacity [[buffer(0)]], constant uint &gray [[buffer(1)]]) { "
"float4 c=tex.sample(smp,in.texcoord); if(gray != 0) c=float4(c.rrr,1); "
"return c * opacity; }\n";

struct pipeline_set {
	void *library;
	void *solid_blend;
	void *solid_copy;
	void *texture_blend;
	void *texture_copy;
	MTLPixelFormat format;
};

static void pipeline_set_finish(struct pipeline_set *set) {
	[(id<MTLRenderPipelineState>)set->solid_blend release];
	[(id<MTLRenderPipelineState>)set->solid_copy release];
	[(id<MTLRenderPipelineState>)set->texture_blend release];
	[(id<MTLRenderPipelineState>)set->texture_copy release];
	[(id<MTLLibrary>)set->library release];
}

static id<MTLRenderPipelineState> create_pipeline(id<MTLDevice> device,
		id<MTLLibrary> library, NSString *vertex_name, NSString *fragment_name,
		MTLPixelFormat format, bool blend) {
	id<MTLFunction> vertex = [library newFunctionWithName:vertex_name];
	id<MTLFunction> fragment = [library newFunctionWithName:fragment_name];
	if (vertex == nil || fragment == nil) {
		[vertex release];
		[fragment release];
		return nil;
	}
	MTLRenderPipelineDescriptor *descriptor =
		[[MTLRenderPipelineDescriptor alloc] init];
	descriptor.vertexFunction = vertex;
	descriptor.fragmentFunction = fragment;
	descriptor.colorAttachments[0].pixelFormat = format;
	descriptor.colorAttachments[0].blendingEnabled = blend;
	if (blend) {
		descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
		descriptor.colorAttachments[0].destinationRGBBlendFactor =
			MTLBlendFactorOneMinusSourceAlpha;
		descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
		descriptor.colorAttachments[0].destinationAlphaBlendFactor =
			MTLBlendFactorOneMinusSourceAlpha;
	}
	NSError *error = nil;
	id<MTLRenderPipelineState> pipeline =
		[device newRenderPipelineStateWithDescriptor:descriptor error:&error];
	if (pipeline == nil) {
		wlf_log(WLF_ERROR, "failed to create Metal pipeline: %s",
			[[error localizedDescription] UTF8String]);
	}
	[descriptor release];
	[vertex release];
	[fragment release];
	return pipeline;
}

static bool pipeline_set_init(struct pipeline_set *set,
		struct wlf_mtl_renderer *renderer, MTLPixelFormat format) {
	if (set->library != NULL && set->format == format) {
		return set->solid_blend != NULL && set->solid_copy != NULL &&
			set->texture_blend != NULL && set->texture_copy != NULL;
	}
	if (set->library != NULL) {
		pipeline_set_finish(set);
		*set = (struct pipeline_set){0};
	}
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->dev->device;
	NSError *error = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:shader_source
		options:nil error:&error];
	if (library == nil) {
		wlf_log(WLF_ERROR, "failed to compile Metal shaders: %s",
			[[error localizedDescription] UTF8String]);
		return false;
	}
	set->library = (__bridge void *)library;
	set->format = format;
	set->solid_blend = (__bridge void *)create_pipeline(device, library,
		@"solid_vertex", @"solid_fragment", format, true);
	set->solid_copy = (__bridge void *)create_pipeline(device, library,
		@"solid_vertex", @"solid_fragment", format, false);
	set->texture_blend = (__bridge void *)create_pipeline(device, library,
		@"texture_vertex", @"texture_fragment", format, true);
	set->texture_copy = (__bridge void *)create_pipeline(device, library,
		@"texture_vertex", @"texture_fragment", format, false);
	bool ok = set->solid_blend != NULL && set->solid_copy != NULL &&
		set->texture_blend != NULL && set->texture_copy != NULL;
	if (!ok) {
		pipeline_set_finish(set);
		*set = (struct pipeline_set){0};
	}
	return ok;
}

static id<MTLRenderCommandEncoder> target_encoder(
		struct wlf_render_target_info *base, struct pipeline_set *pipelines,
		struct wlf_mtl_renderer *renderer) {
	if (!wlf_render_target_info_is_mtl(base)) {
		return nil;
	}
	struct wlf_mtl_render_target_info *target =
		wlf_mtl_render_target_info_from_info(base);
	id<MTLTexture> texture =
		(__bridge id<MTLTexture>)wlf_mtl_buffer_get_texture(target->buffer);
	if (target->renderer != renderer ||
			!pipeline_set_init(pipelines, renderer, texture.pixelFormat)) {
		return nil;
	}
	return (__bridge id<MTLRenderCommandEncoder>)target->encoder;
}

static bool set_scissor(id<MTLRenderCommandEncoder> encoder,
		const pixman_box32_t *rect, const struct wlf_render_target_info *target) {
	int x1 = (int)floor(rect->x1 * target->scale);
	int y1 = (int)floor(rect->y1 * target->scale);
	int x2 = (int)ceil(rect->x2 * target->scale);
	int y2 = (int)ceil(rect->y2 * target->scale);
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > target->buffer_width) x2 = target->buffer_width;
	if (y2 > target->buffer_height) y2 = target->buffer_height;
	if (x2 <= x1 || y2 <= y1) {
		return false;
	}
	[encoder setScissorRect:(MTLScissorRect){
		.x = (NSUInteger)x1, .y = (NSUInteger)y1,
		.width = (NSUInteger)(x2 - x1), .height = (NSUInteger)(y2 - y1),
	}];
	return true;
}

static void draw_with_clip(id<MTLRenderCommandEncoder> encoder,
		const pixman_region32_t *clip, struct wlf_render_target_info *target,
		MTLPrimitiveType type, NSUInteger count) {
	if (clip == NULL) {
		[encoder setScissorRect:(MTLScissorRect){
			.width = (NSUInteger)target->buffer_width,
			.height = (NSUInteger)target->buffer_height,
		}];
		[encoder drawPrimitives:type vertexStart:0 vertexCount:count];
		return;
	}
	int nrects = 0;
	pixman_box32_t *rects = pixman_region32_rectangles(
		(pixman_region32_t *)clip, &nrects);
	for (int i = 0; i < nrects; ++i) {
		if (set_scissor(encoder, &rects[i], target)) {
			[encoder drawPrimitives:type vertexStart:0 vertexCount:count];
		}
	}
}

struct solid_vertex { float x, y, coverage; };
struct texture_vertex { float x, y, u, v; };

struct wlf_mtl_rect_pass {
	struct wlf_rect_pass base;
	struct wlf_mtl_renderer *renderer;
	struct pipeline_set pipelines;
};

static void rect_destroy(struct wlf_rect_pass *base) {
	struct wlf_mtl_rect_pass *pass = wlf_container_of(base, pass, base);
	pipeline_set_finish(&pass->pipelines);
	free(pass);
}

static void rect_render(struct wlf_rect_pass *base,
		struct wlf_render_target_info *target,
		const struct wlf_render_rect_options *options) {
	struct wlf_mtl_rect_pass *pass = wlf_container_of(base, pass, base);
	id<MTLRenderCommandEncoder> encoder = target_encoder(target,
		&pass->pipelines, pass->renderer);
	if (encoder == nil || options->box.width <= 0 || options->box.height <= 0) {
		return;
	}
	float x1 = options->box.x, y1 = options->box.y;
	float x2 = x1 + options->box.width, y2 = y1 + options->box.height;
	struct solid_vertex vertices[6] = {
		{x1, y1, 1}, {x2, y1, 1}, {x1, y2, 1},
		{x1, y2, 1}, {x2, y1, 1}, {x2, y2, 1},
	};
	struct wlf_color color = wlf_color_clamp(&options->color);
	float rgba[4] = {(float)(color.r * color.a),
		(float)(color.g * color.a), (float)(color.b * color.a),
		(float)color.a};
	float viewport[2] = {(float)target->logical_width,
		(float)target->logical_height};
	[encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)(
		options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		pass->pipelines.solid_copy : pass->pipelines.solid_blend)];
	[encoder setVertexBytes:vertices length:sizeof(vertices) atIndex:0];
	[encoder setVertexBytes:viewport length:sizeof(viewport) atIndex:1];
	[encoder setFragmentBytes:rgba length:sizeof(rgba) atIndex:0];
	draw_with_clip(encoder, options->clip, target, MTLPrimitiveTypeTriangle, 6);
}

static const struct wlf_rect_pass_impl rect_impl = {
	.destroy = rect_destroy,
	.render = rect_render,
};

struct wlf_rect_pass *wlf_mtl_rect_pass_create(
		struct wlf_mtl_renderer *renderer) {
	struct wlf_mtl_rect_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) return NULL;
	pass->renderer = renderer;
	wlf_rect_pass_init(&pass->base, &rect_impl);
	return &pass->base;
}

struct wlf_mtl_vector_pass {
	struct wlf_vector_pass base;
	struct wlf_mtl_renderer *renderer;
	struct pipeline_set pipelines;
};

static void vector_destroy(struct wlf_vector_pass *base) {
	struct wlf_mtl_vector_pass *pass = wlf_container_of(base, pass, base);
	pipeline_set_finish(&pass->pipelines);
	free(pass);
}

static void vector_render(struct wlf_vector_pass *base,
		struct wlf_render_target_info *target,
		const struct wlf_vector_options *options) {
	struct wlf_mtl_vector_pass *pass = wlf_container_of(base, pass, base);
	id<MTLRenderCommandEncoder> encoder = target_encoder(target,
		&pass->pipelines, pass->renderer);
	if (encoder == nil || options->vertex_count == 0) return;
	struct wlf_color color = wlf_color_clamp(&options->color);
	float rgba[4] = {(float)(color.r * color.a),
		(float)(color.g * color.a), (float)(color.b * color.a),
		(float)color.a};
	float viewport[2] = {(float)target->logical_width,
		(float)target->logical_height};
	[encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)(
		options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		pass->pipelines.solid_copy : pass->pipelines.solid_blend)];
	[encoder setVertexBytes:options->vertices
		length:options->vertex_count * sizeof(*options->vertices) atIndex:0];
	[encoder setVertexBytes:viewport length:sizeof(viewport) atIndex:1];
	[encoder setFragmentBytes:rgba length:sizeof(rgba) atIndex:0];
	draw_with_clip(encoder, options->clip, target,
		MTLPrimitiveTypeTriangle, options->vertex_count);
}

static const struct wlf_vector_pass_impl vector_impl = {
	.destroy = vector_destroy,
	.render = vector_render,
};

struct wlf_vector_pass *wlf_mtl_vector_pass_create(
		struct wlf_mtl_renderer *renderer) {
	struct wlf_mtl_vector_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) return NULL;
	pass->renderer = renderer;
	wlf_vector_pass_init(&pass->base, &vector_impl);
	return &pass->base;
}

struct wlf_mtl_texture_pass {
	struct wlf_texture_pass base;
	struct wlf_mtl_renderer *renderer;
	struct pipeline_set pipelines;
	void *linear_sampler;
	void *nearest_sampler;
};

static void texture_destroy(struct wlf_texture_pass *base) {
	struct wlf_mtl_texture_pass *pass = wlf_container_of(base, pass, base);
	pipeline_set_finish(&pass->pipelines);
	[(id<MTLSamplerState>)pass->linear_sampler release];
	[(id<MTLSamplerState>)pass->nearest_sampler release];
	free(pass);
}

static void texture_render(struct wlf_texture_pass *base,
		struct wlf_render_target_info *target,
		const struct wlf_render_texture_options *options) {
	struct wlf_mtl_texture_pass *pass = wlf_container_of(base, pass, base);
	if (!wlf_texture_is_mtl(options->texture) || options->opacity <= 0) return;
	struct wlf_mtl_texture *texture =
		wlf_mtl_texture_from_texture(options->texture);
	if (texture->renderer != pass->renderer) return;
	id<MTLRenderCommandEncoder> encoder = target_encoder(target,
		&pass->pipelines, pass->renderer);
	if (encoder == nil) return;
	struct wlf_frect src, dst;
	wlf_render_texture_options_get_src_box(options, &src);
	wlf_render_texture_options_get_dst_box(options, &dst);
	if (src.width <= 0 || src.height <= 0 || dst.width <= 0 || dst.height <= 0) {
		return;
	}
	float x1 = dst.x, y1 = dst.y, x2 = x1 + dst.width, y2 = y1 + dst.height;
	float u1 = src.x / texture->base.width;
	float v1 = src.y / texture->base.height;
	float u2 = (src.x + src.width) / texture->base.width;
	float v2 = (src.y + src.height) / texture->base.height;
	struct texture_vertex vertices[6] = {
		{x1,y1,u1,v1}, {x2,y1,u2,v1}, {x1,y2,u1,v2},
		{x1,y2,u1,v2}, {x2,y1,u2,v1}, {x2,y2,u2,v2},
	};
	float viewport[2] = {(float)target->logical_width,
		(float)target->logical_height};
	float opacity = options->opacity;
	uint32_t gray = texture->format == WLF_FORMAT_R8;
	[encoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)(
		options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		pass->pipelines.texture_copy : pass->pipelines.texture_blend)];
	[encoder setVertexBytes:vertices length:sizeof(vertices) atIndex:0];
	[encoder setVertexBytes:viewport length:sizeof(viewport) atIndex:1];
	[encoder setFragmentTexture:(__bridge id<MTLTexture>)texture->texture atIndex:0];
	[encoder setFragmentSamplerState:(__bridge id<MTLSamplerState>)(
		options->filter_mode == WLF_SCALE_FILTER_NEAREST ?
		pass->nearest_sampler : pass->linear_sampler) atIndex:0];
	[encoder setFragmentBytes:&opacity length:sizeof(opacity) atIndex:0];
	[encoder setFragmentBytes:&gray length:sizeof(gray) atIndex:1];
	draw_with_clip(encoder, options->clip, target, MTLPrimitiveTypeTriangle, 6);
}

static const struct wlf_texture_pass_impl texture_impl = {
	.destroy = texture_destroy,
	.render = texture_render,
};

static id<MTLSamplerState> create_sampler(id<MTLDevice> device,
		MTLSamplerMinMagFilter filter) {
	MTLSamplerDescriptor *descriptor = [[MTLSamplerDescriptor alloc] init];
	descriptor.minFilter = filter;
	descriptor.magFilter = filter;
	descriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
	descriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:descriptor];
	[descriptor release];
	return sampler;
}

struct wlf_texture_pass *wlf_mtl_texture_pass_create(
		struct wlf_mtl_renderer *renderer) {
	struct wlf_mtl_texture_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) return NULL;
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->dev->device;
	pass->linear_sampler = (__bridge void *)create_sampler(device,
		MTLSamplerMinMagFilterLinear);
	pass->nearest_sampler = (__bridge void *)create_sampler(device,
		MTLSamplerMinMagFilterNearest);
	if (pass->linear_sampler == NULL || pass->nearest_sampler == NULL) {
		texture_destroy(&pass->base);
		return NULL;
	}
	pass->renderer = renderer;
	wlf_render_texture_pass_init(&pass->base, &texture_impl);
	return &pass->base;
}
