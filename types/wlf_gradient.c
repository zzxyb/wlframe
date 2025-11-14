/**
 * @file        wlf_gradient.c
 * @brief       Common gradient utility functions for wlframe.
 */

#include "wlf_gradient.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void wlf_gradient_init(struct wlf_gradient *gradient, const struct wlf_gradient_impl *impl)
{
	memset(gradient, 0, sizeof(*gradient));
	gradient->impl = impl;
}

bool wlf_gradient_add_stop(struct wlf_gradient *gradient, double position, struct wlf_color color)
{
	struct wlf_gradient_stop *new_stops =
	    realloc(gradient->stops, sizeof(struct wlf_gradient_stop) * (gradient->stop_count + 1));
	if (!new_stops)
		return false;

	gradient->stops = new_stops;
	gradient->stops[gradient->stop_count].position = fmin(fmax(position, 0.0), 1.0);
	gradient->stops[gradient->stop_count].color = color;
	gradient->stop_count++;
	return true;
}

void wlf_gradient_sort_stops(struct wlf_gradient *gradient)
{
	if (!gradient || gradient->stop_count < 2)
		return;

	for (uint32_t i = 0; i < gradient->stop_count - 1; ++i) {
		for (uint32_t j = i + 1; j < gradient->stop_count; ++j) {
			if (gradient->stops[j].position < gradient->stops[i].position) {
				struct wlf_gradient_stop tmp = gradient->stops[i];
				gradient->stops[i] = gradient->stops[j];
				gradient->stops[j] = tmp;
			}
		}
	}
}

void wlf_gradient_release_stops(struct wlf_gradient *gradient)
{
	if (!gradient)
		return;
	free(gradient->stops);
	gradient->stops = NULL;
	gradient->stop_count = 0;
}
