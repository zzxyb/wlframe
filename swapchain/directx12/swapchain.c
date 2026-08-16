#include "swapchain.h"

#include "wlf/renderer/directx12/renderer.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"
#include "wlf/window/wlf_window.h"

#include <assert.h>
#include <stdlib.h>

#define WLF_DX12_BUFFER_COUNT 2

struct wlf_dx12_buffer {
	struct wlf_buffer base;
	ID3D12Resource *resource;
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
};

struct wlf_dx12_swapchain {
	struct wlf_swapchain base;
	struct wlf_dx12_renderer *renderer;
	IDXGISwapChain3 *swapchain;
	ID3D12DescriptorHeap *rtv_heap;
	HANDLE frame_latency_event;
	struct wlf_dx12_buffer *buffers[WLF_DX12_BUFFER_COUNT];
};

static struct wlf_dx12_swapchain *dx12_swapchain_from_base(
		struct wlf_swapchain *base);

static void dx12_buffer_destroy(struct wlf_buffer *base) {
	struct wlf_dx12_buffer *buffer = wlf_dx12_buffer_from_buffer(base);
	wlf_buffer_finish(base);
	if (buffer->resource != NULL) {
		ID3D12Resource_Release(buffer->resource);
	}
	free(buffer);
}

static const struct wlf_buffer_impl dx12_buffer_impl = {
	.destroy = dx12_buffer_destroy,
};

struct wlf_dx12_buffer *wlf_dx12_buffer_from_buffer(
		struct wlf_buffer *base) {
	if (base == NULL || base->impl != &dx12_buffer_impl) {
		return NULL;
	}
	struct wlf_dx12_buffer *buffer = NULL;
	return wlf_container_of(base, buffer, base);
}

ID3D12Resource *wlf_dx12_buffer_get_resource(struct wlf_dx12_buffer *buffer) {
	return buffer != NULL ? buffer->resource : NULL;
}

D3D12_CPU_DESCRIPTOR_HANDLE wlf_dx12_buffer_get_rtv(
		struct wlf_dx12_buffer *buffer) {
	return buffer != NULL ? buffer->rtv : (D3D12_CPU_DESCRIPTOR_HANDLE){0};
}

static void release_buffers(struct wlf_dx12_swapchain *swapchain) {
	for (size_t i = 0; i < WLF_DX12_BUFFER_COUNT; ++i) {
		if (swapchain->buffers[i] != NULL) {
			wlf_buffer_drop(&swapchain->buffers[i]->base);
			swapchain->buffers[i] = NULL;
		}
	}
	swapchain->base.back = NULL;
}

static bool acquire_buffers(struct wlf_dx12_swapchain *swapchain,
		int width, int height) {
	D3D12_CPU_DESCRIPTOR_HANDLE rtv;
	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(
		swapchain->rtv_heap, &rtv);
	UINT increment = ID3D12Device_GetDescriptorHandleIncrementSize(
		swapchain->renderer->device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0; i < WLF_DX12_BUFFER_COUNT; ++i) {
		struct wlf_dx12_buffer *buffer = calloc(1, sizeof(*buffer));
		if (buffer == NULL) {
			release_buffers(swapchain);
			return false;
		}
		HRESULT hr = IDXGISwapChain3_GetBuffer(swapchain->swapchain, i,
			&IID_ID3D12Resource, (void **)&buffer->resource);
		if (FAILED(hr)) {
			free(buffer);
			release_buffers(swapchain);
			return false;
		}
		buffer->rtv = rtv;
		ID3D12Device_CreateRenderTargetView(swapchain->renderer->device,
			buffer->resource, NULL, rtv);
		wlf_buffer_init(&buffer->base, &dx12_buffer_impl,
			(uint32_t)width, (uint32_t)height);
		swapchain->buffers[i] = buffer;
		rtv.ptr += increment;
	}
	UINT index = IDXGISwapChain3_GetCurrentBackBufferIndex(
		swapchain->swapchain);
	swapchain->base.back = &swapchain->buffers[index]->base;
	return true;
}

static void dx12_swapchain_destroy(struct wlf_swapchain *base) {
	struct wlf_dx12_swapchain *swapchain = dx12_swapchain_from_base(base);
	(void)wlf_dx12_renderer_wait_idle(swapchain->renderer);
	release_buffers(swapchain);
	if (swapchain->frame_latency_event != NULL) {
		CloseHandle(swapchain->frame_latency_event);
	}
	if (swapchain->rtv_heap != NULL) {
		ID3D12DescriptorHeap_Release(swapchain->rtv_heap);
	}
	if (swapchain->swapchain != NULL) {
		IDXGISwapChain3_Release(swapchain->swapchain);
	}
	free(swapchain);
}

