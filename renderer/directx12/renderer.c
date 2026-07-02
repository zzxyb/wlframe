#include "wlf/renderer/directx12/renderer.h"
#include "wlf/platform/windows/backend.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static const char *dx12_hresult_string(HRESULT hr) {
	switch (hr) {
	case S_OK:
		return "S_OK";
	case E_INVALIDARG:
		return "E_INVALIDARG";
	case E_OUTOFMEMORY:
		return "E_OUTOFMEMORY";
	case DXGI_ERROR_UNSUPPORTED:
		return "DXGI_ERROR_UNSUPPORTED";
	case DXGI_ERROR_NOT_FOUND:
		return "DXGI_ERROR_NOT_FOUND";
	default:
		return "HRESULT error";
	}
}

static void dx12_log_error(const char *func, HRESULT hr) {
	wlf_log(WLF_ERROR, "%s failed: %s (0x%08lx)",
		func, dx12_hresult_string(hr), (unsigned long)hr);
}

static const char *dx12_bool_str(BOOL value) {
	return value ? "yes" : "no";
}

static const char *dx12_feature_level_str(D3D_FEATURE_LEVEL level) {
	switch (level) {
	case D3D_FEATURE_LEVEL_12_2:
		return "D3D_FEATURE_LEVEL_12_2";
	case D3D_FEATURE_LEVEL_12_1:
		return "D3D_FEATURE_LEVEL_12_1";
	case D3D_FEATURE_LEVEL_12_0:
		return "D3D_FEATURE_LEVEL_12_0";
	case D3D_FEATURE_LEVEL_11_1:
		return "D3D_FEATURE_LEVEL_11_1";
	case D3D_FEATURE_LEVEL_11_0:
		return "D3D_FEATURE_LEVEL_11_0";
	default:
		return "unknown";
	}
}

static const char *dx12_shader_model_str(D3D_SHADER_MODEL shader_model) {
	switch (shader_model) {
	case D3D_SHADER_MODEL_5_1:
		return "D3D_SHADER_MODEL_5_1";
	case D3D_SHADER_MODEL_6_0:
		return "D3D_SHADER_MODEL_6_0";
	case (D3D_SHADER_MODEL)0x61:
		return "D3D_SHADER_MODEL_6_1";
	case (D3D_SHADER_MODEL)0x62:
		return "D3D_SHADER_MODEL_6_2";
	case (D3D_SHADER_MODEL)0x63:
		return "D3D_SHADER_MODEL_6_3";
	case (D3D_SHADER_MODEL)0x64:
		return "D3D_SHADER_MODEL_6_4";
	case (D3D_SHADER_MODEL)0x65:
		return "D3D_SHADER_MODEL_6_5";
	case (D3D_SHADER_MODEL)0x66:
		return "D3D_SHADER_MODEL_6_6";
	case (D3D_SHADER_MODEL)0x67:
		return "D3D_SHADER_MODEL_6_7";
	default:
		return "unknown";
	}
}

static const char *dx12_resource_binding_tier_str(
		D3D12_RESOURCE_BINDING_TIER tier) {
	switch (tier) {
	case D3D12_RESOURCE_BINDING_TIER_1:
		return "D3D12_RESOURCE_BINDING_TIER_1";
	case D3D12_RESOURCE_BINDING_TIER_2:
		return "D3D12_RESOURCE_BINDING_TIER_2";
	case D3D12_RESOURCE_BINDING_TIER_3:
		return "D3D12_RESOURCE_BINDING_TIER_3";
	default:
		return "unknown";
	}
}

static const char *dx12_resource_heap_tier_str(D3D12_RESOURCE_HEAP_TIER tier) {
	switch (tier) {
	case D3D12_RESOURCE_HEAP_TIER_1:
		return "D3D12_RESOURCE_HEAP_TIER_1";
	case D3D12_RESOURCE_HEAP_TIER_2:
		return "D3D12_RESOURCE_HEAP_TIER_2";
	default:
		return "unknown";
	}
}

