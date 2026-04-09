/**
 * @file        wlf_net_stream.h
 * @brief       Network-backed stream implementation for wlframe.
 * @details     This file defines TCP/UDP stream types and APIs for network
 *              initialization, connection creation, timeout control, peer/local
 *              endpoint query, and conversion from generic stream objects.
 * @author      YaoBing Xiao
 * @date        2026-04-08
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-04-08, initial version\n
 */

#ifndef STREAM_WLF_NET_STREAM_H
#define STREAM_WLF_NET_STREAM_H

#include "wlf/stream/wlf_stream.h"
#include "wlf/config.h"

#include <stdbool.h>
#include <stddef.h>

#if WLF_HAS_WIN_PLATFORM
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define WLF_INVALID_SOCKET_VALUE INVALID_SOCKET
#define wlf_socket_close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
typedef int socket_t;
#define WLF_INVALID_SOCKET_VALUE (-1)
#define wlf_socket_close close
#endif

/**
 * @brief Network stream transport type.
 */
enum wlf_net_stream_type {
	WLF_NET_STREAM_TCP, /**< TCP stream socket. */
	WLF_NET_STREAM_UDP, /**< UDP datagram socket. */
};

/**
 * @brief Network stream object.
 */
struct wlf_net_stream {
	struct wlf_stream base;              /**< Base stream object. */
	socket_t socket;                     /**< Underlying socket handle. */
	enum wlf_net_stream_type stream_type;/**< Transport type. */
	bool is_connected;                   /**< Whether connection is alive. */
	bool owns_socket;                    /**< Whether stream owns socket lifetime. */
	int timeout_ms;                      /**< IO timeout in milliseconds, -1 means blocking. */
	struct sockaddr_in peer_addr;        /**< Remote endpoint address. */
	struct sockaddr_in local_addr;       /**< Local endpoint address. */
	bool has_peer_addr;                  /**< Whether peer address is valid. */
	bool has_local_addr;                 /**< Whether local address is valid. */
};

/**
 * @brief Initializes platform network subsystem.
 * @return Stream status code.
 */
int wlf_net_stream_init(void);

/**
 * @brief Cleans up platform network subsystem.
 */
void wlf_net_stream_cleanup(void);

/**
 * @brief Creates a TCP network stream by connecting to host:port.
 * @param host Target host.
 * @param port Target port.
 * @param timeout_ms Connect and IO timeout in milliseconds.
 * @return Base stream object, or NULL on failure.
 */
struct wlf_stream *wlf_net_stream_connect_tcp(const char *host, int port,
	int timeout_ms);

/**
 * @brief Creates a UDP network stream targeting host:port.
 * @param host Target host.
 * @param port Target port.
 * @return Base stream object, or NULL on failure.
 */
struct wlf_stream *wlf_net_stream_connect_udp(const char *host, int port);

/**
 * @brief Wraps an existing socket as network stream.
 * @param socket Existing socket handle.
 * @param stream_type Transport type.
 * @param take_ownership Whether stream closes socket on destroy.
 * @return Base stream object, or NULL on failure.
 */
struct wlf_stream *wlf_net_stream_create_from_socket(socket_t socket,
	enum wlf_net_stream_type stream_type, bool take_ownership);

/**
 * @brief Sets IO timeout for network stream.
 * @param stream Network stream object.
 * @param timeout_ms Timeout in milliseconds.
 * @return Stream status code.
 */
int wlf_net_stream_set_timeout(struct wlf_net_stream *stream, int timeout_ms);

/**
 * @brief Gets peer endpoint information.
 * @param stream Network stream object.
 * @param host Output host string buffer.
 * @param host_len Host buffer length.
 * @param port Output port.
 * @return Stream status code.
 */
int wlf_net_stream_get_peer_info(struct wlf_net_stream *stream, char *host,
	size_t host_len, int *port);

/**
 * @brief Gets local endpoint information.
 * @param stream Network stream object.
 * @param host Output host string buffer.
 * @param host_len Host buffer length.
 * @param port Output port.
 * @return Stream status code.
 */
int wlf_net_stream_get_local_info(struct wlf_net_stream *stream, char *host,
	size_t host_len, int *port);

/**
 * @brief Checks whether base stream is a network stream.
 * @param stream Base stream object.
 * @return true if network stream, otherwise false.
 */
bool wlf_stream_is_net(const struct wlf_stream *stream);

/**
 * @brief Casts base stream to network stream.
 * @param stream Base stream object.
 * @return Network stream object.
 */
struct wlf_net_stream *wlf_net_stream_from_stream(struct wlf_stream *stream);

#endif // STREAM_WLF_NET_STREAM_H
