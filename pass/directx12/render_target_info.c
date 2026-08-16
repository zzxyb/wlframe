#include "render_target_info.h"

#include "swapchain/directx12/swapchain.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static void transition(ID3D12GraphicsCommandList *commands,
		ID3D12Resource *resource, D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier = {
		.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
		.Transition = {
			.pResource = resource,
			.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			.StateBefore = before,
			.StateAfter = after,
		},
	};
	ID3D12GraphicsCommandList_ResourceBarrier(commands, 1, &barrier);
}

static void target_destroy(struct wlf_render_target_info *base) {
	struct wlf_dx12_render_target_info *target =
		wlf_dx12_render_target_from_info(base);
	transition(target->commands, target->resource,
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	HRESULT hr = ID3D12GraphicsCommandList_Close(target->commands);
	if (SUCCEEDED(hr)) {
		ID3D12CommandList *lists[] = {(ID3D12CommandList *)target->commands};
		ID3D12CommandQueue_ExecuteCommandLists(
			target->renderer->command_queue, 1, lists);
		(void)wlf_dx12_renderer_wait_idle(target->renderer);
	}
	ID3D12GraphicsCommandList_Release(target->commands);
	ID3D12CommandAllocator_Release(target->allocator);
	free(target);
}

static struct wlf_renderer *target_get_renderer(
		struct wlf_render_target_info *base) {
	return &wlf_dx12_render_target_from_info(base)->renderer->base;
}

static const struct wlf_render_target_info_impl target_impl = {
	.destroy = target_destroy,
	.get_renderer = target_get_renderer,
};

struct wlf_render_target_info *wlf_dx12_begin_render_pass(
		struct wlf_dx12_renderer *renderer, struct wlf_buffer *base_buffer) {
	struct wlf_dx12_buffer *buffer = wlf_dx12_buffer_from_buffer(base_buffer);
	if (renderer == NULL || buffer == NULL) return NULL;
	struct wlf_dx12_render_target_info *target = calloc(1, sizeof(*target));
	if (target == NULL) return NULL;
	target->renderer = renderer;
	target->resource = wlf_dx12_buffer_get_resource(buffer);
	target->rtv = wlf_dx12_buffer_get_rtv(buffer);
	HRESULT hr = ID3D12Device_CreateCommandAllocator(renderer->device,
		D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator,
		(void **)&target->allocator);
	if (SUCCEEDED(hr)) {
		hr = ID3D12Device_CreateCommandList(renderer->device, 0,
			D3D12_COMMAND_LIST_TYPE_DIRECT, target->allocator, NULL,
			&IID_ID3D12GraphicsCommandList, (void **)&target->commands);
	}
	if (FAILED(hr)) {
		if (target->allocator != NULL)
			ID3D12CommandAllocator_Release(target->allocator);
		free(target);
		return NULL;
	}
	transition(target->commands, target->resource,
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	ID3D12GraphicsCommandList_OMSetRenderTargets(target->commands, 1,
		&target->rtv, FALSE, NULL);
	wlf_render_target_info_init(&target->base, &target_impl);
	return &target->base;
}

bool wlf_render_target_info_is_dx12(
		const struct wlf_render_target_info *target) {
	return target != NULL && target->impl == &target_impl;
}

struct wlf_dx12_render_target_info *wlf_dx12_render_target_from_info(
		struct wlf_render_target_info *base) {
	assert(wlf_render_target_info_is_dx12(base));
	struct wlf_dx12_render_target_info *target = NULL;
	return wlf_container_of(base, target, base);
}
