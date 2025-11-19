/**
 * @file        wayland_shm_recorder_example.c
 * @brief       Example demonstrating Wayland SHM buffer recording.
 * @details     Shows how to capture and record Wayland SHM buffers.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_video_recorder.h"
#include "wlf/va/wlf_recorder_backend.h"
#include "wlf/va/wlf_recorder_wayland_shm.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

static volatile bool running = true;

static void signal_handler(int signum) {
	(void)signum;
	running = false;
}

/* Helper function to create a test SHM buffer */
static int create_test_shm_buffer(uint32_t width, uint32_t height,
                                   uint32_t stride, void **data_out) {
	size_t size = stride * height;

	/* Create anonymous shared memory */
	int fd = memfd_create("test-shm-buffer", MFD_CLOEXEC);
	if (fd < 0) {
		perror("memfd_create");
		return -1;
	}

	if (ftruncate(fd, size) < 0) {
		perror("ftruncate");
		close(fd);
		return -1;
	}

	void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return -1;
	}

	*data_out = data;
	return fd;
}

/* Helper function to generate a test pattern */
static void generate_test_pattern(void *data, uint32_t width, uint32_t height,
                                   uint32_t stride, uint32_t frame_number) {
	uint32_t *pixels = (uint32_t *)data;

	for (uint32_t y = 0; y < height; y++) {
		for (uint32_t x = 0; x < width; x++) {
			/* Generate animated color gradient */
			uint8_t r = (x * 255) / width;
			uint8_t g = (y * 255) / height;
			uint8_t b = (frame_number * 10) % 255;
			uint8_t a = 255;

			/* ARGB8888 format */
			pixels[y * (stride / 4) + x] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}
}

int main(void) {
	/* Set up signal handlers */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* Initialize logging */
	wlf_log_set_level(WLF_INFO);

	printf("=== Wayland SHM Recorder Example ===\n");
	printf("Recording test pattern to output.mp4\n");
	printf("Press Ctrl+C to stop\n\n");

	/* Video configuration */
	uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t fps = 30;
	uint32_t stride = width * 4;  /* ARGB8888 */
	uint32_t format = 0;  /* WL_SHM_FORMAT_ARGB8888 */

	/* Configure recorder */
	struct wlf_recorder_config config = {0};
	config.output_filename = "wayland_shm_recording.mp4";
	config.format = WLF_RECORDER_FORMAT_MP4;

	/* Encoder configuration */
	config.encoder_config.codec = WLF_VIDEO_CODEC_H264;
	config.encoder_config.width = width;
	config.encoder_config.height = height;
	config.encoder_config.framerate_num = fps;
	config.encoder_config.framerate_den = 1;
	config.encoder_config.chroma = WLF_VIDEO_CHROMA_420;
	config.encoder_config.bit_depth = 8;
	config.encoder_config.rate_control_mode = WLF_VIDEO_RATE_CONTROL_VBR;
	config.encoder_config.target_bitrate = 3000000;  /* 3 Mbps */
	config.encoder_config.max_bitrate = 5000000;     /* 5 Mbps */
	config.encoder_config.gop_size = fps * 2;
	config.encoder_config.num_b_frames = 0;

	/* Audio (disabled) */
	config.enable_audio = false;

	/* Buffer management */
	config.max_buffer_frames = fps * 2;
	config.drop_frames_on_overflow = true;

	/* Create Wayland SHM backend */
	printf("Creating Wayland SHM backend...\n");
	struct wlf_recorder_backend *backend =
		wlf_recorder_wayland_shm_backend_create(NULL, NULL, NULL);
	if (!backend) {
		fprintf(stderr, "Error: Failed to create backend\n");
		return 1;
	}

	/* Create recorder */
	printf("Creating recorder...\n");
	struct wlf_video_recorder *recorder =
		wlf_video_recorder_create(backend, &config);
	if (!recorder) {
		fprintf(stderr, "Error: Failed to create recorder\n");
		return 1;
	}

	/* Start recording */
	printf("Starting recording...\n");
	if (!wlf_video_recorder_start(recorder)) {
		fprintf(stderr, "Error: Failed to start recording\n");
		wlf_video_recorder_destroy(recorder);
		return 1;
	}

	/* Create test SHM buffer */
	void *shm_data = NULL;
	int shm_fd = create_test_shm_buffer(width, height, stride, &shm_data);
	if (shm_fd < 0) {
		fprintf(stderr, "Error: Failed to create SHM buffer\n");
		wlf_video_recorder_stop(recorder);
		wlf_video_recorder_destroy(recorder);
		return 1;
	}

	printf("Recording... (generating test pattern)\n");

	/* Record frames */
	uint64_t start_time = wlf_time_get_microseconds();
	uint64_t frame_duration_us = 1000000 / fps;
	uint32_t frame_number = 0;

	while (running && frame_number < fps * 5) {  /* Record 5 seconds */
		uint64_t frame_time = start_time + frame_number * frame_duration_us;
		uint64_t now = wlf_time_get_microseconds();

		/* Wait until it's time for the next frame */
		if (now < frame_time) {
			usleep(frame_time - now);
		}

		/* Generate test pattern */
		generate_test_pattern(shm_data, width, height, stride, frame_number);

		/* Submit frame to recorder */
		wlf_recorder_wayland_shm_backend_submit_buffer(
			backend, shm_data, width, height, stride, format,
			wlf_time_get_microseconds());

		frame_number++;

		/* Print progress */
		if (frame_number % fps == 0) {
			printf("  Recorded %u frames (%.1f seconds)\n",
				frame_number, frame_number / (float)fps);
		}
	}

	printf("\nStopping recording...\n");

	/* Clean up SHM buffer */
	munmap(shm_data, stride * height);
	close(shm_fd);

	/* Stop recording */
	wlf_video_recorder_stop(recorder);

	/* Get statistics */
	struct wlf_recorder_statistics stats;
	if (wlf_video_recorder_get_statistics(recorder, &stats)) {
		printf("\n=== Recording Statistics ===\n");
		printf("Frames captured:  %lu\n", stats.total_frames_captured);
		printf("Frames encoded:   %lu\n", stats.total_frames_encoded);
		printf("Frames dropped:   %lu\n", stats.total_frames_dropped);
		printf("Average FPS:      %.2f\n", stats.average_fps);
		printf("Output size:      %.2f MB\n",
			stats.total_bytes_written / (1024.0 * 1024.0));
	}

	/* Destroy recorder */
	wlf_video_recorder_destroy(recorder);

	printf("\nRecording saved to: %s\n", config.output_filename);
	printf("Done!\n");

	return 0;
}
