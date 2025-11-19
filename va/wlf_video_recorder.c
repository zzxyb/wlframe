/**
 * @file        wlf_video_recorder.c
 * @brief       Video recorder main implementation.
 * @details     Manages recording lifecycle, frame encoding, and file output.
 *              Inspired by Weston's screen recorder architecture.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_video_recorder.h"
#include "wlf/va/wlf_recorder_backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_time.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @struct wlf_frame_queue_entry
 * @brief Entry in the frame queue for encoding.
 */
struct wlf_frame_queue_entry {
	struct wlf_recorder_frame frame;
	struct wlf_frame_queue_entry *next;
};

/**
 * @struct wlf_video_recorder
 * @brief Main video recorder instance.
 */
struct wlf_video_recorder {
	/* Configuration */
	struct wlf_recorder_config config;

	/* Backend */
	struct wlf_recorder_backend *backend;

	/* Encoder */
	struct wlf_video_encoder *encoder;

	/* State */
	enum wlf_recorder_state state;
	uint64_t start_time_us;
	uint64_t pause_time_us;
	uint64_t total_paused_us;

	/* Frame queue */
	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_cond;
	struct wlf_frame_queue_entry *queue_head;
	struct wlf_frame_queue_entry *queue_tail;
	uint32_t queue_size;
	bool queue_shutdown;

	/* Encoder thread */
	pthread_t encoder_thread;
	bool encoder_running;

	/* Output file */
	int output_fd;
	char *output_filename;

	/* Statistics */
	struct wlf_recorder_statistics stats;
	pthread_mutex_t stats_mutex;
};

/* Frame queue management */

static void queue_push_frame(struct wlf_video_recorder *recorder,
	const struct wlf_recorder_frame *frame) {

	pthread_mutex_lock(&recorder->queue_mutex);

	/* Check if queue is full */
	if (recorder->queue_size >= recorder->config.max_buffer_frames) {
		if (recorder->config.drop_frames_on_overflow) {
			wlf_log(WLF_WARNING, "Frame queue full, dropping frame");
			pthread_mutex_lock(&recorder->stats_mutex);
			recorder->stats.total_frames_dropped++;
			pthread_mutex_unlock(&recorder->stats_mutex);
			pthread_mutex_unlock(&recorder->queue_mutex);
			return;
		} else {
			/* Wait for space */
			while (recorder->queue_size >= recorder->config.max_buffer_frames &&
				   !recorder->queue_shutdown) {
				pthread_cond_wait(&recorder->queue_cond, &recorder->queue_mutex);
			}
		}
	}

	if (recorder->queue_shutdown) {
		pthread_mutex_unlock(&recorder->queue_mutex);
		return;
	}

	/* Allocate queue entry */
	struct wlf_frame_queue_entry *entry = calloc(1, sizeof(*entry));
	if (!entry) {
		wlf_log(WLF_ERROR, "Failed to allocate queue entry");
		pthread_mutex_unlock(&recorder->queue_mutex);
		return;
	}

	/* Copy frame data */
	memcpy(&entry->frame, frame, sizeof(*frame));

	/* Copy DMA-BUF attributes */
	if (!wlf_dmabuf_attributes_copy(&entry->frame.dmabuf, &frame->dmabuf)) {
		wlf_log(WLF_ERROR, "Failed to copy DMA-BUF attributes");
		free(entry);
		pthread_mutex_unlock(&recorder->queue_mutex);
		return;
	}

	/* Add to queue */
	entry->next = NULL;
	if (recorder->queue_tail) {
		recorder->queue_tail->next = entry;
	} else {
		recorder->queue_head = entry;
	}
	recorder->queue_tail = entry;
	recorder->queue_size++;

	/* Update statistics */
	pthread_mutex_lock(&recorder->stats_mutex);
	recorder->stats.total_frames_captured++;
	pthread_mutex_unlock(&recorder->stats_mutex);

	/* Signal encoder thread */
	pthread_cond_signal(&recorder->queue_cond);
	pthread_mutex_unlock(&recorder->queue_mutex);
}

static struct wlf_frame_queue_entry *queue_pop_frame(
	struct wlf_video_recorder *recorder) {

	pthread_mutex_lock(&recorder->queue_mutex);

	while (!recorder->queue_head && !recorder->queue_shutdown) {
		pthread_cond_wait(&recorder->queue_cond, &recorder->queue_mutex);
	}

	if (recorder->queue_shutdown && !recorder->queue_head) {
		pthread_mutex_unlock(&recorder->queue_mutex);
		return NULL;
	}

	struct wlf_frame_queue_entry *entry = recorder->queue_head;
	recorder->queue_head = entry->next;
	if (!recorder->queue_head) {
		recorder->queue_tail = NULL;
	}
	recorder->queue_size--;

	pthread_cond_signal(&recorder->queue_cond);
	pthread_mutex_unlock(&recorder->queue_mutex);

	return entry;
}

