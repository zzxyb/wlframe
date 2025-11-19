/**
 * @file        wlf_recorder_dmabuf.c
 * @brief       DMA-BUF backend for video recorder.
 * @details     Implements direct DMA-BUF frame capture backend.
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

/**
 * @struct wlf_recorder_dmabuf_backend
 * @brief DMA-BUF capture backend implementation.
 */
struct wlf_recorder_dmabuf_backend {
	struct wlf_recorder_backend base;

	/* State */
	bool running;

	/* Threading */
	pthread_mutex_t mutex;

	/* Frame buffer */
	struct wlf_recorder_frame *pending_frame;
};

static bool dmabuf_backend_start(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_dmabuf_backend *dmabuf_backend =
		(struct wlf_recorder_dmabuf_backend *)backend;

	pthread_mutex_lock(&dmabuf_backend->mutex);
	dmabuf_backend->running = true;
	pthread_mutex_unlock(&dmabuf_backend->mutex);

	wlf_log(WLF_INFO, "DMA-BUF recorder backend started");
	return true;
}

static void dmabuf_backend_stop(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_dmabuf_backend *dmabuf_backend =
		(struct wlf_recorder_dmabuf_backend *)backend;

	pthread_mutex_lock(&dmabuf_backend->mutex);
	dmabuf_backend->running = false;

	if (dmabuf_backend->pending_frame) {
		wlf_dmabuf_attributes_finish(&dmabuf_backend->pending_frame->dmabuf);
		free(dmabuf_backend->pending_frame);
		dmabuf_backend->pending_frame = NULL;
	}

	pthread_mutex_unlock(&dmabuf_backend->mutex);

	wlf_log(WLF_INFO, "DMA-BUF recorder backend stopped");
}

static void dmabuf_backend_destroy(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_dmabuf_backend *dmabuf_backend =
		(struct wlf_recorder_dmabuf_backend *)backend;

	dmabuf_backend_stop(backend);
	pthread_mutex_destroy(&dmabuf_backend->mutex);
	free(dmabuf_backend);

	wlf_log(WLF_DEBUG, "DMA-BUF recorder backend destroyed");
}

static const struct wlf_recorder_backend_impl dmabuf_backend_impl = {
	.name = "dmabuf",
	.start = dmabuf_backend_start,
	.stop = dmabuf_backend_stop,
	.destroy = dmabuf_backend_destroy,
};

struct wlf_recorder_backend *wlf_recorder_dmabuf_backend_create(
	struct wlf_video_recorder *recorder,
	wlf_recorder_frame_callback frame_callback,
	void *user_data) {

	struct wlf_recorder_dmabuf_backend *backend =
		calloc(1, sizeof(*backend));
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to allocate DMA-BUF backend");
		return NULL;
	}

	backend->base.impl = &dmabuf_backend_impl;
	backend->base.recorder = recorder;
	backend->base.frame_callback = frame_callback;
	backend->base.user_data = user_data;
	backend->running = false;
	backend->pending_frame = NULL;

	if (pthread_mutex_init(&backend->mutex, NULL) != 0) {
		wlf_log(WLF_ERROR, "Failed to initialize mutex");
		free(backend);
		return NULL;
	}

	wlf_log(WLF_DEBUG, "DMA-BUF recorder backend created");
	return &backend->base;
}

bool wlf_recorder_dmabuf_backend_submit_frame(
	struct wlf_recorder_backend *backend,
	const struct wlf_dmabuf_attributes *attribs,
	uint64_t timestamp_us) {

	if (!backend || !attribs) {
		return false;
	}

	struct wlf_recorder_dmabuf_backend *dmabuf_backend =
		(struct wlf_recorder_dmabuf_backend *)backend;

	pthread_mutex_lock(&dmabuf_backend->mutex);

	if (!dmabuf_backend->running) {
		pthread_mutex_unlock(&dmabuf_backend->mutex);
		return false;
	}

	/* Create frame structure */
	struct wlf_recorder_frame frame;
	memset(&frame, 0, sizeof(frame));

	/* Copy DMA-BUF attributes */
	if (!wlf_dmabuf_attributes_copy(&frame.dmabuf, attribs)) {
		wlf_log(WLF_ERROR, "Failed to copy DMA-BUF attributes");
		pthread_mutex_unlock(&dmabuf_backend->mutex);
		return false;
	}

	frame.timestamp_us = timestamp_us;
	frame.width = attribs->width;
	frame.height = attribs->height;
	frame.format = attribs->format;

	pthread_mutex_unlock(&dmabuf_backend->mutex);

	/* Invoke callback */
	if (backend->frame_callback) {
		backend->frame_callback(backend->recorder, &frame, backend->user_data);
	}

	/* Clean up frame */
	wlf_dmabuf_attributes_finish(&frame.dmabuf);

	return true;
}
