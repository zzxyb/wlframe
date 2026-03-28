#include "wlf/types/wlf_mesh_gradient.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void wlf_mesh_bernstein(double t, double b[4]) {
	double it = 1.0 - t;
	double it2 = it * it;
	double it3 = it2 * it;
	double t2 = t * t;
	double t3 = t2 * t;

	b[0] = it3;
	b[1] = 3.0 * it2 * t;
	b[2] = 3.0 * it * t2;
	b[3] = t3;
}

static struct wlf_color wlf_mesh_bezier_color(
	const struct wlf_mesh_gradient_patch *patch, double u, double v) {
	double bu[4];
	double bv[4];
	wlf_mesh_bernstein(u, bu);
	wlf_mesh_bernstein(v, bv);

	struct wlf_color out = WLF_COLOR_TRANSPARENT;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			double w = bu[i] * bv[j];
			out.r += patch->colors[j][i].r * w;
			out.g += patch->colors[j][i].g * w;
			out.b += patch->colors[j][i].b * w;
			out.a += patch->colors[j][i].a * w;
		}
	}

	return out;
}

static struct wlf_color gradient_sample(struct wlf_gradient *gradient,
		const struct wlf_fpoint *p) {
	struct wlf_mesh_gradient *mesh =
		wlf_mesh_gradient_from_gradient(gradient);

	if (!p || !mesh->patches || mesh->patch_columns == 0 || mesh->patch_rows == 0 ||
		mesh->size.x == 0.0 || mesh->size.y == 0.0) {
		return WLF_COLOR_TRANSPARENT;
	}

	double u = (p->x - mesh->origin.x) / mesh->size.x;
	double v = (p->y - mesh->origin.y) / mesh->size.y;

	u = fmax(0.0, fmin(1.0, u));
	v = fmax(0.0, fmin(1.0, v));

	double gx = u * mesh->patch_columns;
	double gy = v * mesh->patch_rows;
	uint32_t px = (uint32_t)fmin((double)(mesh->patch_columns - 1), floor(gx));
	uint32_t py = (uint32_t)fmin((double)(mesh->patch_rows - 1), floor(gy));

	double local_u = gx - px;
	double local_v = gy - py;

	const struct wlf_mesh_gradient_patch *patch =
		&mesh->patches[py * mesh->patch_columns + px];

	return wlf_mesh_bezier_color(patch, local_u, local_v);
}

static void gradient_destroy(struct wlf_gradient *gradient) {
	struct wlf_mesh_gradient *mesh =
		wlf_mesh_gradient_from_gradient(gradient);
	wlf_gradient_destroy_stops(gradient);
	free(mesh->patches);
	free(mesh);
}

static const struct wlf_gradient_impl gradient_impl = {
	.sample = gradient_sample,
	.destroy = gradient_destroy,
};

struct wlf_mesh_gradient *wlf_mesh_gradient_create(struct wlf_fpoint origin,
		struct wlf_fpoint size, uint32_t patch_columns, uint32_t patch_rows,
		const struct wlf_mesh_gradient_patch *patches) {
	if (patch_columns == 0 || patch_rows == 0) {
		return NULL;
	}

	struct wlf_mesh_gradient *mesh = calloc(1, sizeof(*mesh));
	if (mesh == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_mesh_gradient");
		return NULL;
	}

	wlf_gradient_init(&mesh->base, &gradient_impl);
	mesh->origin = origin;
	mesh->size = size;
	mesh->patch_columns = patch_columns;
	mesh->patch_rows = patch_rows;

	size_t count = (size_t)patch_columns * (size_t)patch_rows;
	mesh->patches = calloc(count, sizeof(*mesh->patches));
	if (mesh->patches == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate mesh patches");
		wlf_gradient_destroy(&mesh->base);
		return NULL;
	}

	memcpy(mesh->patches, patches, sizeof(*mesh->patches) * count);

	return mesh;
}

bool wlf_gradient_is_mesh(const struct wlf_gradient *gradient) {
	return gradient->impl == &gradient_impl;
}

struct wlf_mesh_gradient *wlf_mesh_gradient_from_gradient(
		struct wlf_gradient *gradient) {
	assert(gradient->impl == &gradient_impl);

	struct wlf_mesh_gradient *mesh_gradient =
		wlf_container_of(gradient, mesh_gradient, base);

	return mesh_gradient;
}
