#ifndef WLF_TEXTURE_DIRECTX12_TEXTURE_H
#define WLF_TEXTURE_DIRECTX12_TEXTURE_H

#include "wlf/renderer/directx12/renderer.h"
#include "wlf/texture/wlf_texture.h"

struct wlf_dx12_texture {
	struct wlf_texture base;
	struct wlf_dx12_renderer *renderer;
	ID3D12Resource *resource;
	ID3D12DescriptorHeap *srv_heap;
	DXGI_FORMAT format;
};

struct wlf_texture *wlf_dx12_texture_from_buffer(
	struct wlf_dx12_renderer *renderer, struct wlf_buffer *buffer);
bool wlf_texture_is_dx12(const struct wlf_texture *texture);
struct wlf_dx12_texture *wlf_dx12_texture_from_texture(
	struct wlf_texture *texture);

#endif
