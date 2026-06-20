#include "wlf/platform/windows/backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_compat.h"

#include <assert.h>
#include <stdlib.h>

static void windows_backend_destroy(struct wlf_backend *backend) {
	struct wlf_windows_backend *windows = wlf_windows_backend_from_backend(backend);
	free(windows);
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
	struct wlf_windows_backend *backend = calloc(1, sizeof(*backend));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_windows_backend");
		return NULL;
	}

	wlf_backend_init(&backend->base, &windows_backend_impl);
	backend->instance = GetModuleHandleW(NULL);
	backend->thread_id = GetCurrentThreadId();

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
