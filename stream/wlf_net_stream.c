#include "stream.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

// 网络流数据结构
struct NetStreamData {
    socket_t socket;
    int stream_type;  // TCP 或 UDP
    bool is_connected;
    bool is_owner;
    int timeout_ms;
    struct sockaddr_in peer_addr;
    struct sockaddr_in local_addr;
    bool has_peer_addr;
    bool has_local_addr;
};

static bool g_network_initialized = false;

// ============================================================================
// 网络库初始化
// ============================================================================

int netstream_init(void) {
    if (g_network_initialized) {
        return STREAM_SUCCESS;
    }

#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        return STREAM_ERROR_NETWORK_INIT_FAILED;
    }
#endif

    g_network_initialized = true;
    return STREAM_SUCCESS;
}

void netstream_cleanup(void) {
    if (!g_network_initialized) {
        return;
    }

#ifdef _WIN32
    WSACleanup();
#endif

    g_network_initialized = false;
}

// ============================================================================
// 工具函数
// ============================================================================

static int get_socket_error(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

static bool is_socket_error_would_block(int error) {
#ifdef _WIN32
    return error == WSAEWOULDBLOCK;
#else
    return error == EAGAIN || error == EWOULDBLOCK;
#endif
}

static int set_socket_nonblocking(socket_t socket, bool nonblocking) {
#ifdef _WIN32
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
        return STREAM_ERROR_NETWORK_TIMEOUT;
    } else if (result < 0) {
        return STREAM_ERROR_NETWORK_CONNECTION_FAILED;
    }

    return STREAM_SUCCESS;
}

// ============================================================================
// 网络流实现
// ============================================================================

static int netstream_read(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read) {
    struct NetStreamData* data = (struct NetStreamData*)stream->data;

    if (!data->is_connected) {
        return STREAM_ERROR_NETWORK_DISCONNECTED;
    }

    ssize_t result;

    if (data->timeout_ms > 0) {
        int wait_result = wait_for_socket(data->socket, true, data->timeout_ms);
        if (wait_result != STREAM_SUCCESS) {
            return wait_result;
        }
    }

    if (data->stream_type == NETWORK_STREAM_TCP) {
        result = recv(data->socket, (char*)buffer, (int)size, 0);
    } else {  // UDP
        socklen_t addr_len = sizeof(data->peer_addr);
        result = recvfrom(data->socket, (char*)buffer, (int)size, 0,
                         (struct sockaddr*)&data->peer_addr, &addr_len);
        if (result >= 0) {
            data->has_peer_addr = true;
        }
    }

    if (result < 0) {
        int error = get_socket_error();
        if (is_socket_error_would_block(error)) {
            if (bytes_read) *bytes_read = 0;
            return STREAM_SUCCESS;
        }
        data->is_connected = false;
        return STREAM_ERROR_NETWORK_RECV_FAILED;
    } else if (result == 0 && data->stream_type == NETWORK_STREAM_TCP) {
        // TCP连接已关闭
        data->is_connected = false;
        if (bytes_read) *bytes_read = 0;
        return STREAM_SUCCESS;
    }

    if (bytes_read) {
        *bytes_read = (size_t)result;
    }

    return STREAM_SUCCESS;
}

static int netstream_write(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written) {
    struct NetStreamData* data = (struct NetStreamData*)stream->data;

    if (!data->is_connected) {
        return STREAM_ERROR_NETWORK_DISCONNECTED;
    }

    ssize_t result;

    if (data->timeout_ms > 0) {
        int wait_result = wait_for_socket(data->socket, false, data->timeout_ms);
        if (wait_result != STREAM_SUCCESS) {
            return wait_result;
        }
    }

    if (data->stream_type == NETWORK_STREAM_TCP) {
        result = send(data->socket, (const char*)buffer, (int)size, 0);
    } else {  // UDP
        if (!data->has_peer_addr) {
            return STREAM_ERROR_INVALID_OPERATION;
        }
        result = sendto(data->socket, (const char*)buffer, (int)size, 0,
                       (const struct sockaddr*)&data->peer_addr, sizeof(data->peer_addr));
    }

    if (result < 0) {
        int error = get_socket_error();
        if (is_socket_error_would_block(error)) {
            if (bytes_written) *bytes_written = 0;
            return STREAM_SUCCESS;
        }
        data->is_connected = false;
        return STREAM_ERROR_NETWORK_SEND_FAILED;
    }

    if (bytes_written) {
        *bytes_written = (size_t)result;
    }

    return STREAM_SUCCESS;
}

