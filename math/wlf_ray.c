#include "wlf/math/wlf_ray.h"

#include <stdlib.h>
#include <stdio.h>

struct wlf_ray wlf_ray_create(struct wlf_vector3 origin, struct wlf_vector3 direction) {
	struct wlf_ray ray = {origin, direction};
	return ray;
}

char* wlf_ray_to_str(const struct wlf_ray *ray) {
	char *buffer = malloc(128);
	if (buffer) {
		snprintf(buffer, 128, "Ray(Origin: %s, Direction: %s)", 
				wlf_vector3_to_str(&ray->origin), 
				wlf_vector3_to_str(&ray->direction));
	}
	return buffer;
}

struct wlf_vector3 wlf_ray_point_at_parameter(const struct wlf_ray *ray, double t) {
	return wlf_vector3_create(
			ray->origin.x + t * ray->direction.x,
			ray->origin.y + t * ray->direction.y,
			ray->origin.z + t * ray->direction.z
	);
}

bool wlf_ray_equal(const struct wlf_ray *a, const struct wlf_ray *b) {
	return wlf_vector3_equal(&a->origin, &b->origin) && 
			wlf_vector3_equal(&a->direction, &b->direction);
}

bool wlf_ray_nearly_equal(const struct wlf_ray *a, const struct wlf_ray *b, double epsilon) {
	return wlf_vector3_nearly_equal(&a->origin, &b->origin, epsilon) && 
			wlf_vector3_nearly_equal(&a->direction, &b->direction, epsilon);
}