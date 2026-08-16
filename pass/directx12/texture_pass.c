#include "texture_pass.h"

#include "render_target_info.h"
#include "texture/directx12/texture.h"
#include "wlf/renderer/directx12/renderer.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <d3dcompiler.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct texture_vertex {
	float x, y;
	float u, v;
};

struct dx12_texture_pass {
	struct wlf_texture_pass base;
	struct wlf_dx12_renderer *renderer;
	ID3D12RootSignature *root_signature;
	ID3D12PipelineState *opaque_pipeline;
	ID3D12PipelineState *blend_pipeline;
	ID3D12DescriptorHeap *sampler_heap;
	UINT sampler_increment;
};

static const char texture_shader[] =
	"cbuffer Params : register(b0) { float4 target; float opacity; };"
	"Texture2D image : register(t0); SamplerState image_sampler : register(s0);"
	"struct Input { float2 position : POSITION; float2 uv : TEXCOORD; };"
	"struct Output { float4 position : SV_POSITION; float2 uv : TEXCOORD; };"
	"Output vs_main(Input i) { Output o; o.position=float4("
	"i.position.x/target.x*2-1,1-i.position.y/target.y*2,0,1);"
	"o.uv=i.uv; return o; }"
	"float4 ps_main(Output i) : SV_TARGET {"
	"return image.Sample(image_sampler,i.uv)*opacity; }";

