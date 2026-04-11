/**
 * @file        svg_info_test.c
 * @brief       Example: load an SVG file, print metadata, and optionally save.
 */

#include "wlf/svg/wlf_svg.h"
#include "wlf/shapes/wlf_text_shape.h"
#include "wlf/types/wlf_gradient.h"
#include "wlf/types/wlf_linear_gradient.h"
#include "wlf/types/wlf_radial_gradient.h"
#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const char *svg_paint_desc(const struct wlf_gradient *grad,
	const char *gradient_id) {
	if (grad == NULL && (!gradient_id || gradient_id[0] == '\0')) {
		return "none";
	}
	if (gradient_id && gradient_id[0] != '\0') {
		return "gradient";
	}
	if (wlf_gradient_is_linear(grad)) {
		return "linear-gradient";
	}
	if (wlf_gradient_is_radial(grad)) {
		return "radial-gradient";
	}
	return "solid";
}

static void print_usage(const char *program_name) {
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("wlframe SVG Info Test Program\n\n");
	printf("Options:\n");
	printf("  -i, --input <path>     Input SVG file path to load\n");
	printf("  -o, --output <path>    Output SVG file path to save (optional)\n");
	printf("  -v, --verbose          Enable verbose logging\n");
	printf("  -h, --help             Show this help message\n\n");
	printf("Examples:\n");
	printf("  %s -i input.svg\n", program_name);
	printf("  %s -i input.svg -o out.svg\n", program_name);
}

int main(int argc, char *argv[]) {
	char *input_path = NULL;
	char *output_path = NULL;
	bool verbose = false;
	bool show_help = false;

	struct wlf_cmd_option options[] = {
		{WLF_OPTION_STRING, "input", 'i', &input_path},
		{WLF_OPTION_STRING, "output", 'o', &output_path},
		{WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose},
		{WLF_OPTION_BOOLEAN, "help", 'h', &show_help}
	};

	int remaining_args = wlf_cmd_parse_options(options, 4, &argc, argv);
	if (remaining_args < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return EXIT_FAILURE;
	}

	if (show_help) {
		print_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	const char *filename = input_path ? input_path : "examples/svg/resources/test.svg";
	int32_t log_level = verbose ? WLF_DEBUG : WLF_INFO;
	wlf_log_init(log_level, NULL);
	wlf_log(WLF_INFO, "Loading SVG: %s", filename);

	struct wlf_svg_image *image = wlf_svg_parse_from_file(filename, "px", 96.0f);
	if (!image) {
		wlf_log(WLF_ERROR, "Failed to parse SVG file: %s", filename);
		free(input_path);
		free(output_path);
		return EXIT_FAILURE;
	}

	struct wlf_svg_info info;
	wlf_svg_get_info(image, &info);

	wlf_log(WLF_INFO, "SVG canvas   : %.1f x %.1f px", info.width, info.height);
	wlf_log(WLF_INFO, "Shape count  : %d", info.n_shapes);
	wlf_log(WLF_INFO, "Path count   : %d", info.n_paths);
	if (info.n_paths > 0) {
		wlf_log(WLF_INFO, "Geometry bbox: [%.2f, %.2f, %.2f, %.2f]",
			info.bounds[0], info.bounds[1], info.bounds[2], info.bounds[3]);
	}

	int shape_idx = 0;
	int text_count = 0;
	for (struct wlf_shape *base = image->shapes; base != NULL;
		base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next, shape_idx++) {
		struct wlf_svg_shape *s = wlf_svg_shape_from_shape(base);
		if (s->geometry && wlf_shape_is_text(s->geometry)) {
			struct wlf_text_shape *text = wlf_text_shape_from_shape(s->geometry);
			text_count++;
			wlf_log(WLF_DEBUG, "    text=\"%s\" font=\"%s\" size=%.2f",
				text->text, text->font_family, text->font_size);
		}
		wlf_log(WLF_DEBUG, "  Shape[%d] id=\"%s\" opacity=%.2f visible=%s",
			shape_idx, s->id[0] ? s->id : "(none)", s->opacity,
			(s->flags & WLF_SVG_FLAGS_VISIBLE) ? "yes" : "no");
		wlf_log(WLF_DEBUG, "    fill=%s  stroke=%s  stroke_width=%.2f",
			svg_paint_desc(s->fill, s->fill_gradient),
			svg_paint_desc(s->stroke, s->stroke_gradient),
			s->stroke_width);
	}
	wlf_log(WLF_INFO, "Text count   : %d", text_count);

	if (output_path) {
		if (wlf_svg_save(image, output_path)) {
			wlf_log(WLF_INFO, "Saved SVG: %s", output_path);

			struct wlf_svg_image *saved =
				wlf_svg_parse_from_file(output_path, "px", 96.0f);
			if (saved == NULL) {
				wlf_log(WLF_ERROR, "Failed to re-load saved SVG: %s", output_path);
				wlf_svg_destroy(image);
				free(input_path);
				free(output_path);
				return EXIT_FAILURE;
			}

			int saved_text_count = 0;
			for (struct wlf_shape *base = saved->shapes; base != NULL;
				base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next) {
				struct wlf_svg_shape *s = wlf_svg_shape_from_shape(base);
				if (s->geometry && wlf_shape_is_text(s->geometry)) {
					saved_text_count++;
				}
			}

			wlf_log(WLF_INFO, "Saved text count: %d", saved_text_count);
			if (text_count > 0 && saved_text_count == 0) {
				wlf_log(WLF_ERROR, "Text export verification failed");
				wlf_svg_destroy(saved);
				wlf_svg_destroy(image);
				free(input_path);
				free(output_path);
				return EXIT_FAILURE;
			}

			wlf_svg_destroy(saved);
		} else {
			wlf_log(WLF_ERROR, "Failed to save SVG: %s", output_path);
			wlf_svg_destroy(image);
			free(input_path);
			free(output_path);
			return EXIT_FAILURE;
		}
	}

	wlf_svg_destroy(image);
	free(input_path);
	free(output_path);
	return EXIT_SUCCESS;
}
