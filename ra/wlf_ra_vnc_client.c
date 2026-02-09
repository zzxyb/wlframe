#include "wlf/ra/vnc/wlf_ra_vnc_client.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LIBVNCCLIENT
#include <rfb/rfbclient.h>
#endif

/**
 * @brief Internal structure for libvncclient backend data.
 */
struct wlf_ra_vnc_client_libvncclient {
#ifdef HAVE_LIBVNCCLIENT
	rfbClient *rfb_client;
#endif
	struct wlf_ra_vnc_options options;
	bool quit_flag;
};

#ifdef HAVE_LIBVNCCLIENT
/* ========== libvncclient callbacks ========== */

static rfbBool libvncclient_malloc_framebuffer(rfbClient *rfb_client) {
	struct wlf_ra_vnc_client *client = rfbClientGetClientData(rfb_client, NULL);
	if (!client) {
		return FALSE;
	}

	/* Destroy old framebuffer if exists */
	if (client->framebuffer) {
		wlf_ra_framebuffer_destroy(client->framebuffer);
	}

	/* Create new framebuffer with server dimensions */
	client->framebuffer = wlf_ra_framebuffer_create(
		rfb_client->width,
		rfb_client->height,
		rfb_client->format.bitsPerPixel
	);

	if (!client->framebuffer) {
		return FALSE;
	}

	/* Point libvncclient to our framebuffer */
	rfb_client->frameBuffer = (uint8_t *)wlf_ra_framebuffer_get_data(client->framebuffer);

	/* Update pixel format */
	client->framebuffer->format.bits_per_pixel = rfb_client->format.bitsPerPixel;
	client->framebuffer->format.depth = rfb_client->format.depth;
	client->framebuffer->format.big_endian = rfb_client->format.bigEndian;
	client->framebuffer->format.true_color = rfb_client->format.trueColour;
	client->framebuffer->format.red_max = rfb_client->format.redMax;
	client->framebuffer->format.green_max = rfb_client->format.greenMax;
	client->framebuffer->format.blue_max = rfb_client->format.blueMax;
	client->framebuffer->format.red_shift = rfb_client->format.redShift;
	client->framebuffer->format.green_shift = rfb_client->format.greenShift;
	client->framebuffer->format.blue_shift = rfb_client->format.blueShift;

	return TRUE;
}

static void libvncclient_framebuffer_update(rfbClient *rfb_client, int x, int y, int w, int h) {
	struct wlf_ra_vnc_client *client = rfbClientGetClientData(rfb_client, NULL);
	if (!client) {
		return;
	}

	/* Emit frame_update signal */
	struct wlf_ra_frame_update_event event = {
		.x = x,
		.y = y,
		.width = w,
		.height = h,
	};
	wlf_signal_emit_mutable(&client->events.frame_update, &event);
}

static void libvncclient_got_cut_text(rfbClient *rfb_client, const char *text, int len) {
	(void)rfb_client;
	(void)text;
	(void)len;
	/* Could emit clipboard signal if needed */
}

static char *libvncclient_get_password(rfbClient *rfb_client) {
	struct wlf_ra_vnc_client *client = rfbClientGetClientData(rfb_client, NULL);
	if (!client) {
		return NULL;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend) {
		return NULL;
	}

	/* Emit password_required signal */
	wlf_signal_emit_mutable(&client->events.password_required, NULL);

	if (backend->options.password) {
		return strdup(backend->options.password);
	}

	return NULL;
}

static void libvncclient_finished_frame_buffer_update(rfbClient *rfb_client) {
	(void)rfb_client;
}

/* ========== libvncclient implementation functions ========== */

static int libvncclient_connect(struct wlf_ra_vnc_client *client) {
	assert(client);

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend || !backend->rfb_client) {
		struct wlf_ra_error_event error_event = {
			.message = "Invalid backend data",
		};
		wlf_signal_emit_mutable(&client->events.error, &error_event);
		return -1;
	}

	if (!rfbInitClient(backend->rfb_client, NULL, NULL)) {
		struct wlf_ra_error_event error_event = {
			.message = "Failed to connect to VNC server",
		};
		wlf_signal_emit_mutable(&client->events.error, &error_event);
		return -1;
	}

	client->connected_flag = true;

	/* Emit connected signal */
	wlf_signal_emit_mutable(&client->events.connected, NULL);

	return 0;
}

