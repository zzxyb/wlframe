#include "wlf/va/wayland/va_display.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <wayland-client-protocol.h>

#include <va/va.h>
#include <va/va_str.h>
#include <va/va_wayland.h>

static void wl_va_display_destroy(struct wlf_va_display *display) {
	struct wlf_wl_va_display *wl_display = wlf_wl_va_display_from_va_display(display);
	wlf_signal_emit(&display->events.destroy, display);

	if (display->display != NULL) {
		vaTerminate(display->display);
	}

	free(wl_display);
}

static VAStatus wl_va_display_put_surface(struct wlf_va_display *display,
		VASurfaceID va_surface, void *native_surface,
		const VARectangle *src_rect, const VARectangle *dst_rect) {
	VAStatus va_status;
	struct wl_buffer *buffer;
	struct wl_surface *surface = native_surface;
	va_status = vaGetSurfaceBufferWl(display->display, va_surface, VA_FRAME_PICTURE, &buffer);
	if (va_status != VA_STATUS_SUCCESS) {
		return va_status;
	}

	wl_surface_attach(surface, buffer, 0, 0);
	wl_surface_damage(
		surface,
		dst_rect->x, dst_rect->y, dst_rect->width, dst_rect->height
	);

	wl_surface_commit(surface);
	struct wlf_backend_wayland *wl_backend = wlf_backend_wayland_from_backend(display->backend);
	wl_display_flush(wl_backend->display->base);

	return VA_STATUS_SUCCESS;
}

static const struct wlf_va_display_impl wl_va_display_impl = {
	.name = "wayland",
	.destroy = wl_va_display_destroy,
	.put_surface = wl_va_display_put_surface,
};

struct wlf_va_display *wl_va_display_create(struct wlf_backend_wayland *backend) {
	VAStatus va_status;
	int major_version, minor_version;
	const char *driver;
	struct wlf_wl_va_display *display = calloc(1, sizeof(*display));
	if (display == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_wl_va_display");
		return NULL;
	}

	wlf_va_display_init(&display->base, &wl_va_display_impl, &backend->base);

	display->base.display = vaGetDisplayWl(backend->display->base);
	if (display->base.display == NULL) {
		wlf_log(WLF_ERROR, "failed to create VA display from Wayland display");
		goto error;
	}

	va_status = vaInitialize(display->base.display, &major_version, &minor_version);
	wlf_log(WLF_DEBUG, "wlframe: VA-API version: %d.%d", major_version, minor_version);
	if (va_status != VA_STATUS_SUCCESS) {
		goto error;
	}

	driver = vaQueryVendorString(display->base.display);
	wlf_log(WLF_DEBUG, "wlframe: VA Driver version: %s\n", driver ? driver : "<unknown>");

	int num_entrypoint = vaMaxNumEntrypoints(display->base.display);
	VAEntrypoint *entrypoints = malloc(num_entrypoint * sizeof(VAEntrypoint));
	if (entrypoints == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate memory for entrypoint list");
		goto error;
	}

	int max_num_profiles = vaMaxNumProfiles(display->base.display);
	VAProfile *profile_list = malloc(max_num_profiles * sizeof(VAProfile));

	if (profile_list == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate memory for profile list");
		goto error;
	}

	int num_profiles;
	va_status = vaQueryConfigProfiles(display->base.display, profile_list, &num_profiles);

	wlf_log(WLF_DEBUG, "Supported profile and entrypoints");
	for (int i = 0; i < num_profiles; i++) {
		VAProfile profile = profile_list[i];
		va_status = vaQueryConfigEntrypoints(display->base.display, profile, entrypoints,
			&num_entrypoint);

		if (va_status == VA_STATUS_ERROR_UNSUPPORTED_PROFILE)
			continue;

		for (int entrypoint = 0; entrypoint < num_entrypoint; entrypoint++) {
			wlf_log(WLF_DEBUG, "%-32s:	%s", vaProfileStr(profile),
				vaEntrypointStr(entrypoints[entrypoint]));
		}
	}

	return &display->base;

error:
	if (display->base.display != NULL) {
		vaTerminate(display->base.display);
	}

	if (display) {
		free(display);
	}

	return NULL;
}

bool wlf_va_display_is_wayland(const struct wlf_va_display *display) {
	return (display && display->impl == &wl_va_display_impl);
}

struct wlf_wl_va_display *wlf_wl_va_display_from_va_display(struct wlf_va_display *display) {
	assert(display && display->impl == &wl_va_display_impl);
	struct wlf_wl_va_display *wl_display = wlf_container_of(display, wl_display, base);

	return wl_display;
}
