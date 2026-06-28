#include "wlf/platform/macos/backend.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>

enum {
	WLF_MACOS_EVENT_LOOP_TIMEOUT_MS = 16,
};

static bool backend_ensure_appkit(struct wlf_backend_macos *macos) {
	WLF_UNUSED(macos);

	@autoreleasepool {
		if (NSApp == nil) {
			[NSApplication sharedApplication];
		}

		[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	}

	return true;
}

static void backend_dispatch_event_sources(struct wlf_backend *backend,
		int timeout_ms) {
	if (backend->event_source_count == 0) {
		if (timeout_ms > 0) {
			(void)poll(NULL, 0, timeout_ms);
		}
		return;
	}

	struct pollfd *fds = calloc(backend->event_source_count, sizeof(*fds));
	if (fds == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate pollfd array");
		backend->running = false;
		return;
	}

	for (size_t i = 0; i < backend->event_source_count; ++i) {
		fds[i].fd = backend->event_sources[i].fd;
		fds[i].events = backend->event_sources[i].events;
	}

	int poll_ret = poll(fds, backend->event_source_count, timeout_ms);
	if (poll_ret < 0) {
		if (errno != EINTR) {
			wlf_log_errno(WLF_ERROR, "poll() failed in macOS event loop");
			backend->running = false;
		}
		free(fds);
		return;
	}

	if (poll_ret == 0) {
		free(fds);
		return;
	}

	for (size_t i = 0; i < backend->event_source_count; ++i) {
		if (fds[i].revents == 0 || backend->event_sources[i].dispatch == NULL) {
			continue;
		}

		backend->event_sources[i].dispatch(
			backend,
			backend->event_sources[i].fd,
			(uint32_t)fds[i].revents,
			backend->event_sources[i].data);
	}

	free(fds);
}

static bool backend_process_appkit_events(struct wlf_backend_macos *macos) {
	bool handled_event = false;

	@autoreleasepool {
		if (!backend_ensure_appkit(macos)) {
			return false;
		}

		for (;;) {
			NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
				untilDate:[NSDate distantPast]
				inMode:NSDefaultRunLoopMode
				dequeue:YES];
			if (event == nil) {
				break;
			}

			handled_event = true;
			[NSApp sendEvent:event];
			[NSApp updateWindows];
		}
	}

	return handled_event;
}

static void *backend_native_display(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = wlf_backend_macos_from_backend(backend);
	if (!backend_ensure_appkit(macos)) {
		return NULL;
	}

	return (__bridge void *)NSApp;
}

static void backend_destroy(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = wlf_backend_macos_from_backend(backend);

	free(macos);
}

static void backend_exe(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = wlf_backend_macos_from_backend(backend);

	if (!backend_ensure_appkit(macos)) {
		backend->running = false;
		return;
	}

	backend->running = true;

	while (backend->running) {
		bool handled_appkit_event = backend_process_appkit_events(macos);

		/*
		 * Pump AppKit opportunistically, then block briefly on registered
		 * descriptors so external sources can participate in the backend loop.
		 */
		int timeout_ms = handled_appkit_event ? 0 : WLF_MACOS_EVENT_LOOP_TIMEOUT_MS;
		backend_dispatch_event_sources(backend, timeout_ms);
	}
}

static const struct wlf_backend_impl macos_backend_impl = {
	.name = "macOS",
	.destroy = backend_destroy,
	.exe = backend_exe,
	.native_display = backend_native_display,
};

struct wlf_backend *macos_backend_create(void) {
	struct wlf_backend_macos *backend = calloc(1, sizeof(struct wlf_backend_macos));
	if (backend == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_backend_macos");
		return NULL;
	}

	wlf_backend_init(&backend->base, &macos_backend_impl);
	backend->base.data = backend;

	if (!backend_ensure_appkit(backend)) {
		free(backend);
		return NULL;
	}

	return &backend->base;
}

bool wlf_backend_macos_register(void) {
	wlf_log(WLF_DEBUG, "macOS backend registration is a no-op");
	return true;
}

bool wlf_backend_is_macos(struct wlf_backend *backend) {
	return backend != NULL && backend->impl == &macos_backend_impl;
}

struct wlf_backend_macos *wlf_backend_macos_from_backend(struct wlf_backend *backend) {
	struct wlf_backend_macos *macos = NULL;

	assert(backend != NULL);
	assert(backend->impl == &macos_backend_impl);

	return wlf_container_of(backend, macos, base);
}