static bool dx12_swapchain_resize(struct wlf_swapchain *base,
		int width, int height) {
	if (width <= 0 || height <= 0) {
		return false;
	}
	struct wlf_dx12_swapchain *swapchain = dx12_swapchain_from_base(base);
	if (!wlf_dx12_renderer_wait_idle(swapchain->renderer)) {
		return false;
	}
	release_buffers(swapchain);
	HRESULT hr = IDXGISwapChain3_ResizeBuffers(swapchain->swapchain,
		WLF_DX12_BUFFER_COUNT, (UINT)width, (UINT)height,
		DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	if (FAILED(hr) || !acquire_buffers(swapchain, width, height)) {
		wlf_log(WLF_ERROR, "Failed to resize DXGI swapchain: 0x%08lx",
			(unsigned long)hr);
		return false;
	}
	base->width = width;
	base->height = height;
	return true;
}

static void dx12_swapchain_present(struct wlf_swapchain *base,
		const pixman_region32_t *damage) {
	struct wlf_dx12_swapchain *swapchain = dx12_swapchain_from_base(base);
	int count = 0;
	pixman_box32_t *boxes = damage == NULL ? NULL :
		pixman_region32_rectangles((pixman_region32_t *)damage, &count);
	RECT *rects = count > 0 ? malloc((size_t)count * sizeof(*rects)) : NULL;
	if (count > 0 && rects == NULL) {
		return;
	}
	for (int i = 0; i < count; ++i) {
		rects[i] = (RECT){boxes[i].x1, boxes[i].y1,
			boxes[i].x2, boxes[i].y2};
	}
	DXGI_PRESENT_PARAMETERS parameters = {
		.DirtyRectsCount = (UINT)count,
		.pDirtyRects = rects,
	};
	HRESULT hr = IDXGISwapChain3_Present1(swapchain->swapchain, 1, 0,
		&parameters);
	free(rects);
	if (FAILED(hr)) {
		wlf_log(WLF_ERROR, "IDXGISwapChain3_Present1 failed: 0x%08lx",
			(unsigned long)hr);
		return;
	}
	UINT index = IDXGISwapChain3_GetCurrentBackBufferIndex(
		swapchain->swapchain);
	base->back = &swapchain->buffers[index]->base;
}

static const struct wlf_swapchain_impl dx12_swapchain_impl = {
	.destroy = dx12_swapchain_destroy,
	.resize = dx12_swapchain_resize,
	.present = dx12_swapchain_present,
};

struct wlf_swapchain *wlf_dx12_swapchain_create(struct wlf_window *window,
		int width, int height, const struct wlf_render_format *format) {
	if (window == NULL || !wlf_renderer_is_dx12(window->state.renderer) ||
			width <= 0 || height <= 0 ||
			(format->format != WLF_FORMAT_XRGB8888 &&
			format->format != WLF_FORMAT_ARGB8888)) {
		return NULL;
	}
	struct wlf_dx12_swapchain *swapchain = calloc(1, sizeof(*swapchain));
	if (swapchain == NULL) {
		return NULL;
	}
	swapchain->renderer = wlf_dx12_renderer_from_renderer(
		window->state.renderer);
	wlf_swapchain_init(&swapchain->base, NULL, &dx12_swapchain_impl,
		width, height);
	swapchain->base.window = window;
	if (!wlf_render_format_copy(&swapchain->base.format, format)) {
		dx12_swapchain_destroy(&swapchain->base);
		return NULL;
	}

	DXGI_SWAP_CHAIN_DESC1 desc = {
		.Width = (UINT)width,
		.Height = (UINT)height,
		.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
		.Stereo = FALSE,
		.SampleDesc = {.Count = 1},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = WLF_DX12_BUFFER_COUNT,
		.Scaling = DXGI_SCALING_STRETCH,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
		.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT,
	};
	IDXGISwapChain1 *swapchain1 = NULL;
	HRESULT hr = IDXGIFactory6_CreateSwapChainForHwnd(
		swapchain->renderer->factory,
		(IUnknown *)swapchain->renderer->command_queue,
		wlf_window_native_handle(window), &desc, NULL, NULL, &swapchain1);
	if (SUCCEEDED(hr)) {
		hr = IDXGISwapChain1_QueryInterface(swapchain1,
			&IID_IDXGISwapChain3, (void **)&swapchain->swapchain);
		IDXGISwapChain1_Release(swapchain1);
	}
	if (FAILED(hr)) {
		wlf_log(WLF_ERROR, "CreateSwapChainForHwnd failed: 0x%08lx",
			(unsigned long)hr);
		dx12_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	IDXGIFactory6_MakeWindowAssociation(swapchain->renderer->factory,
		wlf_window_native_handle(window), DXGI_MWA_NO_ALT_ENTER);
	IDXGISwapChain3_SetMaximumFrameLatency(swapchain->swapchain, 1);
	swapchain->frame_latency_event =
		IDXGISwapChain3_GetFrameLatencyWaitableObject(swapchain->swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = WLF_DX12_BUFFER_COUNT,
	};
	hr = ID3D12Device_CreateDescriptorHeap(swapchain->renderer->device,
		&heap_desc, &IID_ID3D12DescriptorHeap,
		(void **)&swapchain->rtv_heap);
	if (FAILED(hr) || !acquire_buffers(swapchain, width, height)) {
		dx12_swapchain_destroy(&swapchain->base);
		return NULL;
	}
	return &swapchain->base;
}

bool wlf_swapchain_is_dx12(const struct wlf_swapchain *swapchain) {
	return swapchain != NULL && swapchain->impl == &dx12_swapchain_impl;
}

HANDLE wlf_dx12_swapchain_get_frame_latency_event(
		struct wlf_swapchain *base) {
	return wlf_swapchain_is_dx12(base) ?
		dx12_swapchain_from_base(base)->frame_latency_event : NULL;
}

static struct wlf_dx12_swapchain *dx12_swapchain_from_base(
		struct wlf_swapchain *base) {
	assert(wlf_swapchain_is_dx12(base));
	struct wlf_dx12_swapchain *swapchain = NULL;
	return wlf_container_of(base, swapchain, base);
}
