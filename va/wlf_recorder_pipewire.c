/**
 * @file        wlf_recorder_pipewire.c
 * @brief       PipeWire backend for video recorder.
 * @details     Implements PipeWire screen capture backend for recording.
 *              Based on PipeWire API for capturing Wayland compositor output.
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
#include <spa/param/video/format-utils.h>
#include <spa/debug/types.h>
#include <pipewire/pipewire.h>

/**
 * @struct wlf_recorder_pipewire_backend
 * @brief PipeWire capture backend implementation.
 */
struct wlf_recorder_pipewire_backend {
	struct wlf_recorder_backend base;

	/* PipeWire context */
	struct pw_thread_loop *loop;
	struct pw_context *context;
	struct pw_core *core;
	struct pw_stream *stream;

	/* Configuration */
	uint32_t node_id;
	char *node_name;

	/* State */
	bool running;
	bool stream_ready;

	/* Format information */
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t framerate;

	/* Statistics */
	uint64_t frames_captured;
};

/* PipeWire callbacks */

static void on_stream_param_changed(void *data, uint32_t id,
	const struct spa_pod *param) {

	struct wlf_recorder_pipewire_backend *backend = data;

	if (!param || id != SPA_PARAM_Format) {
		return;
	}

	struct spa_video_info_raw info;
	if (spa_format_video_raw_parse(param, &info) < 0) {
		wlf_log(WLF_ERROR, "Failed to parse video format");
		return;
	}

	backend->width = info.size.width;
	backend->height = info.size.height;
	backend->format = info.format;
	backend->framerate = info.framerate.num;

	wlf_log(WLF_INFO, "PipeWire stream format: %ux%u @ %u fps, format=%u",
		backend->width, backend->height, backend->framerate, backend->format);

	backend->stream_ready = true;
}

static void on_stream_process(void *data) {
	struct wlf_recorder_pipewire_backend *backend = data;
	struct pw_buffer *pw_buf;

	if (!backend->running || !backend->stream_ready) {
		return;
	}

	pw_buf = pw_stream_dequeue_buffer(backend->stream);
	if (!pw_buf) {
		wlf_log(WLF_WARNING, "No buffer available from PipeWire stream");
		return;
	}

	struct spa_buffer *spa_buf = pw_buf->buffer;
	struct spa_data *spa_data = &spa_buf->datas[0];

	if (spa_data->type != SPA_DATA_DmaBuf) {
		wlf_log(WLF_WARNING, "Received non-DMA-BUF buffer from PipeWire");
		pw_stream_queue_buffer(backend->stream, pw_buf);
		return;
	}

	/* Create frame from DMA-BUF */
	struct wlf_recorder_frame frame;
	memset(&frame, 0, sizeof(frame));

	frame.dmabuf.width = backend->width;
	frame.dmabuf.height = backend->height;
	frame.dmabuf.format = backend->format;
	frame.dmabuf.modifier = spa_data->chunk->stride; /* Simplified */
	frame.dmabuf.n_planes = 1; /* Simplified for single plane */
	frame.dmabuf.fd[0] = spa_data->fd;
	frame.dmabuf.stride[0] = spa_data->chunk->stride;
	frame.dmabuf.offset[0] = spa_data->chunk->offset;

	frame.timestamp_us = spa_buf->datas[0].chunk->timestamp;
	frame.width = backend->width;
	frame.height = backend->height;
	frame.format = backend->format;

	/* Invoke callback */
	if (backend->base.frame_callback) {
		backend->base.frame_callback(backend->base.recorder, &frame,
			backend->base.user_data);
	}

	backend->frames_captured++;

	/* Return buffer to stream */
	pw_stream_queue_buffer(backend->stream, pw_buf);
}

static void on_stream_state_changed(void *data, enum pw_stream_state old,
	enum pw_stream_state state, const char *error) {

	struct wlf_recorder_pipewire_backend *backend = data;

	wlf_log(WLF_INFO, "PipeWire stream state changed: %s -> %s",
		pw_stream_state_as_string(old),
		pw_stream_state_as_string(state));

	if (state == PW_STREAM_STATE_ERROR) {
		wlf_log(WLF_ERROR, "PipeWire stream error: %s", error ? error : "unknown");
		backend->running = false;
	}
}

static const struct pw_stream_events stream_events = {
	PW_VERSION_STREAM_EVENTS,
	.param_changed = on_stream_param_changed,
	.process = on_stream_process,
	.state_changed = on_stream_state_changed,
};

/* Backend implementation */

