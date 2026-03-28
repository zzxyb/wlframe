#include "wlf/types/wlf_radial_gradient.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static struct wlf_color gradient_sample(struct wlf_gradient *gradient,
		const struct wlf_fpoint *p) {
	struct wlf_radial_gradient *radial =
		wlf_radial_gradient_from_gradient(gradient);

	if (p == NULL || radial->radius <= 0.0) {
		return WLF_COLOR_TRANSPARENT;
	}

	double fx = radial->focal.x;
	double fy = radial->focal.y;
	double cx = radial->center.x;
	double cy = radial->center.y;
	double r = radial->radius;

	if (fx == cx && fy == cy) {
		double dx = p->x - cx;
		double dy = p->y - cy;
		double t = sqrt(dx * dx + dy * dy) / r;
		return wlf_gradient_sample_stops(gradient, t);
	}

	double dx = p->x - fx;
	double dy = p->y - fy;
	double dd = dx * dx + dy * dy;
	if (dd == 0.0) {
		return wlf_gradient_sample_stops(gradient, 0.0);
	}

	double mx = fx - cx;
	double my = fy - cy;
	double b = mx * dx + my * dy;
	double c = mx * mx + my * my - r * r;
	double disc = b * b - dd * c;
	if (disc < 0.0) {
		return wlf_gradient_sample_stops(gradient, 0.0);
	}

	double t_max = (-b + sqrt(disc)) / dd;
	if (t_max <= 0.0) {
		return wlf_gradient_sample_stops(gradient, 0.0);
	}

	double t = 1.0 / t_max;
	return wlf_gradient_sample_stops(gradient, t);
}

static void gradient_destroy(struct wlf_gradient *gradient) {
	struct wlf_radial_gradient *radial =
		wlf_radial_gradient_from_gradient(gradient);
	wlf_gradient_destroy_stops(gradient);
	free(radial);
}

static const struct wlf_gradient_impl gradient_impl = {
	.sample = gradient_sample,
	.destroy = gradient_destroy,
};

struct wlf_radial_gradient *wlf_radial_gradient_create(
	struct wlf_fpoint center, struct wlf_fpoint focal, double radius,
	const struct wlf_gradient_stop *stops, size_t stop_count) {
	struct wlf_radial_gradient *radial = calloc(1, sizeof(*radial));
	if (radial == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_radial_gradient");
		return NULL;
	}

	wlf_gradient_init(&radial->base, &gradient_impl);
	radial->center = center;
	radial->focal = focal;
	radial->radius = radius;

	if (!wlf_gradient_set_stops(&radial->base, stops, stop_count)) {
		wlf_gradient_destroy(&radial->base);
		return NULL;
	}

	return radial;
}

bool wlf_gradient_is_radial(const struct wlf_gradient *gradient) {
	return gradient->impl == &gradient_impl;
}

struct wlf_radial_gradient *wlf_radial_gradient_from_gradient(
		struct wlf_gradient *gradient) {
	assert(gradient->impl == &gradient_impl);

	struct wlf_radial_gradient *radial_gradient =
		wlf_container_of(gradient, radial_gradient, base);

	return radial_gradient;
}