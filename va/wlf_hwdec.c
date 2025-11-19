#include "wlf/va/wlf_hwdec.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include <stdlib.h>
#include <string.h>

/* External backend implementations */
#ifdef WLF_HAS_VULKAN
extern const struct wlf_hwdec_device_impl wlf_hwdec_vulkan_impl;
#endif
#ifdef WLF_HAS_VAAPI
extern const struct wlf_hwdec_device_impl wlf_hwdec_vaapi_impl;
#endif
extern const struct wlf_hwdec_device_impl wlf_hwdec_software_impl;

/* Available backend implementations */
static const struct wlf_hwdec_device_impl *hwdec_backends[] = {
#ifdef WLF_HAS_VULKAN
	&wlf_hwdec_vulkan_impl,
#endif
#ifdef WLF_HAS_VAAPI
	&wlf_hwdec_vaapi_impl,
#endif
	&wlf_hwdec_software_impl,  /* Always available as fallback */
	NULL,
};

static struct wlf_hwdec_device *hwdec_device_create(
	const struct wlf_hwdec_device_impl *impl) {

	if (!impl) {
		return NULL;
	}

	struct wlf_hwdec_device *device = calloc(1, sizeof(struct wlf_hwdec_device));
	if (!device) {
		wlf_log(WLF_ERROR, "Failed to allocate hwdec device");
		return NULL;
	}

	device->impl = impl;
	wlf_signal_init(&device->events.destroy);

	if (impl->init && !impl->init(device)) {
		wlf_log(WLF_ERROR, "Failed to initialize %s backend", impl->name);
		free(device);
		return NULL;
	}

	wlf_log(WLF_INFO, "Initialized %s hwdec backend", impl->name);
	return device;
}

static void hwdec_device_destroy(struct wlf_hwdec_device *device) {
	if (!device) {
		return;
	}

	wlf_signal_emit(&device->events.destroy, device);

	if (device->impl && device->impl->destroy) {
		device->impl->destroy(device);
	}

	free(device);
}

struct wlf_hwdec_context *wlf_hwdec_context_create(
	const char *preferred_backend,
	bool auto_fallback) {

	struct wlf_hwdec_context *ctx = calloc(1, sizeof(struct wlf_hwdec_context));
	if (!ctx) {
		wlf_log(WLF_ERROR, "Failed to allocate hwdec context");
		return NULL;
	}

	/* Check environment variable for backend preference */
	const char *env_backend = wlf_get_env("WLF_HWDEC_BACKEND");
	if (env_backend) {
		preferred_backend = env_backend;
		wlf_log(WLF_INFO, "Using hwdec backend from environment: %s", env_backend);
	}

	ctx->preferred_backend = preferred_backend;
	ctx->auto_fallback = auto_fallback;

	bool auto_mode = !preferred_backend || strcmp(preferred_backend, "auto") == 0;

	wlf_log(WLF_INFO, "Creating hwdec context (preferred: %s, fallback: %s)",
		preferred_backend ? preferred_backend : "auto",
		auto_fallback ? "enabled" : "disabled");

	/* Initialize all available backends */
	for (int i = 0; hwdec_backends[i]; i++) {
		const struct wlf_hwdec_device_impl *impl = hwdec_backends[i];

		/* Skip if we have a specific preference and this isn't it */
		if (!auto_mode && strcmp(impl->name, preferred_backend) != 0) {
			/* Unless it's software fallback and fallback is enabled */
			if (!(auto_fallback && strcmp(impl->name, "software") == 0)) {
				continue;
			}
		}

		struct wlf_hwdec_device *device = hwdec_device_create(impl);
		if (device) {
			ctx->num_devices++;
			ctx->devices = realloc(ctx->devices,
				ctx->num_devices * sizeof(struct wlf_hwdec_device *));
			ctx->devices[ctx->num_devices - 1] = device;
		}
	}

	if (ctx->num_devices == 0) {
		wlf_log(WLF_ERROR, "No hwdec backends available");
		free(ctx);
		return NULL;
	}

	wlf_log(WLF_INFO, "Initialized %u hwdec backend(s)", ctx->num_devices);
	return ctx;
}

void wlf_hwdec_context_destroy(struct wlf_hwdec_context *ctx) {
	if (!ctx) {
		return;
	}

	for (uint32_t i = 0; i < ctx->num_devices; i++) {
		hwdec_device_destroy(ctx->devices[i]);
	}

	free(ctx->devices);
	free(ctx);
}

struct wlf_hwdec_device *wlf_hwdec_get_device(
	struct wlf_hwdec_context *ctx,
	enum wlf_video_codec codec) {

	if (!ctx || !ctx->devices) {
		return NULL;
	}

	/* Try preferred backend first */
	if (ctx->preferred_backend && strcmp(ctx->preferred_backend, "auto") != 0) {
		for (uint32_t i = 0; i < ctx->num_devices; i++) {
			struct wlf_hwdec_device *dev = ctx->devices[i];
			if (strcmp(dev->impl->name, ctx->preferred_backend) == 0 &&
			    dev->impl->supports_codec &&
			    dev->impl->supports_codec(dev, codec)) {
				return dev;
			}
		}
	}

	/* Try all backends in order of preference */
	/* When in auto mode, prefer vaapi over vulkan */
	const char *priority[] = {
		"vaapi",
		"vulkan",
		"software",
		NULL,
	};

	for (int p = 0; priority[p]; p++) {
		for (uint32_t i = 0; i < ctx->num_devices; i++) {
			struct wlf_hwdec_device *dev = ctx->devices[i];
			if (strcmp(dev->impl->name, priority[p]) == 0 &&
			    dev->impl->supports_codec &&
			    dev->impl->supports_codec(dev, codec)) {
				wlf_log(WLF_INFO, "Selected %s backend for %s",
					dev->impl->name, wlf_video_codec_to_string(codec));
				return dev;
			}
		}
	}

	wlf_log(WLF_WARN, "No suitable hwdec backend for %s",
		wlf_video_codec_to_string(codec));
	return NULL;
}

struct wlf_hwdec_device *wlf_hwdec_get_device_by_name(
	struct wlf_hwdec_context *ctx,
	const char *name) {

	if (!ctx || !ctx->devices || !name) {
		return NULL;
	}

	for (uint32_t i = 0; i < ctx->num_devices; i++) {
		if (strcmp(ctx->devices[i]->impl->name, name) == 0) {
			return ctx->devices[i];
		}
	}

	return NULL;
}

void wlf_hwdec_set_wayland_display(
	struct wlf_hwdec_device *device,
	struct wl_display *wl_display) {

	if (!device) {
		return;
	}

	device->wayland_display = wl_display;
}

struct wl_buffer *wlf_hwdec_export_to_wl_buffer(
	struct wlf_hwdec_device *device,
	struct wlf_video_image *image,
	struct wl_display *wl_display) {

	if (!device || !image) {
		wlf_log(WLF_ERROR, "Invalid device or image");
		return NULL;
	}

	if (!device->impl->export_to_wl_buffer) {
		wlf_log(WLF_ERROR, "Backend %s does not support wl_buffer export",
			device->impl->name);
		return NULL;
	}

	/* Use provided display or device's display */
	struct wl_display *display = wl_display ? wl_display : device->wayland_display;
	if (!display) {
		wlf_log(WLF_ERROR, "No Wayland display available for export");
		return NULL;
	}

	return device->impl->export_to_wl_buffer(device, image, display);
}