static bool pipewire_backend_start(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_pipewire_backend *pw_backend =
		(struct wlf_recorder_pipewire_backend *)backend;

	/* Initialize PipeWire */
	pw_init(NULL, NULL);

	/* Create thread loop */
	pw_backend->loop = pw_thread_loop_new("wlframe-recorder", NULL);
	if (!pw_backend->loop) {
		wlf_log(WLF_ERROR, "Failed to create PipeWire thread loop");
		return false;
	}

	pw_backend->context = pw_context_new(
		pw_thread_loop_get_loop(pw_backend->loop), NULL, 0);
	if (!pw_backend->context) {
		wlf_log(WLF_ERROR, "Failed to create PipeWire context");
		pw_thread_loop_destroy(pw_backend->loop);
		return false;
	}

	/* Connect to PipeWire */
	pw_backend->core = pw_context_connect(pw_backend->context, NULL, 0);
	if (!pw_backend->core) {
		wlf_log(WLF_ERROR, "Failed to connect to PipeWire");
		pw_context_destroy(pw_backend->context);
		pw_thread_loop_destroy(pw_backend->loop);
		return false;
	}

	/* Create stream */
	pw_backend->stream = pw_stream_new(pw_backend->core,
		"wlframe-recorder-stream",
		pw_properties_new(
			PW_KEY_MEDIA_TYPE, "Video",
			PW_KEY_MEDIA_CATEGORY, "Capture",
			PW_KEY_MEDIA_ROLE, "Screen",
			NULL));

	if (!pw_backend->stream) {
		wlf_log(WLF_ERROR, "Failed to create PipeWire stream");
		pw_core_disconnect(pw_backend->core);
		pw_context_destroy(pw_backend->context);
		pw_thread_loop_destroy(pw_backend->loop);
		return false;
	}

	/* Add stream event listener */
	pw_stream_add_listener(pw_backend->stream,
		&(struct spa_hook){0},
		&stream_events,
		pw_backend);

	/* Prepare stream parameters */
	uint8_t buffer[1024];
	struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

	const struct spa_pod *params[1];
	params[0] = spa_pod_builder_add_object(&b,
		SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
		SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
		SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
		SPA_FORMAT_VIDEO_format, SPA_POD_CHOICE_ENUM_Id(5,
			SPA_VIDEO_FORMAT_BGRx,
			SPA_VIDEO_FORMAT_RGBx,
			SPA_VIDEO_FORMAT_BGRA,
			SPA_VIDEO_FORMAT_RGBA,
			SPA_VIDEO_FORMAT_NV12),
		SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(
			&SPA_RECTANGLE(1920, 1080),  /* Default */
			&SPA_RECTANGLE(1, 1),        /* Min */
			&SPA_RECTANGLE(8192, 8192)), /* Max */
		SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(
			&SPA_FRACTION(30, 1),        /* Default */
			&SPA_FRACTION(0, 1),         /* Min */
			&SPA_FRACTION(120, 1)));     /* Max */

	/* Connect stream */
	enum pw_stream_flags flags = PW_STREAM_FLAG_AUTOCONNECT |
		PW_STREAM_FLAG_MAP_BUFFERS |
		PW_STREAM_FLAG_ALLOC_BUFFERS;

	if (pw_stream_connect(pw_backend->stream,
			PW_DIRECTION_INPUT,
			pw_backend->node_id,
			flags,
			params, 1) < 0) {
		wlf_log(WLF_ERROR, "Failed to connect PipeWire stream");
		pw_stream_destroy(pw_backend->stream);
		pw_core_disconnect(pw_backend->core);
		pw_context_destroy(pw_backend->context);
		pw_thread_loop_destroy(pw_backend->loop);
		return false;
	}

	/* Start thread loop */
	if (pw_thread_loop_start(pw_backend->loop) < 0) {
		wlf_log(WLF_ERROR, "Failed to start PipeWire thread loop");
		pw_stream_destroy(pw_backend->stream);
		pw_core_disconnect(pw_backend->core);
		pw_context_destroy(pw_backend->context);
		pw_thread_loop_destroy(pw_backend->loop);
		return false;
	}

	pw_backend->running = true;
	wlf_log(WLF_INFO, "PipeWire recorder backend started");

	return true;
}

static void pipewire_backend_stop(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_pipewire_backend *pw_backend =
		(struct wlf_recorder_pipewire_backend *)backend;

	pw_backend->running = false;

	if (pw_backend->loop) {
		pw_thread_loop_stop(pw_backend->loop);
	}

	if (pw_backend->stream) {
		pw_stream_destroy(pw_backend->stream);
		pw_backend->stream = NULL;
	}

	if (pw_backend->core) {
		pw_core_disconnect(pw_backend->core);
		pw_backend->core = NULL;
	}

	if (pw_backend->context) {
		pw_context_destroy(pw_backend->context);
		pw_backend->context = NULL;
	}

	if (pw_backend->loop) {
		pw_thread_loop_destroy(pw_backend->loop);
		pw_backend->loop = NULL;
	}

	pw_deinit();

	wlf_log(WLF_INFO, "PipeWire recorder backend stopped, captured %lu frames",
		pw_backend->frames_captured);
}

static void pipewire_backend_destroy(struct wlf_recorder_backend *backend) {
	struct wlf_recorder_pipewire_backend *pw_backend =
		(struct wlf_recorder_pipewire_backend *)backend;

	pipewire_backend_stop(backend);

	free(pw_backend->node_name);
	free(pw_backend);

	wlf_log(WLF_DEBUG, "PipeWire recorder backend destroyed");
}

static const struct wlf_recorder_backend_impl pipewire_backend_impl = {
	.name = "pipewire",
	.start = pipewire_backend_start,
	.stop = pipewire_backend_stop,
	.destroy = pipewire_backend_destroy,
};

struct wlf_recorder_backend *wlf_recorder_pipewire_backend_create(
	struct wlf_video_recorder *recorder,
	uint32_t node_id,
	const char *node_name,
	wlf_recorder_frame_callback frame_callback,
	void *user_data) {

	struct wlf_recorder_pipewire_backend *backend =
		calloc(1, sizeof(*backend));
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to allocate PipeWire backend");
		return NULL;
	}

	backend->base.impl = &pipewire_backend_impl;
	backend->base.recorder = recorder;
	backend->base.frame_callback = frame_callback;
	backend->base.user_data = user_data;

	backend->node_id = node_id;
	if (node_name) {
		backend->node_name = strdup(node_name);
	}

	backend->running = false;
	backend->stream_ready = false;
	backend->frames_captured = 0;

	wlf_log(WLF_DEBUG, "PipeWire recorder backend created (node_id=%u, name=%s)",
		node_id, node_name ? node_name : "auto");

	return &backend->base;
}
