#include "wlf/stream/wlf_net_stream.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if WLF_HAS_WIN_PLATFORM
#pragma comment(lib, "ws2_32.lib")
#else
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#endif

static bool g_network_initialized = false;

static void stream_destroy(struct wlf_stream *stream) {
    struct wlf_net_stream *net_stream = wlf_net_stream_from_stream(stream);

    if (net_stream->owns_socket &&
	    net_stream->socket != WLF_INVALID_SOCKET_VALUE) {
		wlf_socket_close(net_stream->socket);
    }

	net_stream->socket = WLF_INVALID_SOCKET_VALUE;
    free(stream);
}

int wlf_net_stream_init(void) {
    if (g_network_initialized) {
        return WLF_STREAM_SUCCESS;
    }

#if WLF_HAS_WIN_PLATFORM
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return WLF_STREAM_ERROR_NETWORK_INIT_FAILED;
    }
#endif

    g_network_initialized = true;
    return WLF_STREAM_SUCCESS;
}

void wlf_net_stream_cleanup(void) {
    if (!g_network_initialized) {
        return;
    }

#if WLF_HAS_WIN_PLATFORM
    WSACleanup();
#endif

    g_network_initialized = false;
}

static int get_socket_error(void) {
#if WLF_HAS_WIN_PLATFORM
    return WSAGetLastError();
#else
    return errno;
#endif
}

static bool is_socket_error_would_block(int error) {
#if WLF_HAS_WIN_PLATFORM
    return error == WSAEWOULDBLOCK;
#elif WLF_HAS_LINUX_PLATFORM
    return error == EAGAIN;
#else
    return error == EAGAIN || error == EWOULDBLOCK;
#endif
}

static int set_socket_nonblocking(socket_t socket, bool nonblocking) {
#if WLF_HAS_WIN_PLATFORM
    u_long mode = nonblocking ? 1 : 0;
    return ioctlsocket(socket, FIONBIO, &mode);
#else
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return -1;

    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    return fcntl(socket, F_SETFL, flags);
#endif
}

static int wait_for_socket(socket_t socket, bool for_read, int timeout_ms) {
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);

    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
    }

    int result = select((int)socket + 1,
                       for_read ? &fds : NULL,
                       for_read ? NULL : &fds,
                       NULL,
                       timeout_ms >= 0 ? &tv : NULL);

    if (result == 0) {
        return WLF_STREAM_ERROR_NETWORK_TIMEOUT;
    } else if (result < 0) {
        return WLF_STREAM_ERROR_NETWORK_CONNECTION_FAILED;
    }

    return WLF_STREAM_SUCCESS;
}

static int stream_read(struct wlf_stream *stream, void *buffer,
	    size_t size, size_t *bytes_read) {
    struct wlf_net_stream *net_stream = wlf_net_stream_from_stream(stream);

    if (!net_stream->is_connected) {
        return WLF_STREAM_ERROR_NETWORK_DISCONNECTED;
    }

    int result;

    if (net_stream->timeout_ms > 0) {
        int wait_result = wait_for_socket(net_stream->socket, true,
		net_stream->timeout_ms);
        if (wait_result != WLF_STREAM_SUCCESS) {
            return wait_result;
        }
    }

    if (net_stream->stream_type == WLF_NET_STREAM_TCP) {
        result = recv(net_stream->socket, (char*)buffer, (int)size, 0);
    } else {
        socklen_t addr_len = sizeof(net_stream->peer_addr);
        result = recvfrom(net_stream->socket, (char*)buffer, (int)size, 0,
                         (struct sockaddr*)&net_stream->peer_addr, &addr_len);
        if (result >= 0) {
            net_stream->has_peer_addr = true;
        }
    }

    if (result < 0) {
        int error = get_socket_error();
        if (is_socket_error_would_block(error)) {
            if (bytes_read) *bytes_read = 0;
            return WLF_STREAM_SUCCESS;
        }
        net_stream->is_connected = false;

        return WLF_STREAM_ERROR_NETWORK_RECV_FAILED;
    } else if (result == 0 && net_stream->stream_type == WLF_NET_STREAM_TCP) {
        net_stream->is_connected = false;
        if (bytes_read) *bytes_read = 0;

        return WLF_STREAM_SUCCESS;
    }

    if (bytes_read) {
        *bytes_read = (size_t)result;
    }

    return WLF_STREAM_SUCCESS;
}

