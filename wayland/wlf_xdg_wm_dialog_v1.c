#include "wlf/wayland/wlf_xdg_wm_dialog_v1.h"
#include "wlf/wayland/wlf_xdg_wm_base.h"
#include "wayland/protocols/xdg-dialog-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct wlf_xdg_wm_dialog_v1 *wlf_xdg_wm_dialog_v1_create(
		struct wl_registry *wl_registry, uint32_t name, uint32_t version) {
	assert(wl_registry);

	uint32_t bind_ver = (uint32_t)xdg_wm_dialog_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_xdg_wm_dialog_v1 *manager = calloc(1, sizeof(*manager));
	if (manager == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_xdg_wm_dialog_v1");
		return NULL;
	}

	manager->base = wl_registry_bind(wl_registry, name,
		&xdg_wm_dialog_v1_interface, bind_ver);
	if (manager->base == NULL) {
		wlf_log(WLF_ERROR, "wl_registry_bind failed for xdg_wm_dialog_v1 (name: %u)", name);
		free(manager);
		return NULL;
	}

	manager->version = bind_ver;
	wlf_signal_init(&manager->events.destroy);

	wlf_log(WLF_DEBUG, "bound xdg_wm_dialog_v1 (name: %u, version: %u)", name, bind_ver);

	return manager;
}

void wlf_xdg_wm_dialog_v1_destroy(struct wlf_xdg_wm_dialog_v1 *manager) {
	if (manager == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&manager->events.destroy, manager);
	assert(wlf_linked_list_empty(&manager->events.destroy.listener_list));
	if (manager->base != NULL) {
		xdg_wm_dialog_v1_destroy(manager->base);
		manager->base = NULL;
	}

	free(manager);
}

struct wlf_xdg_dialog_v1 *wlf_xdg_wm_dialog_v1_get_xdg_dialog(
		struct wlf_xdg_wm_dialog_v1 *manager,
		struct xdg_toplevel *toplevel) {
	assert(manager);
	assert(manager->base);
	assert(toplevel);

	struct wlf_xdg_dialog_v1 *dialog = calloc(1, sizeof(*dialog));
	if (dialog == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_xdg_dialog_v1");
		return NULL;
	}

	dialog->base = xdg_wm_dialog_v1_get_xdg_dialog(manager->base, toplevel);
	if (dialog->base == NULL) {
		wlf_log(WLF_ERROR, "xdg_wm_dialog_v1_get_xdg_dialog() returned NULL");
		free(dialog);
		return NULL;
	}

	dialog->version = xdg_dialog_v1_get_version(dialog->base);
	dialog->is_modal = false;
	wlf_signal_init(&dialog->events.destroy);
	return dialog;
}

struct wlf_xdg_dialog_v1 *wlf_xdg_wm_dialog_v1_get_dialog_for_toplevel(
		struct wlf_xdg_wm_dialog_v1 *manager,
		struct wlf_xdg_toplevel *toplevel) {
	assert(toplevel);
	assert(toplevel->base);

	return wlf_xdg_wm_dialog_v1_get_xdg_dialog(manager, toplevel->base);
}

void wlf_xdg_dialog_v1_set_modal(struct wlf_xdg_dialog_v1 *dialog) {
	assert(dialog);
	assert(dialog->base);

	xdg_dialog_v1_set_modal(dialog->base);
	dialog->is_modal = true;
}

void wlf_xdg_dialog_v1_unset_modal(struct wlf_xdg_dialog_v1 *dialog) {
	assert(dialog);
	assert(dialog->base);

	xdg_dialog_v1_unset_modal(dialog->base);
	dialog->is_modal = false;
}

void wlf_xdg_dialog_v1_destroy(struct wlf_xdg_dialog_v1 *dialog) {
	if (dialog == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&dialog->events.destroy, dialog);
	assert(wlf_linked_list_empty(&dialog->events.destroy.listener_list));

	if (dialog->base != NULL) {
		xdg_dialog_v1_destroy(dialog->base);
		dialog->base = NULL;
	}

	free(dialog);
}
