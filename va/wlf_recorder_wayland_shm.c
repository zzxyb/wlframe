/**
 * @file        wlf_recorder_wayland_shm.c
 * @brief       Wayland SHM backend for video recorder.
 * @details     Implements Wayland shared memory buffer capture backend.
 *              Captures frames from wl_shm buffers for recording.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_recorder_backend.h"
#include "wlf/va/wlf_video_recorder.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @struct wlf_recorder_wayland_shm_backend
 * @brief Wayland SHM capture backend implementation.
 */
struct wlf_recorder_wayland_shm_backend {
	struct wlf_recorder_backend base;

	/* State */
	bool running;

	/* Threading */
	pthread_mutex_t mutex;

	/* SHM buffer information */
	uint32_t width;
	uint32_t height;
	uint32_t format;  /* wl_shm_format */
	uint32_t stride;

	/* Statistics */
	uint64_t frames_captured;
};

static bool wayland_shm_backend_start(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_wayland_shm_backend *shm_backend =
		(struct wlf_recorder_wayland_shm_backend *)backend;

	pthread_mutex_lock(&shm_backend->mutex);
	shm_backend->running = true;
	pthread_mutex_unlock(&shm_backend->mutex);

	wlf_log(WLF_INFO, "Wayland SHM recorder backend started");
	return true;
}

static void wayland_shm_backend_stop(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_wayland_shm_backend *shm_backend =
		(struct wlf_recorder_wayland_shm_backend *)backend;

	pthread_mutex_lock(&shm_backend->mutex);
	shm_backend->running = false;
	pthread_mutex_unlock(&shm_backend->mutex);

	wlf_log(WLF_INFO, "Wayland SHM recorder backend stopped, captured %lu frames",
		shm_backend->frames_captured);
}

static void wayland_shm_backend_destroy(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_wayland_shm_backend *shm_backend =
		(struct wlf_recorder_wayland_shm_backend *)backend;

	wayland_shm_backend_stop(backend);
	pthread_mutex_destroy(&shm_backend->mutex);
	free(shm_backend);

	wlf_log(WLF_DEBUG, "Wayland SHM recorder backend destroyed");
}

static const struct wlf_recorder_backend_impl wayland_shm_backend_impl = {
	.name = "wayland-shm",
	.start = wayland_shm_backend_start,
	.stop = wayland_shm_backend_stop,
	.destroy = wayland_shm_backend_destroy,
};

struct wlf_recorder_backend *wlf_recorder_wayland_shm_backend_create(
	struct wlf_video_recorder *recorder,
	wlf_recorder_frame_callback frame_callback,
	void *user_data) {

	struct wlf_recorder_wayland_shm_backend *backend =
		calloc(1, sizeof(*backend));
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to allocate Wayland SHM backend");
		return NULL;
	}

	backend->base.impl = &wayland_shm_backend_impl;
	backend->base.recorder = recorder;
	backend->base.frame_callback = frame_callback;
	backend->base.user_data = user_data;

	backend->running = false;
	backend->frames_captured = 0;
	backend->width = 0;
	backend->height = 0;
	backend->format = 0;
	backend->stride = 0;

	if (pthread_mutex_init(&backend->mutex, NULL) != 0) {
		wlf_log(WLF_ERROR, "Failed to initialize mutex");
		free(backend);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "Wayland SHM recorder backend created");
	return &backend->base;
}

/**
 * @brief Submits a Wayland SHM buffer for recording.
 *
 * @param backend Backend instance.
 * @param shm_data Pointer to shared memory data.
 * @param width Buffer width.
 * @param height Buffer height.
 * @param stride Buffer stride.
 * @param format wl_shm_format format.
 * @param timestamp_us Timestamp in microseconds.
 * @return true on success, false on failure.
 */
