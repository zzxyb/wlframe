/**
 * @file        wlf_ra_rfb_server.h
 * @brief       RFB (VNC) server implementation.
 * @details     Provides VNC server functionality to share screen and accept
 *              client connections with authentication and input event handling.
 * @author      YaoBing Xiao
 * @date        2026-02-10
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-10, initial version\n
 */

#ifndef RA_SERVER_WLF_RA_RFB_SERVER_H
#define RA_SERVER_WLF_RA_RFB_SERVER_H

#include "../common/wlf_ra_types.h"
#include "../common/wlf_ra_framebuffer.h"
#include "wlf/utils/wlf_signal.h"

/**
 * @brief RFB server virtual methods implementation.
 */
struct wlf_ra_rfb_server_impl {
	/**
	 * @brief Starts RFB server.
	 * @param server Server instance.
	 * @return 0 on success, negative on error.
	 */
	int (*start)(struct wlf_ra_rfb_server *server);

	/**
	 * @brief Stops RFB server.
	 * @param server Server instance.
	 */
	void (*stop)(struct wlf_ra_rfb_server *server);

	/**
	 * @brief Sets framebuffer to be shared.
	 * @param server Server instance.
	 * @param fb Framebuffer to share.
	 */
	void (*set_framebuffer)(struct wlf_ra_rfb_server *server, struct wlf_ra_framebuffer *fb);

	/**
	 * @brief Updates screen region.
	 * @param server Server instance.
	 * @param rect Rectangle to mark as dirty.
	 */
	void (*mark_rect_dirty)(struct wlf_ra_rfb_server *server, const struct wlf_ra_rect *rect);

	/**
	 * @brief Updates cursor position.
	 * @param server Server instance.
	 * @param pos Cursor position.
	 */
	void (*update_cursor)(struct wlf_ra_rfb_server *server, const struct wlf_ra_point *pos);

	/**
	 * @brief Processes server events.
	 * @param server Server instance.
	 * @param timeout_ms Timeout in milliseconds.
	 * @return Number of events processed, or negative on error.
	 */
	int (*process_events)(struct wlf_ra_rfb_server *server, int timeout_ms);

	/**
	 * @brief Sends clipboard text to all clients.
	 * @param server Server instance.
	 * @param text Text to send.
	 */
	void (*send_clipboard)(struct wlf_ra_rfb_server *server, const char *text);

	/**
	 * @brief Destroys the server.
	 * @param server Server instance.
	 */
	void (*destroy)(struct wlf_ra_rfb_server *server);
};

/**
 * @brief RFB (Remote Framebuffer) Server.
 */
struct wlf_ra_rfb_server {
	const struct wlf_ra_rfb_server_impl *impl;  /**< Virtual method table */

	struct {
		struct wlf_signal client_connected;      /**< Signal emitted when client connects */
		struct wlf_signal client_disconnected;   /**< Signal emitted when client disconnects */
		struct wlf_signal password_check;        /**< Signal emitted for password verification */
		struct wlf_signal key_event;             /**< Signal emitted on key event */
		struct wlf_signal pointer_event;         /**< Signal emitted on pointer event */
		struct wlf_signal clipboard_text;        /**< Signal emitted on clipboard text */
	} events;

	int port;                                    /**< Listening port */
	int client_count;                            /**< Number of connected clients */
	void *backend_data;                          /**< Backend-specific data */
};

/**
 * @brief RFB server configuration.
 */
struct wlf_ra_rfb_server_config {
	const char *listen_address;                               /**< Listen address, default "0.0.0.0" */
	int port;                                                 /**< Listen port, 0 for auto */
	const char *password;                                     /**< Server password, NULL for no auth */
	bool allow_multiple_clients;                              /**< Allow multiple simultaneous clients */
	void *user_data;                                          /**< User data */
};

/**
 * @brief Creates RFB server.
 * @param config Server configuration.
 * @return Server handle or NULL on error.
 */
struct wlf_ra_rfb_server *wlf_ra_rfb_server_create(
		const struct wlf_ra_rfb_server_config *config);

/**
 * @brief Destroys RFB server.
 * @param server Server to destroy.
 */
void wlf_ra_rfb_server_destroy(struct wlf_ra_rfb_server *server);

/**
 * @brief Starts RFB server.
 * @param server Server instance.
 * @return 0 on success, negative on error.
 */
int wlf_ra_rfb_server_start(struct wlf_ra_rfb_server *server);

/**
 * @brief Stops RFB server.
 * @param server Server instance.
 */
void wlf_ra_rfb_server_stop(struct wlf_ra_rfb_server *server);

/**
 * @brief Sets framebuffer to be shared.
 * @param server Server instance.
 * @param fb Framebuffer to share.
 */
void wlf_ra_rfb_server_set_framebuffer(struct wlf_ra_rfb_server *server,
		struct wlf_ra_framebuffer *fb);

/**
 * @brief Updates screen region (marks as dirty).
 * @param server Server instance.
 * @param rect Rectangle to mark as dirty.
 */
void wlf_ra_rfb_server_mark_rect_dirty(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_rect *rect);

/**
 * @brief Updates entire screen.
 * @param server Server instance.
 */
void wlf_ra_rfb_server_mark_screen_dirty(struct wlf_ra_rfb_server *server);

/**
 * @brief Updates cursor position.
 * @param server Server instance.
 * @param pos Cursor position.
 */
void wlf_ra_rfb_server_update_cursor(struct wlf_ra_rfb_server *server,
		const struct wlf_ra_point *pos);

/**
 * @brief Processes server events (call regularly from main loop).
 * @param server Server instance.
 * @param timeout_ms Timeout in milliseconds.
 * @return Number of events processed, or negative on error.
 */
int wlf_ra_rfb_server_process_events(struct wlf_ra_rfb_server *server,
		int timeout_ms);

/**
 * @brief Gets actual listening port (useful when port=0).
 * @param server Server instance.
 * @return Port number or negative on error.
 */
int wlf_ra_rfb_server_get_port(struct wlf_ra_rfb_server *server);

/**
 * @brief Gets number of connected clients.
 * @param server Server instance.
 * @return Number of clients.
 */
int wlf_ra_rfb_server_get_client_count(struct wlf_ra_rfb_server *server);

/**
 * @brief Sends clipboard text to all clients.
 * @param server Server instance.
 * @param text Text to send.
 */
void wlf_ra_rfb_server_send_clipboard(struct wlf_ra_rfb_server *server,
		const char *text);

#endif /* RA_SERVER_WLF_RA_RFB_SERVER_H */
