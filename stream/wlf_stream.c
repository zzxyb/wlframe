#include "stream.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 文件流数据结构
struct FileStreamData {
    FILE* file;
    bool is_owner;
};

// 内存流数据结构
struct MemStreamData {
    uint8_t* buffer;
    size_t size;
    size_t capacity;
    size_t position;
    bool is_writable;
    bool is_owner;
    bool is_expandable;
};

// ============================================================================
// 通用流接口实现
// ============================================================================

int stream_read(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read) {
    if (!stream || !stream->read) {
        return STREAM_ERROR_NULL_POINTER;
    }
    return stream->read(stream, buffer, size, bytes_read);
}

int stream_write(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written) {
    if (!stream || !stream->write) {
        return STREAM_ERROR_NULL_POINTER;
    }
    return stream->write(stream, buffer, size, bytes_written);
}

int stream_seek(struct Stream* stream, long offset, int whence) {
    if (!stream || !stream->seek) {
        return STREAM_ERROR_NULL_POINTER;
    }
    return stream->seek(stream, offset, whence);
}

int stream_tell(struct Stream* stream, long* position) {
    if (!stream || !stream->tell) {
        return STREAM_ERROR_NULL_POINTER;
    }
    return stream->tell(stream, position);
}

int stream_get_size(struct Stream* stream, size_t* size) {
    if (!stream || !stream->get_size) {
        return STREAM_ERROR_NULL_POINTER;
    }
    return stream->get_size(stream, size);
}

int stream_flush(struct Stream* stream) {
    if (!stream) {
        return STREAM_ERROR_NULL_POINTER;
    }
    if (!stream->flush) {
        return STREAM_SUCCESS; // 有些流不需要flush
    }
    return stream->flush(stream);
}

void stream_close(struct Stream* stream) {
    if (!stream) {
        return;
    }
    if (stream->close) {
        stream->close(stream);
    }
    free(stream);
}

// ============================================================================
// 文件流实现
// ============================================================================

static int filestream_read(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;
    size_t result = fread(buffer, 1, size, data->file);

    if (bytes_read) {
        *bytes_read = result;
    }

    if (result < size && ferror(data->file)) {
        return STREAM_ERROR_READ_FAILED;
    }

    return STREAM_SUCCESS;
}

static int filestream_write(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;
    size_t result = fwrite(buffer, 1, size, data->file);

    if (bytes_written) {
        *bytes_written = result;
    }

    if (result < size) {
        return STREAM_ERROR_WRITE_FAILED;
    }

    return STREAM_SUCCESS;
}

static int filestream_seek(struct Stream* stream, long offset, int whence) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;

    if (fseek(data->file, offset, whence) != 0) {
        return STREAM_ERROR_SEEK_FAILED;
    }

    return STREAM_SUCCESS;
}

static int filestream_tell(struct Stream* stream, long* position) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;
    long pos = ftell(data->file);

    if (pos == -1L) {
        return STREAM_ERROR_SEEK_FAILED;
    }

    if (position) {
        *position = pos;
    }

    return STREAM_SUCCESS;
}

static int filestream_get_size(struct Stream* stream, size_t* size) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;
    long current_pos = ftell(data->file);

    if (current_pos == -1L) {
        return STREAM_ERROR_SEEK_FAILED;
    }

    if (fseek(data->file, 0, SEEK_END) != 0) {
        return STREAM_ERROR_SEEK_FAILED;
    }

    long file_size = ftell(data->file);
    if (file_size == -1L) {
        fseek(data->file, current_pos, SEEK_SET);
        return STREAM_ERROR_SEEK_FAILED;
    }

    if (fseek(data->file, current_pos, SEEK_SET) != 0) {
        return STREAM_ERROR_SEEK_FAILED;
    }

    if (size) {
        *size = (size_t)file_size;
    }

    return STREAM_SUCCESS;
}

static int filestream_flush(struct Stream* stream) {
    struct FileStreamData* data = (struct FileStreamData*)stream->data;

    if (fflush(data->file) != 0) {
        return STREAM_ERROR_WRITE_FAILED;
    }

    return STREAM_SUCCESS;
}