static const char *dx12_tiled_resources_tier_str(
		D3D12_TILED_RESOURCES_TIER tier) {
	switch (tier) {
	case D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED:
		return "D3D12_TILED_RESOURCES_TIER_NOT_SUPPORTED";
	case D3D12_TILED_RESOURCES_TIER_1:
		return "D3D12_TILED_RESOURCES_TIER_1";
	case D3D12_TILED_RESOURCES_TIER_2:
		return "D3D12_TILED_RESOURCES_TIER_2";
	case D3D12_TILED_RESOURCES_TIER_3:
		return "D3D12_TILED_RESOURCES_TIER_3";
	case D3D12_TILED_RESOURCES_TIER_4:
		return "D3D12_TILED_RESOURCES_TIER_4";
	default:
		return "unknown";
	}
}

static const char *dx12_conservative_rasterization_tier_str(
		D3D12_CONSERVATIVE_RASTERIZATION_TIER tier) {
	switch (tier) {
	case D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED:
		return "D3D12_CONSERVATIVE_RASTERIZATION_TIER_NOT_SUPPORTED";
	case D3D12_CONSERVATIVE_RASTERIZATION_TIER_1:
		return "D3D12_CONSERVATIVE_RASTERIZATION_TIER_1";
	case D3D12_CONSERVATIVE_RASTERIZATION_TIER_2:
		return "D3D12_CONSERVATIVE_RASTERIZATION_TIER_2";
	case D3D12_CONSERVATIVE_RASTERIZATION_TIER_3:
		return "D3D12_CONSERVATIVE_RASTERIZATION_TIER_3";
	default:
		return "unknown";
	}
}

static const char *dx12_raytracing_tier_str(D3D12_RAYTRACING_TIER tier) {
	switch (tier) {
	case D3D12_RAYTRACING_TIER_NOT_SUPPORTED:
		return "D3D12_RAYTRACING_TIER_NOT_SUPPORTED";
	case D3D12_RAYTRACING_TIER_1_0:
		return "D3D12_RAYTRACING_TIER_1_0";
	case D3D12_RAYTRACING_TIER_1_1:
		return "D3D12_RAYTRACING_TIER_1_1";
	default:
		return "unknown";
	}
}

static const char *dx12_variable_shading_rate_tier_str(
		D3D12_VARIABLE_SHADING_RATE_TIER tier) {
	switch (tier) {
	case D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED:
		return "D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED";
	case D3D12_VARIABLE_SHADING_RATE_TIER_1:
		return "D3D12_VARIABLE_SHADING_RATE_TIER_1";
	case D3D12_VARIABLE_SHADING_RATE_TIER_2:
		return "D3D12_VARIABLE_SHADING_RATE_TIER_2";
	default:
		return "unknown";
	}
}

static void dx12_wide_to_utf8(const WCHAR *src, char *dst, size_t dst_len) {
	if (dst_len == 0) {
		return;
	}

	dst[0] = '\0';
	int len = WideCharToMultiByte(CP_UTF8, 0, src, -1, dst,
		(int)dst_len, NULL, NULL);
	if (len == 0) {
		snprintf(dst, dst_len, "unknown");
	}
}

