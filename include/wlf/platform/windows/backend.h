#ifndef WINDOWS_BACKEND_H
#define WINDOWS_BACKEND_H

#include "wlf/platform/wlf_backend.h"

#include <stdbool.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct wlf_windows_backend {
	struct wlf_backend base;
	HINSTANCE instance;
	DWORD thread_id;
};

struct wlf_backend *windows_backend_create(void);
bool wlf_backend_is_windows(const struct wlf_backend *backend);
struct wlf_windows_backend *wlf_windows_backend_from_backend(
	struct wlf_backend *backend);

#endif // WINDOWS_BACKEND_H
