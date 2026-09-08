#ifndef WLF_PASS_METAL_VECTOR_PASS_H
#define WLF_PASS_METAL_VECTOR_PASS_H

#include "wlf/pass/wlf_vector_pass.h"

struct wlf_mtl_renderer;

struct wlf_vector_pass *wlf_mtl_vector_pass_create(
	struct wlf_mtl_renderer *renderer);

#endif /* WLF_PASS_METAL_VECTOR_PASS_H */
