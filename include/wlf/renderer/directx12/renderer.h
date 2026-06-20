#ifndef DIRECTX12_RENDERER_H
#define DIRECTX12_RENDERER_H

#include "wlf/config.h"
#include "wlf/renderer/wlf_renderer.h"

#include <stdbool.h>

struct wlf_backend;

#ifndef COBJMACROS
#define COBJMACROS
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <d3d12.h>
#include <dxgi1_6.h>

struct wlf_dx12_renderer {
	struct wlf_renderer base;
	struct wlf_backend *backend;

	IDXGIFactory6 *factory;
	IDXGIAdapter1 *adapter;
	ID3D12Device *device;
	ID3D12CommandQueue *command_queue;
	ID3D12Fence *fence;
	HANDLE fence_event;
	UINT64 fence_value;
};

struct wlf_renderer *wlf_dx12_renderer_create_from_backend(
	struct wlf_backend *backend);
bool wlf_renderer_is_dx12(const struct wlf_renderer *renderer);
struct wlf_dx12_renderer *wlf_dx12_renderer_from_renderer(
	struct wlf_renderer *renderer);

#endif // DIRECTX12_RENDERER_H
