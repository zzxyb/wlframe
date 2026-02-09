#include "wlf/ra/server/wlf_ra_rfb_server.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_LIBVNCSERVER
#include <rfb/rfb.h>
#endif

/**
 * @brief Internal libvncserver-specific data.
 */
struct wlf_ra_rfb_server_libvncserver {
#ifdef HAVE_LIBVNCSERVER
	rfbScreenInfoPtr screen;
#endif
	struct wlf_ra_rfb_server_config config;
	struct wlf_ra_framebuffer *framebuffer;
	bool running;
};

#ifdef HAVE_LIBVNCSERVER
/* Forward declarations */
static int libvncserver_start(struct wlf_ra_rfb_server *server);
static void libvncserver_stop(struct wlf_ra_rfb_server *server);
static void libvncserver_set_framebuffer(struct wlf_ra_rfb_server *server,
		struct wlf_ra_framebuffer *fb);
static void libvncserver_mark_rect_dirty(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_rect *rect);
static void libvncserver_update_cursor(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_point *pos);
static int libvncserver_process_events(struct wlf_ra_rfb_server *server,
		int timeout_ms);
static void libvncserver_send_clipboard(struct wlf_ra_rfb_server *server,
		const char *text);
static void libvncserver_destroy(struct wlf_ra_rfb_server *server);

/* libvncserver implementation vtable */
static const struct wlf_ra_rfb_server_impl libvncserver_impl = {
	.start = libvncserver_start,
	.stop = libvncserver_stop,
	.set_framebuffer = libvncserver_set_framebuffer,
	.mark_rect_dirty = libvncserver_mark_rect_dirty,
	.update_cursor = libvncserver_update_cursor,
	.process_events = libvncserver_process_events,
	.send_clipboard = libvncserver_send_clipboard,
	.destroy = libvncserver_destroy,
};

/* Get internal data from server */
static inline struct wlf_ra_rfb_server_libvncserver *
get_libvncserver_data(struct wlf_ra_rfb_server *server) {
	return (struct wlf_ra_rfb_server_libvncserver *)server->backend_data;
}

/* libvncserver callback: new client connection */
static enum rfbNewClientAction on_new_client(rfbClientPtr cl) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server) {
		return RFB_CLIENT_REFUSE;
	}

	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);

	/* Check if multiple clients allowed */
	if (!impl->config.allow_multiple_clients && server->client_count > 0) {
		return RFB_CLIENT_REFUSE;
	}

	server->client_count++;

	/* Set client gone hook */
	cl->clientGoneHook = on_client_gone;

	/* Emit client_connected signal */
	struct wlf_ra_client_event event = {
		.address = cl->host,
	};
	wlf_signal_emit_mutable(&server->events.client_connected, &event);

	return RFB_CLIENT_ACCEPT;
}

/* libvncserver callback: client disconnected */
static void on_client_gone(rfbClientPtr cl) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server) {
		return;
	}

	server->client_count--;

	/* Emit client_disconnected signal */
	struct wlf_ra_client_event event = {
		.address = cl->host,
	};
	wlf_signal_emit_mutable(&server->events.client_disconnected, &event);
}

/* libvncserver callback: password check */
static rfbBool on_password_check(rfbClientPtr cl, const char *encrypted_password, int len) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server) {
		return FALSE;
	}

	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);

	/* If no password configured, accept */
	if (!impl->config.password) {
		return TRUE;
	}

	/* Emit password_check signal */
	bool result = false;
	struct wlf_ra_password_check_event event = {
		.encrypted_password = encrypted_password,
		.result = &result,
	};
	wlf_signal_emit_mutable(&server->events.password_check, &event);

	/* If signal handled and returned true, accept */
	if (result) {
		return TRUE;
	}

	/* Otherwise use VNC authentication */
	char *correct_encrypted = rfbEncryptPasswordString(impl->config.password);
	rfbBool vnc_result = (memcmp(encrypted_password, correct_encrypted, len) == 0);
	free(correct_encrypted);

	return vnc_result;
}

/* libvncserver callback: keyboard event */
static void on_keyboard_event(rfbBool down, rfbKeySym keySym, rfbClientPtr cl) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server) {
		return;
	}

	/* Emit key_event signal */
	struct wlf_ra_key_event event = {
		.keysym = keySym,
		.pressed = (down != 0),
	};
	wlf_signal_emit_mutable(&server->events.key_event, &event);
}

/* libvncserver callback: pointer event */
static void on_pointer_event(int button_mask, int x, int y, rfbClientPtr cl) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server) {
		return;
	}

	/* Emit pointer_event signal */
	struct wlf_ra_pointer_event event = {
		.x = x,
		.y = y,
		.button_mask = button_mask,
	};
	wlf_signal_emit_mutable(&server->events.pointer_event, &event);
}

