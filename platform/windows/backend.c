#include "wlf/platform/windows/backend.h"
#include "wlf/platform/windows/output.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>

static void windows_backend_destroy(struct wlf_backend *backend) {
	struct wlf_windows_backend *windows = wlf_windows_backend_from_backend(backend);
	struct wlf_output *output, *temporary;
	wlf_linked_list_for_each_safe(output, temporary, &backend->outputs, link) {
		wlf_signal_emit_mutable(&backend->events.output_removed, output);
		wlf_output_destroy(output);
	}
	free(windows);
}

static BOOL CALLBACK add_monitor(HMONITOR monitor, HDC dc, LPRECT rect,
		LPARAM data) {
	WLF_UNUSED(dc);
	WLF_UNUSED(rect);
	struct wlf_windows_backend *backend = (struct wlf_windows_backend *)data;
	struct wlf_output *output = wlf_windows_output_create(monitor);
	if (output == NULL) {
		return TRUE;
	}
	wlf_linked_list_insert(backend->base.outputs.prev, &output->link);
	wlf_signal_emit_mutable(&backend->base.events.output_added, output);
	return TRUE;
}

static void windows_backend_exe(struct wlf_backend *backend) {
	backend->running = true;

	while (backend->running) {
		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				backend->running = false;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		if (backend->running) {
			MsgWaitForMultipleObjects(0, NULL, FALSE, INFINITE, QS_ALLINPUT);
		}
	}
}

static void *windows_backend_native_display(struct wlf_backend *backend) {
	struct wlf_windows_backend *windows = wlf_windows_backend_from_backend(backend);
	return windows->instance;
}

static const struct wlf_backend_impl windows_backend_impl = {
	.name = "Windows",
	.destroy = windows_backend_destroy,
	.exe = windows_backend_exe,
	.native_display = windows_backend_native_display,
};

struct wlf_backend *windows_backend_create(void) {
	if (!SetProcessDpiAwarenessContext(
			DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) &&
			GetLastError() != ERROR_ACCESS_DENIED) {
		wlf_log(WLF_INFO, "Failed to enable per-monitor DPI awareness: %lu",
			(unsigned long)GetLastError());
	}

	struct wlf_windows_backend *backend = calloc(1, sizeof(*backend));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_windows_backend");
		return NULL;
	}

	wlf_backend_init(&backend->base, &windows_backend_impl);
	backend->base.features.server_side_decorations = true;
	backend->instance = GetModuleHandleW(NULL);
	backend->thread_id = GetCurrentThreadId();
	EnumDisplayMonitors(NULL, NULL, add_monitor, (LPARAM)backend);

	wlf_log(WLF_DEBUG, "Created %s backend", backend->base.impl->name);

	return &backend->base;
}

bool wlf_backend_is_windows(const struct wlf_backend *backend) {
	return backend != NULL && backend->impl == &windows_backend_impl;
}

struct wlf_windows_backend *wlf_windows_backend_from_backend(
		struct wlf_backend *wlf_backend) {
	assert(wlf_backend && wlf_backend->impl == &windows_backend_impl);

	struct wlf_windows_backend *backend =
		wlf_container_of(wlf_backend, backend, base);
	return backend;
}
