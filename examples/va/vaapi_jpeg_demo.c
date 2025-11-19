/**
 * @file        vaapi_jpeg_demo.c
 * @brief       Demo for VA-API JPEG encoding and decoding with DMA-BUF.
 * @details     This demo demonstrates:
 *              1. Loading a JPEG image using VA-API and exporting to DMA-BUF
 *              2. Converting DMA-BUF back to JPEG using VA-API
 *              3. Zero-copy workflow between JPEG and DMA-BUF
 *
 * @author      YaoBing Xiao
 * @date        2026-01-31
 */

#include "wlf/va/wlf_va_jpeg.h"
#include "wlf/va/wlf_va_display.h"
#include "wlf/platform/wlf_backend.h"
#include "wlf/utils/wlf_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static void print_usage(const char *program) {
	printf("Usage: %s [options]\n", program);
	printf("Options:\n");
	printf("  -i, --input <file>    Input JPEG file\n");
	printf("  -o, --output <file>   Output JPEG file\n");
	printf("  -q, --quality <1-100> JPEG encoding quality (default: 85)\n");
	printf("  -h, --help            Show this help message\n");
	printf("\n");
	printf("Examples:\n");
	printf("  %s -i input.jpg -o output.jpg -q 90\n", program);
	printf("    Decode input.jpg to DMA-BUF and encode back to output.jpg with quality 90\n");
}

int main(int argc, char *argv[]) {
	const char *input_file = NULL;
	const char *output_file = NULL;
	uint32_t quality = 85;

	/* Parse command line arguments */
	struct option long_options[] = {
		{ "input", required_argument, 0, 'i' },
		{ "output", required_argument, 0, 'o' },
		{ "quality", required_argument, 0, 'q' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "i:o:q:h", long_options, NULL)) != -1) {
		switch (opt) {
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'q':
			quality = atoi(optarg);
			if (quality < 1 || quality > 100) {
				fprintf(stderr, "Error: Quality must be between 1 and 100\n");
				return 1;
			}
			break;
		case 'h':
			print_usage(argv[0]);
			return 0;
		case '?':
			fprintf(stderr, "Error: Unknown option\n");
			print_usage(argv[0]);
			return 1;
		}
	}

	if (!input_file || !output_file) {
		fprintf(stderr, "Error: Input and output files are required\n");
		print_usage(argv[0]);
		return 1;
	}

	wlf_log_init(WLF_DEBUG, NULL);
	wlf_log(WLF_INFO, "VA-API JPEG Demo");
	wlf_log(WLF_INFO, "Input: %s", input_file);
	wlf_log(WLF_INFO, "Output: %s", output_file);
	wlf_log(WLF_INFO, "Quality: %u", quality);

	/* Create backend (needed for VA display) */
	struct wlf_backend *backend = wlf_backend_autocreate();
	if (!backend) {
		wlf_log(WLF_ERROR, "Failed to create backend");
		return 1;
	}

	/* Create VA display */
	struct wlf_va_display *va_display = wlf_va_display_autocreate(backend);
	if (!va_display) {
		wlf_log(WLF_ERROR, "Failed to create VA display");
		wlf_backend_destroy(backend);
		return 1;
	}

	wlf_log(WLF_INFO, "VA display created successfully");

	/* Step 1: Decode JPEG to DMA-BUF */
	wlf_log(WLF_INFO, "Step 1: Decoding JPEG to DMA-BUF...");

	struct wlf_va_jpeg_decoder *decoder = wlf_va_jpeg_decoder_create(va_display);
	if (!decoder) {
		wlf_log(WLF_ERROR, "Failed to create JPEG decoder");
		wlf_va_display_destroy(va_display);
		wlf_backend_destroy(backend);
		return 1;
	}

	struct wlf_dmabuf_attributes attribs;
	memset(&attribs, 0, sizeof(attribs));

	if (!wlf_va_jpeg_decode_file_to_dmabuf(decoder, input_file, &attribs)) {
		wlf_log(WLF_ERROR, "Failed to decode JPEG to DMA-BUF");
		wlf_va_jpeg_decoder_destroy(decoder);
		wlf_va_display_destroy(va_display);
		wlf_backend_destroy(backend);
		return 1;
	}

	wlf_log(WLF_INFO, "JPEG decoded successfully:");
	wlf_log(WLF_INFO, "  Size: %dx%d", attribs.width, attribs.height);
	wlf_log(WLF_INFO, "  Format: 0x%08x", attribs.format);
	wlf_log(WLF_INFO, "  Modifier: 0x%016lx", attribs.modifier);
	wlf_log(WLF_INFO, "  Planes: %d", attribs.n_planes);

	for (int i = 0; i < attribs.n_planes; i++) {
		wlf_log(WLF_INFO, "  Plane %d: fd=%d, stride=%u, offset=%u",
			i, attribs.fd[i], attribs.stride[i], attribs.offset[i]);
	}

	/* Step 2: Encode DMA-BUF back to JPEG */
	wlf_log(WLF_INFO, "Step 2: Encoding DMA-BUF to JPEG...");

	struct wlf_va_jpeg_encoder *encoder = wlf_va_jpeg_encoder_create(va_display, quality);
	if (!encoder) {
		wlf_log(WLF_ERROR, "Failed to create JPEG encoder");
		wlf_dmabuf_attributes_finish(&attribs);
		wlf_va_jpeg_decoder_destroy(decoder);
		wlf_va_display_destroy(va_display);
		wlf_backend_destroy(backend);
		return 1;
	}

	if (!wlf_va_jpeg_encode_dmabuf_to_file(encoder, &attribs, output_file)) {
		wlf_log(WLF_ERROR, "Failed to encode DMA-BUF to JPEG");
		wlf_va_jpeg_encoder_destroy(encoder);
		wlf_dmabuf_attributes_finish(&attribs);
		wlf_va_jpeg_decoder_destroy(decoder);
		wlf_va_display_destroy(va_display);
		wlf_backend_destroy(backend);
		return 1;
	}

	wlf_log(WLF_INFO, "JPEG encoded successfully to %s", output_file);

	/* Cleanup */
	wlf_log(WLF_INFO, "Cleaning up...");
	wlf_va_jpeg_encoder_destroy(encoder);
	wlf_dmabuf_attributes_finish(&attribs);
	wlf_va_jpeg_decoder_destroy(decoder);
	wlf_va_display_destroy(va_display);
	wlf_backend_destroy(backend);

	wlf_log(WLF_INFO, "Demo completed successfully!");
	return 0;
}