/* libvncserver callback: clipboard text */
static void on_clipboard_text(char *str, int len, rfbClientPtr cl) {
	struct wlf_ra_rfb_server *server = cl->screen->screenData;
	if (!server || !str) {
		return;
	}

	/* Make null-terminated copy */
	char *text = malloc(len + 1);
	if (!text) {
		return;
	}
	memcpy(text, str, len);
	text[len] = '\0';

	/* Emit clipboard_text signal */
	struct wlf_ra_clipboard_event event = {
		.text = text,
	};
	wlf_signal_emit_mutable(&server->events.clipboard_text, &event);

	free(text);
}

/* libvncserver implementation functions */
static int libvncserver_start(struct wlf_ra_rfb_server *server) {
	assert(server);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (!impl->framebuffer) {
		return -1;
	}

	/* Create screen */
	impl->screen = rfbGetScreen(NULL, NULL,
		impl->framebuffer->width,
		impl->framebuffer->height,
		8, 3, impl->framebuffer->depth / 8);

	if (!impl->screen) {
		return -1;
	}

	/* Configure screen */
	impl->screen->frameBuffer = impl->framebuffer->data;
	impl->screen->alwaysShared = impl->config.allow_multiple_clients;
	impl->screen->screenData = server;

	/* Set host and port */
	if (impl->config.listen_address) {
		strncpy(impl->screen->thisHost, impl->config.listen_address, 254);
	}

	if (impl->config.port == 0) {
		impl->screen->autoPort = TRUE;
	} else {
		impl->screen->port = impl->config.port;
	}

	/* Set callbacks */
	impl->screen->newClientHook = on_new_client;
	impl->screen->ptrAddEvent = on_pointer_event;
	impl->screen->kbdAddEvent = on_keyboard_event;
	impl->screen->setXCutText = on_clipboard_text;

	/* Set client disconnection callback */
	rfbClientIteratorPtr iterator = rfbGetClientIterator(impl->screen);
	rfbClientPtr cl;
	while ((cl = rfbClientIteratorNext(iterator)) != NULL) {
		cl->clientGoneHook = on_client_gone;
	}
	rfbReleaseClientIterator(iterator);

	/* Set password if configured */
	if (impl->config.password) {
		impl->screen->authPasswdData = (void *)impl->config.password;
		impl->screen->passwordCheck = on_password_check;
	}

	/* Initialize server */
	rfbInitServer(impl->screen);

	/* Get actual port */
	server->port = impl->screen->port;

	/* Run server in threaded mode */
	rfbRunEventLoop(impl->screen, -1, TRUE);

	impl->running = true;

	return 0;
}

static void libvncserver_stop(struct wlf_ra_rfb_server *server) {
	assert(server);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (!impl->running) {
		return;
	}

	if (impl->screen) {
		rfbShutdownServer(impl->screen, TRUE);
		rfbScreenCleanup(impl->screen);
		impl->screen = NULL;
	}

	impl->running = false;
}

static void libvncserver_set_framebuffer(struct wlf_ra_rfb_server *server,
		struct wlf_ra_framebuffer *fb) {
	assert(server);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	impl->framebuffer = fb;

	if (impl->screen) {
		impl->screen->frameBuffer = fb->data;
		impl->screen->width = fb->width;
		impl->screen->height = fb->height;
	}
}

static void libvncserver_mark_rect_dirty(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_rect *rect) {
	assert(server && rect);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (impl->screen) {
		rfbMarkRectAsModified(impl->screen,
			rect->x, rect->y,
			rect->x + rect->width,
			rect->y + rect->height);
	}
}

static void libvncserver_update_cursor(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_point *pos) {
	assert(server && pos);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (impl->screen) {
		impl->screen->cursorX = pos->x;
		impl->screen->cursorY = pos->y;
	}
}

static int libvncserver_process_events(struct wlf_ra_rfb_server *server,
		int timeout_ms) {
	assert(server);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (!impl->running) {
		return -1;
	}

	if (impl->screen) {
		long usec = timeout_ms * 1000;
		rfbProcessEvents(impl->screen, usec);
		return 0;
	}

	return -1;
}

static void libvncserver_send_clipboard(struct wlf_ra_rfb_server *server,
		const char *text) {
	assert(server && text);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	if (impl->screen) {
		rfbSendServerCutText(impl->screen, (char *)text, strlen(text));
	}
}

