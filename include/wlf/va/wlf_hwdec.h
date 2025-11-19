/**
 * @file        wlf_hwdec.h
 * @brief       Hardware decoder backend abstraction for wlframe.
 * @details     This file defines the hardware decoder backend interface,
 *              supporting multiple backends (Vulkan, VA-API, software).
 *              Inspired by mpv's hwdec architecture.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-23
 * @version     v2.0
 * @par Copyright:
 * @par History:
 *      version: v2.0, YaoBing Xiao, 2026-01-23, multi-backend support\n
 */

#ifndef VA_WLF_HWDEC_H
#define VA_WLF_HWDEC_H

#include "wlf/va/wlf_video_common.h"
#include "wlf/utils/wlf_signal.h"
#include <stdbool.h>
#include <wayland-client.h>

struct wlf_hwdec_device;
struct wlf_hwdec_context;

/**
 * @struct wlf_hwdec_device_impl
 * @brief Hardware decoder device interface.
 */
struct wlf_hwdec_device_impl {
	const char *name;

	/* Initialization */
	bool (*init)(struct wlf_hwdec_device *device);
	void (*destroy)(struct wlf_hwdec_device *device);

	/* Capabilities */
	bool (*supports_codec)(struct wlf_hwdec_device *device, enum wlf_video_codec codec);
	bool (*supports_format)(struct wlf_hwdec_device *device, uint32_t format);

	/* Decoding */
	bool (*decode_frame)(struct wlf_hwdec_device *device,
		const uint8_t *bitstream, size_t size,
		struct wlf_video_image *output);

	/* Wayland integration */
	struct wl_buffer *(*export_to_wl_buffer)(struct wlf_hwdec_device *device,
		struct wlf_video_image *image,
		struct wl_display *wl_display);
};

/**
 * @struct wlf_hwdec_device
 * @brief Hardware decoder device instance.
 */
struct wlf_hwdec_device {
	const struct wlf_hwdec_device_impl *impl;

	struct {
		struct wlf_signal destroy;
	} events;

	void *device_context;   /**< Backend-specific device */
	void *priv;             /**< Private data */
	struct wl_display *wayland_display; /**< Wayland display for buffer export */
};

/**
 * @struct wlf_hwdec_context
 * @brief Hardware decoder context managing multiple backends.
 */
struct wlf_hwdec_context {
	struct wlf_hwdec_device **devices;
	uint32_t num_devices;

	const char *preferred_backend; /**< Preferred backend name (NULL for auto) */
	bool auto_fallback;            /**< Auto fallback to software on failure */
};

/**
 * @brief Create hardware decoder context.
 *
 * @param preferred_backend Preferred backend name (NULL or "auto" for auto-select)
 *                          Supported values: "vulkan", "vaapi", "software", "auto", or NULL
 *                          Can be overridden by WLF_HWDEC_BACKEND environment variable
 * @param auto_fallback Enable automatic fallback to software decode
 * @return Pointer to context, or NULL on failure
 *
 * @note When in auto mode (preferred_backend is NULL or "auto"), backends are
 *       selected in this priority order: VA-API > Vulkan > Software
 * @note Environment variable WLF_HWDEC_BACKEND takes precedence over preferred_backend
 */
struct wlf_hwdec_context *wlf_hwdec_context_create(
	const char *preferred_backend,
	bool auto_fallback);

/**
 * @brief Destroy hardware decoder context.
 *
 * @param ctx Pointer to context
 */
void wlf_hwdec_context_destroy(struct wlf_hwdec_context *ctx);

/**
 * @brief Get best device for codec.
 *
 * @param ctx Pointer to context
 * @param codec Video codec
 * @return Pointer to device, or NULL if none available
 */
struct wlf_hwdec_device *wlf_hwdec_get_device(
	struct wlf_hwdec_context *ctx,
	enum wlf_video_codec codec);

/**
 * @brief Get device by backend name.
 *
 * @param ctx Pointer to context
 * @param name Backend name (e.g., "vulkan", "vaapi", "software")
 * @return Pointer to device, or NULL if not available
 */
struct wlf_hwdec_device *wlf_hwdec_get_device_by_name(
	struct wlf_hwdec_context *ctx,
	const char *name);

/**
 * @brief Export decoded image to wl_buffer for Wayland compositing.
 *
 * @param device Pointer to hwdec device
 * @param image Decoded video image
 * @param wl_display Wayland display connection
 * @return wl_buffer handle, or NULL on failure
 *
 * @note The returned wl_buffer can be directly attached to a wl_surface
 *       and committed to the Wayland compositor for zero-copy presentation
 */
struct wl_buffer *wlf_hwdec_export_to_wl_buffer(
	struct wlf_hwdec_device *device,
	struct wlf_video_image *image,
	struct wl_display *wl_display);

/**
 * @brief Set Wayland display for the hwdec device.
 *
 * @param device Pointer to hwdec device
 * @param wl_display Wayland display connection
 */
void wlf_hwdec_set_wayland_display(
	struct wlf_hwdec_device *device,
	struct wl_display *wl_display);

/* Backend registration functions */
extern const struct wlf_hwdec_device_impl wlf_hwdec_vulkan_impl;
extern const struct wlf_hwdec_device_impl wlf_hwdec_vaapi_impl;
extern const struct wlf_hwdec_device_impl wlf_hwdec_software_impl;

#endif /* VA_WLF_HWDEC_H */