static void dx12_log_renderer_info(struct wlf_dx12_renderer *renderer) {
	DXGI_ADAPTER_DESC1 desc;
	HRESULT hr = IDXGIAdapter1_GetDesc1(renderer->adapter, &desc);
	if (FAILED(hr)) {
		dx12_log_error("IDXGIAdapter1_GetDesc1", hr);
		return;
	}

	char adapter_name[256];
	dx12_wide_to_utf8(desc.Description, adapter_name, sizeof(adapter_name));

	wlf_log(WLF_INFO, "Creating DirectX 12 renderer");
	wlf_log(WLF_INFO, "DXGI adapter: %s", adapter_name);
	wlf_log(WLF_INFO,
		"DXGI adapter IDs: vendor=0x%04x device=0x%04x subsys=0x%08x revision=%u luid=%ld:%lu",
		desc.VendorId, desc.DeviceId, desc.SubSysId, desc.Revision,
		desc.AdapterLuid.HighPart, desc.AdapterLuid.LowPart);
	wlf_log(WLF_INFO,
		"DXGI adapter memory: dedicated_vram=%llu MiB dedicated_system=%llu MiB shared_system=%llu MiB",
		(unsigned long long)(desc.DedicatedVideoMemory / 1024 / 1024),
		(unsigned long long)(desc.DedicatedSystemMemory / 1024 / 1024),
		(unsigned long long)(desc.SharedSystemMemory / 1024 / 1024));
	wlf_log(WLF_INFO, "DXGI adapter flags: software=%s",
		dx12_bool_str((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0));

	IDXGIAdapter3 *adapter3 = NULL;
	hr = IDXGIAdapter1_QueryInterface(renderer->adapter,
		&IID_IDXGIAdapter3, (void **)&adapter3);
	if (SUCCEEDED(hr)) {
		DXGI_QUERY_VIDEO_MEMORY_INFO local_info;
		hr = IDXGIAdapter3_QueryVideoMemoryInfo(adapter3, 0,
			DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &local_info);
		if (SUCCEEDED(hr)) {
			wlf_log(WLF_INFO,
				"DXGI local memory budget: budget=%llu MiB current_usage=%llu MiB available_for_reservation=%llu MiB current_reservation=%llu MiB",
				(unsigned long long)(local_info.Budget / 1024 / 1024),
				(unsigned long long)(local_info.CurrentUsage / 1024 / 1024),
				(unsigned long long)(local_info.AvailableForReservation / 1024 / 1024),
				(unsigned long long)(local_info.CurrentReservation / 1024 / 1024));
		}

		DXGI_QUERY_VIDEO_MEMORY_INFO non_local_info;
		hr = IDXGIAdapter3_QueryVideoMemoryInfo(adapter3, 0,
			DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &non_local_info);
		if (SUCCEEDED(hr)) {
			wlf_log(WLF_INFO,
				"DXGI non-local memory budget: budget=%llu MiB current_usage=%llu MiB available_for_reservation=%llu MiB current_reservation=%llu MiB",
				(unsigned long long)(non_local_info.Budget / 1024 / 1024),
				(unsigned long long)(non_local_info.CurrentUsage / 1024 / 1024),
				(unsigned long long)(non_local_info.AvailableForReservation / 1024 / 1024),
				(unsigned long long)(non_local_info.CurrentReservation / 1024 / 1024));
		}
		IDXGIAdapter3_Release(adapter3);
	}

	D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_data = {
		.NumFeatureLevels = sizeof(feature_levels) / sizeof(feature_levels[0]),
		.pFeatureLevelsRequested = feature_levels,
		.MaxSupportedFeatureLevel = D3D_FEATURE_LEVEL_11_0,
	};
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_FEATURE_LEVELS, &feature_level_data,
		sizeof(feature_level_data));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO, "D3D12 feature level: %s",
			dx12_feature_level_str(feature_level_data.MaxSupportedFeatureLevel));
	}

	D3D12_FEATURE_DATA_SHADER_MODEL shader_model_data = {
		.HighestShaderModel = (D3D_SHADER_MODEL)0x67,
	};
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_SHADER_MODEL, &shader_model_data,
		sizeof(shader_model_data));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO, "D3D12 shader model: %s",
			dx12_shader_model_str(shader_model_data.HighestShaderModel));
	}

	D3D12_FEATURE_DATA_ROOT_SIGNATURE root_signature_data = {
		.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1,
	};
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_ROOT_SIGNATURE, &root_signature_data,
		sizeof(root_signature_data));
	if (FAILED(hr)) {
		root_signature_data.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
	wlf_log(WLF_INFO, "D3D12 root signature: %s",
		root_signature_data.HighestVersion == D3D_ROOT_SIGNATURE_VERSION_1_1 ?
			"1.1" : "1.0");

	D3D12_FEATURE_DATA_D3D12_OPTIONS options;
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO,
			"D3D12 resource tiers: binding=%s heap=%s tiled=%s conservative_rasterization=%s",
			dx12_resource_binding_tier_str(options.ResourceBindingTier),
			dx12_resource_heap_tier_str(options.ResourceHeapTier),
			dx12_tiled_resources_tier_str(options.TiledResourcesTier),
			dx12_conservative_rasterization_tier_str(
				options.ConservativeRasterizationTier));
		wlf_log(WLF_INFO,
			"D3D12 options: typed_uav_load_additional_formats=%s standard_swizzle_64kb=%s cross_adapter_row_major_texture=%s vp_rt_array_index_without_gs=%s output_merger_logic_op=%s",
			dx12_bool_str(options.TypedUAVLoadAdditionalFormats),
			dx12_bool_str(options.StandardSwizzle64KBSupported),
			dx12_bool_str(options.CrossAdapterRowMajorTextureSupported),
			dx12_bool_str(
				options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation),
			dx12_bool_str(options.OutputMergerLogicOp));
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS1 options1;
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_D3D12_OPTIONS1, &options1, sizeof(options1));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO,
			"D3D12 shader options: wave_ops=%s wave_lane_count_min=%u wave_lane_count_max=%u total_lane_count=%u expanded_compute_resource_states=%s int64_shader_ops=%s",
			dx12_bool_str(options1.WaveOps), options1.WaveLaneCountMin,
			options1.WaveLaneCountMax, options1.TotalLaneCount,
			dx12_bool_str(options1.ExpandedComputeResourceStates),
			dx12_bool_str(options1.Int64ShaderOps));
	}

	D3D12_FEATURE_DATA_ARCHITECTURE1 architecture = {
		.NodeIndex = 0,
	};
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_ARCHITECTURE1, &architecture, sizeof(architecture));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO,
			"D3D12 architecture: tile_based_renderer=%s uma=%s cache_coherent_uma=%s isolated_mmu=%s",
			dx12_bool_str(architecture.TileBasedRenderer),
			dx12_bool_str(architecture.UMA),
			dx12_bool_str(architecture.CacheCoherentUMA),
			dx12_bool_str(architecture.IsolatedMMU));
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5;
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO,
			"D3D12 advanced options: raytracing=%s render_passes=%s",
			dx12_raytracing_tier_str(options5.RaytracingTier),
			dx12_bool_str(options5.RenderPassesTier !=
				D3D12_RENDER_PASS_TIER_0));
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6;
	hr = ID3D12Device_CheckFeatureSupport(renderer->device,
		D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6));
	if (SUCCEEDED(hr)) {
		wlf_log(WLF_INFO,
			"D3D12 variable shading rate: tier=%s additional_rates=%s per_primitive_with_viewport_indexing=%s tile_size=%u background_processing=%s",
			dx12_variable_shading_rate_tier_str(
				options6.VariableShadingRateTier),
			dx12_bool_str(options6.AdditionalShadingRatesSupported),
			dx12_bool_str(
				options6.PerPrimitiveShadingRateSupportedWithViewportIndexing),
			options6.ShadingRateImageTileSize,
			dx12_bool_str(options6.BackgroundProcessingSupported));
	}
}

