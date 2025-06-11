
// test
// 文件流
// struct Stream* fs = filestream_open("test.txt", "rb");
// char buffer[1024];
// size_t bytes_read;
// stream_read(fs, buffer, sizeof(buffer), &bytes_read);
// stream_close(fs);

// // 内存流
// struct Stream* ms = memstream_create_empty(1024);
// const char* data = "Hello, World!";
// stream_write(ms, data, strlen(data), NULL);
// stream_close(ms);

//test 网络流
// #include "stream.h"
// #include <stdio.h>
// #include <string.h>

// int main() {
//     // 初始化网络
//     if (netstream_init() != STREAM_SUCCESS) {
//         fprintf(stderr, "Failed to initialize network\n");
//         return 1;
//     }

//     // 创建TCP连接
//     struct Stream* tcp_stream = netstream_connect_tcp("www.example.com", 80, 5000);
//     if (!tcp_stream) {
//         fprintf(stderr, "Failed to connect to server\n");
//         netstream_cleanup();
//         return 1;
//     }

//     // 发送HTTP请求
//     const char* request = "GET / HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n";
//     size_t bytes_written;
//     int result = stream_write(tcp_stream, request, strlen(request), &bytes_written);
//     if (result != STREAM_SUCCESS) {
//         fprintf(stderr, "Failed to send request: %s\n", stream_error_string(result));
//         stream_close(tcp_stream);
//         netstream_cleanup();
//         return 1;
//     }

//     // 读取响应
//     char buffer[1024];
//     size_t bytes_read;
//     while ((result = stream_read(tcp_stream, buffer, sizeof(buffer) - 1, &bytes_read)) == STREAM_SUCCESS && bytes_read > 0) {
//         buffer[bytes_read] = '\0';
//         printf("%s", buffer);
//     }

//     // 获取连接信息
//     char peer_host[256];
//     int peer_port;
//     if (netstream_get_peer_info(tcp_stream, peer_host, sizeof(peer_host), &peer_port) == STREAM_SUCCESS) {
//         printf("\nConnected to: %s:%d\n", peer_host, peer_port);
//     }

//     // 清理
//     stream_close(tcp_stream);
//     netstream_cleanup();

//     return 0;
// }

#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#define socket_close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
typedef int socket_t;
#define INVALID_SOCKET_VALUE -1
#define socket_close close
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
enum {
    STREAM_SUCCESS = 0,
    STREAM_ERROR_NULL_POINTER,
    STREAM_ERROR_INVALID_OPERATION,
    STREAM_ERROR_OUT_OF_MEMORY,
    STREAM_ERROR_READ_FAILED,
    STREAM_ERROR_WRITE_FAILED,
    STREAM_ERROR_SEEK_FAILED,
    STREAM_ERROR_INVALID_POSITION,
    STREAM_ERROR_FILE_NOT_FOUND,
    STREAM_ERROR_PERMISSION_DENIED,
    STREAM_ERROR_NETWORK_INIT_FAILED,
    STREAM_ERROR_NETWORK_CONNECTION_FAILED,
    STREAM_ERROR_NETWORK_SEND_FAILED,
    STREAM_ERROR_NETWORK_RECV_FAILED,
    STREAM_ERROR_NETWORK_TIMEOUT,
    STREAM_ERROR_NETWORK_HOST_NOT_FOUND,
    STREAM_ERROR_NETWORK_DISCONNECTED
};

// 流类型枚举
enum {
    STREAM_TYPE_FILE,
    STREAM_TYPE_MEMORY,
    STREAM_TYPE_NETWORK
};

// 网络流类型
enum {
    NETWORK_STREAM_TCP,
    NETWORK_STREAM_UDP
};

// 前向声明
struct Stream;

// 函数指针类型定义
typedef int (*stream_read_fn)(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read);
typedef int (*stream_write_fn)(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written);
typedef int (*stream_seek_fn)(struct Stream* stream, long offset, int whence);
typedef int (*stream_tell_fn)(struct Stream* stream, long* position);
typedef int (*stream_size_fn)(struct Stream* stream, size_t* size);
typedef int (*stream_flush_fn)(struct Stream* stream);
typedef void (*stream_close_fn)(struct Stream* stream);

// 流接口结构体
struct Stream {
    int type;

    // 虚函数表
    stream_read_fn read;
    stream_write_fn write;
    stream_seek_fn seek;
    stream_tell_fn tell;
    stream_size_fn get_size;
    stream_flush_fn flush;
    stream_close_fn close;

    // 流数据
    void* data;
    bool is_owner;  // 是否拥有data的所有权
};

// 通用流接口函数
int stream_read(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read);
int stream_write(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written);
int stream_seek(struct Stream* stream, long offset, int whence);
int stream_tell(struct Stream* stream, long* position);
int stream_get_size(struct Stream* stream, size_t* size);
int stream_flush(struct Stream* stream);
void stream_close(struct Stream* stream);

// 文件流相关函数
struct Stream* filestream_create_from_file(FILE* file, bool take_ownership);
struct Stream* filestream_open(const char* filename, const char* mode);

// 内存流相关函数
struct Stream* memstream_create(void* buffer, size_t size, bool is_writable, bool take_ownership);
struct Stream* memstream_create_empty(size_t initial_capacity);
struct Stream* memstream_create_from_data(const void* data, size_t size);

// 内存流特有函数
int memstream_get_buffer(struct Stream* stream, void** buffer, size_t* size);
int memstream_detach_buffer(struct Stream* stream, void** buffer, size_t* size);

// 网络流相关函数
int netstream_init(void);  // 初始化网络库
void netstream_cleanup(void);  // 清理网络库

struct Stream* netstream_connect_tcp(const char* host, int port, int timeout_ms);
struct Stream* netstream_connect_udp(const char* host, int port);
struct Stream* netstream_create_from_socket(socket_t socket, int stream_type, bool take_ownership);

// 网络流特有函数
int netstream_set_timeout(struct Stream* stream, int timeout_ms);
int netstream_get_peer_info(struct Stream* stream, char* host, size_t host_len, int* port);
int netstream_get_local_info(struct Stream* stream, char* host, size_t host_len, int* port);
bool netstream_is_connected(struct Stream* stream);

// 工具函数
const char* stream_error_string(int error);
bool stream_is_readable(struct Stream* stream);
bool stream_is_writable(struct Stream* stream);

#ifdef __cplusplus
}
#endif

#endif // STREAM_H
