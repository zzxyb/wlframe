#include "wlf/capture/wayland/wlf_wl_dmabuf.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <drm_fourcc.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static void linux_dmabuf_feedback_handle_main_device(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1, struct wl_array *device_arr) {
	struct wlf_wl_dmabuf *dmabuf = data;

	assert(dmabuf->drm_device == NULL);

	dev_t device;
	assert(device_arr->size == sizeof(device));
	memcpy(&device, device_arr->data, sizeof(device));

	if (drmGetDeviceFromDevId(device, 0, &dmabuf->drm_device) != 0) {
		wlf_log(WLF_ERROR, "unable to open main device");
		dmabuf->force_mod_linear = true;
		return;
	}
}

static void linux_dmabuf_feedback_format_table(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1, int fd, uint32_t size) {
	struct wlf_wl_dmabuf *dmabuf = data;

	wl_array_release(&dmabuf->format_modifier_pairs);
	wl_array_init(&dmabuf->format_modifier_pairs);

	dmabuf->feedback_data.format_table_data = mmap(NULL , size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (dmabuf->feedback_data.format_table_data == MAP_FAILED) {
		dmabuf->feedback_data.format_table_data = NULL;
		dmabuf->feedback_data.format_table_size = 0;
		return;
	}

	dmabuf->feedback_data.format_table_size = size;
}

static void linux_dmabuf_feedback_handle_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1) {
	struct wlf_wl_dmabuf *dmabuf = data;

	if (dmabuf->feedback_data.format_table_data) {
		munmap(dmabuf->feedback_data.format_table_data,
			dmabuf->feedback_data.format_table_size);
	}
	dmabuf->feedback_data.format_table_data = NULL;
	dmabuf->feedback_data.format_table_size = 0;
}

static void linux_dmabuf_feedback_tranche_target_devices(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1, struct wl_array *device_arr) {
	// struct wlf_wl_dmabuf *dmabuf = data;
	dev_t device;
	assert(device_arr->size == sizeof(device));
	memcpy(&device, device_arr->data, sizeof(device));

	drmDevice *drmDev;
	if (drmGetDeviceFromDevId(device, /* flags */ 0, &drmDev) != 0) {
		return;
	}

	// TODO: handle multiple target devices (if any)
}

static void linux_dmabuf_feedback_tranche_flags(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1, uint32_t flags) {
	WLF_UNUSED(data);
	WLF_UNUSED(zwp_linux_dmabuf_feedback_v1);
	WLF_UNUSED(flags);
}

static void linux_dmabuf_feedback_tranche_formats(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1, struct wl_array *indices) {
	struct wlf_wl_dmabuf *dmabuf = data;

	WLF_UNUSED(zwp_linux_dmabuf_feedback_v1);

	if (!dmabuf->feedback_data.device_used || !dmabuf->feedback_data.format_table_data) {
		return;
	}
	struct fm_entry {
		uint32_t format;
		uint32_t padding;
		uint64_t modifier;
	};
	// An entry in the table has to be 16 bytes long
	assert(sizeof(struct fm_entry) == 16);

	uint32_t n_modifiers = dmabuf->feedback_data.format_table_size/sizeof(struct fm_entry);
	struct fm_entry *fm_entry = dmabuf->feedback_data.format_table_data;
	uint16_t *idx;
	wl_array_for_each(idx, indices) {
		if (*idx >= n_modifiers) {
			continue;
		}
		wlf_wl_dmabuf_add_format_modifier_pair(dmabuf, (fm_entry + *idx)->format, (fm_entry + *idx)->modifier);
	}
}

static void linux_dmabuf_feedback_tranche_done(void *data,
		struct zwp_linux_dmabuf_feedback_v1 *zwp_linux_dmabuf_feedback_v1) {
	struct wlf_wl_dmabuf *dmabuf = data;
	dmabuf->feedback_data.device_used = false;
}

static const struct zwp_linux_dmabuf_feedback_v1_listener linux_dmabuf_listener_feedback = {
	.main_device = linux_dmabuf_feedback_handle_main_device,
	.format_table = linux_dmabuf_feedback_format_table,
	.done = linux_dmabuf_feedback_handle_done,
	.tranche_target_device = linux_dmabuf_feedback_tranche_target_devices,
	.tranche_flags = linux_dmabuf_feedback_tranche_flags,
	.tranche_formats = linux_dmabuf_feedback_tranche_formats,
	.tranche_done = linux_dmabuf_feedback_tranche_done,
};

static void linux_dmabuf_handle_format(void *data,
		struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1, uint32_t format) {
	WLF_UNUSED(data);
	WLF_UNUSED(zwp_linux_dmabuf_v1);
	WLF_UNUSED(format);
}

static void linux_dmabuf_handle_modifier(void *data,
		struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1,
		uint32_t format, uint32_t modifier_hi, uint32_t modifier_lo) {
	struct wlf_wl_dmabuf *dmabuf = data;
	uint64_t modifier = (((uint64_t)modifier_hi) << 32) | modifier_lo;
	wlf_wl_dmabuf_add_format_modifier_pair(dmabuf, format, modifier);
}

static const struct zwp_linux_dmabuf_v1_listener linux_dmabuf_listener = {
	.format = linux_dmabuf_handle_format,
	.modifier = linux_dmabuf_handle_modifier,
};