/* Frame callback from backend */

static void on_frame_captured(struct wlf_video_recorder *recorder,
	const struct wlf_recorder_frame *frame,
	void *user_data) {

	(void)user_data;

	if (recorder->state != WLF_RECORDER_STATE_RECORDING) {
		return;
	}

	/* Add frame to encoding queue */
	queue_push_frame(recorder, frame);
}

/* Encoder thread */

static void *encoder_thread_func(void *data) {
	struct wlf_video_recorder *recorder = data;

	wlf_log(WLF_DEBUG, "Encoder thread started");

	while (recorder->encoder_running) {
		struct wlf_frame_queue_entry *entry = queue_pop_frame(recorder);
		if (!entry) {
			break;  /* Shutdown */
		}

		uint64_t encode_start = wlf_time_get_microseconds();

		/* TODO: Convert DMA-BUF to video image format for encoder */
		/* This would involve importing the DMA-BUF into the encoder's
		 * hardware context (VA-API, Vulkan, etc.) */

		/* For now, just simulate encoding */
		struct wlf_video_encoded_frame encoded_frame;
		memset(&encoded_frame, 0, sizeof(encoded_frame));

		/* TODO: Call encoder
		bool success = wlf_video_encoder_encode_frame(
			recorder->encoder,
			&video_image,
			&encoded_frame);
		*/
		bool success = true; /* Placeholder */

		if (success) {
			/* Write encoded data to file */
			if (recorder->output_fd >= 0 && encoded_frame.data) {
				ssize_t written = write(recorder->output_fd,
					encoded_frame.data, encoded_frame.size);
				if (written < 0) {
					wlf_log_errno(WLF_ERROR, "Failed to write encoded data");
				} else {
					pthread_mutex_lock(&recorder->stats_mutex);
					recorder->stats.total_bytes_written += written;
					pthread_mutex_unlock(&recorder->stats_mutex);
				}

				free(encoded_frame.data);
			}

			pthread_mutex_lock(&recorder->stats_mutex);
			recorder->stats.total_frames_encoded++;
			pthread_mutex_unlock(&recorder->stats_mutex);
		} else {
			wlf_log(WLF_WARNING, "Failed to encode frame");
		}

		uint64_t encode_end = wlf_time_get_microseconds();
		double encode_time_ms = (encode_end - encode_start) / 1000.0;

		pthread_mutex_lock(&recorder->stats_mutex);
		/* Update average encode time (simple moving average) */
		recorder->stats.average_encode_time_ms =
			(recorder->stats.average_encode_time_ms *
			 (recorder->stats.total_frames_encoded - 1) + encode_time_ms) /
			recorder->stats.total_frames_encoded;
		pthread_mutex_unlock(&recorder->stats_mutex);

		/* Clean up frame */
		wlf_dmabuf_attributes_finish(&entry->frame.dmabuf);
		free(entry);
	}

	wlf_log(WLF_DEBUG, "Encoder thread stopped");
	return NULL;
}

/* Public API */

struct wlf_video_recorder *wlf_video_recorder_create(
	struct wlf_recorder_backend *backend,
	const struct wlf_recorder_config *config) {

	if (!backend || !config || !config->output_filename) {
		wlf_log(WLF_ERROR, "Invalid recorder configuration");
		return NULL;
	}

	struct wlf_video_recorder *recorder = calloc(1, sizeof(*recorder));
	if (!recorder) {
		wlf_log(WLF_ERROR, "Failed to allocate video recorder");
		return NULL;
	}

	/* Copy configuration */
	memcpy(&recorder->config, config, sizeof(*config));
	recorder->output_filename = strdup(config->output_filename);

	/* Set defaults */
	if (recorder->config.max_buffer_frames == 0) {
		recorder->config.max_buffer_frames = 30;  /* 1 second at 30fps */
	}

	/* Initialize state */
	recorder->state = WLF_RECORDER_STATE_IDLE;
	recorder->output_fd = -1;

	/* Initialize mutexes and conditions */
	pthread_mutex_init(&recorder->queue_mutex, NULL);
	pthread_cond_init(&recorder->queue_cond, NULL);
	pthread_mutex_init(&recorder->stats_mutex, NULL);

	recorder->queue_head = NULL;
	recorder->queue_tail = NULL;
	recorder->queue_size = 0;
	recorder->queue_shutdown = false;

	/* Set backend */
	recorder->backend = backend;
	backend->recorder = recorder;
	backend->frame_callback = (wlf_recorder_frame_callback)on_frame_captured;
	backend->user_data = NULL;

	/* TODO: Create video encoder
	recorder->encoder = wlf_video_encoder_create(&config->encoder_config);
	if (!recorder->encoder) {
		wlf_log(WLF_ERROR, "Failed to create video encoder");
		wlf_video_recorder_destroy(recorder);
		return NULL;
	}
	*/

	const char *backend_name = backend->impl && backend->impl->name ?
		backend->impl->name : "unknown";
	wlf_log(WLF_INFO, "Video recorder created: backend=%s, output=%s",
		backend_name, config->output_filename);

	return recorder;
}