static int stream_write(struct wlf_stream *stream, const void *buffer,
	    size_t size, size_t *bytes_written) {
    struct wlf_net_stream *net_stream = wlf_net_stream_from_stream(stream);

    if (!net_stream->is_connected) {
        return WLF_STREAM_ERROR_NETWORK_DISCONNECTED;
    }

    int result;

    if (net_stream->timeout_ms > 0) {
        int wait_result = wait_for_socket(net_stream->socket, false,
		net_stream->timeout_ms);
        if (wait_result != WLF_STREAM_SUCCESS) {
            return wait_result;
        }
    }

    if (net_stream->stream_type == WLF_NET_STREAM_TCP) {
        result = send(net_stream->socket, (const char*)buffer, (int)size, 0);
    } else {
        if (!net_stream->has_peer_addr) {
            return WLF_STREAM_ERROR_INVALID_OPERATION;
        }
        result = sendto(net_stream->socket, (const char*)buffer, (int)size, 0,
                       (const struct sockaddr*)&net_stream->peer_addr,
		       sizeof(net_stream->peer_addr));
    }

    if (result < 0) {
        int error = get_socket_error();
        if (is_socket_error_would_block(error)) {
            if (bytes_written) *bytes_written = 0;

            return WLF_STREAM_SUCCESS;
        }
        net_stream->is_connected = false;

        return WLF_STREAM_ERROR_NETWORK_SEND_FAILED;
    }

    if (bytes_written != NULL) {
        *bytes_written = (size_t)result;
    }

    return WLF_STREAM_SUCCESS;
}

static int stream_seek(struct wlf_stream *stream, long offset,
	    int whence) {
    WLF_UNUSED(stream);
    WLF_UNUSED(offset);
    WLF_UNUSED(whence);

    return WLF_STREAM_ERROR_INVALID_OPERATION;
}

static int stream_tell(struct wlf_stream *stream, long *position) {
    WLF_UNUSED(stream);
    WLF_UNUSED(position);

    return WLF_STREAM_ERROR_INVALID_OPERATION;
}

static int stream_get_size(struct wlf_stream *stream, size_t *size) {
    WLF_UNUSED(stream);
    WLF_UNUSED(size);

    return WLF_STREAM_ERROR_INVALID_OPERATION;
}

static int stream_flush(struct wlf_stream *stream) {
    WLF_UNUSED(stream);

    return WLF_STREAM_SUCCESS;
}

static const struct wlf_stream_impl net_stream_impl = {
    .destroy = stream_destroy,
    .read = stream_read,
    .write = stream_write,
    .seek = stream_seek,
    .tell = stream_tell,
    .get_size = stream_get_size,
    .flush = stream_flush,
};

struct wlf_stream *wlf_net_stream_create_from_socket(socket_t socket,
	    enum wlf_net_stream_type stream_type, bool take_ownership) {
    if (socket == WLF_INVALID_SOCKET_VALUE) {
        return NULL;
    }

    if (wlf_net_stream_init() != WLF_STREAM_SUCCESS) {
        return NULL;
    }

    struct wlf_net_stream *stream =
		malloc(sizeof(struct wlf_net_stream));
    if (stream == NULL) {
        return NULL;
    }

    memset(stream, 0, sizeof(struct wlf_net_stream));
        wlf_stream_init(&stream->base, &net_stream_impl);
    stream->socket = socket;
    stream->stream_type = stream_type;
    stream->is_connected = true;
    stream->owns_socket = take_ownership;
    stream->timeout_ms = -1;

    return &stream->base;
}

