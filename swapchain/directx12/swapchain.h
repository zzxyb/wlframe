#ifndef WLF_SWAPCHAIN_DIRECTX12_SWAPCHAIN_H
#define WLF_SWAPCHAIN_DIRECTX12_SWAPCHAIN_H

#include "wlf/swapchain/wlf_swapchain.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef COBJMACROS
#define COBJMACROS
#endif
#include <windows.h>
#include <d3d12.h>

struct wlf_dx12_buffer;

struct wlf_swapchain *wlf_dx12_swapchain_create(struct wlf_window *window,
	int width, int height, const struct wlf_render_format *format);
bool wlf_swapchain_is_dx12(const struct wlf_swapchain *swapchain);
struct wlf_dx12_buffer *wlf_dx12_buffer_from_buffer(
	struct wlf_buffer *buffer);
ID3D12Resource *wlf_dx12_buffer_get_resource(struct wlf_dx12_buffer *buffer);
D3D12_CPU_DESCRIPTOR_HANDLE wlf_dx12_buffer_get_rtv(
	struct wlf_dx12_buffer *buffer);
HANDLE wlf_dx12_swapchain_get_frame_latency_event(
	struct wlf_swapchain *swapchain);

#endif
