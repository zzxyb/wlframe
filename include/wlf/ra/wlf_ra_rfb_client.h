/**
 * @file        wlf_ra_rfb_client.h
 * @brief       RFB client connection management (server-side).
 * @details     Manages individual client connections on the RFB server side,
 *              including control permissions and disconnection.
 * @author      YaoBing Xiao
 * @date        2026-02-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-10, initial version\n
 */

#ifndef RA_CLIENT_WLF_RA_RFB_CLIENT_H
#define RA_CLIENT_WLF_RA_RFB_CLIENT_H

#include "../common/wlf_ra_types.h"

/**
 * @brief RFB Client - represents a connected client to our RFB server.
 */
struct wlf_ra_rfb_client;

/**
 * @brief Client information.
 */
struct wlf_ra_rfb_client_info {
	char address[128];    /**< Client IP address */
	int port;             /**< Client port */
	bool authenticated;   /**< Whether client is authenticated */
	bool control_enabled; /**< Whether client can send input events */
};

/**
 * @brief Gets client information.
 * @param client Client instance.
 * @param info Output client information.
 */
void wlf_ra_rfb_client_get_info(struct wlf_ra_rfb_client *client,
		struct wlf_ra_rfb_client_info *info);

/**
 * @brief Enables/disables client control (input events).
 * @param client Client instance.
 * @param enabled True to enable, false to disable.
 */
void wlf_ra_rfb_client_set_control_enabled(struct wlf_ra_rfb_client *client,
		bool enabled);

/**
 * @brief Checks if client can send control events.
 * @param client Client instance.
 * @return True if control is enabled, false otherwise.
 */
bool wlf_ra_rfb_client_control_enabled(struct wlf_ra_rfb_client *client);

/**
 * @brief Disconnects client.
 * @param client Client instance.
 */
void wlf_ra_rfb_client_disconnect(struct wlf_ra_rfb_client *client);

/**
 * @brief Sends text to client clipboard.
 * @param client Client instance.
 * @param text Text to send.
 */
void wlf_ra_rfb_client_send_clipboard(struct wlf_ra_rfb_client *client,
		const char *text);

#endif /* RA_CLIENT_WLF_RA_RFB_CLIENT_H */