static void dx12_release_debug(ID3D12Debug *debug) {
	if (debug != NULL) {
		ID3D12Debug_Release(debug);
	}
}

static bool dx12_enable_debug_layer(void) {
	ID3D12Debug *debug = NULL;
	HRESULT hr = D3D12GetDebugInterface(&IID_ID3D12Debug, (void **)&debug);
	if (FAILED(hr)) {
		dx12_log_error("D3D12GetDebugInterface", hr);
		return false;
	}

	ID3D12Debug_EnableDebugLayer(debug);
	dx12_release_debug(debug);
	return true;
}

static void dx12_renderer_destroy(struct wlf_renderer *wlf_renderer) {
	struct wlf_dx12_renderer *renderer =
		wlf_dx12_renderer_from_renderer(wlf_renderer);

	if (renderer->fence_event != NULL) {
		CloseHandle(renderer->fence_event);
	}
	if (renderer->fence != NULL) {
		ID3D12Fence_Release(renderer->fence);
	}
	if (renderer->command_queue != NULL) {
		ID3D12CommandQueue_Release(renderer->command_queue);
	}
	if (renderer->device != NULL) {
		ID3D12Device_Release(renderer->device);
	}
	if (renderer->adapter != NULL) {
		IDXGIAdapter1_Release(renderer->adapter);
	}
	if (renderer->factory != NULL) {
		IDXGIFactory6_Release(renderer->factory);
	}

	free(renderer);
}

static struct wlf_texture *dx12_renderer_texture_from_buffer(
		struct wlf_renderer *renderer, struct wlf_buffer *buffer) {
	WLF_UNUSED(renderer);
	WLF_UNUSED(buffer);
	wlf_log(WLF_ERROR, "DirectX 12 texture import is not implemented yet");
	return NULL;
}

static const struct wlf_renderer_impl dx12_renderer_impl = {
	.destroy = dx12_renderer_destroy,
	.texture_from_buffer = dx12_renderer_texture_from_buffer,
};