struct wlf_wl_dmabuf *wlf_wl_dmabuf_create(struct wl_registry *wl_registry,
		uint32_t name, uint32_t version) {
	assert(wl_registry != NULL);

	struct wlf_wl_dmabuf *dmabuf = malloc(sizeof(struct wlf_wl_dmabuf));
	if (dmabuf == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_wl_dmabuf");
		return NULL;
	}

	dmabuf->linux_dmabuf = NULL;
	dmabuf->linux_dmabuf_feedback = NULL;
	dmabuf->drm_device = NULL;
	dmabuf->force_mod_linear = false;

	uint32_t bind_version = version;
	if (version > (uint32_t)zwp_linux_dmabuf_v1_interface.version) {
		wlf_log(WLF_DEBUG, "Server linux_dmabuf version %u is higher than client version %u, "
			"using client version", version, (uint32_t)zwp_linux_dmabuf_v1_interface.version);
		bind_version = (uint32_t)zwp_linux_dmabuf_v1_interface.version;
	}

	dmabuf->linux_dmabuf = wl_registry_bind(wl_registry, name, &zwp_linux_dmabuf_v1_interface, bind_version);
	if (dmabuf->linux_dmabuf == NULL) {
		wlf_log(WLF_ERROR, "Failed to bind linux_dmabuf interface with name %u", name);
		free(dmabuf);
		return NULL;
	}

	if (zwp_linux_dmabuf_v1_get_version(dmabuf->linux_dmabuf) >= ZWP_LINUX_DMABUF_V1_GET_DEFAULT_FEEDBACK_SINCE_VERSION) {
		dmabuf->linux_dmabuf_feedback =
			zwp_linux_dmabuf_v1_get_default_feedback(dmabuf->linux_dmabuf);
		zwp_linux_dmabuf_feedback_v1_add_listener(dmabuf->linux_dmabuf_feedback, &linux_dmabuf_listener_feedback, dmabuf);
	} else {
		zwp_linux_dmabuf_v1_add_listener(dmabuf->linux_dmabuf, &linux_dmabuf_listener, dmabuf);
	}

	wlf_signal_init(&dmabuf->events.destroy);
	wl_array_init(&dmabuf->format_modifier_pairs);
	wlf_log(WLF_DEBUG, "Successfully bound linux_dmabuf interface (name: %u, version: %u)",
		name, bind_version);

	return dmabuf;
}

void wlf_wl_dmabuf_destroy(struct wlf_wl_dmabuf *dmabuf) {
	if (dmabuf == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&dmabuf->events.destroy, dmabuf);

	if (dmabuf->linux_dmabuf_feedback != NULL) {
		zwp_linux_dmabuf_feedback_v1_destroy(dmabuf->linux_dmabuf_feedback);
	}

	if (dmabuf->linux_dmabuf != NULL) {
		zwp_linux_dmabuf_v1_destroy(dmabuf->linux_dmabuf);
	}

	wl_array_release(&dmabuf->format_modifier_pairs);
	free(dmabuf);
}

void wlf_wl_dmabuf_add_format_modifier_pair(struct wlf_wl_dmabuf *dmabuf,
		uint32_t format, uint64_t modifier) {
	struct format_modifier_pair *fm_pair;
	wl_array_for_each(fm_pair, &dmabuf->format_modifier_pairs) {
		if (fm_pair->fourcc == format && fm_pair->modifier == modifier) {
			wlf_log(WLF_DEBUG, "skipping duplicated format %u (%lu)", fm_pair->fourcc, fm_pair->modifier);
			return;
		}
	}

	fm_pair = wl_array_add(&dmabuf->format_modifier_pairs, sizeof(struct format_modifier_pair));
	fm_pair->fourcc = format;
	fm_pair->modifier = modifier;
}

struct gbm_device *gbm_device_create(drmDevice *device) {
	if (!(device->available_nodes & (1 << DRM_NODE_RENDER))) {
		wlf_log(WLF_ERROR, "DRM device has no render node");
		return NULL;
	}

	const char *render_node = device->nodes[DRM_NODE_RENDER];
	wlf_log(WLF_INFO, "Using render node %s", render_node);

	int fd = open(render_node, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		wlf_log(WLF_ERROR, "Could not open render node %s", render_node);
		return NULL;
	}

	return gbm_create_device(fd);
}

void gbm_device_update(struct wlf_wl_dmabuf *dmabuf) {
	// drmDevice *new_dev = NULL;
	// if (drmGetDeviceFromDevId(cast->current_constraints.dmabuf_device, 0, &new_dev) != 0) {
	// 	// No device or invalid device, nothing to do
	// 	return;
	// }

	// if (!cast->ctx->gbm) {
	// 	// Our first device
	// 	cast->ctx->gbm = xdpw_gbm_device_create(new_dev);
	// 	drmFreeDevice(&new_dev);
	// 	return;
	// }

	// drmDevice *old_dev = NULL;
	// int fd = gbm_device_get_fd(cast->ctx->gbm);
	// if (drmGetDevice(fd, &old_dev) != 0 || !drmDevicesEqual(new_dev, old_dev)) {
	// 	// We either couldn't identify the old device or they didn't match, recreate
	// 	gbm_device_destroy(cast->ctx->gbm);
	// 	close(fd);
	// 	cast->ctx->gbm = xdpw_gbm_device_create(new_dev);
	// }

	// drmFreeDevice(&old_dev);
	// drmFreeDevice(&new_dev);
}
