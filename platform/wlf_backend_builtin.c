#include "wlf/platform/wlf_backend_builtin.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/platform/wlf_backend_wayland.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/config.h"

bool wlf_backend_builtin_init(void) {
	wlf_backend_init();
	bool success = false;

#if WLF_HAS_LINUX_PLATFORM
	if (wlf_get_env("WAYLAND_DISPLAY") || wlf_get_env("WAYLAND_SOCKET")) {
		wlf_log(WLF_INFO, "Create Wayland backend");
		success = wlf_backend_wayland_register();
		if (!success) {
			wlf_log(WLF_ERROR, "Failed to register Wayland backend");
			success = false;
		}
	}
#endif

	if (success) {
		wlf_log(WLF_INFO, "All built-in backends registered successfully");
	} else {
		wlf_log(WLF_INFO, "Some built-in backends failed to register");
	}

	return success;
}

void wlf_backend_builtin_cleanup(void) {
	wlf_log(WLF_INFO, "Cleaning up built-in backends");
	wlf_backend_finish();
}