static void libvncclient_disconnect(struct wlf_ra_vnc_client *client) {
	assert(client);

	if (!client->connected_flag) {
		return;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (backend) {
		backend->quit_flag = true;
	}

	client->connected_flag = false;

	/* Emit disconnected signal */
	wlf_signal_emit_mutable(&client->events.disconnected, NULL);
}

static void libvncclient_send_key(struct wlf_ra_vnc_client *client,
		uint32_t keysym, bool pressed) {
	assert(client);

	if (!client->connected_flag) {
		return;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend || backend->options.view_only) {
		return;
	}

	SendKeyEvent(backend->rfb_client, keysym, pressed);
}

static void libvncclient_send_pointer(struct wlf_ra_vnc_client *client,
		int x, int y, uint32_t button_mask) {
	assert(client);

	if (!client->connected_flag) {
		return;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend || backend->options.view_only) {
		return;
	}

	SendPointerEvent(backend->rfb_client, x, y, button_mask);
}

static void libvncclient_send_clipboard(struct wlf_ra_vnc_client *client,
		const char *text) {
	assert(client);

	if (!client->connected_flag || !text) {
		return;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend) {
		return;
	}

	SendClientCutText(backend->rfb_client, (char *)text, strlen(text));
}

static int libvncclient_process_events(struct wlf_ra_vnc_client *client) {
	assert(client);

	if (!client->connected_flag) {
		return -1;
	}

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend || !backend->rfb_client) {
		return -1;
	}

	int ret = WaitForMessage(backend->rfb_client, 500);
	if (ret < 0) {
		struct wlf_ra_error_event error_event = {
			.message = "Connection error",
		};
		wlf_signal_emit_mutable(&client->events.error, &error_event);
		return -1;
	}

	if (ret > 0) {
		if (!HandleRFBServerMessage(backend->rfb_client)) {
			struct wlf_ra_error_event error_event = {
				.message = "Protocol error",
			};
			wlf_signal_emit_mutable(&client->events.error, &error_event);
			return -1;
		}
	}

	return 0;
}

static void libvncclient_destroy(struct wlf_ra_vnc_client *client) {
	assert(client);

	struct wlf_ra_vnc_client_libvncclient *backend = client->backend_data;
	if (!backend) {
		return;
	}

	if (backend->rfb_client) {
		rfbClientCleanup(backend->rfb_client);
	}

	free((void *)backend->options.host);
	free((void *)backend->options.password);
	free(backend);

	client->backend_data = NULL;
}

static const struct wlf_ra_vnc_client_impl libvncclient_impl = {
	.connect = libvncclient_connect,
	.disconnect = libvncclient_disconnect,
	.send_key = libvncclient_send_key,
	.send_pointer = libvncclient_send_pointer,
	.send_clipboard = libvncclient_send_clipboard,
	.process_events = libvncclient_process_events,
	.destroy = libvncclient_destroy,
};

#endif /* HAVE_LIBVNCCLIENT */

/* ========== Public API Functions ========== */

