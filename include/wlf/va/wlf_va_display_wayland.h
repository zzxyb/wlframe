#ifndef WLF_VA_DISPLAY_WAYLAND_H
#define WLF_VA_DISPLAY_WAYLAND_H

#include <va/va.h>
#include <wayland-client.h>
#include "wlf/va/wlf_video_common.h"

/**
 * Create VA-API display from Wayland display
 *
 * @param wl_display Wayland display connection
 * @return VA display handle, or NULL on failure
 */
VADisplay wlf_va_display_create_wayland(struct wl_display *wl_display);

// /**
//  * Destroy VA-API display
//  *
//  * @param va_display VA display handle
//  */
// void wlf_va_display_destroy(VADisplay va_display);

/**
 * Query codec support in VA-API
 *
 * @param va_display VA display handle
 * @param codec Codec to query
 * @param decode Output: decode support (can be NULL)
 * @param encode Output: encode support (can be NULL)
 * @return true if query succeeded
 */
bool wlf_va_query_codec_support(VADisplay va_display,
	enum wlf_video_codec codec, bool *decode, bool *encode);

#endif /* WLF_VA_DISPLAY_WAYLAND_H */