static void libvncserver_destroy(struct wlf_ra_rfb_server *server) {
	assert(server);
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	assert(impl);

	/* Stop server if running */
	if (impl->running) {
		libvncserver_stop(server);
	}

	/* Free config strings */
	free((void *)impl->config.listen_address);
	free((void *)impl->config.password);

	/* Free implementation data */
	free(impl);
}

#endif /* HAVE_LIBVNCSERVER */

/* Public API implementations */

/* Public API implementations */

struct wlf_ra_rfb_server *wlf_ra_rfb_server_create(
		const struct wlf_ra_rfb_server_config *config) {
	if (!config) {
		return NULL;
	}

#ifndef HAVE_LIBVNCSERVER
	fprintf(stderr, "RFB Server: libvncserver support not compiled in\n");
	return NULL;
#else
	/* Allocate server structure */
	struct wlf_ra_rfb_server *server = calloc(1, sizeof(*server));
	if (!server) {
		return NULL;
	}

	/* Allocate backend data */
	struct wlf_ra_rfb_server_libvncserver *impl =
		calloc(1, sizeof(*impl));
	if (!impl) {
		free(server);
		return NULL;
	}

	/* Copy config */
	impl->config = *config;
	if (config->listen_address) {
		impl->config.listen_address = strdup(config->listen_address);
	}
	if (config->password) {
		impl->config.password = strdup(config->password);
	}

	/* Initialize server structure */
	server->impl = &libvncserver_impl;
	server->backend_data = impl;
	server->port = 0;
	server->client_count = 0;

	/* Initialize all signals */
	wlf_signal_init(&server->events.client_connected);
	wlf_signal_init(&server->events.client_disconnected);
	wlf_signal_init(&server->events.password_check);
	wlf_signal_init(&server->events.key_event);
	wlf_signal_init(&server->events.pointer_event);
	wlf_signal_init(&server->events.clipboard_text);

	return server;
#endif
}

void wlf_ra_rfb_server_destroy(struct wlf_ra_rfb_server *server) {
	if (!server) {
		return;
	}

	assert(server->impl && server->impl->destroy);
	server->impl->destroy(server);

	/* Free server structure */
	free(server);
}

int wlf_ra_rfb_server_start(struct wlf_ra_rfb_server *server) {
	if (!server) {
		return -1;
	}

	assert(server->impl && server->impl->start);
	return server->impl->start(server);
}

void wlf_ra_rfb_server_stop(struct wlf_ra_rfb_server *server) {
	if (!server) {
		return;
	}

	assert(server->impl && server->impl->stop);
	server->impl->stop(server);
}

void wlf_ra_rfb_server_set_framebuffer(struct wlf_ra_rfb_server *server,
		struct wlf_ra_framebuffer *fb) {
	if (!server) {
		return;
	}

	assert(server->impl && server->impl->set_framebuffer);
	server->impl->set_framebuffer(server, fb);
}

void wlf_ra_rfb_server_mark_rect_dirty(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_rect *rect) {
	if (!server || !rect) {
		return;
	}

	assert(server->impl && server->impl->mark_rect_dirty);
	server->impl->mark_rect_dirty(server, rect);
}

void wlf_ra_rfb_server_mark_screen_dirty(struct wlf_ra_rfb_server *server) {
	if (!server) {
		return;
	}

#ifdef HAVE_LIBVNCSERVER
	struct wlf_ra_rfb_server_libvncserver *impl = get_libvncserver_data(server);
	if (!impl || !impl->framebuffer) {
		return;
	}

	struct wlf_ra_rect rect = {
		.x = 0,
		.y = 0,
		.width = impl->framebuffer->width,
		.height = impl->framebuffer->height,
	};

	wlf_ra_rfb_server_mark_rect_dirty(server, &rect);
#endif
}

void wlf_ra_rfb_server_update_cursor(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_point *pos) {
	if (!server || !pos) {
		return;
	}

	assert(server->impl && server->impl->update_cursor);
	server->impl->update_cursor(server, pos);
}

int wlf_ra_rfb_server_process_events(struct wlf_ra_rfb_server *server,
		int timeout_ms) {
	if (!server) {
		return -1;
	}

	assert(server->impl && server->impl->process_events);
	return server->impl->process_events(server, timeout_ms);
}

int wlf_ra_rfb_server_get_port(struct wlf_ra_rfb_server *server) {
	return server ? server->port : -1;
}

int wlf_ra_rfb_server_get_client_count(struct wlf_ra_rfb_server *server) {
	return server ? server->client_count : 0;
}

void wlf_ra_rfb_server_send_clipboard(struct wlf_ra_rfb_server *server,
		const char *text) {
	if (!server || !text) {
		return;
	}

	assert(server->impl && server->impl->send_clipboard);
	server->impl->send_clipboard(server, text);
}
