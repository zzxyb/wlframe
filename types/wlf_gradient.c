#include "wlf/types/wlf_gradient.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int wlf_gradient_stop_cmp(const void *a, const void *b) {
	const struct wlf_gradient_stop *sa = a;
	const struct wlf_gradient_stop *sb = b;
	if (sa->offset < sb->offset) {
		return -1;
	}
	if (sa->offset > sb->offset) {
		return 1;
	}
	return 0;
}

static bool wlf_gradient_xform_is_identity(const float xform[6]) {
	return xform[0] == 1.0f && xform[1] == 0.0f &&
		xform[2] == 0.0f && xform[3] == 1.0f &&
		xform[4] == 0.0f && xform[5] == 0.0f;
}

static struct wlf_fpoint wlf_gradient_apply_xform(const struct wlf_gradient *gradient,
		const struct wlf_fpoint *p) {
	const float *m = gradient->xform;
	struct wlf_fpoint out = {
		.x = (double)m[0] * p->x + (double)m[2] * p->y + (double)m[4],
		.y = (double)m[1] * p->x + (double)m[3] * p->y + (double)m[5],
	};
	return out;
}

void wlf_gradient_init(struct wlf_gradient *gradient,
		const struct wlf_gradient_impl *impl) {
	assert(gradient);
	assert(impl);
	assert(impl->destroy);
	assert(impl->sample);

	*gradient = (struct wlf_gradient){
		.impl = impl,
		.stop_count = 0,
		.stops = NULL,
		.xform = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		.has_xform = false,
	};
}

bool wlf_gradient_set_stops(struct wlf_gradient *gradient,
		const struct wlf_gradient_stop *stops, size_t stop_count) {
	wlf_gradient_destroy_stops(gradient);

	if (stop_count == 0) {
		return true;
	}

	struct wlf_gradient_stop *copy = calloc(stop_count, sizeof(*copy));
	if (copy == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_gradient_stop array");
		return false;
	}

	memcpy(copy, stops, sizeof(*copy) * stop_count);
	qsort(copy, stop_count, sizeof(*copy), wlf_gradient_stop_cmp);

	gradient->stops = copy;
	gradient->stop_count = stop_count;
	return true;
}

void wlf_gradient_destroy_stops(struct wlf_gradient *gradient) {
	if (gradient == NULL || gradient->stops == NULL) {
		return;
	}

	free(gradient->stops);
	gradient->stops = NULL;
	gradient->stop_count = 0;
}

struct wlf_color wlf_gradient_sample_stops(struct wlf_gradient *gradient,
		double t) {
	if (gradient->stop_count == 0) {
		return WLF_COLOR_TRANSPARENT;
	}

	if (gradient->stop_count == 1) {
		return gradient->stops[0].color;
	}

	if (t <= gradient->stops[0].offset) {
		return gradient->stops[0].color;
	}

	for (size_t i = 1; i < gradient->stop_count; i++) {
		if (t <= gradient->stops[i].offset) {
			double t0 = gradient->stops[i - 1].offset;
			double t1 = gradient->stops[i].offset;
			double local = (t1 == t0) ? 0.0 : (t - t0) / (t1 - t0);
			return wlf_color_lerp(&gradient->stops[i - 1].color,
				&gradient->stops[i].color, local);
		}
	}

	return gradient->stops[gradient->stop_count - 1].color;
}

struct wlf_color wlf_gradient_sample(struct wlf_gradient *gradient,
		const struct wlf_fpoint *p) {
	const struct wlf_fpoint *sample_point = p;
	struct wlf_fpoint local;
	if (gradient->has_xform) {
		local = wlf_gradient_apply_xform(gradient, p);
		sample_point = &local;
	}

	return gradient->impl->sample(gradient, sample_point);
}

void wlf_gradient_set_transform(struct wlf_gradient *gradient,
		const float xform[6]) {
	memcpy(gradient->xform, xform, sizeof(float) * 6);
	gradient->has_xform = !wlf_gradient_xform_is_identity(gradient->xform);
}

void wlf_gradient_set_identity(struct wlf_gradient *gradient) {
	gradient->xform[0] = 1.0f;
	gradient->xform[1] = 0.0f;
	gradient->xform[2] = 0.0f;
	gradient->xform[3] = 1.0f;
	gradient->xform[4] = 0.0f;
	gradient->xform[5] = 0.0f;
	gradient->has_xform = false;
}

void wlf_gradient_destroy(struct wlf_gradient *gradient) {
	if (gradient == NULL) {
		return;
	}

	if (gradient->impl && gradient->impl->destroy) {
		gradient->impl->destroy(gradient);
	} else {
		free(gradient);
	}
}
