
#ifndef WAYLAND_WLF_WL_DMABUF_H
#define WAYLAND_WLF_WL_DMABUF_H

#include "wlf/utils/wlf_signal.h"
#include "protocols/linux-dmabuf-v1-client-protocol.h"

#include <xf86drm.h>
#include <gbm.h>

#include <wayland-util.h>

struct format_modifier_pair {
	uint32_t fourcc;
	uint64_t modifier;
};

struct dmabuf_feedback_data {
	void *format_table_data;
	uint32_t format_table_size;
	bool device_used;
};

struct wlf_wl_dmabuf {
	struct gbm_device *gbm_device;
	struct zwp_linux_dmabuf_v1 *linux_dmabuf;
	struct zwp_linux_dmabuf_feedback_v1 *linux_dmabuf_feedback;
	drmDevice *drm_device;

	struct {
		struct wlf_signal destroy;       /**< Signal emitted when dmabuf is destroyed */
	} events;

	struct wl_array format_modifier_pairs;
	bool force_mod_linear;
	struct dmabuf_feedback_data feedback_data;
};

struct wlf_wl_dmabuf *wlf_wl_dmabuf_create(
	struct wl_registry *wl_registry, uint32_t name, uint32_t version);

void wlf_wl_dmabuf_destroy(struct wlf_wl_dmabuf *dmabuf);

void wlf_wl_dmabuf_add_format_modifier_pair(struct wlf_wl_dmabuf *dmabuf,
	uint32_t format, uint64_t modifier);
struct gbm_device *gbm_device_create(drmDevice *device);
void gbm_device_update(struct wlf_wl_dmabuf *dmabuf);

#endif // WAYLAND_WLF_WL_DMABUF_H
