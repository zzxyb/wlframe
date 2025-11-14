/**
 * @file        wlf_gradient_linear.c
 * @brief       Implementation of linear gradient for wlframe.
 */

#include "wlf_gradient_linear.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Forward declarations */
static void wlf_gradient_linear_destroy(struct wlf_gradient *gradient);
static struct wlf_color wlf_gradient_linear_sample(const struct wlf_gradient *gradient, double t);

/* Impl table */
static const struct wlf_gradient_impl wlf_gradient_linear_impl = {
	.destroy = wlf_gradient_linear_destroy,
	.sample  = wlf_gradient_linear_sample,
};

/* ------------------------------------------------------------------------- */

struct wlf_gradient_linear *wlf_gradient_linear_create(double x0, double y0,
                                                       double x1, double y1)
{
	struct wlf_gradient_linear *lin = calloc(1, sizeof(*lin));
	if (!lin)
		return NULL;

	wlf_gradient_init(&lin->base, &wlf_gradient_linear_impl);
	lin->x0 = x0;
	lin->y0 = y0;
	lin->x1 = x1;
	lin->y1 = y1;
	return lin;
}

static void wlf_gradient_linear_destroy(struct wlf_gradient *gradient)
{
	struct wlf_gradient_linear *self = (struct wlf_gradient_linear *)gradient;
	wlf_gradient_release_stops(&self->base);
	free(self);
}

static struct wlf_color wlf_gradient_linear_sample(const struct wlf_gradient *gradient, double t)
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