struct wlf_stream *wlf_net_stream_connect_tcp(const char *host, int port,
	    int timeout_ms) {
    if (host == NULL || port <= 0 || port > 65535) {
        return NULL;
    }

    if (wlf_net_stream_init() != WLF_STREAM_SUCCESS) {
        return NULL;
    }

    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == WLF_INVALID_SOCKET_VALUE) {
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    struct hostent* he = gethostbyname(host);
    if (he == NULL) {
        wlf_socket_close(sock);
        return NULL;
    }

    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    if (timeout_ms > 0) {
        set_socket_nonblocking(sock, true);
    }

    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    if (result < 0) {
        int error = get_socket_error();

        if (timeout_ms > 0 && is_socket_error_would_block(error)) {
            int wait_result = wait_for_socket(sock, false, timeout_ms);
            if (wait_result != WLF_STREAM_SUCCESS) {
                wlf_socket_close(sock);
                return NULL;
            }

            int sock_error;
            socklen_t len = sizeof(sock_error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&sock_error, &len) < 0 || sock_error != 0) {
                wlf_socket_close(sock);

                return NULL;
            }
        } else {
            wlf_socket_close(sock);

            return NULL;
        }
    }

    if (timeout_ms > 0) {
        set_socket_nonblocking(sock, false);
    }

    struct wlf_stream *stream =
		wlf_net_stream_create_from_socket(sock, WLF_NET_STREAM_TCP, true);
    if (stream == NULL) {
        wlf_socket_close(sock);

        return NULL;
    }

    // 保存地址信息
    struct wlf_net_stream *net_stream = wlf_net_stream_from_stream(stream);
    net_stream->peer_addr = addr;
    net_stream->has_peer_addr = true;
    net_stream->timeout_ms = timeout_ms;

    // 获取本地地址
    socklen_t local_len = sizeof(net_stream->local_addr);
    if (getsockname(sock, (struct sockaddr*)&net_stream->local_addr,
		&local_len) == 0) {
        net_stream->has_local_addr = true;
    }

    return stream;
}

struct wlf_stream *wlf_net_stream_connect_udp(const char *host, int port) {
    if (host == NULL || port <= 0 || port > 65535) {
        return NULL;
    }

    if (wlf_net_stream_init() != WLF_STREAM_SUCCESS) {
        return NULL;
    }

    socket_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == WLF_INVALID_SOCKET_VALUE) {
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    struct hostent* he = gethostbyname(host);
    if (he == NULL) {
        wlf_socket_close(sock);

        return NULL;
    }

    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    struct wlf_stream *stream =
		wlf_net_stream_create_from_socket(sock, WLF_NET_STREAM_UDP, true);
    if (stream == NULL) {
        wlf_socket_close(sock);

        return NULL;
    }

    struct wlf_net_stream *net_stream = wlf_net_stream_from_stream(stream);
    net_stream->peer_addr = addr;
    net_stream->has_peer_addr = true;

    return stream;
}

int wlf_net_stream_set_timeout(struct wlf_net_stream *stream, int timeout_ms) {
    stream->timeout_ms = timeout_ms;

    return WLF_STREAM_SUCCESS;
}

int wlf_net_stream_get_peer_info(struct wlf_net_stream *stream, char *host,
	    size_t host_len, int *port) {
    if (!stream->has_peer_addr) {
        return WLF_STREAM_ERROR_INVALID_OPERATION;
    }

    if (host != NULL && host_len > 0) {
        const char* addr_str = inet_ntoa(stream->peer_addr.sin_addr);
        strncpy(host, addr_str, host_len - 1);
        host[host_len - 1] = '\0';
    }

    if (port != NULL) {
        *port = ntohs(stream->peer_addr.sin_port);
    }

    return WLF_STREAM_SUCCESS;
}

int wlf_net_stream_get_local_info(struct wlf_net_stream *stream, char *host,
	    size_t host_len, int *port) {
    if (!stream->has_local_addr) {
        return WLF_STREAM_ERROR_INVALID_OPERATION;
    }

    if (host != NULL && host_len > 0) {
        const char* addr_str = inet_ntoa(stream->local_addr.sin_addr);
        strncpy(host, addr_str, host_len - 1);
        host[host_len - 1] = '\0';
    }

    if (port != NULL) {
        *port = ntohs(stream->local_addr.sin_port);
    }

    return WLF_STREAM_SUCCESS;
}

bool wlf_stream_is_net(const struct wlf_stream *stream) {
    return stream->impl == &net_stream_impl;
}

struct wlf_net_stream *wlf_net_stream_from_stream(struct wlf_stream *stream) {
    assert(stream->impl == &net_stream_impl);

    struct wlf_net_stream *net_stream =
        wlf_container_of(stream, net_stream, base);

    return net_stream;
}
