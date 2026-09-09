#include "wlf/effect/wlf_gaussian_blur.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void blur_destroy(struct wlf_shape *shape) {
	free(wlf_gaussian_blur_from_shape(shape));
}

static struct wlf_shape *blur_clone(struct wlf_shape *shape) {
	struct wlf_gaussian_blur *src = wlf_gaussian_blur_from_shape(shape);
	struct wlf_gaussian_blur *dst = wlf_gaussian_blur_create(src->std_deviation_x,
		src->std_deviation_y);
	if (dst != NULL) {
		memcpy(dst->input, src->input, sizeof(dst->input));
		memcpy(dst->result, src->result, sizeof(dst->result));
	}
	return dst != NULL ? &dst->base : NULL;
}

static const struct wlf_shape_impl blur_impl = {
	.destroy = blur_destroy,
	.clone = blur_clone,
};

struct wlf_gaussian_blur *wlf_gaussian_blur_create(float std_deviation_x,
		float std_deviation_y) {
	struct wlf_gaussian_blur *blur = calloc(1, sizeof(*blur));
	if (blur == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_gaussian_blur");
		return NULL;
	}

	wlf_shape_init(&blur->base, &blur_impl);
	blur->std_deviation_x = std_deviation_x;
	blur->std_deviation_y = std_deviation_y;
	return blur;
}

bool wlf_shape_is_gaussian_blur(struct wlf_shape *shape) {
	return shape != NULL && shape->impl == &blur_impl;
}

struct wlf_gaussian_blur *wlf_gaussian_blur_from_shape(struct wlf_shape *shape) {
	assert(shape->impl == &blur_impl);
	return wlf_container_of(shape, (struct wlf_gaussian_blur *)NULL, base);
}
