/**
 * @file        wlf_gradient_radial.c
 * @brief       Implementation of radial gradient for wlframe.
 */

#include "wlf_gradient_radial.h"
#include <stdlib.h>
#include <math.h>

/* Forward declarations */
static void wlf_gradient_radial_destroy(struct wlf_gradient *gradient);
static struct wlf_color wlf_gradient_radial_sample(const struct wlf_gradient *gradient, double t);

/* Impl table */
static const struct wlf_gradient_impl wlf_gradient_radial_impl = {
	.destroy = wlf_gradient_radial_destroy,
	.sample  = wlf_gradient_radial_sample,
};

/* ------------------------------------------------------------------------- */

struct wlf_gradient_radial *wlf_gradient_radial_create(double cx0, double cy0, double r0,
                                                       double cx1, double cy1, double r1)
{
	struct wlf_gradient_radial *rad = calloc(1, sizeof(*rad));
	if (!rad)
		return NULL;

	wlf_gradient_init(&rad->base, &wlf_gradient_radial_impl);
	rad->cx0 = cx0;
	rad->cy0 = cy0;
	rad->r0 = r0;
	rad->cx1 = cx1;
	rad->cy1 = cy1;
	rad->r1 = r1;
	return rad;
}

static void wlf_gradient_radial_destroy(struct wlf_gradient *gradient)
{
	struct wlf_gradient_radial *self = (struct wlf_gradient_radial *)gradient;
	wlf_gradient_release_stops(&self->base);
	free(self);
}

static struct wlf_color wlf_gradient_radial_sample(const struct wlf_gradient *gradient, double t)
{
	if (!gradient || gradient->stop_count == 0)
		return WLF_COLOR_TRANSPARENT;

	/* clamp */
	if (t < 0.0) t = 0.0;
	else if (t > 1.0) t = 1.0;

	const struct wlf_gradient *g = gradient;
	wlf_gradient_sort_stops((struct wlf_gradient *)g);

	if (g->stop_count == 1)
		return g->stops[0].color;

	for (uint32_t i = 0; i < g->stop_count - 1; ++i) {
		const struct wlf_gradient_stop *a = &g->stops[i];
		const struct wlf_gradient_stop *b = &g->stops[i + 1];
		if (t >= a->position && t <= b->position) {
			double local_t = (t - a->position) / (b->position - a->position);
			return wlf_color_lerp(&a->color, &b->color, local_t);
		}
	}
	return g->stops[g->stop_count - 1].color;
}
