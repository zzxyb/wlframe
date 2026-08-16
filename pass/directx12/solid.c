#include "solid.h"

#include "wlf/utils/wlf_log.h"

#include <d3dcompiler.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct wlf_dx12_solid_pipeline {
	struct wlf_dx12_renderer *renderer;
	ID3D12RootSignature *root_signature;
	ID3D12PipelineState *opaque_pipeline;
	ID3D12PipelineState *blend_pipeline;
};

static const char solid_shader[] =
	"cbuffer Params : register(b0) { float4 target; float4 color; };"
	"struct Input { float2 position : POSITION; float coverage : COVERAGE; };"
	"struct Output { float4 position : SV_POSITION; float coverage : COVERAGE; };"
	"Output vs_main(Input i) { Output o; o.position=float4("
	"i.position.x/target.x*2-1,1-i.position.y/target.y*2,0,1);"
	"o.coverage=i.coverage; return o; }"
	"float4 ps_main(Output i) : SV_TARGET {"
	"return float4(color.rgb*color.a,color.a)*saturate(i.coverage); }";

static bool compile_shader(const char *entry, const char *target,
		ID3DBlob **blob) {
	ID3DBlob *errors = NULL;
	HRESULT hr = D3DCompile(solid_shader, sizeof(solid_shader) - 1, NULL,
		NULL, NULL, entry, target, D3DCOMPILE_ENABLE_STRICTNESS, 0,
		blob, &errors);
	if (FAILED(hr)) {
		const char *message = errors != NULL ?
			ID3D10Blob_GetBufferPointer(errors) : "unknown shader error";
		wlf_log(WLF_ERROR, "D3DCompile %s failed: %s", entry, message);
	}
	if (errors != NULL) ID3D10Blob_Release(errors);
	return SUCCEEDED(hr);
}

static ID3D12PipelineState *create_pipeline(
		struct wlf_dx12_solid_pipeline *pipeline, bool blend) {
	ID3DBlob *vs = NULL, *ps = NULL;
	if (!compile_shader("vs_main", "vs_5_1", &vs) ||
			!compile_shader("ps_main", "ps_5_1", &ps)) {
		if (vs != NULL) ID3D10Blob_Release(vs);
		if (ps != NULL) ID3D10Blob_Release(ps);
		return NULL;
	}
	D3D12_INPUT_ELEMENT_DESC elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COVERAGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
		.pRootSignature = pipeline->root_signature,
		.VS = {ID3D10Blob_GetBufferPointer(vs),
			ID3D10Blob_GetBufferSize(vs)},
		.PS = {ID3D10Blob_GetBufferPointer(ps),
			ID3D10Blob_GetBufferSize(ps)},
		.BlendState = {.RenderTarget = {{
			.BlendEnable = blend,
			.SrcBlend = D3D12_BLEND_ONE,
			.DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
			.BlendOp = D3D12_BLEND_OP_ADD,
			.SrcBlendAlpha = D3D12_BLEND_ONE,
			.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA,
			.BlendOpAlpha = D3D12_BLEND_OP_ADD,
			.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
		}}},
		.SampleMask = UINT_MAX,
		.RasterizerState = {
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_NONE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
			.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
		},
		.DepthStencilState = {.DepthEnable = FALSE, .StencilEnable = FALSE},
		.InputLayout = {elements, 2},
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.RTVFormats = {DXGI_FORMAT_B8G8R8A8_UNORM},
		.SampleDesc = {.Count = 1},
	};
	ID3D12PipelineState *state = NULL;
	HRESULT hr = ID3D12Device_CreateGraphicsPipelineState(
		pipeline->renderer->device, &desc, &IID_ID3D12PipelineState,
		(void **)&state);
	ID3D10Blob_Release(vs);
	ID3D10Blob_Release(ps);
	return SUCCEEDED(hr) ? state : NULL;
}

struct wlf_dx12_solid_pipeline *wlf_dx12_solid_pipeline_create(
		struct wlf_dx12_renderer *renderer) {
	struct wlf_dx12_solid_pipeline *pipeline = calloc(1, sizeof(*pipeline));
	if (pipeline == NULL) return NULL;
	pipeline->renderer = renderer;
	D3D12_ROOT_PARAMETER parameter = {
		.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
		.Constants = {.ShaderRegister = 0, .Num32BitValues = 8},
		.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
	};
	D3D12_ROOT_SIGNATURE_DESC desc = {
		.NumParameters = 1, .pParameters = &parameter,
		.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	};
	ID3DBlob *signature = NULL, *errors = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&desc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
	if (SUCCEEDED(hr)) {
		hr = ID3D12Device_CreateRootSignature(renderer->device, 0,
			ID3D10Blob_GetBufferPointer(signature),
			ID3D10Blob_GetBufferSize(signature), &IID_ID3D12RootSignature,
			(void **)&pipeline->root_signature);
	}
	if (signature != NULL) ID3D10Blob_Release(signature);
	if (errors != NULL) ID3D10Blob_Release(errors);
	if (FAILED(hr)) goto error;
	pipeline->opaque_pipeline = create_pipeline(pipeline, false);
	pipeline->blend_pipeline = create_pipeline(pipeline, true);
	if (pipeline->opaque_pipeline == NULL || pipeline->blend_pipeline == NULL)
		goto error;
	return pipeline;
error:
	wlf_dx12_solid_pipeline_destroy(pipeline);
	return NULL;
}