static int netstream_seek(struct Stream* stream, long offset, int whence) {
    // 网络流不支持seek操作
    return STREAM_ERROR_INVALID_OPERATION;
}

static int netstream_tell(struct Stream* stream, long* position) {
    // 网络流不支持tell操作
    return STREAM_ERROR_INVALID_OPERATION;
}

static int netstream_get_size(struct Stream* stream, size_t* size) {
    // 网络流无法获取大小
    return STREAM_ERROR_INVALID_OPERATION;
}

static int netstream_flush(struct Stream* stream) {
    // 网络流flush是无操作的
    return STREAM_SUCCESS;
}

static void netstream_close(struct Stream* stream) {
    if (!stream || !stream->data) {
        return;
    }

    struct NetStreamData* data = (struct NetStreamData*)stream->data;

    if (data->is_owner && data->socket != INVALID_SOCKET_VALUE) {
        socket_close(data->socket);
    }

    free(data);
    stream->data = NULL;
}

// ============================================================================
// 创建函数
// ============================================================================

struct Stream* netstream_create_from_socket(socket_t socket, int stream_type, bool take_ownership) {
    if (socket == INVALID_SOCKET_VALUE) {
        return NULL;
    }

    if (netstream_init() != STREAM_SUCCESS) {
        return NULL;
    }

    struct Stream* stream = malloc(sizeof(struct Stream));
    if (!stream) {
        return NULL;
    }

    struct NetStreamData* data = malloc(sizeof(struct NetStreamData));
    if (!data) {
        free(stream);
        return NULL;
    }

    memset(data, 0, sizeof(struct NetStreamData));
    data->socket = socket;
    data->stream_type = stream_type;
    data->is_connected = true;
    data->is_owner = take_ownership;
    data->timeout_ms = -1;  // 无超时

    stream->type = STREAM_TYPE_NETWORK;
    stream->data = data;
    stream->is_owner = true;

    stream->read = netstream_read;
    stream->write = netstream_write;
    stream->seek = netstream_seek;
    stream->tell = netstream_tell;
    stream->get_size = netstream_get_size;
    stream->flush = netstream_flush;
    stream->close = netstream_close;

    return stream;
}

struct Stream* netstream_connect_tcp(const char* host, int port, int timeout_ms) {
    if (!host || port <= 0 || port > 65535) {
        return NULL;
    }

    if (netstream_init() != STREAM_SUCCESS) {
        return NULL;
    }

    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_VALUE) {
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    // 解析主机名
    struct hostent* he = gethostbyname(host);
    if (!he) {
        socket_close(sock);
        return NULL;
    }

    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    // 如果设置了超时，使用非阻塞连接
    if (timeout_ms > 0) {
        set_socket_nonblocking(sock, true);
    }

    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    if (result < 0) {
        int error = get_socket_error();

        if (timeout_ms > 0 && is_socket_error_would_block(error)) {
            // 等待连接完成
            int wait_result = wait_for_socket(sock, false, timeout_ms);
            if (wait_result != STREAM_SUCCESS) {
                socket_close(sock);
                return NULL;
            }

            // 检查连接是否成功
            int sock_error;
            socklen_t len = sizeof(sock_error);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&sock_error, &len) < 0 || sock_error != 0) {
                socket_close(sock);
                return NULL;
            }
        } else {
            socket_close(sock);
            return NULL;
        }
    }

    // 恢复阻塞模式
    if (timeout_ms > 0) {
        set_socket_nonblocking(sock, false);
    }

    struct Stream* stream = netstream_create_from_socket(sock, NETWORK_STREAM_TCP, true);
    if (!stream) {
        socket_close(sock);
        return NULL;
    }

    // 保存地址信息
    struct NetStreamData* data = (struct NetStreamData*)stream->data;
    data->peer_addr = addr;
    data->has_peer_addr = true;
    data->timeout_ms = timeout_ms;

    // 获取本地地址
    socklen_t local_len = sizeof(data->local_addr);
    if (getsockname(sock, (struct sockaddr*)&data->local_addr, &local_len) == 0) {
        data->has_local_addr = true;
    }

    return stream;
}

