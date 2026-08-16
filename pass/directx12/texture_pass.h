#ifndef WLF_PASS_DIRECTX12_TEXTURE_PASS_H
#define WLF_PASS_DIRECTX12_TEXTURE_PASS_H

#include "wlf/pass/wlf_texture_pass.h"

struct wlf_texture_pass *wlf_dx12_texture_pass_create(
	struct wlf_renderer *renderer);

#endif
