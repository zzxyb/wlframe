#include "wlf/ra/xdp/wlf_ra_xdp.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifdef HAVE_PIPEWIRE
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#include <spa/param/props.h>
#endif

/**
 * @brief Internal XDP context data.
 */
struct wlf_ra_xdp_internal {
	struct wlf_ra_framebuffer *framebuffer;

#ifdef HAVE_PIPEWIRE
	struct pw_thread_loop *loop;
	struct pw_context *context;
	struct pw_core *core;
	struct pw_stream *stream;
	struct spa_hook stream_listener;
#endif

	/* Session info */
	uint32_t node_id;
	bool active;

	/* Cursor position */
	struct wlf_ra_point cursor_pos;
};

#ifdef HAVE_PIPEWIRE
/* PipeWire stream event handlers */
static void on_stream_state_changed(void *data, enum pw_stream_state old,
		enum pw_stream_state state, const char *error) {
	struct wlf_ra_xdp_internal *ctx = data;

	switch (state) {
	case PW_STREAM_STATE_ERROR:
		fprintf(stderr, "XDP: Stream error: %s\n", error);
		ctx->active = false;
		break;
	case PW_STREAM_STATE_STREAMING:
		printf("XDP: Stream started\n");
		ctx->active = true;
		break;
	case PW_STREAM_STATE_PAUSED:
		printf("XDP: Stream paused\n");
		break;
	default:
		break;
	}
}

static void on_stream_process(void *data) {
	struct wlf_ra_xdp_internal *ctx = data;
	struct pw_buffer *buf;
	struct spa_buffer *spa_buf;

	if (!ctx->stream) {
		return;
	}

	buf = pw_stream_dequeue_buffer(ctx->stream);
	if (!buf) {
		return;
	}

	spa_buf = buf->buffer;

	if (spa_buf->datas[0].data && ctx->framebuffer) {
		/* Update framebuffer with captured data */
		uint8_t *data = spa_buf->datas[0].data;
		uint32_t stride = spa_buf->datas[0].chunk->stride;
		uint32_t size = spa_buf->datas[0].chunk->size;

		/* Calculate dimensions from stride and size */
		int height = size / stride;
		int width = stride / 4; /* Assuming 32bpp */

		wlf_ra_framebuffer_update(ctx->framebuffer, (const char *)data,
			width, height, stride);
	}

	pw_stream_queue_buffer(ctx->stream, buf);
}

static const struct pw_stream_events stream_events = {
	PW_VERSION_STREAM_EVENTS,
	.state_changed = on_stream_state_changed,
	.process = on_stream_process,
};
#endif /* HAVE_PIPEWIRE */

struct wlf_ra_xdp_context *wlf_ra_xdp_create(void) {
	struct wlf_ra_xdp_context *ctx = calloc(1, sizeof(struct wlf_ra_xdp_context));
	if (!ctx) {
		return NULL;
	}

	/* Initialize signals */
	wlf_signal_init(&ctx->events.frame_captured);
	wlf_signal_init(&ctx->events.cursor_moved);
	wlf_signal_init(&ctx->events.capture_started);
	wlf_signal_init(&ctx->events.capture_stopped);
	wlf_signal_init(&ctx->events.error);

	/* Allocate internal data */
	struct wlf_ra_xdp_internal *internal = calloc(1, sizeof(struct wlf_ra_xdp_internal));
	if (!internal) {
		free(ctx);
		return NULL;
	}
	ctx->internal = internal;

#ifdef HAVE_PIPEWIRE
	pw_init(NULL, NULL);

	internal->loop = pw_thread_loop_new("xdp-capture", NULL);
	if (!internal->loop) {
		free(internal);
		free(ctx);
		return NULL;
	}

	internal->context = pw_context_new(pw_thread_loop_get_loop(internal->loop),
		NULL, 0);
	if (!internal->context) {
		pw_thread_loop_destroy(internal->loop);
		free(internal);
		free(ctx);
		return NULL;
	}

	internal->core = pw_context_connect(internal->context, NULL, 0);
	if (!internal->core) {
		pw_context_destroy(internal->context);
		pw_thread_loop_destroy(internal->loop);
		free(internal);
		free(ctx);
		return NULL;
	}
#endif

	return ctx;
}

void wlf_ra_xdp_destroy(struct wlf_ra_xdp_context *ctx) {
	if (ctx == NULL) {
		return;
	}

	struct wlf_ra_xdp_internal *internal = ctx->internal;

	wlf_ra_xdp_stop_capture(ctx);

#ifdef HAVE_PIPEWIRE
	if (internal && internal->core) {
		pw_core_disconnect(internal->core);
	}

	if (internal && internal->context) {
		pw_context_destroy(internal->context);
	}

	if (internal && internal->loop) {
		pw_thread_loop_destroy(internal->loop);
	}

	pw_deinit();
#endif

	free(internal);
	free(ctx);
}