void wlf_video_recorder_destroy(struct wlf_video_recorder *recorder) {
	if (!recorder) {
		return;
	}

	/* Stop recording if active */
	if (recorder->state == WLF_RECORDER_STATE_RECORDING ||
		recorder->state == WLF_RECORDER_STATE_PAUSED) {
		wlf_video_recorder_stop(recorder);
	}

	/* Destroy backend */
	if (recorder->backend && recorder->backend->impl->destroy) {
		recorder->backend->impl->destroy(recorder->backend);
	}

	/* Destroy encoder */
	if (recorder->encoder) {
		/* wlf_video_encoder_destroy(recorder->encoder); */
	}

	/* Clean up queue */
	while (recorder->queue_head) {
		struct wlf_frame_queue_entry *entry = recorder->queue_head;
		recorder->queue_head = entry->next;
		wlf_dmabuf_attributes_finish(&entry->frame.dmabuf);
		free(entry);
	}

	/* Clean up resources */
	pthread_mutex_destroy(&recorder->queue_mutex);
	pthread_cond_destroy(&recorder->queue_cond);
	pthread_mutex_destroy(&recorder->stats_mutex);

	free(recorder->output_filename);
	free(recorder);

	wlf_log(WLF_DEBUG, "Video recorder destroyed");
}

