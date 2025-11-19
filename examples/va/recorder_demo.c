/**
 * @file        recorder_demo.c
 * @brief       Video recorder demonstration program.
 * @details     Demonstrates usage of the video recorder API with both
 *              dmabuf and pipewire backends.
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_video_recorder.h"
#include "wlf/va/wlf_recorder_backend.h"
#include "wlf/va/wlf_va_display.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

/* Backend creation functions */
extern struct wlf_recorder_backend *wlf_recorder_dmabuf_backend_create(
	struct wlf_video_recorder *recorder,
	wlf_recorder_frame_callback frame_callback,
	void *user_data);

extern struct wlf_recorder_backend *wlf_recorder_pipewire_backend_create(
	struct wlf_video_recorder *recorder,
	uint32_t node_id,
	const char *node_name,
	wlf_recorder_frame_callback frame_callback,
	void *user_data);

extern struct wlf_recorder_backend *wlf_recorder_wayland_shm_backend_create(
	struct wlf_video_recorder *recorder,
	wlf_recorder_frame_callback frame_callback,
	void *user_data);

static volatile bool running = true;

static void signal_handler(int signum) {
	(void)signum;
	running = false;
}

static void print_usage(const char *prog_name) {
	printf("Usage: %s [OPTIONS]\n", prog_name);
	printf("\n");
	printf("Options:\n");
	printf("  -b, --backend TYPE     Backend type: dmabuf, pipewire, or wayland-shm (default: pipewire)\n");
	printf("  -o, --output FILE      Output filename (default: recording.mp4)\n");
	printf("  -w, --width WIDTH      Video width (default: 1920)\n");
	printf("  -h, --height HEIGHT    Video height (default: 1080)\n");
	printf("  -f, --fps FPS          Frame rate (default: 30)\n");
	printf("  -c, --codec CODEC      Codec: h264, h265, av1 (default: h264)\n");
	printf("  -q, --quality QUALITY  Quality 1-100 (default: 85)\n");
	printf("  -d, --duration SECS    Recording duration in seconds (default: 10)\n");
	printf("  -n, --node-id ID       PipeWire node ID (pipewire backend only)\n");
	printf("  --help                 Show this help message\n");
	printf("\n");
	printf("Examples:\n");
	printf("  # Record 10 seconds using PipeWire backend\n");
	printf("  %s -b pipewire -o screen.mp4 -d 10\n", prog_name);
	printf("\n");
	printf("  # Record using dmabuf backend with H.265 codec\n");
	printf("  %s -b dmabuf -o output.mp4 -c h265 -q 90 -d 30\n", prog_name);
	printf("\n");
}