static void filestream_close(struct Stream* stream) {
    if (!stream || !stream->data) {
        return;
    }

    struct FileStreamData* data = (struct FileStreamData*)stream->data;

    if (data->is_owner && data->file) {
        fclose(data->file);
    }

    free(data);
    stream->data = NULL;
}

struct Stream* filestream_create_from_file(FILE* file, bool take_ownership) {
    if (!file) {
        return NULL;
    }

    struct Stream* stream = malloc(sizeof(struct Stream));
    if (!stream) {
        return NULL;
    }

    struct FileStreamData* data = malloc(sizeof(struct FileStreamData));
    if (!data) {
        free(stream);
        return NULL;
    }

    data->file = file;
    data->is_owner = take_ownership;

    stream->type = STREAM_TYPE_FILE;
    stream->data = data;
    stream->is_owner = true;

    stream->read = filestream_read;
    stream->write = filestream_write;
    stream->seek = filestream_seek;
    stream->tell = filestream_tell;
    stream->get_size = filestream_get_size;
    stream->flush = filestream_flush;
    stream->close = filestream_close;

    return stream;
}

struct Stream* filestream_open(const char* filename, const char* mode) {
    if (!filename || !mode) {
        return NULL;
    }

    FILE* file = fopen(filename, mode);
    if (!file) {
        return NULL;
    }

    return filestream_create_from_file(file, true);
}

// ============================================================================
// 内存流实现
// ============================================================================

static int memstream_ensure_capacity(struct MemStreamData* data, size_t required_size) {
    if (!data->is_expandable || required_size <= data->capacity) {
        return STREAM_SUCCESS;
    }

    size_t new_capacity = data->capacity;
    if (new_capacity == 0) {
        new_capacity = 1024;
    }

    while (new_capacity < required_size) {
        new_capacity *= 2;
    }

    uint8_t* new_buffer = realloc(data->buffer, new_capacity);
    if (!new_buffer) {
        return STREAM_ERROR_OUT_OF_MEMORY;
    }

    data->buffer = new_buffer;
    data->capacity = new_capacity;

    return STREAM_SUCCESS;
}

static int memstream_read(struct Stream* stream, void* buffer, size_t size, size_t* bytes_read) {
    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    size_t available = (data->position < data->size) ? (data->size - data->position) : 0;
    size_t to_read = (size < available) ? size : available;

    if (to_read > 0) {
        memcpy(buffer, data->buffer + data->position, to_read);
        data->position += to_read;
    }

    if (bytes_read) {
        *bytes_read = to_read;
    }

    return STREAM_SUCCESS;
}

static int memstream_write(struct Stream* stream, const void* buffer, size_t size, size_t* bytes_written) {
    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (!data->is_writable) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    size_t required_size = data->position + size;
    int result = memstream_ensure_capacity(data, required_size);
    if (result != STREAM_SUCCESS) {
        return result;
    }

    memcpy(data->buffer + data->position, buffer, size);
    data->position += size;

    if (data->size < data->position) {
        data->size = data->position;
    }

    if (bytes_written) {
        *bytes_written = size;
    }

    return STREAM_SUCCESS;
}

static int memstream_seek(struct Stream* stream, long offset, int whence) {
    struct MemStreamData* data = (struct MemStreamData*)stream->data;
    long new_position;

    switch (whence) {
        case SEEK_SET:
            new_position = offset;
            break;
        case SEEK_CUR:
            new_position = (long)data->position + offset;
            break;
        case SEEK_END:
            new_position = (long)data->size + offset;
            break;
        default:
            return STREAM_ERROR_INVALID_OPERATION;
    }

    if (new_position < 0) {
        return STREAM_ERROR_INVALID_POSITION;
    }

    data->position = (size_t)new_position;
    return STREAM_SUCCESS;
}

static int memstream_tell(struct Stream* stream, long* position) {
    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (position) {
        *position = (long)data->position;
    }

    return STREAM_SUCCESS;
}

