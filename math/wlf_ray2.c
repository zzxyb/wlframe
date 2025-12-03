#include "wlf/math/wlf_ray2.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WLF_RAY2_STRLEN 256

struct wlf_ray2 wlf_ray2_make(struct wlf_vector2 origin, struct wlf_vector2 direction) {
	struct wlf_ray2 ray = {origin, direction};
	return ray;
}

char* wlf_ray2_to_str(const struct wlf_ray2 *ray) {
	if (ray == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_RAY2_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	char *origin_str = wlf_vector2_to_str(&ray->origin);
	char *direction_str = wlf_vector2_to_str(&ray->direction);

	if (origin_str && direction_str) {
		snprintf(buffer, WLF_RAY2_STRLEN, "(Origin: %s, Direction: %s)",
				origin_str, direction_str);
	} else {
		free(origin_str);
		free(direction_str);
		return strdup("(NULL)");
	}

	free(origin_str);
	free(direction_str);

	return buffer;
}

struct wlf_vector2 wlf_ray2_point_at_parameter(const struct wlf_ray2 *ray, double t) {
	return (struct wlf_vector2) {
			.u = ray->origin.u + t * ray->direction.u,
			.v = ray->origin.v + t * ray->direction.v,
		};
}

bool wlf_ray2_equal(const struct wlf_ray2 *a, const struct wlf_ray2 *b) {
	return wlf_vector2_equal(&a->origin, &b->origin) &&
			wlf_vector2_equal(&a->direction, &b->direction);
}

bool wlf_ray2_nearly_equal(const struct wlf_ray2 *a, const struct wlf_ray2 *b, double epsilon) {
	return wlf_vector2_nearly_equal(&a->origin, &b->origin, epsilon) &&
			wlf_vector2_nearly_equal(&a->direction, &b->direction, epsilon);
}