void wlf_dx12_solid_pipeline_destroy(
		struct wlf_dx12_solid_pipeline *pipeline) {
	if (pipeline == NULL) return;
	if (pipeline->blend_pipeline != NULL)
		ID3D12PipelineState_Release(pipeline->blend_pipeline);
	if (pipeline->opaque_pipeline != NULL)
		ID3D12PipelineState_Release(pipeline->opaque_pipeline);
	if (pipeline->root_signature != NULL)
		ID3D12RootSignature_Release(pipeline->root_signature);
	free(pipeline);
}

void wlf_dx12_solid_draw(struct wlf_dx12_solid_pipeline *pipeline,
		struct wlf_render_target_info *base,
		const struct wlf_vector_vertex *vertices, size_t vertex_count,
		struct wlf_color color, const pixman_region32_t *clip,
		enum wlf_render_blend_mode blend_mode) {
	if (!wlf_render_target_info_is_dx12(base) || vertex_count == 0) return;
	struct wlf_dx12_render_target_info *target =
		wlf_dx12_render_target_from_info(base);
	UINT64 size = vertex_count * sizeof(*vertices);
	D3D12_HEAP_PROPERTIES heap = {.Type = D3D12_HEAP_TYPE_UPLOAD};
	D3D12_RESOURCE_DESC resource_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = size, .Height = 1, .DepthOrArraySize = 1,
		.MipLevels = 1, .SampleDesc = {.Count = 1},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	};
	ID3D12Resource *upload = NULL;
	HRESULT hr = ID3D12Device_CreateCommittedResource(
		pipeline->renderer->device, &heap, D3D12_HEAP_FLAG_NONE,
		&resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		&IID_ID3D12Resource, (void **)&upload);
	if (FAILED(hr)) return;
	void *mapped = NULL;
	D3D12_RANGE read_range = {0, 0};
	hr = ID3D12Resource_Map(upload, 0, &read_range, &mapped);
	if (FAILED(hr)) { ID3D12Resource_Release(upload); return; }
	memcpy(mapped, vertices, (size_t)size);
	ID3D12Resource_Unmap(upload, 0, NULL);
	if (!wlf_dx12_render_target_retain_resource(target, upload)) {
		ID3D12Resource_Release(upload); return;
	}
	D3D12_VERTEX_BUFFER_VIEW view = {
		.BufferLocation = ID3D12Resource_GetGPUVirtualAddress(upload),
		.SizeInBytes = (UINT)size,
		.StrideInBytes = sizeof(*vertices),
	};
	D3D12_VIEWPORT viewport = {0, 0, (float)base->buffer_width,
		(float)base->buffer_height, 0, 1};
	ID3D12GraphicsCommandList_RSSetViewports(target->commands, 1, &viewport);
	ID3D12GraphicsCommandList_SetGraphicsRootSignature(target->commands,
		pipeline->root_signature);
	ID3D12GraphicsCommandList_SetPipelineState(target->commands,
		blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		pipeline->opaque_pipeline : pipeline->blend_pipeline);
	ID3D12GraphicsCommandList_IASetPrimitiveTopology(target->commands,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D12GraphicsCommandList_IASetVertexBuffers(target->commands, 0, 1, &view);
	float constants[] = {(float)base->logical_width,
		(float)base->logical_height, 0.0f, 0.0f,
		(float)color.r, (float)color.g, (float)color.b, (float)color.a};
	ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants(
		target->commands, 0, 8, constants, 0);
	pixman_region32_t scaled;
	pixman_region32_init(&scaled);
	if (clip != NULL) wlf_render_target_info_scale_region(base, clip, &scaled);
	else pixman_region32_union_rect(&scaled, &scaled, 0, 0,
		base->buffer_width, base->buffer_height);
	int count = 0;
	pixman_box32_t *boxes = pixman_region32_rectangles(&scaled, &count);
	for (int i = 0; i < count; ++i) {
		D3D12_RECT scissor = {boxes[i].x1, boxes[i].y1,
			boxes[i].x2, boxes[i].y2};
		ID3D12GraphicsCommandList_RSSetScissorRects(target->commands, 1,
			&scissor);
		ID3D12GraphicsCommandList_DrawInstanced(target->commands,
			(UINT)vertex_count, 1, 0, 0);
	}
	pixman_region32_fini(&scaled);
}
