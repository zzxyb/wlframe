#include "wlf/render/wlf_render.h"
#include "wlf/platform/wlf_backend.h"

#include <stdlib.h>

struct wlf_render *wlf_renderer_autocreate(struct wlf_backend *backend) {
	// TODO: 实现自动创建渲染器的逻辑
	(void)backend; // 避免未使用参数警告
	return NULL;
}

// 临时存根函数，避免链接错误
void wlf_renderer_set_alpha(struct wlf_render *render, float alpha) {
	(void)render;
	(void)alpha;
}

void wlf_renderer_push_transform(struct wlf_render *render, const struct wlf_matrix4x4 *transform) {
	(void)render;
	(void)transform;
}

void wlf_renderer_pop_transform(struct wlf_render *render) {
	(void)render;
}