int wlf_ra_xdp_start_capture(struct wlf_ra_xdp_context *ctx,
		struct wlf_ra_framebuffer *fb) {
	if (!ctx || !fb) {
		return -1;
	}

	struct wlf_ra_xdp_internal *internal = ctx->internal;

	internal->framebuffer = fb;

#ifdef HAVE_PIPEWIRE
	uint8_t buffer[1024];
	struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

	/* Create stream */
	internal->stream = pw_stream_new(internal->core, "screen-capture",
		pw_properties_new(
			PW_KEY_MEDIA_TYPE, "Video",
			PW_KEY_MEDIA_CATEGORY, "Capture",
			PW_KEY_MEDIA_ROLE, "Screen",
			NULL));

	if (!internal->stream) {
		return -1;
	}

	pw_stream_add_listener(internal->stream, &internal->stream_listener,
		&stream_events, internal);

	/* Build format parameters */
	const struct spa_pod *params[1];
	params[0] = spa_pod_builder_add_object(&b,
		SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
		SPA_FORMAT_mediaType, SPA_POD_Id(SPA_MEDIA_TYPE_video),
		SPA_FORMAT_mediaSubtype, SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
		SPA_FORMAT_VIDEO_format, SPA_POD_Id(SPA_VIDEO_FORMAT_BGRx),
		SPA_FORMAT_VIDEO_size, SPA_POD_CHOICE_RANGE_Rectangle(
			&SPA_RECTANGLE(640, 480),
			&SPA_RECTANGLE(1, 1),
			&SPA_RECTANGLE(8192, 8192)),
		SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(
			&SPA_FRACTION(30, 1),
			&SPA_FRACTION(0, 1),
			&SPA_FRACTION(240, 1)));

	/* Connect stream */
	int ret = pw_stream_connect(internal->stream,
		PW_DIRECTION_INPUT,
		PW_ID_ANY,
		PW_STREAM_FLAG_AUTOCONNECT |
		PW_STREAM_FLAG_MAP_BUFFERS,
		params, 1);

	if (ret < 0) {
		pw_stream_destroy(internal->stream);
		internal->stream = NULL;
		return -1;
	}

	/* Start thread loop */
	pw_thread_loop_start(internal->loop);

	/* Emit capture started signal */
	wlf_signal_emit(&ctx->events.capture_started, ctx);

	return 0;
#else
	fprintf(stderr, "XDP: PipeWire support not compiled in\n");
	return -1;
#endif
}

void wlf_ra_xdp_stop_capture(struct wlf_ra_xdp_context *ctx) {
	if (ctx == NULL) {
		return;
	}

	struct wlf_ra_xdp_internal *internal = ctx->internal;

	if (internal) {
		internal->active = false;
	}

#ifdef HAVE_PIPEWIRE
	if (internal && internal->loop) {
		pw_thread_loop_stop(internal->loop);
	}

	if (internal && internal->stream) {
		pw_stream_destroy(internal->stream);
		internal->stream = NULL;
	}
#endif

	/* Emit capture stopped signal */
	wlf_signal_emit(&ctx->events.capture_stopped, ctx);
}

struct wlf_ra_point wlf_ra_xdp_get_cursor_position(struct wlf_ra_xdp_context *ctx) {
	struct wlf_ra_point pos = {0, 0};

	if (ctx) {
		struct wlf_ra_xdp_internal *internal = ctx->internal;
		if (internal) {
			pos = internal->cursor_pos;
		}
	}

	return pos;
}

void wlf_ra_xdp_send_keyboard(struct wlf_ra_xdp_context *ctx,
		uint32_t keysym, bool pressed) {
	if (ctx == NULL) {
		return;
	}

	/* TODO: Implement via RemoteDesktop portal DBus interface */
	fprintf(stderr, "XDP: Keyboard input not yet implemented\n");
}

void wlf_ra_xdp_send_pointer(struct wlf_ra_xdp_context *ctx,
		int x, int y, uint32_t button_mask) {
	if (ctx == NULL) {
		return;
	}

	struct wlf_ra_xdp_internal *internal = ctx->internal;

	if (internal) {
		internal->cursor_pos.x = x;
		internal->cursor_pos.y = y;
	}

	/* TODO: Implement via RemoteDesktop portal DBus interface */
	fprintf(stderr, "XDP: Pointer input not yet implemented\n");
}

bool wlf_ra_xdp_is_available(void) {
#ifdef HAVE_PIPEWIRE
	/* Check if XDG Desktop Portal is available */
	return access("/usr/libexec/xdg-desktop-portal", X_OK) == 0 ||
		access("/usr/lib/xdg-desktop-portal", X_OK) == 0;
#else
	return false;
#endif
}