static bool compile_shader(const char *entry, const char *target,
		ID3DBlob **blob) {
	ID3DBlob *errors = NULL;
	HRESULT hr = D3DCompile(texture_shader, sizeof(texture_shader) - 1, NULL,
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
		struct dx12_texture_pass *pass, bool blend) {
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
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {
		.pRootSignature = pass->root_signature,
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
	ID3D12PipelineState *pipeline = NULL;
	HRESULT hr = ID3D12Device_CreateGraphicsPipelineState(pass->renderer->device,
		&desc, &IID_ID3D12PipelineState, (void **)&pipeline);
	ID3D10Blob_Release(vs);
	ID3D10Blob_Release(ps);
	return SUCCEEDED(hr) ? pipeline : NULL;
}

static bool create_root_signature(struct dx12_texture_pass *pass) {
	D3D12_DESCRIPTOR_RANGE ranges[] = {
		{.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			.NumDescriptors = 1, .BaseShaderRegister = 0,
			.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
		{.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,
			.NumDescriptors = 1, .BaseShaderRegister = 0,
			.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND},
	};
	D3D12_ROOT_PARAMETER parameters[] = {
		{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
			.Constants = {.ShaderRegister = 0, .Num32BitValues = 5},
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL},
		{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			.DescriptorTable = {.NumDescriptorRanges = 1, .pDescriptorRanges = &ranges[0]},
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL},
		{.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
			.DescriptorTable = {.NumDescriptorRanges = 1, .pDescriptorRanges = &ranges[1]},
			.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL},
	};
	D3D12_ROOT_SIGNATURE_DESC desc = {
		.NumParameters = 3,
		.pParameters = parameters,
		.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
	};
	ID3DBlob *signature = NULL, *errors = NULL;
	HRESULT hr = D3D12SerializeRootSignature(&desc,
		D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
	if (SUCCEEDED(hr)) {
		hr = ID3D12Device_CreateRootSignature(pass->renderer->device, 0,
			ID3D10Blob_GetBufferPointer(signature),
			ID3D10Blob_GetBufferSize(signature), &IID_ID3D12RootSignature,
			(void **)&pass->root_signature);
	}
	if (signature != NULL) ID3D10Blob_Release(signature);
	if (errors != NULL) ID3D10Blob_Release(errors);
	return SUCCEEDED(hr);
}

static bool create_samplers(struct dx12_texture_pass *pass) {
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		.NumDescriptors = 2,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	HRESULT hr = ID3D12Device_CreateDescriptorHeap(pass->renderer->device,
		&heap_desc, &IID_ID3D12DescriptorHeap, (void **)&pass->sampler_heap);
	if (FAILED(hr)) return false;
	pass->sampler_increment = ID3D12Device_GetDescriptorHandleIncrementSize(
		pass->renderer->device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(
		pass->sampler_heap, &handle);
	D3D12_SAMPLER_DESC sampler = {
		.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		.MaxLOD = D3D12_FLOAT32_MAX,
	};
	ID3D12Device_CreateSampler(pass->renderer->device, &sampler, handle);
	handle.ptr += pass->sampler_increment;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	ID3D12Device_CreateSampler(pass->renderer->device, &sampler, handle);
	return true;
}

static void pass_destroy(struct wlf_texture_pass *base) {
	struct dx12_texture_pass *pass = (struct dx12_texture_pass *)base;
	if (pass->sampler_heap != NULL)
		ID3D12DescriptorHeap_Release(pass->sampler_heap);
	if (pass->blend_pipeline != NULL)
		ID3D12PipelineState_Release(pass->blend_pipeline);
	if (pass->opaque_pipeline != NULL)
		ID3D12PipelineState_Release(pass->opaque_pipeline);
	if (pass->root_signature != NULL)
		ID3D12RootSignature_Release(pass->root_signature);
	free(pass);
}

static void pass_render(struct wlf_texture_pass *base,
		struct wlf_render_target_info *target_base,
		const struct wlf_render_texture_options *options) {
	struct dx12_texture_pass *pass = (struct dx12_texture_pass *)base;
	if (!wlf_render_target_info_is_dx12(target_base) ||
			!wlf_texture_is_dx12(options->texture) || options->opacity <= 0.0f) {
		return;
	}
	struct wlf_frect src, dst;
	wlf_render_texture_options_get_src_box(options, &src);
	wlf_render_texture_options_get_dst_box(options, &dst);
	if (src.width <= 0 || src.height <= 0 || dst.width <= 0 || dst.height <= 0)
		return;

	struct wlf_dx12_texture *texture =
		wlf_dx12_texture_from_texture(options->texture);
	float x1 = (float)dst.x, y1 = (float)dst.y;
	float x2 = (float)(dst.x + dst.width), y2 = (float)(dst.y + dst.height);
	float u1 = (float)(src.x / texture->base.width);
	float v1 = (float)(src.y / texture->base.height);
	float u2 = (float)((src.x + src.width) / texture->base.width);
	float v2 = (float)((src.y + src.height) / texture->base.height);
	struct texture_vertex vertices[] = {
		{x1, y1, u1, v1}, {x2, y1, u2, v1}, {x2, y2, u2, v2},
		{x1, y1, u1, v1}, {x2, y2, u2, v2}, {x1, y2, u1, v2},
	};
	D3D12_HEAP_PROPERTIES heap = {.Type = D3D12_HEAP_TYPE_UPLOAD};
	D3D12_RESOURCE_DESC desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = sizeof(vertices), .Height = 1, .DepthOrArraySize = 1,
		.MipLevels = 1, .SampleDesc = {.Count = 1},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	};
	ID3D12Resource *upload = NULL;
	HRESULT hr = ID3D12Device_CreateCommittedResource(pass->renderer->device,
		&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL, &IID_ID3D12Resource, (void **)&upload);
	if (FAILED(hr)) return;
	void *mapped = NULL;
	D3D12_RANGE read_range = {0, 0};
	hr = ID3D12Resource_Map(upload, 0, &read_range, &mapped);
	if (FAILED(hr)) { ID3D12Resource_Release(upload); return; }
	memcpy(mapped, vertices, sizeof(vertices));
	ID3D12Resource_Unmap(upload, 0, NULL);
	struct wlf_dx12_render_target_info *target =
		wlf_dx12_render_target_from_info(target_base);
	if (!wlf_dx12_render_target_retain_resource(target, upload)) {
		ID3D12Resource_Release(upload);
		return;
	}

	D3D12_VERTEX_BUFFER_VIEW view = {
		.BufferLocation = ID3D12Resource_GetGPUVirtualAddress(upload),
		.SizeInBytes = sizeof(vertices),
		.StrideInBytes = sizeof(vertices[0]),
	};
	D3D12_VIEWPORT viewport = {0, 0, (float)target_base->buffer_width,
		(float)target_base->buffer_height, 0, 1};
	ID3D12GraphicsCommandList_RSSetViewports(target->commands, 1, &viewport);
	ID3D12GraphicsCommandList_SetGraphicsRootSignature(target->commands,
		pass->root_signature);
	ID3D12GraphicsCommandList_SetPipelineState(target->commands,
		options->blend_mode == WLF_RENDER_BLEND_MODE_NONE ?
		pass->opaque_pipeline : pass->blend_pipeline);
	ID3D12GraphicsCommandList_IASetPrimitiveTopology(target->commands,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D12GraphicsCommandList_IASetVertexBuffers(target->commands, 0, 1, &view);
	ID3D12DescriptorHeap *heaps[] = {texture->srv_heap, pass->sampler_heap};
	ID3D12GraphicsCommandList_SetDescriptorHeaps(target->commands, 2, heaps);
	D3D12_GPU_DESCRIPTOR_HANDLE srv;
	ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(texture->srv_heap, &srv);
	D3D12_GPU_DESCRIPTOR_HANDLE sampler;
	ID3D12DescriptorHeap_GetGPUDescriptorHandleForHeapStart(pass->sampler_heap,
		&sampler);
	if (options->filter_mode != WLF_SCALE_FILTER_NEAREST)
		sampler.ptr += pass->sampler_increment;
	ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable(target->commands, 1,
		srv);
	ID3D12GraphicsCommandList_SetGraphicsRootDescriptorTable(target->commands, 2,
		sampler);
	float constants[] = {(float)target_base->logical_width,
		(float)target_base->logical_height, 0.0f, 0.0f, options->opacity};
	ID3D12GraphicsCommandList_SetGraphicsRoot32BitConstants(target->commands, 0,
		5, constants, 0);

	int logical_x = (int)floor(dst.x), logical_y = (int)floor(dst.y);
	int logical_width = (int)ceil(dst.x + dst.width) - logical_x;
	int logical_height = (int)ceil(dst.y + dst.height) - logical_y;
	pixman_region32_t logical, clipped, scaled;
	pixman_region32_init_rect(&logical, logical_x, logical_y,
		logical_width, logical_height);
	pixman_region32_init(&clipped);
	if (options->clip != NULL)
		pixman_region32_intersect(&clipped, &logical, options->clip);
	else pixman_region32_copy(&clipped, &logical);
	pixman_region32_init(&scaled);
	wlf_render_target_info_scale_region(target_base, &clipped, &scaled);
	int count = 0;
	pixman_box32_t *boxes = pixman_region32_rectangles(&scaled, &count);
	for (int i = 0; i < count; ++i) {
		D3D12_RECT scissor = {boxes[i].x1, boxes[i].y1,
			boxes[i].x2, boxes[i].y2};
		ID3D12GraphicsCommandList_RSSetScissorRects(target->commands, 1,
			&scissor);
		ID3D12GraphicsCommandList_DrawInstanced(target->commands, 6, 1, 0, 0);
	}
	pixman_region32_fini(&scaled);
	pixman_region32_fini(&clipped);
	pixman_region32_fini(&logical);
}

static const struct wlf_texture_pass_impl pass_impl = {
	.destroy = pass_destroy,
	.render = pass_render,
};

struct wlf_texture_pass *wlf_dx12_texture_pass_create(
		struct wlf_renderer *renderer) {
	if (!wlf_renderer_is_dx12(renderer)) return NULL;
	struct dx12_texture_pass *pass = calloc(1, sizeof(*pass));
	if (pass == NULL) return NULL;
	pass->renderer = wlf_dx12_renderer_from_renderer(renderer);
	if (!create_root_signature(pass) || !create_samplers(pass)) goto error;
	pass->opaque_pipeline = create_pipeline(pass, false);
	pass->blend_pipeline = create_pipeline(pass, true);
	if (pass->opaque_pipeline == NULL || pass->blend_pipeline == NULL) goto error;
	wlf_render_texture_pass_init(&pass->base, &pass_impl);
	return &pass->base;
error:
	pass_destroy(&pass->base);
	return NULL;
}
