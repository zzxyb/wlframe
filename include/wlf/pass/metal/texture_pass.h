#ifndef WLF_PASS_METAL_TEXTURE_PASS_H
#define WLF_PASS_METAL_TEXTURE_PASS_H

#include "wlf/pass/wlf_texture_pass.h"

struct wlf_mtl_renderer;

struct wlf_texture_pass *wlf_mtl_texture_pass_create(
	struct wlf_mtl_renderer *renderer);

#endif /* WLF_PASS_METAL_TEXTURE_PASS_H */
