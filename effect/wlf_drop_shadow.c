#include "wlf/effect/wlf_drop_shadow.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void shadow_destroy(struct wlf_shape *shape) {
	free(wlf_drop_shadow_from_shape(shape));
}

static struct wlf_shape *shadow_clone(struct wlf_shape *shape) {
	struct wlf_drop_shadow *src = wlf_drop_shadow_from_shape(shape);
	struct wlf_drop_shadow *dst = wlf_drop_shadow_create(src->dx, src->dy,
		src->std_deviation_x, src->std_deviation_y, src->color, src->opacity);
	if (dst != NULL) {
		memcpy(dst->input, src->input, sizeof(dst->input));
		memcpy(dst->result, src->result, sizeof(dst->result));
	}
	return dst != NULL ? &dst->base : NULL;
}

static const struct wlf_shape_impl shadow_impl = {
	.destroy = shadow_destroy,
	.clone = shadow_clone,
};

struct wlf_drop_shadow *wlf_drop_shadow_create(float dx, float dy,
		float std_deviation_x, float std_deviation_y,
		struct wlf_color color, float opacity) {
	struct wlf_drop_shadow *shadow = calloc(1, sizeof(*shadow));
	if (shadow == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_drop_shadow");
		return NULL;
	}

	wlf_shape_init(&shadow->base, &shadow_impl);
	shadow->dx = dx;
	shadow->dy = dy;
	shadow->std_deviation_x = std_deviation_x;
	shadow->std_deviation_y = std_deviation_y;
	shadow->color = color;
	shadow->opacity = opacity;
	return shadow;
}

bool wlf_shape_is_drop_shadow(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &shadow_impl;
}

struct wlf_drop_shadow *wlf_drop_shadow_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &shadow_impl);
	return wlf_container_of(shape, (struct wlf_drop_shadow *)NULL, base);
}