bool wlf_video_recorder_start(struct wlf_video_recorder *recorder) {
	if (!recorder) {
		return false;
	}

	if (recorder->state != WLF_RECORDER_STATE_IDLE) {
		wlf_log(WLF_ERROR, "Recorder is not in idle state");
		return false;
	}

	/* Open output file */
	recorder->output_fd = open(recorder->output_filename,
		O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (recorder->output_fd < 0) {
		wlf_log_errno(WLF_ERROR, "Failed to open output file: %s",
			recorder->output_filename);
		return false;
	}

	/* Reset statistics */
	memset(&recorder->stats, 0, sizeof(recorder->stats));
	recorder->start_time_us = wlf_time_get_microseconds();
	recorder->total_paused_us = 0;

	/* Start encoder thread */
	recorder->encoder_running = true;
	recorder->queue_shutdown = false;

	if (pthread_create(&recorder->encoder_thread, NULL,
			encoder_thread_func, recorder) != 0) {
		wlf_log_errno(WLF_ERROR, "Failed to create encoder thread");
		close(recorder->output_fd);
		recorder->output_fd = -1;
		return false;
	}

	/* Start backend */
	if (!recorder->backend->impl->start(recorder->backend)) {
		wlf_log(WLF_ERROR, "Failed to start recorder backend");
		recorder->encoder_running = false;
		recorder->queue_shutdown = true;
		pthread_cond_signal(&recorder->queue_cond);
		pthread_join(recorder->encoder_thread, NULL);
		close(recorder->output_fd);
		recorder->output_fd = -1;
		return false;
	}

	recorder->state = WLF_RECORDER_STATE_RECORDING;
	wlf_log(WLF_INFO, "Recording started");

	return true;
}

bool wlf_video_recorder_stop(struct wlf_video_recorder *recorder) {
	if (!recorder) {
		return false;
	}

	if (recorder->state != WLF_RECORDER_STATE_RECORDING &&
		recorder->state != WLF_RECORDER_STATE_PAUSED) {
		wlf_log(WLF_ERROR, "Recorder is not recording");
		return false;
	}

	/* Stop backend */
	if (recorder->backend && recorder->backend->impl->stop) {
		recorder->backend->impl->stop(recorder->backend);
	}

	/* Stop encoder thread */
	recorder->queue_shutdown = true;
	pthread_cond_signal(&recorder->queue_cond);

	if (recorder->encoder_running) {
		pthread_join(recorder->encoder_thread, NULL);
		recorder->encoder_running = false;
	}

	/* Update statistics */
	uint64_t end_time = wlf_time_get_microseconds();
	pthread_mutex_lock(&recorder->stats_mutex);
	recorder->stats.recording_duration_us =
		end_time - recorder->start_time_us - recorder->total_paused_us;

	if (recorder->stats.recording_duration_us > 0) {
		recorder->stats.average_fps =
			(double)recorder->stats.total_frames_encoded * 1000000.0 /
			recorder->stats.recording_duration_us;
	}
	pthread_mutex_unlock(&recorder->stats_mutex);

	/* Close output file */
	if (recorder->output_fd >= 0) {
		close(recorder->output_fd);
		recorder->output_fd = -1;
	}

	recorder->state = WLF_RECORDER_STATE_IDLE;

	wlf_log(WLF_INFO, "Recording stopped: %lu frames, %.2f MB, %.2f fps",
		recorder->stats.total_frames_encoded,
		recorder->stats.total_bytes_written / (1024.0 * 1024.0),
		recorder->stats.average_fps);

	return true;
}

bool wlf_video_recorder_pause(struct wlf_video_recorder *recorder) {
	if (!recorder) {
		return false;
	}

	if (recorder->state != WLF_RECORDER_STATE_RECORDING) {
		return false;
	}

	recorder->pause_time_us = wlf_time_get_microseconds();
	recorder->state = WLF_RECORDER_STATE_PAUSED;

	wlf_log(WLF_INFO, "Recording paused");
	return true;
}

bool wlf_video_recorder_resume(struct wlf_video_recorder *recorder) {
	if (!recorder) {
		return false;
	}

	if (recorder->state != WLF_RECORDER_STATE_PAUSED) {
		return false;
	}

	uint64_t resume_time = wlf_time_get_microseconds();
	recorder->total_paused_us += (resume_time - recorder->pause_time_us);
	recorder->state = WLF_RECORDER_STATE_RECORDING;

	wlf_log(WLF_INFO, "Recording resumed");
	return true;
}

bool wlf_video_recorder_submit_dmabuf(struct wlf_video_recorder *recorder,
	const struct wlf_dmabuf_attributes *attribs,
	uint64_t timestamp_us) {

	if (!recorder || !attribs) {
		return false;
	}

	/* Check if backend supports direct dmabuf submission */
	if (!recorder->backend || !recorder->backend->impl ||
	    !recorder->backend->impl->name ||
	    strcmp(recorder->backend->impl->name, "dmabuf") != 0) {
		wlf_log(WLF_ERROR, "Cannot submit dmabuf to non-dmabuf backend");
		return false;
	}

	/* Forward to backend */
	extern bool wlf_recorder_dmabuf_backend_submit_frame(
		struct wlf_recorder_backend *backend,
		const struct wlf_dmabuf_attributes *attribs,
		uint64_t timestamp_us);

	return wlf_recorder_dmabuf_backend_submit_frame(
		recorder->backend, attribs, timestamp_us);
}

enum wlf_recorder_state wlf_video_recorder_get_state(
	const struct wlf_video_recorder *recorder) {

	if (!recorder) {
		return WLF_RECORDER_STATE_ERROR;
	}

	return recorder->state;
}

bool wlf_video_recorder_get_statistics(
	const struct wlf_video_recorder *recorder,
	struct wlf_recorder_statistics *stats) {

	if (!recorder || !stats) {
		return false;
	}

	pthread_mutex_lock((pthread_mutex_t *)&recorder->stats_mutex);
	memcpy(stats, &recorder->stats, sizeof(*stats));
	pthread_mutex_unlock((pthread_mutex_t *)&recorder->stats_mutex);

	return true;
}

bool wlf_video_recorder_set_quality(struct wlf_video_recorder *recorder,
	uint32_t quality) {

	if (!recorder) {
		return false;
	}

	if (quality < 1 || quality > 100) {
		wlf_log(WLF_ERROR, "Quality must be between 1 and 100");
		return false;
	}

	/* TODO: Adjust encoder parameters based on quality */
	/* This would involve reconfiguring the encoder */

	wlf_log(WLF_INFO, "Recording quality set to %u", quality);
	return true;
}
