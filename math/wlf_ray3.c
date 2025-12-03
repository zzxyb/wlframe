#include "wlf/math/wlf_ray3.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WLF_RAY3_STRLEN 384

struct wlf_ray3 wlf_ray3_make(struct wlf_vector3 origin, struct wlf_vector3 direction) {
	struct wlf_ray3 ray = {origin, direction};
	return ray;
}

char* wlf_ray3_to_str(const struct wlf_ray3 *ray) {
	if (ray == NULL) {
		return strdup("(NULL)");
	}

	char *buffer = malloc(WLF_RAY3_STRLEN);
	if (buffer == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate buffer");
		return NULL;
	}

	char *origin_str = wlf_vector3_to_str(&ray->origin);
	char *direction_str = wlf_vector3_to_str(&ray->direction);

	if (origin_str && direction_str) {
		snprintf(buffer, WLF_RAY3_STRLEN, "(Origin: %s, Direction: %s)",
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

struct wlf_vector3 wlf_ray3_point_at_parameter(const struct wlf_ray3 *ray, double t) {
	return (struct wlf_vector3) {
			.x = ray->origin.x + t * ray->direction.x,
			.y = ray->origin.y + t * ray->direction.y,
			.z = ray->origin.z + t * ray->direction.z,
		};
}

bool wlf_ray3_equal(const struct wlf_ray3 *a, const struct wlf_ray3 *b) {
	return wlf_vector3_equal(&a->origin, &b->origin) &&
			wlf_vector3_equal(&a->direction, &b->direction);
}

bool wlf_ray3_nearly_equal(const struct wlf_ray3 *a, const struct wlf_ray3 *b, double epsilon) {
	return wlf_vector3_nearly_equal(&a->origin, &b->origin, epsilon) &&
			wlf_vector3_nearly_equal(&a->direction, &b->direction, epsilon);
}
