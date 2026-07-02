/**
 * @file        renderer.h
 * @brief       DirectX 12 renderer backend implementation for wlframe.
 * @details     This file defines the DirectX 12 renderer implementation used on Windows.
 *              It extends the generic `wlf_renderer` interface and manages the DXGI
 *              factory, selected adapter, D3D12 device, command queue, and synchronization
 *              objects needed by the renderer backend.
 * @author      YaoBing Xiao
 * @date        2026-07-02
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-07-02, initial version\n
 */

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

/**
 * @brief DirectX 12 renderer implementation.
 *
 * This structure derives from `wlf_renderer` and stores the Windows-specific
 * DirectX 12 objects used to render frames and synchronize GPU work.
 *
 * @note The `base` member must remain the first field so a generic
 *       `wlf_renderer` can be safely converted back to `wlf_dx12_renderer`.
 */
struct wlf_dx12_renderer {
	struct wlf_renderer base;              /**< Base renderer interface. */
	struct wlf_backend *backend;           /**< Backend that owns this renderer. */

	IDXGIFactory6 *factory;                /**< DXGI factory used to enumerate adapters. */
	IDXGIAdapter1 *adapter;                /**< Selected DXGI adapter for D3D12 rendering. */
	ID3D12Device *device;                  /**< D3D12 logical device. */
	ID3D12CommandQueue *command_queue;     /**< Direct command queue for GPU submissions. */
	ID3D12Fence *fence;                    /**< Fence used for GPU/CPU synchronization. */
	HANDLE fence_event;                    /**< Event signaled when the fence reaches a value. */
	UINT64 fence_value;                    /**< Next fence value used for synchronization. */
};

/**
 * @brief Creates a DirectX 12 renderer from a wlframe backend.
 *
 * This function creates the DXGI factory, selects a hardware adapter, creates
 * the D3D12 device, and initializes the command queue and synchronization fence.
 *
 * @param backend Pointer to the wlframe Windows backend.
 * @return Pointer to the created `wlf_renderer`, or NULL on failure.
 */
struct wlf_renderer *wlf_dx12_renderer_create_from_backend(
	struct wlf_backend *backend);

/**
 * @brief Checks whether a renderer is a DirectX 12 renderer.
 *
 * @param renderer Pointer to a generic renderer instance.
 * @return true if the renderer uses DirectX 12, false otherwise.
 */
bool wlf_renderer_is_dx12(const struct wlf_renderer *renderer);

/**
 * @brief Converts a generic renderer pointer to a DirectX 12 renderer pointer.
 *
 * The caller must only pass a renderer for which wlf_renderer_is_dx12()
 * returns true.
 *
 * @param renderer Pointer to a generic renderer instance.
 * @return Pointer to the underlying DirectX 12 renderer.
 */
struct wlf_dx12_renderer *wlf_dx12_renderer_from_renderer(
	struct wlf_renderer *renderer);

#endif // DIRECTX12_RENDERER_H