bool wlf_recorder_wayland_shm_backend_submit_buffer(
	struct wlf_recorder_backend *backend,
	const void *shm_data,
	uint32_t width,
	uint32_t height,
	uint32_t stride,
	uint32_t format,
	uint64_t timestamp_us) {

	if (!backend || !shm_data) {
		return false;
	}

	struct wlf_recorder_wayland_shm_backend *shm_backend =
		(struct wlf_recorder_wayland_shm_backend *)backend;

	pthread_mutex_lock(&shm_backend->mutex);

	if (!shm_backend->running) {
		pthread_mutex_unlock(&shm_backend->mutex);
		return false;
	}

	/* Update format info if changed */
	if (shm_backend->width != width || shm_backend->height != height ||
		shm_backend->format != format || shm_backend->stride != stride) {
		shm_backend->width = width;
		shm_backend->height = height;
		shm_backend->format = format;
		shm_backend->stride = stride;

		wlf_log(WLF_DEBUG, "SHM buffer format: %ux%u, stride=%u, format=0x%x",
			width, height, stride, format);
	}

	/* Create a temporary DMA-BUF-like structure for compatibility */
	/* In a real implementation, we would either:
	 * 1. Convert SHM to DMA-BUF
	 * 2. Create a separate SHM frame type
	 * 3. Copy the data to a format the encoder can use
	 */

	/* For now, we need to copy the SHM data */
	size_t buffer_size = stride * height;

	/* Create a temporary file descriptor for the data */
	int fd = memfd_create("wlf-shm-buffer", MFD_CLOEXEC);
	if (fd < 0) {
		wlf_log_errno(WLF_ERROR, "Failed to create memfd");
		pthread_mutex_unlock(&shm_backend->mutex);
		return false;
	}

	if (ftruncate(fd, buffer_size) < 0) {
		wlf_log_errno(WLF_ERROR, "Failed to truncate memfd");
		close(fd);
		pthread_mutex_unlock(&shm_backend->mutex);
		return false;
	}

	void *mapped = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE,
		MAP_SHARED, fd, 0);
	if (mapped == MAP_FAILED) {
		wlf_log_errno(WLF_ERROR, "Failed to mmap memfd");
		close(fd);
		pthread_mutex_unlock(&shm_backend->mutex);
		return false;
	}

	/* Copy SHM data to our mapped memory */
	memcpy(mapped, shm_data, buffer_size);
	munmap(mapped, buffer_size);

	/* Create frame structure with the fd */
	struct wlf_recorder_frame frame;
	memset(&frame, 0, sizeof(frame));

	frame.dmabuf.width = width;
	frame.dmabuf.height = height;
	frame.dmabuf.format = format;  /* Note: this is wl_shm_format, not DRM fourcc */
	frame.dmabuf.modifier = 0;
	frame.dmabuf.n_planes = 1;
	frame.dmabuf.fd[0] = fd;
	frame.dmabuf.stride[0] = stride;
	frame.dmabuf.offset[0] = 0;

	frame.timestamp_us = timestamp_us;
	frame.width = width;
	frame.height = height;
	frame.format = format;

	shm_backend->frames_captured++;

	pthread_mutex_unlock(&shm_backend->mutex);

	/* Invoke callback */
	if (backend->frame_callback) {
		backend->frame_callback(backend->recorder, &frame, backend->user_data);
	}

	/* Close the fd after callback (callback should have duplicated it if needed) */
	close(fd);

	return true;
}

/**
 * @brief Submits a Wayland SHM buffer with file descriptor.
 *
 * @param backend Backend instance.
 * @param shm_fd Shared memory file descriptor.
 * @param offset Offset in the shared memory.
 * @param width Buffer width.
 * @param height Buffer height.
 * @param stride Buffer stride.
 * @param format wl_shm_format format.
 * @param timestamp_us Timestamp in microseconds.
 * @return true on success, false on failure.
 */
bool wlf_recorder_wayland_shm_backend_submit_buffer_fd(
	struct wlf_recorder_backend *backend,
	int shm_fd,
	uint32_t offset,
	uint32_t width,
	uint32_t height,
	uint32_t stride,
	uint32_t format,
	uint64_t timestamp_us) {

	if (!backend || shm_fd < 0) {
		return false;
	}

	/* Map the SHM buffer */
	size_t buffer_size = stride * height;
	void *shm_data = mmap(NULL, buffer_size, PROT_READ, MAP_SHARED,
		shm_fd, offset);
	if (shm_data == MAP_FAILED) {
		wlf_log_errno(WLF_ERROR, "Failed to map SHM buffer");
		return false;
	}

	/* Submit the buffer data */
	bool result = wlf_recorder_wayland_shm_backend_submit_buffer(
		backend, shm_data, width, height, stride, format, timestamp_us);

	munmap(shm_data, buffer_size);
	return result;
}
