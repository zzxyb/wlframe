#include "texture.h"

#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static DXGI_FORMAT texture_format(uint32_t format) {
	switch (format) {
	case WLF_FORMAT_ARGB8888:
	case WLF_FORMAT_XRGB8888:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case WLF_FORMAT_ABGR8888:
	case WLF_FORMAT_XBGR8888:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}

static void texture_destroy(struct wlf_texture *base) {
	struct wlf_dx12_texture *texture = wlf_dx12_texture_from_texture(base);
	if (texture->srv_heap != NULL)
		ID3D12DescriptorHeap_Release(texture->srv_heap);
	if (texture->resource != NULL)
		ID3D12Resource_Release(texture->resource);
	free(texture);
}

static uint32_t texture_preferred_read_format(struct wlf_texture *base) {
	struct wlf_dx12_texture *texture = wlf_dx12_texture_from_texture(base);
	return texture->format == DXGI_FORMAT_B8G8R8A8_UNORM ?
		WLF_FORMAT_ARGB8888 : WLF_FORMAT_ABGR8888;
}

static const struct wlf_texture_impl texture_impl = {
	.preferred_read_format = texture_preferred_read_format,
	.destroy = texture_destroy,
};

static bool upload_texture(struct wlf_dx12_texture *texture,
		const void *data, size_t stride) {
	struct wlf_dx12_renderer *renderer = texture->renderer;
	D3D12_RESOURCE_DESC desc;
	ID3D12Resource_GetDesc(texture->resource, &desc);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT rows = 0;
	UINT64 row_size = 0, upload_size = 0;
	ID3D12Device_GetCopyableFootprints(renderer->device, &desc, 0, 1, 0,
		&footprint, &rows, &row_size, &upload_size);

	D3D12_HEAP_PROPERTIES upload_heap = {.Type = D3D12_HEAP_TYPE_UPLOAD};
	D3D12_RESOURCE_DESC upload_desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Width = upload_size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.SampleDesc = {.Count = 1},
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
	};
	ID3D12Resource *upload = NULL;
	HRESULT hr = ID3D12Device_CreateCommittedResource(renderer->device,
		&upload_heap, D3D12_HEAP_FLAG_NONE, &upload_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource,
		(void **)&upload);
	if (FAILED(hr)) return false;

	void *mapped = NULL;
	D3D12_RANGE read_range = {0, 0};
	hr = ID3D12Resource_Map(upload, 0, &read_range, &mapped);
	if (FAILED(hr)) goto error;
	for (UINT row = 0; row < rows; ++row) {
		memcpy((uint8_t *)mapped + footprint.Offset +
			(size_t)row * footprint.Footprint.RowPitch,
			(const uint8_t *)data + (size_t)row * stride,
			(size_t)row_size);
	}
	ID3D12Resource_Unmap(upload, 0, NULL);

	ID3D12CommandAllocator *allocator = NULL;
	ID3D12GraphicsCommandList *commands = NULL;
	hr = ID3D12Device_CreateCommandAllocator(renderer->device,
		D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator,
		(void **)&allocator);
	if (SUCCEEDED(hr)) {
		hr = ID3D12Device_CreateCommandList(renderer->device, 0,
			D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, NULL,
			&IID_ID3D12GraphicsCommandList, (void **)&commands);
	}
	if (FAILED(hr)) goto command_error;

	D3D12_TEXTURE_COPY_LOCATION dst = {
		.pResource = texture->resource,
		.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
		.SubresourceIndex = 0,
	};
	D3D12_TEXTURE_COPY_LOCATION src = {
		.pResource = upload,
		.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
		.PlacedFootprint = footprint,
	};
	ID3D12GraphicsCommandList_CopyTextureRegion(commands, &dst, 0, 0, 0,
		&src, NULL);
	D3D12_RESOURCE_BARRIER barrier = {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Transition = {
			.pResource = texture->resource,
			.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
			.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		},
	};
	ID3D12GraphicsCommandList_ResourceBarrier(commands, 1, &barrier);
	hr = ID3D12GraphicsCommandList_Close(commands);
	if (SUCCEEDED(hr)) {
		ID3D12CommandList *lists[] = {(ID3D12CommandList *)commands};
		ID3D12CommandQueue_ExecuteCommandLists(renderer->command_queue, 1, lists);
		if (!wlf_dx12_renderer_wait_idle(renderer)) hr = E_FAIL;
	}
	ID3D12GraphicsCommandList_Release(commands);
	ID3D12CommandAllocator_Release(allocator);
	ID3D12Resource_Release(upload);
	return SUCCEEDED(hr);