static IDXGIAdapter1 *dx12_select_adapter(IDXGIFactory6 *factory) {
	IDXGIAdapter1 *adapter = NULL;

	for (UINT i = 0; ; i++) {
		IDXGIAdapter1 *candidate = NULL;
		HRESULT hr = IDXGIFactory6_EnumAdapterByGpuPreference(factory, i,
			DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			&IID_IDXGIAdapter1, (void **)&candidate);
		if (hr == DXGI_ERROR_NOT_FOUND) {
			break;
		}
		if (FAILED(hr)) {
			dx12_log_error("IDXGIFactory6_EnumAdapterByGpuPreference", hr);
			break;
		}

		DXGI_ADAPTER_DESC1 desc;
		hr = IDXGIAdapter1_GetDesc1(candidate, &desc);
		if (FAILED(hr)) {
			dx12_log_error("IDXGIAdapter1_GetDesc1", hr);
			IDXGIAdapter1_Release(candidate);
			continue;
		}

		if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
			IDXGIAdapter1_Release(candidate);
			continue;
		}

		hr = D3D12CreateDevice((IUnknown *)candidate,
			D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, NULL);
		if (SUCCEEDED(hr)) {
			adapter = candidate;
			break;
		}

		IDXGIAdapter1_Release(candidate);
	}

	return adapter;
}

struct wlf_renderer *wlf_dx12_renderer_create_from_backend(
		struct wlf_backend *backend) {
	if (!wlf_backend_is_windows(backend)) {
		wlf_log(WLF_ERROR, "DirectX 12 renderer requires a Windows backend");
		return NULL;
	}

	if (wlf_env_parse_bool("WLF_RENDER_DEBUG")) {
		dx12_enable_debug_layer();
	}

	struct wlf_dx12_renderer *renderer = calloc(1, sizeof(*renderer));
	if (renderer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_dx12_renderer");
		return NULL;
	}

	wlf_renderer_init(&renderer->base, &dx12_renderer_impl);
	renderer->backend = backend;
	renderer->base.type = GPU;
	renderer->base.features.damage = true;
	renderer->fence_value = 1;

	UINT factory_flags = wlf_env_parse_bool("WLF_RENDER_DEBUG") ?
		DXGI_CREATE_FACTORY_DEBUG : 0;
	HRESULT hr = CreateDXGIFactory2(factory_flags,
		&IID_IDXGIFactory6, (void **)&renderer->factory);
	if (FAILED(hr)) {
		dx12_log_error("CreateDXGIFactory2", hr);
		goto error;
	}

	renderer->adapter = dx12_select_adapter(renderer->factory);
	if (renderer->adapter == NULL) {
		wlf_log(WLF_ERROR, "Failed to find a DirectX 12 capable adapter");
		goto error;
	}

	hr = D3D12CreateDevice((IUnknown *)renderer->adapter,
		D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device,
		(void **)&renderer->device);
	if (FAILED(hr)) {
		dx12_log_error("D3D12CreateDevice", hr);
		goto error;
	}

	D3D12_COMMAND_QUEUE_DESC queue_desc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
		.NodeMask = 0,
	};
	hr = ID3D12Device_CreateCommandQueue(renderer->device,
		&queue_desc, &IID_ID3D12CommandQueue,
		(void **)&renderer->command_queue);
	if (FAILED(hr)) {
		dx12_log_error("ID3D12Device_CreateCommandQueue", hr);
		goto error;
	}

	hr = ID3D12Device_CreateFence(renderer->device, 0,
		D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence,
		(void **)&renderer->fence);
	if (FAILED(hr)) {
		dx12_log_error("ID3D12Device_CreateFence", hr);
		goto error;
	}

	renderer->fence_event = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (renderer->fence_event == NULL) {
		wlf_log(WLF_ERROR, "CreateEventW failed for DirectX 12 fence");
		goto error;
	}

	dx12_log_renderer_info(renderer);
	return &renderer->base;

error:
	wlf_renderer_destroy(&renderer->base);
	return NULL;
}

bool wlf_renderer_is_dx12(const struct wlf_renderer *renderer) {
	return renderer != NULL && renderer->impl == &dx12_renderer_impl;
}

struct wlf_dx12_renderer *wlf_dx12_renderer_from_renderer(
		struct wlf_renderer *renderer) {
	assert(renderer && renderer->impl == &dx12_renderer_impl);
	struct wlf_dx12_renderer *dx12_renderer =
		wlf_container_of(renderer, dx12_renderer, base);
	return dx12_renderer;
}