static int memstream_get_size(struct Stream* stream, size_t* size) {
    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (size) {
        *size = data->size;
    }

    return STREAM_SUCCESS;
}

static void memstream_close(struct Stream* stream) {
    if (!stream || !stream->data) {
        return;
    }

    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (data->is_owner && data->buffer) {
        free(data->buffer);
    }

    free(data);
    stream->data = NULL;
}

struct Stream* memstream_create(void* buffer, size_t size, bool is_writable, bool take_ownership) {
    if (!buffer || size == 0) {
        return NULL;
    }

    struct Stream* stream = malloc(sizeof(struct Stream));
    if (!stream) {
        return NULL;
    }

    struct MemStreamData* data = malloc(sizeof(struct MemStreamData));
    if (!data) {
        free(stream);
        return NULL;
    }

    data->buffer = (uint8_t*)buffer;
    data->size = size;
    data->capacity = size;
    data->position = 0;
    data->is_writable = is_writable;
    data->is_owner = take_ownership;
    data->is_expandable = false;

    stream->type = STREAM_TYPE_MEMORY;
    stream->data = data;
    stream->is_owner = true;

    stream->read = memstream_read;
    stream->write = is_writable ? memstream_write : NULL;
    stream->seek = memstream_seek;
    stream->tell = memstream_tell;
    stream->get_size = memstream_get_size;
    stream->flush = NULL; // 内存流不需要flush
    stream->close = memstream_close;

    return stream;
}

struct Stream* memstream_create_empty(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 1024;
    }

    uint8_t* buffer = malloc(initial_capacity);
    if (!buffer) {
        return NULL;
    }

    struct Stream* stream = malloc(sizeof(struct Stream));
    if (!stream) {
        free(buffer);
        return NULL;
    }

    struct MemStreamData* data = malloc(sizeof(struct MemStreamData));
    if (!data) {
        free(buffer);
        free(stream);
        return NULL;
    }

    data->buffer = buffer;
    data->size = 0;
    data->capacity = initial_capacity;
    data->position = 0;
    data->is_writable = true;
    data->is_owner = true;
    data->is_expandable = true;

    stream->type = STREAM_TYPE_MEMORY;
    stream->data = data;
    stream->is_owner = true;

    stream->read = memstream_read;
    stream->write = memstream_write;
    stream->seek = memstream_seek;
    stream->tell = memstream_tell;
    stream->get_size = memstream_get_size;
    stream->flush = NULL;
    stream->close = memstream_close;

    return stream;
}

struct Stream* memstream_create_from_data(const void* data, size_t size) {
    if (!data || size == 0) {
        return NULL;
    }

    uint8_t* buffer = malloc(size);
    if (!buffer) {
        return NULL;
    }

    memcpy(buffer, data, size);

    struct Stream* stream = memstream_create(buffer, size, false, true);
    if (!stream) {
        free(buffer);
        return NULL;
    }

    return stream;
}

int memstream_get_buffer(struct Stream* stream, void** buffer, size_t* size) {
    if (!stream || stream->type != STREAM_TYPE_MEMORY) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (buffer) {
        *buffer = data->buffer;
    }

    if (size) {
        *size = data->size;
    }

    return STREAM_SUCCESS;
}

int memstream_detach_buffer(struct Stream* stream, void** buffer, size_t* size) {
    if (!stream || stream->type != STREAM_TYPE_MEMORY) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    struct MemStreamData* data = (struct MemStreamData*)stream->data;

    if (!data->is_owner) {
        return STREAM_ERROR_INVALID_OPERATION;
    }

    if (buffer) {
        *buffer = data->buffer;
    }

    if (size) {
        *size = data->size;
    }

    data->buffer = NULL;
    data->size = 0;
    data->capacity = 0;
    data->is_owner = false;

    return STREAM_SUCCESS;
}

// ============================================================================
// 工具函数实现
// ============================================================================

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
        default:
            return "Unknown error";
    }
}

bool stream_is_readable(struct Stream* stream) {
    return stream && stream->read;
}

bool stream_is_writable(struct Stream* stream) {
    return stream && stream->write;
}
