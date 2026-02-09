/**
 * @file        wlf_ra_vnc_client.h
 * @brief       VNC client implementation for remote desktop connection.
 * @details     Provides VNC client functionality to connect to remote VNC servers,
 *              receive screen updates, and send input events.
 * @author      YaoBing Xiao
 * @date        2026-02-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-10, initial version\n
 */

#ifndef RA_VNC_WLF_RA_VNC_CLIENT_H
#define RA_VNC_WLF_RA_VNC_CLIENT_H

#include "../common/wlf_ra_types.h"
#include "../common/wlf_ra_framebuffer.h"
#include "wlf/utils/wlf_signal.h"

/**
 * @brief VNC client virtual methods implementation.
 */
struct wlf_ra_vnc_client_impl {
	/**
	 * @brief Connects to VNC server.
	 * @param client VNC client.
	 * @return 0 on success, negative on error.
	 */
	int (*connect)(struct wlf_ra_vnc_client *client);

	/**
	 * @brief Disconnects from VNC server.
	 * @param client VNC client.
	 */
	void (*disconnect)(struct wlf_ra_vnc_client *client);

	/**
	 * @brief Sends keyboard event to server.
	 * @param client VNC client.
	 * @param keysym X11 keysym.
	 * @param pressed True if pressed, false if released.
	 */
	void (*send_key)(struct wlf_ra_vnc_client *client, uint32_t keysym, bool pressed);

	/**
	 * @brief Sends pointer event to server.
	 * @param client VNC client.
	 * @param x X coordinate.
	 * @param y Y coordinate.
	 * @param button_mask Button mask.
	 */
	void (*send_pointer)(struct wlf_ra_vnc_client *client, int x, int y, uint32_t button_mask);

	/**
	 * @brief Sends clipboard text to server.
	 * @param client VNC client.
	 * @param text Text to send.
	 */
	void (*send_clipboard)(struct wlf_ra_vnc_client *client, const char *text);

	/**
	 * @brief Processes pending events.
	 * @param client VNC client.
	 * @return 0 on success, negative on error.
	 */
	int (*process_events)(struct wlf_ra_vnc_client *client);

	/**
	 * @brief Destroys the client.
	 * @param client VNC client.
	 */
	void (*destroy)(struct wlf_ra_vnc_client *client);
};

/**
 * @brief VNC client connection.
 */
struct wlf_ra_vnc_client {
	const struct wlf_ra_vnc_client_impl *impl;  /**< Virtual method table */

	struct {
		struct wlf_signal connected;             /**< Signal emitted when connected */
		struct wlf_signal disconnected;          /**< Signal emitted when disconnected */
		struct wlf_signal frame_update;          /**< Signal emitted on frame update */
		struct wlf_signal password_required;     /**< Signal emitted when password is required */
		struct wlf_signal error;                 /**< Signal emitted on error */
	} events;

	struct wlf_ra_framebuffer *framebuffer;      /**< Associated framebuffer */
	bool connected_flag;                         /**< Connection status */
	void *backend_data;                          /**< Backend-specific data */
};

/**
 * @brief VNC connection options.
 */
struct wlf_ra_vnc_options {
	const char *host;                              /**< Server hostname */
	int port;                                      /**< Server port */
	const char *password;                          /**< Password */
	bool view_only;                                /**< View-only mode */
	bool use_local_cursor;                         /**< Use local cursor */
	int quality;                                   /**< Quality 0-9, where 9 is best */
	void *user_data;                               /**< User data */
};

/**
 * @brief Creates VNC client connection.
 * @param options Connection options.
 * @return VNC client handle or NULL on error.
 */
struct wlf_ra_vnc_client *wlf_ra_vnc_client_create(
		const struct wlf_ra_vnc_options *options);

/**
 * @brief Destroys VNC client connection.
 * @param client VNC client to destroy.
 */
void wlf_ra_vnc_client_destroy(struct wlf_ra_vnc_client *client);

/**
 * @brief Connects to VNC server.
 * @param client VNC client.
 * @return 0 on success, negative on error.
 */
int wlf_ra_vnc_client_connect(struct wlf_ra_vnc_client *client);

/**
 * @brief Disconnects from VNC server.
 * @param client VNC client.
 */
void wlf_ra_vnc_client_disconnect(struct wlf_ra_vnc_client *client);

/**
 * @brief Sends keyboard event to server.
 * @param client VNC client.
 * @param keysym X11 keysym.
 * @param pressed True if pressed, false if released.
 */
void wlf_ra_vnc_client_send_key(struct wlf_ra_vnc_client *client,
		uint32_t keysym, bool pressed);

/**
 * @brief Sends pointer event to server.
 * @param client VNC client.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param button_mask Button mask.
 */
void wlf_ra_vnc_client_send_pointer(struct wlf_ra_vnc_client *client,
		int x, int y, uint32_t button_mask);

/**
 * @brief Sends clipboard text to server.
 * @param client VNC client.
 * @param text Text to send.
 */
void wlf_ra_vnc_client_send_clipboard(struct wlf_ra_vnc_client *client,
		const char *text);

/**
 * @brief Gets framebuffer.
 * @param client VNC client.
 * @return Framebuffer instance.
 */
struct wlf_ra_framebuffer *wlf_ra_vnc_client_get_framebuffer(
		struct wlf_ra_vnc_client *client);

/**
 * @brief Gets connection status.
 * @param client VNC client.
 * @return True if connected, false otherwise.
 */
bool wlf_ra_vnc_client_is_connected(struct wlf_ra_vnc_client *client);

/**
 * @brief Processes pending events (call regularly from main loop).
 * @param client VNC client.
 * @return 0 on success, negative on error.
 */
int wlf_ra_vnc_client_process_events(struct wlf_ra_vnc_client *client);

#endif /* RA_VNC_WLF_RA_VNC_CLIENT_H */
