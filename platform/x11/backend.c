#include "wlf/platform/x11/backend.h"
#include "wlf/platform/x11/xresource.h"
#include "wlf/window/x11/window.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void backend_destroy(struct wlf_backend *base) {
	struct wlf_x11_backend *backend = wlf_x11_backend_from_backend(base);
	assert(wlf_linked_list_empty(&backend->windows));
	wlf_pointer_destroy(&backend->pointer);
	wlf_keyboard_destroy(&backend->keyboard);
	wlf_xresource_manager_destroy(backend->resources);
	xcb_disconnect(backend->xcb);
	free(backend);
}

static void *backend_native_display(struct wlf_backend *base) {
	return wlf_x11_backend_from_backend(base)->xcb;
}

static int64_t monotonic_msec(void) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (int64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static void backend_exe(struct wlf_backend *base) {
	struct wlf_x11_backend *backend = wlf_x11_backend_from_backend(base);
	int64_t next_frame = monotonic_msec() + 16;
	base->running = true;
	while (base->running && !xcb_connection_has_error(backend->xcb)) {
		xcb_generic_event_t *event;
		while (base->running && (event = xcb_poll_for_event(backend->xcb)) != NULL) {
			if (!wlf_xresource_manager_handle_event(backend->resources, event)) {
				wlf_x11_window_handle_event(backend, event);
			}
			free(event);
		}
		if (!base->running || xcb_connection_has_error(backend->xcb)) {
			break;
		}

		/* Snapshot IDs: frame callbacks may destroy or create windows. */
		size_t count = 0;
		struct wlf_x11_window *window;
		wlf_linked_list_for_each(window, &backend->windows, link) {
			if (window->frame_pending && window->base.state.visible) {
				count++;
			}
		}
		xcb_window_t *frames = calloc(count ? count : 1, sizeof(*frames));
		size_t source_count = base->event_source_count;
		struct pollfd *fds = calloc(source_count + 1, sizeof(*fds));
		/* External callbacks may change the event-source array. */
		size_t source_size = source_count * sizeof(*base->event_sources);
		void *sources = malloc(source_size ? source_size : 1);
		if (frames == NULL || fds == NULL || sources == NULL) {
			free(frames);
			free(fds);
			free(sources);
			break;
		}
		if (source_count > 0) {
			memcpy(sources, base->event_sources, source_size);
		}
		size_t n = 0;
		wlf_linked_list_for_each(window, &backend->windows, link) {
			if (window->frame_pending && window->base.state.visible) {
				frames[n++] = window->window;
			}
		}
		fds[0] = (struct pollfd){ .fd = xcb_get_file_descriptor(backend->xcb),
			.events = POLLIN };
		for (size_t i = 0; i < source_count; i++) {
			fds[i + 1] = (struct pollfd){ .fd = base->event_sources[i].fd,
				.events = base->event_sources[i].events };
		}
		xcb_flush(backend->xcb);
		int64_t delay = next_frame - monotonic_msec();
		int ret = poll(fds, source_count + 1, count > 0 ? (delay > 0 ? delay : 0) : -1);
		if (ret < 0 && errno != EINTR) {
			wlf_log_errno(WLF_ERROR, "poll failed in X11 event loop");
			base->running = false;
		}
		/* Only deliver sources still registered after previous callbacks. */
		for (size_t i = 0; ret >= 0 && base->running && i < source_count; i++) {
			if (fds[i + 1].revents == 0) {
				continue;
			}
			for (size_t j = 0; j < base->event_source_count; j++) {
				if (memcmp((char *)sources + i * sizeof(*base->event_sources),
						&base->event_sources[j], sizeof(*base->event_sources)) == 0) {
					base->event_sources[j].dispatch(base, fds[i + 1].fd,
						fds[i + 1].revents, base->event_sources[j].data);
					break;
				}
			}
		}
		bool frame_due = monotonic_msec() >= next_frame;
		if (frame_due) {
			next_frame = monotonic_msec() + 16;
		}
		for (size_t i = 0; ret >= 0 && frame_due && base->running && i < count; i++) {
			wlf_linked_list_for_each(window, &backend->windows, link) {
				if (window->window == frames[i]) {
					if (!window->base.state.visible) {
						break;
					}
					window->frame_pending = false;
					wlf_signal_emit_mutable(&window->base.events.expose, NULL);
					break;
				}
			}
		}
		free(sources);
		free(fds);
		free(frames);
	}
	base->running = false;
}

static const struct wlf_backend_impl backend_impl = {
	.name = "X11",
	.destroy = backend_destroy,
	.exe = backend_exe,
	.native_display = backend_native_display,
};

/* Core input devices are embedded in the backend. */
static void pointer_destroy(struct wlf_pointer *pointer) {
}

static void keyboard_destroy(struct wlf_keyboard *keyboard) {
}

static const struct wlf_pointer_impl pointer_impl = {
	.name = "X11 pointer",
	.destroy = pointer_destroy,
};

static const struct wlf_keyboard_impl keyboard_impl = {
	.name = "X11 keyboard",
	.destroy = keyboard_destroy,
};

struct wlf_x11_backend *wlf_x11_backend_create(void) {
	struct wlf_x11_backend *backend = calloc(1, sizeof(*backend));
	if (backend == NULL) {
		return NULL;
	}
	wlf_backend_init(&backend->base, &backend_impl);
	wlf_pointer_init(&backend->pointer, &pointer_impl);
	wlf_keyboard_init(&backend->keyboard, &keyboard_impl);
	wlf_linked_list_init(&backend->windows);
	backend->xcb = xcb_connect(NULL, &backend->screen_number);
	if (xcb_connection_has_error(backend->xcb)) {
		wlf_log(WLF_ERROR, "Failed to connect to X server");
		goto error;
	}
	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(backend->xcb));
	for (int i = 0; i < backend->screen_number && iter.rem; i++) {
		xcb_screen_next(&iter);
	}
	if (!iter.rem) {
		goto error;
	}
	backend->screen = iter.data;
	struct {
		const char *name;
		xcb_atom_t *atom;
	} atoms[] = {
		{ "WM_PROTOCOLS", &backend->atoms.wm_protocols },
		{ "WM_DELETE_WINDOW", &backend->atoms.wm_delete_window },
		{ "UTF8_STRING", &backend->atoms.utf8_string },
		{ "_NET_WM_NAME", &backend->atoms.net_wm_name },
		{ "_NET_WM_STATE", &backend->atoms.net_wm_state },
		{ "_NET_WM_STATE_FULLSCREEN", &backend->atoms.net_wm_state_fullscreen },
		{ "_NET_WM_STATE_MAXIMIZED_VERT", &backend->atoms.net_wm_state_maximized_vert },
		{ "_NET_WM_STATE_MAXIMIZED_HORZ", &backend->atoms.net_wm_state_maximized_horz },
		{ "_MOTIF_WM_HINTS", &backend->atoms.motif_wm_hints },
	};
	xcb_intern_atom_cookie_t cookies[(sizeof(atoms) / sizeof(atoms[0]))];
	for (size_t i = 0; i < (sizeof(atoms) / sizeof(atoms[0])); i++) {
		cookies[i] = xcb_intern_atom(backend->xcb, false, strlen(atoms[i].name), atoms[i].name);
	}
	for (size_t i = 0; i < (sizeof(atoms) / sizeof(atoms[0])); i++) {
		xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(backend->xcb, cookies[i], NULL);
		if (reply == NULL) {
			goto error;
		}
		*atoms[i].atom = reply->atom;
		free(reply);
	}
	backend->resources = wlf_xresource_manager_create(backend->xcb);
	if (backend->resources == NULL) {
		goto error;
	}
	backend->base.features.server_side_decorations = true;
	return backend;
error:
	wlf_backend_destroy(&backend->base);
	return NULL;
}

bool wlf_backend_is_x11(const struct wlf_backend *backend) {
	return backend != NULL && backend->impl == &backend_impl;
}

struct wlf_x11_backend *wlf_x11_backend_from_backend(struct wlf_backend *base) {
	assert(wlf_backend_is_x11(base));
	struct wlf_x11_backend *backend = wlf_container_of(base, backend, base);
	return backend;
}
