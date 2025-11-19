#include "wlf/va/wlf_hwdec.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

struct software_hwdec_priv {
	const AVCodec *codec;
	AVCodecContext *codec_ctx;
	AVFrame *frame;
	AVPacket *packet;
	struct SwsContext *sws_ctx;

	/* Shared memory for wl_shm buffer */
	int shm_fd;
	void *shm_data;
	size_t shm_size;
	struct wl_shm_pool *shm_pool;
};

static bool software_init(struct wlf_hwdec_device *device) {
	struct software_hwdec_priv *priv = calloc(1, sizeof(struct software_hwdec_priv));
	if (!priv) {
		return false;
	}

	device->priv = priv;

	/* Allocate frame and packet */
	priv->frame = av_frame_alloc();
	priv->packet = av_packet_alloc();

	if (!priv->frame || !priv->packet) {
		wlf_log(WLF_ERROR, "Failed to allocate FFmpeg structures");
		software_destroy(device);
		return false;
	}

	wlf_log(WLF_DEBUG, "Software hwdec backend initialized (FFmpeg)");
	return true;
}

static void software_destroy(struct wlf_hwdec_device *device) {
	struct software_hwdec_priv *priv = device->priv;
	if (!priv) {
		return;
	}

	if (priv->codec_ctx) {
		avcodec_free_context(&priv->codec_ctx);
	}

	if (priv->frame) {
		av_frame_free(&priv->frame);
	}

	if (priv->packet) {
		av_packet_free(&priv->packet);
	}

	if (priv->sws_ctx) {
		sws_freeContext(priv->sws_ctx);
	}

	if (priv->shm_pool) {
		wl_shm_pool_destroy(priv->shm_pool);
	}

	if (priv->shm_data) {
		munmap(priv->shm_data, priv->shm_size);
	}

	if (priv->shm_fd >= 0) {
		close(priv->shm_fd);
	}

	free(priv);
	device->priv = NULL;
}

static bool software_supports_codec(struct wlf_hwdec_device *device,
	enum wlf_video_codec codec) {

	/* Software decode supports all codecs via FFmpeg */
	return true;
}

static bool software_supports_format(struct wlf_hwdec_device *device, uint32_t format) {
	/* Software supports all formats */
	return true;
}

static enum AVCodecID codec_to_ffmpeg_id(enum wlf_video_codec codec) {
	switch (codec) {
		case WLF_VIDEO_CODEC_H264:
			return AV_CODEC_ID_H264;
		case WLF_VIDEO_CODEC_H265:
			return AV_CODEC_ID_HEVC;
		case WLF_VIDEO_CODEC_AV1:
			return AV_CODEC_ID_AV1;
		case WLF_VIDEO_CODEC_VP9:
			return AV_CODEC_ID_VP9;
		default:
			return AV_CODEC_ID_NONE;
	}
}

static bool software_decode_frame(struct wlf_hwdec_device *device,
	const uint8_t *bitstream, size_t size,
	struct wlf_video_image *output) {

	struct software_hwdec_priv *priv = device->priv;
	if (!priv) {
		return false;
	}

	/* Set packet data */
	priv->packet->data = (uint8_t *)bitstream;
	priv->packet->size = size;

	/* Send packet to decoder */
	int ret = avcodec_send_packet(priv->codec_ctx, priv->packet);
	if (ret < 0) {
		wlf_log(WLF_ERROR, "Failed to send packet to decoder: %d", ret);
		return false;
	}

	/* Receive decoded frame */
	ret = avcodec_receive_frame(priv->codec_ctx, priv->frame);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
		return true; /* Need more data */
	}

	if (ret < 0) {
		wlf_log(WLF_ERROR, "Failed to receive frame: %d", ret);
		return false;
	}

	wlf_log(WLF_DEBUG, "Software decoded frame: %dx%d",
		priv->frame->width, priv->frame->height);

	/* Store frame data in output */
	if (output) {
		output->width = priv->frame->width;
		output->height = priv->frame->height;
		/* Store AVFrame pointer for later export */
		output->image = (VkImage)(uintptr_t)priv->frame;
	}

	return true;
}

