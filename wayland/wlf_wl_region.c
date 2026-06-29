#include "wlf/wayland/wlf_wl_region.h"
#include "wlf/wayland/wlf_wl_compositor.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-client-protocol.h>

struct wlf_wl_region *wlf_wl_region_create(struct wlf_wl_compositor *compositor) {
	assert(compositor != NULL);

	struct wlf_wl_region *region = calloc(1, sizeof(*region));
	if (region == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_region");
		return NULL;
	}

	region->wl_region = wl_compositor_create_region(compositor->base);
	if (region->wl_region == NULL) {
		wlf_log(WLF_ERROR, "wl_compositor_create_region failed");
		free(region);
		return NULL;
	}

	return region;
}

void wlf_wl_region_destroy(struct wlf_wl_region *region) {
	if (region == NULL) {
		return;
	}
	wl_region_destroy(region->wl_region);
	free(region);
}

void wlf_wl_region_add(struct wlf_wl_region *region,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	assert(region != NULL);
	wl_region_add(region->wl_region, x, y, width, height);
}

void wlf_wl_region_subtract(struct wlf_wl_region *region,
		int32_t x, int32_t y, int32_t width, int32_t height) {
	assert(region != NULL);
	wl_region_subtract(region->wl_region, x, y, width, height);
}