command_error:
	if (commands != NULL) ID3D12GraphicsCommandList_Release(commands);
	if (allocator != NULL) ID3D12CommandAllocator_Release(allocator);
error:
	ID3D12Resource_Release(upload);
	return false;
}

struct wlf_texture *wlf_dx12_texture_from_buffer(
		struct wlf_dx12_renderer *renderer, struct wlf_buffer *buffer) {
	void *data = NULL;
	uint32_t format = WLF_FORMAT_INVALID;
	size_t stride = 0;
	if (!wlf_buffer_begin_data_ptr_access(buffer,
			WLF_BUFFER_DATA_PTR_ACCESS_READ, &data, &format, &stride)) {
		return NULL;
	}
	DXGI_FORMAT dxgi_format = texture_format(format);
	if (dxgi_format == DXGI_FORMAT_UNKNOWN || stride < (size_t)buffer->width * 4) {
		wlf_buffer_end_data_ptr_access(buffer);
		return NULL;
	}

	struct wlf_dx12_texture *texture = calloc(1, sizeof(*texture));
	if (texture == NULL) {
		wlf_buffer_end_data_ptr_access(buffer);
		return NULL;
	}
	texture->renderer = renderer;
	texture->format = dxgi_format;
	wlf_texture_init(&texture->base, &renderer->base, &texture_impl,
		buffer->width, buffer->height);

	D3D12_HEAP_PROPERTIES heap = {.Type = D3D12_HEAP_TYPE_DEFAULT};
	D3D12_RESOURCE_DESC desc = {
		.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		.Width = buffer->width,
		.Height = buffer->height,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = dxgi_format,
		.SampleDesc = {.Count = 1},
		.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
	};
	HRESULT hr = ID3D12Device_CreateCommittedResource(renderer->device, &heap,
		D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		&IID_ID3D12Resource, (void **)&texture->resource);
	if (FAILED(hr) || !upload_texture(texture, data, stride)) goto error;
	wlf_buffer_end_data_ptr_access(buffer);

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = 1,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	hr = ID3D12Device_CreateDescriptorHeap(renderer->device, &heap_desc,
		&IID_ID3D12DescriptorHeap, (void **)&texture->srv_heap);
	if (FAILED(hr)) {
		wlf_texture_destroy(&texture->base);
		return NULL;
	}
	D3D12_SHADER_RESOURCE_VIEW_DESC srv = {
		.Format = dxgi_format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {.MipLevels = 1},
	};
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(
		texture->srv_heap, &handle);
	ID3D12Device_CreateShaderResourceView(renderer->device, texture->resource,
		&srv, handle);
	return &texture->base;

error:
	wlf_buffer_end_data_ptr_access(buffer);
	wlf_texture_destroy(&texture->base);
	return NULL;
}

bool wlf_texture_is_dx12(const struct wlf_texture *texture) {
	return texture != NULL && texture->impl == &texture_impl;
}

struct wlf_dx12_texture *wlf_dx12_texture_from_texture(
		struct wlf_texture *base) {
	assert(wlf_texture_is_dx12(base));
	struct wlf_dx12_texture *texture = NULL;
	return wlf_container_of(base, texture, base);
}