int main(int argc, char *argv[]) {
	/* Default configuration */
	const char *backend_type = "pipewire";
	const char *output_file = "recording.mp4";
	uint32_t width = 1920;
	uint32_t height = 1080;
	uint32_t fps = 30;
	enum wlf_video_codec codec = WLF_VIDEO_CODEC_H264;
	uint32_t quality = 85;
	uint32_t duration_secs = 10;
	uint32_t pipewire_node_id = 0;  /* Auto-detect */

	/* Parse command line arguments */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backend") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing backend type\n");
				return 1;
			}
			if (strcmp(argv[i], "dmabuf") == 0 ||
			    strcmp(argv[i], "pipewire") == 0 ||
			    strcmp(argv[i], "wayland-shm") == 0) {
				backend_type = argv[i];
			} else {
				fprintf(stderr, "Error: Unknown backend type: %s\n", argv[i]);
				return 1;
			}
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing output filename\n");
				return 1;
			}
			output_file = argv[i];
		} else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing width\n");
				return 1;
			}
			width = atoi(argv[i]);
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing height\n");
				return 1;
			}
			height = atoi(argv[i]);
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fps") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing fps\n");
				return 1;
			}
			fps = atoi(argv[i]);
		} else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--codec") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing codec\n");
				return 1;
			}
			if (strcmp(argv[i], "h264") == 0) {
				codec = WLF_VIDEO_CODEC_H264;
			} else if (strcmp(argv[i], "h265") == 0 || strcmp(argv[i], "hevc") == 0) {
				codec = WLF_VIDEO_CODEC_H265;
			} else if (strcmp(argv[i], "av1") == 0) {
				codec = WLF_VIDEO_CODEC_AV1;
			} else {
				fprintf(stderr, "Error: Unknown codec: %s\n", argv[i]);
				return 1;
			}
		} else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quality") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing quality\n");
				return 1;
			}
			quality = atoi(argv[i]);
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--duration") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing duration\n");
				return 1;
			}
			duration_secs = atoi(argv[i]);
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--node-id") == 0) {
			if (++i >= argc) {
				fprintf(stderr, "Error: Missing node ID\n");
				return 1;
			}
			pipewire_node_id = atoi(argv[i]);
		} else if (strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else {
			fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}

	/* Set up signal handlers */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	/* Initialize logging */
	wlf_log_set_level(WLF_INFO);

	printf("=== WLFrame Video Recorder Demo ===\n");
	printf("Backend:  %s\n", backend_type);
	printf("Output:   %s\n", output_file);
	printf("Format:   %ux%u @ %u fps\n", width, height, fps);
	printf("Codec:    %s\n", codec == WLF_VIDEO_CODEC_H264 ? "H.264" :
		codec == WLF_VIDEO_CODEC_H265 ? "H.265" : "AV1");
	printf("Quality:  %u\n", quality);
	printf("Duration: %u seconds\n", duration_secs);
	printf("\n");

	/* Configure recorder */
	struct wlf_recorder_config config = {0};
	config.output_filename = output_file;
	config.format = WLF_RECORDER_FORMAT_MP4;

	/* Encoder configuration */
	config.encoder_config.codec = codec;
	config.encoder_config.width = width;
	config.encoder_config.height = height;
	config.encoder_config.framerate_num = fps;
	config.encoder_config.framerate_den = 1;
	config.encoder_config.chroma = WLF_VIDEO_CHROMA_420;
	config.encoder_config.bit_depth = 8;
	config.encoder_config.rate_control_mode = WLF_VIDEO_RATE_CONTROL_VBR;
	config.encoder_config.target_bitrate = 5000000;  /* 5 Mbps */
	config.encoder_config.max_bitrate = 8000000;     /* 8 Mbps */
	config.encoder_config.gop_size = fps * 2;        /* 2 second GOP */
	config.encoder_config.num_b_frames = 0;

	/* Audio (disabled for now) */
	config.enable_audio = false;

	/* PipeWire specific */
	config.pipewire_node_id = pipewire_node_id;
	config.pipewire_node_name = NULL;

	/* Buffer management */
	config.max_buffer_frames = fps * 2;  /* 2 seconds buffer */
	config.drop_frames_on_overflow = true;

	/* Create backend */
	printf("Creating backend...\n");
	struct wlf_recorder_backend *backend = NULL;
	if (strcmp(backend_type, "dmabuf") == 0) {
		backend = wlf_recorder_dmabuf_backend_create(NULL, NULL, NULL);
	} else if (strcmp(backend_type, "pipewire") == 0) {
		backend = wlf_recorder_pipewire_backend_create(NULL,
			pipewire_node_id, NULL, NULL, NULL);
	} else if (strcmp(backend_type, "wayland-shm") == 0) {
		backend = wlf_recorder_wayland_shm_backend_create(NULL, NULL, NULL);
	} else {
		fprintf(stderr, "Error: Unknown backend type: %s\n", backend_type);
		return 1;
	}

	if (!backend) {
		fprintf(stderr, "Error: Failed to create backend\n");
		return 1;
	}

	/* Create recorder */
	printf("Creating recorder...\n");
	struct wlf_video_recorder *recorder = wlf_video_recorder_create(backend, &config);
	if (!recorder) {
		fprintf(stderr, "Error: Failed to create video recorder\n");
		return 1;
	}

	/* Start recording */
	printf("Starting recording...\n");
	if (!wlf_video_recorder_start(recorder)) {
		fprintf(stderr, "Error: Failed to start recording\n");
		wlf_video_recorder_destroy(recorder);
		return 1;
	}

	printf("Recording in progress (press Ctrl+C to stop early)...\n");

	/* Record for specified duration */
	uint64_t start_time = wlf_time_get_microseconds();
	uint64_t duration_us = duration_secs * 1000000ULL;

	while (running) {
		uint64_t elapsed = wlf_time_get_microseconds() - start_time;

		if (elapsed >= duration_us) {
			break;
		}

		/* Print statistics every second */
		if ((elapsed % 1000000) < 100000) {
			struct wlf_recorder_statistics stats;
			if (wlf_video_recorder_get_statistics(recorder, &stats)) {
				printf("\rCaptured: %lu frames | Encoded: %lu | Dropped: %lu | "
					   "FPS: %.1f | Size: %.2f MB",
					stats.total_frames_captured,
					stats.total_frames_encoded,
					stats.total_frames_dropped,
					stats.average_fps,
					stats.total_bytes_written / (1024.0 * 1024.0));
				fflush(stdout);
			}
		}

		usleep(100000);  /* 100ms */
	}

	printf("\n\nStopping recording...\n");

	/* Stop recording */
	if (!wlf_video_recorder_stop(recorder)) {
		fprintf(stderr, "Error: Failed to stop recording\n");
		wlf_video_recorder_destroy(recorder);
		return 1;
	}

	/* Print final statistics */
	struct wlf_recorder_statistics stats;
	if (wlf_video_recorder_get_statistics(recorder, &stats)) {
		printf("\n=== Recording Statistics ===\n");
		printf("Frames captured:  %lu\n", stats.total_frames_captured);
		printf("Frames encoded:   %lu\n", stats.total_frames_encoded);
		printf("Frames dropped:   %lu\n", stats.total_frames_dropped);
		printf("Average FPS:      %.2f\n", stats.average_fps);
		printf("Average encode:   %.2f ms/frame\n", stats.average_encode_time_ms);
		printf("Duration:         %.2f seconds\n",
			stats.recording_duration_us / 1000000.0);
		printf("Output size:      %.2f MB\n",
			stats.total_bytes_written / (1024.0 * 1024.0));
		printf("Bitrate:          %.2f Mbps\n",
			(stats.total_bytes_written * 8.0) / stats.recording_duration_us);
		printf("\n");
	}

	printf("Recording saved to: %s\n", output_file);

	/* Clean up */
	wlf_video_recorder_destroy(recorder);

	printf("Done!\n");
	return 0;
}
