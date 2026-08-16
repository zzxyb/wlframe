#ifndef WLF_PASS_DIRECTX12_RENDER_TARGET_INFO_H
#define WLF_PASS_DIRECTX12_RENDER_TARGET_INFO_H

#include "wlf/pass/wlf_render_target_info.h"
#include "wlf/renderer/directx12/renderer.h"

struct wlf_dx12_render_target_info {
	struct wlf_render_target_info base;
	struct wlf_dx12_renderer *renderer;
	ID3D12Resource *resource;
	ID3D12CommandAllocator *allocator;
	ID3D12GraphicsCommandList *commands;
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	ID3D12Resource **temporary_resources;
	size_t temporary_resource_count;
	size_t temporary_resource_capacity;
};

struct wlf_render_target_info *wlf_dx12_begin_render_pass(
	struct wlf_dx12_renderer *renderer, struct wlf_buffer *buffer);
bool wlf_render_target_info_is_dx12(
	const struct wlf_render_target_info *target);
struct wlf_dx12_render_target_info *wlf_dx12_render_target_from_info(
	struct wlf_render_target_info *target);
bool wlf_dx12_render_target_retain_resource(
	struct wlf_dx12_render_target_info *target, ID3D12Resource *resource);

#endif
