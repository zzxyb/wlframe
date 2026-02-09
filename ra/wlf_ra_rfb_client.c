#include "wlf/ra/client/wlf_ra_rfb_client.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_LIBVNCSERVER
#include <rfb/rfb.h>
#endif

/**
 * @brief RFB client structure (server-side representation).
 */
struct wlf_ra_rfb_client {
#ifdef HAVE_LIBVNCSERVER
	rfbClientPtr rfb_client;
#endif
	bool control_enabled;
};

void wlf_ra_rfb_client_get_info(struct wlf_ra_rfb_client *client,
		struct wlf_ra_rfb_client_info *info) {
	if (!client || !info) {
		return;
	}

	memset(info, 0, sizeof(struct wlf_ra_rfb_client_info));

#ifdef HAVE_LIBVNCSERVER
	if (client->rfb_client) {
		snprintf(info->address, sizeof(info->address), "%s",
			client->rfb_client->host);
		info->authenticated = (client->rfb_client->state == RFB_NORMAL);
		info->control_enabled = client->control_enabled;
	}
#endif
}

void wlf_ra_rfb_client_set_control_enabled(struct wlf_ra_rfb_client *client,
		bool enabled) {
	if (!client) {
		return;
	}

	client->control_enabled = enabled;
}

bool wlf_ra_rfb_client_control_enabled(struct wlf_ra_rfb_client *client) {
	return client ? client->control_enabled : false;
}

void wlf_ra_rfb_client_disconnect(struct wlf_ra_rfb_client *client) {
	if (!client) {
		return;
	}

#ifdef HAVE_LIBVNCSERVER
	if (client->rfb_client) {
		rfbCloseClient(client->rfb_client);
	}
#endif
}

void wlf_ra_rfb_client_send_clipboard(struct wlf_ra_rfb_client *client,
		const char *text) {
	if (!client || !text) {
		return;
	}

#ifdef HAVE_LIBVNCSERVER
	if (client->rfb_client && client->rfb_client->screen) {
		rfbSendServerCutText(client->rfb_client->screen, (char *)text, strlen(text));
	}
#endif
}
