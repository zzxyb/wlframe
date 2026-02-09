/**
 * @file        wlf_ra_types.h
 * @brief       Remote Assistance common types and structures.
 * @details     Provides common data types, structures and definitions used
 *              across remote assistance modules (XDP, VNC, RFB server/client).
 * @author      YaoBing Xiao
 * @date        2026-02-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-02-09, initial version\n
 */

#ifndef RA_WLF_RA_TYPES_H
#define RA_WLF_RA_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/* Forward declarations */
typedef struct wlf_ra_framebuffer wlf_ra_framebuffer_t;
typedef struct wlf_ra_rfb_server wlf_ra_rfb_server_t;
typedef struct wlf_ra_rfb_client wlf_ra_rfb_client_t;
typedef struct wlf_ra_vnc_connection wlf_ra_vnc_connection_t;

/**
 * @brief Pixel format structure.
 */
struct wlf_ra_pixel_format {
	uint8_t bits_per_pixel;  /**< Bits per pixel */
	uint8_t depth;           /**< Color depth */
	bool big_endian;         /**< Byte order */
	bool true_color;         /**< True color or palette */
	uint16_t red_max;        /**< Red maximum value */
	uint16_t green_max;      /**< Green maximum value */
	uint16_t blue_max;       /**< Blue maximum value */
	uint8_t red_shift;       /**< Red bit shift */
	uint8_t green_shift;     /**< Green bit shift */
	uint8_t blue_shift;      /**< Blue bit shift */
};

/**
 * @brief Rectangle structure.
 */
struct wlf_ra_rect {
	int x;       /**< X coordinate */
	int y;       /**< Y coordinate */
	int width;   /**< Width in pixels */
	int height;  /**< Height in pixels */
};

/**
 * @brief Point structure.
 */
struct wlf_ra_point {
	int x;  /**< X coordinate */
	int y;  /**< Y coordinate */
};

/**
 * @brief Event types.
 */
enum wlf_ra_event_type {
	WLF_RA_EVENT_KEY_PRESS,
	WLF_RA_EVENT_KEY_RELEASE,
	WLF_RA_EVENT_POINTER_MOTION,
	WLF_RA_EVENT_POINTER_BUTTON,
};

/**
 * @brief Key event structure.
 */
struct wlf_ra_key_event {
	uint32_t keysym;  /**< X11 keysym */
	bool pressed;     /**< True if pressed, false if released */
};

/**
 * @brief Pointer event structure.
 */
struct wlf_ra_pointer_event {
	int x;                   /**< X coordinate */
	int y;                   /**< Y coordinate */
	uint32_t button_mask;    /**< Button mask */
};

/**
 * @brief Clipboard event structure.
 */
struct wlf_ra_clipboard_event {
	const char *text;  /**< Clipboard text content */
};

/**
 * @brief Client event structure.
 */
struct wlf_ra_client_event {
	const char *address;  /**< Client address */
};

/**
 * @brief Password check event structure.
 */
struct wlf_ra_password_check_event {
	const char *encrypted_password;  /**< Encrypted password */
	bool *result;                     /**< Pointer to result (set by handler) */
};

/**
 * @brief Frame update event structure.
 */
struct wlf_ra_frame_update_event {
	int x;       /**< X coordinate */
	int y;       /**< Y coordinate */
	int width;   /**< Width in pixels */
	int height;  /**< Height in pixels */
};

/**
 * @brief Error event structure.
 */
struct wlf_ra_error_event {
	const char *message;  /**< Error message */
};

/* Callback function types */
typedef void (*wlf_ra_frame_callback_t)(void *user_data, const char *frame,
		int width, int height, int stride);
typedef void (*wlf_ra_client_connected_callback_t)(void *user_data, void *client);
typedef void (*wlf_ra_client_disconnected_callback_t)(void *user_data, void *client);
typedef bool (*wlf_ra_password_check_callback_t)(void *user_data, const char *password);

#endif /* RA_WLF_RA_TYPES_H */