struct wlf_ra_vnc_client *wlf_ra_vnc_client_create(
		const struct wlf_ra_vnc_options *options) {
	if (!options || !options->host) {
		return NULL;
	}

#ifndef HAVE_LIBVNCCLIENT
	/* No VNC support compiled in */
	return NULL;
#else
	struct wlf_ra_vnc_client *client = calloc(1, sizeof(struct wlf_ra_vnc_client));
	if (!client) {
		return NULL;
	}

	/* Initialize signals */
	wlf_signal_init(&client->events.connected);
	wlf_signal_init(&client->events.disconnected);
	wlf_signal_init(&client->events.frame_update);
	wlf_signal_init(&client->events.password_required);
	wlf_signal_init(&client->events.error);

	/* Create backend data */
	struct wlf_ra_vnc_client_libvncclient *backend = 
		calloc(1, sizeof(struct wlf_ra_vnc_client_libvncclient));
	if (!backend) {
		free(client);
		return NULL;
	}

	/* Copy options */
	backend->options = *options;
	if (options->host) {
		backend->options.host = strdup(options->host);
	}
	if (options->password) {
		backend->options.password = strdup(options->password);
	}

	/* Create RFB client */
	backend->rfb_client = rfbGetClient(8, 3, 4);
	if (!backend->rfb_client) {
		free((void *)backend->options.host);
		free((void *)backend->options.password);
		free(backend);
		free(client);
		return NULL;
	}

	/* Set callbacks */
	backend->rfb_client->MallocFrameBuffer = libvncclient_malloc_framebuffer;
	backend->rfb_client->GotFrameBufferUpdate = libvncclient_framebuffer_update;
	backend->rfb_client->GotXCutText = libvncclient_got_cut_text;
	backend->rfb_client->GetPassword = libvncclient_get_password;
	backend->rfb_client->FinishedFrameBufferUpdate = libvncclient_finished_frame_buffer_update;

	/* Store our data in rfb client */
	rfbClientSetClientData(backend->rfb_client, NULL, client);

	/* Configure quality */
	if (options->quality >= 0 && options->quality <= 9) {
		backend->rfb_client->appData.compressLevel = 9 - options->quality;
		backend->rfb_client->appData.qualityLevel = options->quality;
		backend->rfb_client->appData.enableJPEG = (options->quality < 9);
	}

	/* Configure connection */
	backend->rfb_client->serverHost = strdup(options->host);
	backend->rfb_client->serverPort = options->port > 0 ? options->port : 5900;

	/* Set implementation and backend data */
	client->impl = &libvncclient_impl;
	client->backend_data = backend;
	client->framebuffer = NULL;
	client->connected_flag = false;

	return client;
#endif
}

void wlf_ra_vnc_client_destroy(struct wlf_ra_vnc_client *client) {
	if (!client) {
		return;
	}

	/* Disconnect first */
	if (client->impl && client->impl->disconnect) {
		client->impl->disconnect(client);
	}

	/* Destroy framebuffer */
	if (client->framebuffer) {
		wlf_ra_framebuffer_destroy(client->framebuffer);
	}

	/* Call backend destroy */
	if (client->impl && client->impl->destroy) {
		client->impl->destroy(client);
	}

	free(client);
}

int wlf_ra_vnc_client_connect(struct wlf_ra_vnc_client *client) {
	if (!client || !client->impl || !client->impl->connect) {
		return -1;
	}

	return client->impl->connect(client);
}

void wlf_ra_vnc_client_disconnect(struct wlf_ra_vnc_client *client) {
	if (!client || !client->impl || !client->impl->disconnect) {
		return;
	}

	client->impl->disconnect(client);
}

void wlf_ra_vnc_client_send_key(struct wlf_ra_vnc_client *client,
		uint32_t keysym, bool pressed) {
	if (!client || !client->impl || !client->impl->send_key) {
		return;
	}

	client->impl->send_key(client, keysym, pressed);
}

void wlf_ra_vnc_client_send_pointer(struct wlf_ra_vnc_client *client,
		int x, int y, uint32_t button_mask) {
	if (!client || !client->impl || !client->impl->send_pointer) {
		return;
	}

	client->impl->send_pointer(client, x, y, button_mask);
}

void wlf_ra_vnc_client_send_clipboard(struct wlf_ra_vnc_client *client,
		const char *text) {
	if (!client || !client->impl || !client->impl->send_clipboard) {
		return;
	}

	client->impl->send_clipboard(client, text);
}

struct wlf_ra_framebuffer *wlf_ra_vnc_client_get_framebuffer(
		struct wlf_ra_vnc_client *client) {
	return client ? client->framebuffer : NULL;
}

bool wlf_ra_vnc_client_is_connected(struct wlf_ra_vnc_client *client) {
	return client ? client->connected_flag : false;
}

int wlf_ra_vnc_client_process_events(struct wlf_ra_vnc_client *client) {
	if (!client || !client->impl || !client->impl->process_events) {
		return -1;
	}

	return client->impl->process_events(client);
}