struct Stream* netstream_connect_udp(const char* host, int port) {
    if (!host || port <= 0 || port > 65535) {
        return NULL;
    }

    if (netstream_init() != STREAM_SUCCESS) {
        return NULL;
    }

    socket_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET_VALUE) {
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);

    // 解析主机名
    struct hostent* he = gethostbyname(host);
    if (!he) {
        socket_close(sock);
        return NULL;
    }

    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);

    struct Stream* stream = netstream_create_from_socket(sock, NETWORK_STREAM_UDP, true);
    if (!stream) {
        socket_close(sock);
        return NULL;
    }

    // 保存地址信息
    struct NetStreamData* data = (struct NetStreamData*)stream->data;
    data->peer_addr = addr;
    data->has_peer_addr = true;

    return stream;
}

// ============================================================================
// 网络流特有函数
// ============================================================================

int netstream_set_timeout(struct Stream* stream, int timeout_ms) {
    if (!stream || stream->type != STREAM_TYPE_NETWORK) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    struct NetStreamData* data = (struct NetStreamData*)stream->data;
    data->timeout_ms = timeout_ms;

    return STREAM_SUCCESS;
}

int netstream_get_peer_info(struct Stream* stream, char* host, size_t host_len, int* port) {
    if (!stream || stream->type != STREAM_TYPE_NETWORK) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    struct NetStreamData* data = (struct NetStreamData*)stream->data;

    if (!data->has_peer_addr) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    if (host && host_len > 0) {
        const char* addr_str = inet_ntoa(data->peer_addr.sin_addr);
        strncpy(host, addr_str, host_len - 1);
        host[host_len - 1] = '\0';
    }

    if (port) {
        *port = ntohs(data->peer_addr.sin_port);
    }

    return STREAM_SUCCESS;
}

int netstream_get_local_info(struct Stream* stream, char* host, size_t host_len, int* port) {
    if (!stream || stream->type != STREAM_TYPE_NETWORK) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    struct NetStreamData* data = (struct NetStreamData*)stream->data;

    if (!data->has_local_addr) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    if (host && host_len > 0) {
        const char* addr_str = inet_ntoa(data->local_addr.sin_addr);
        strncpy(host, addr_str, host_len - 1);
        host[host_len - 1] = '\0';
    }

    if (port) {
        *port = ntohs(data->local_addr.sin_port);
    }

    return STREAM_SUCCESS;
}

bool netstream_is_connected(struct Stream* stream) {
    if (!stream || stream->type != STREAM_TYPE_NETWORK) {
        return false;
    }

    struct NetStreamData* data = (struct NetStreamData*)stream->data;
    return data->is_connected;
}

// 更新错误字符串函数
const char* stream_error_string(int error) {
    switch (error) {
        case STREAM_SUCCESS:
            return "Success";
        case STREAM_ERROR_NULL_POINTER:
            return "Null pointer";
        case STREAM_ERROR_INVALID_OPERATION:
            return "Invalid operation";
        case STREAM_ERROR_OUT_OF_MEMORY:
            return "Out of memory";
        case STREAM_ERROR_READ_FAILED:
            return "Read failed";
        case STREAM_ERROR_WRITE_FAILED:
            return "Write failed";
        case STREAM_ERROR_SEEK_FAILED:
            return "Seek failed";
        case STREAM_ERROR_INVALID_POSITION:
            return "Invalid position";
        case STREAM_ERROR_FILE_NOT_FOUND:
            return "File not found";
        case STREAM_ERROR_PERMISSION_DENIED:
            return "Permission denied";
        case STREAM_ERROR_NETWORK_INIT_FAILED:
            return "Network initialization failed";
        case STREAM_ERROR_NETWORK_CONNECTION_FAILED:
            return "Network connection failed";
        case STREAM_ERROR_NETWORK_SEND_FAILED:
            return "Network send failed";
        case STREAM_ERROR_NETWORK_RECV_FAILED:
            return "Network receive failed";
        case STREAM_ERROR_NETWORK_TIMEOUT:
            return "Network timeout";
        case STREAM_ERROR_NETWORK_HOST_NOT_FOUND:
            return "Host not found";
        case STREAM_ERROR_NETWORK_DISCONNECTED:
            return "Network disconnected";
        default:
            return "Unknown error";
    }
}
