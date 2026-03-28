#include "wlf/types/wlf_linear_gradient.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static struct wlf_color gradient_sample(struct wlf_gradient *gradient,
		const struct wlf_fpoint *p) {
	struct wlf_linear_gradient *linear =
		wlf_linear_gradient_from_gradient(gradient);

	double dx = linear->end.x - linear->start.x;
	double dy = linear->end.y - linear->start.y;
	double denom = dx * dx + dy * dy;
	double t = 0.0;

	if (denom > 0.0) {
		t = ((p->x - linear->start.x) * dx + (p->y - linear->start.y) * dy) / denom;
	}

	return wlf_gradient_sample_stops(gradient, t);
}

static void gradient_destroy(struct wlf_gradient *gradient) {
	struct wlf_linear_gradient *linear =
		wlf_linear_gradient_from_gradient(gradient);
	wlf_gradient_destroy_stops(gradient);
	free(linear);
}

static const struct wlf_gradient_impl gradient_impl = {
	.sample = gradient_sample,
	.destroy = gradient_destroy,
};

struct wlf_linear_gradient *wlf_linear_gradient_create(
	struct wlf_fpoint start, struct wlf_fpoint end,
	const struct wlf_gradient_stop *stops, size_t stop_count) {
	struct wlf_linear_gradient *linear = malloc(sizeof(*linear));
	if (linear == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_linear_gradient");
		return NULL;
	}

	wlf_gradient_init(&linear->base, &gradient_impl);
	linear->start = start;
	linear->end = end;

	if (!wlf_gradient_set_stops(&linear->base, stops, stop_count)) {
		wlf_gradient_destroy(&linear->base);
		return NULL;
	}

	return linear;
}

bool wlf_gradient_is_linear(const struct wlf_gradient *gradient) {
	return gradient->impl == &gradient_impl;
}

struct wlf_linear_gradient *wlf_linear_gradient_from_gradient(
		struct wlf_gradient *gradient) {
	assert(gradient->impl == &gradient_impl);

	struct wlf_linear_gradient *linear_gradient =
		wlf_container_of(gradient, linear_gradient, base);

	return linear_gradient;
}
