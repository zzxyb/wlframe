#ifndef WLF_PASS_METAL_RECT_PASS_H
#define WLF_PASS_METAL_RECT_PASS_H

#include "wlf/pass/wlf_rect_pass.h"

struct wlf_mtl_renderer;

struct wlf_rect_pass *wlf_mtl_rect_pass_create(
	struct wlf_mtl_renderer *renderer);

#endif /* WLF_PASS_METAL_RECT_PASS_H */