static struct wl_buffer *software_export_to_wl_buffer(
	struct wlf_hwdec_device *device,
	struct wlf_video_image *image,
	struct wl_display *wl_display) {

	struct software_hwdec_priv *priv = device->priv;
	.export_to_wl_buffer = software_export_to_wl_buffer,
	if (!priv || !image) {
		return NULL;
	}

	AVFrame *frame = (AVFrame *)(uintptr_t)image->image;
	if (!frame) {
		wlf_log(WLF_ERROR, "Invalid frame");
		return NULL;
	}

	/* Get wl_shm interface */
	struct wl_registry *registry = wl_display_get_registry(wl_display);
	if (!registry) {
		wlf_log(WLF_ERROR, "Failed to get Wayland registry");
		return NULL;
	}

	/* TODO: Need to get wl_shm from registry properly
	 * For now, this is a placeholder showing the concept
	 */
	struct wl_shm *shm = NULL;  /* Would get from registry */
	if (!shm) {
		wlf_log(WLF_ERROR, "wl_shm not available");
		wl_registry_destroy(registry);
		return NULL;
	}

	/* Calculate buffer size for ARGB8888 format */
	uint32_t stride = frame->width * 4;
	size_t size = stride * frame->height;

	/* Create shared memory */
	if (priv->shm_size < size) {
		if (priv->shm_data) {
			munmap(priv->shm_data, priv->shm_size);
		}
		if (priv->shm_fd >= 0) {
			close(priv->shm_fd);
		}

		priv->shm_fd = memfd_create("wlframe-video", MFD_CLOEXEC);
		if (priv->shm_fd < 0) {
			wlf_log(WLF_ERROR, "Failed to create memfd");
			wl_registry_destroy(registry);
			return NULL;
		}

		if (ftruncate(priv->shm_fd, size) < 0) {
			wlf_log(WLF_ERROR, "Failed to truncate memfd");
			close(priv->shm_fd);
			priv->shm_fd = -1;
			wl_registry_destroy(registry);
			return NULL;
		}

		priv->shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE,
			MAP_SHARED, priv->shm_fd, 0);
		if (priv->shm_data == MAP_FAILED) {
			wlf_log(WLF_ERROR, "Failed to mmap memfd");
			close(priv->shm_fd);
			priv->shm_fd = -1;
			priv->shm_data = NULL;
			wl_registry_destroy(registry);
			return NULL;
		}

		priv->shm_size = size;
	}

	/* Convert frame to ARGB8888 and copy to shared memory */
	if (!priv->sws_ctx) {
		priv->sws_ctx = sws_getContext(
			frame->width, frame->height, frame->format,
			frame->width, frame->height, AV_PIX_FMT_BGRA,
			SWS_BILINEAR, NULL, NULL, NULL
		);
	}

	if (priv->sws_ctx) {
		uint8_t *dst_data[1] = { priv->shm_data };
		int dst_linesize[1] = { stride };

		sws_scale(priv->sws_ctx,
			(const uint8_t * const*)frame->data, frame->linesize,
			0, frame->height,
			dst_data, dst_linesize);
	}

	/* Create wl_shm_pool and buffer */
	if (!priv->shm_pool) {
		priv->shm_pool = wl_shm_create_pool(shm, priv->shm_fd, size);
	}

	struct wl_buffer *buffer = wl_shm_pool_create_buffer(
		priv->shm_pool, 0,
		frame->width, frame->height, stride,
		WL_SHM_FORMAT_ARGB8888
	);

	wl_registry_destroy(registry);
	wlf_log(WLF_DEBUG, "Exported software decoded frame to wl_buffer");

	return buffer;
}

const struct wlf_hwdec_device_impl wlf_hwdec_software_impl = {
	.name = "software",
	.init = software_init,
	.destroy = software_destroy,
	.supports_codec = software_supports_codec,
	.supports_format = software_supports_format,
	.decode_frame = software_decode_frame,
};
