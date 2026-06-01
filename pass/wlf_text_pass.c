#include "wlf/pass/wlf_text_pass.h"

#include <assert.h>
#include <stdlib.h>

static size_t text_codepoint_count(const char *text) {
	if (text == NULL) {
		return 0;
	}

	size_t count = 0;
	for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; ++p) {
		if ((*p & 0xc0) != 0x80) {
			++count;
		}
	}

	return count;
}

static int text_newline_count(const char *text) {
	if (text == NULL || text[0] == '\0') {
		return 0;
	}

	int count = 1;
	for (const char *p = text; *p != '\0'; ++p) {
		if (*p == '\n') {
			++count;
		}
	}

	return count;
}

static double text_line_height(const struct wlf_render_text_options *options) {
	return options->line_height > 0.0f ? options->line_height : options->font_size;
}

static double text_inner_width(const struct wlf_render_text_options *options) {
	double width = options->box.width;
	if (width <= 0.0) {
		return 0.0;
	}

	double inner = width - options->padding_left - options->padding_right;
	return inner > 0.0 ? inner : 0.0;
}

static int text_estimated_chars_per_line(const struct wlf_render_text_options *options,
		double char_width) {
	double inner_width = text_inner_width(options);
	if (inner_width <= 0.0 || char_width <= 0.0) {
		return 0;
	}

	int chars = (int)(inner_width / char_width);
	return chars > 0 ? chars : 1;
}

void wlf_render_text_pass_init(struct wlf_render_text_pass *pass,
		const struct wlf_render_text_pass_impl *impl) {
	assert(impl->destroy);
	assert(impl->render);
	*pass = (struct wlf_render_text_pass){
		.impl = impl,
	};

	wlf_signal_init(&pass->events.destroy);
}

void wlf_render_text_pass_destroy(struct wlf_render_text_pass *pass) {
	if (pass == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&pass->events.destroy, pass);
	assert(wlf_linked_list_empty(&pass->events.destroy.listener_list));

	if (pass->impl->destroy != NULL) {
		pass->impl->destroy(pass);
	} else {
		free(pass);
	}
}

void wlf_render_text_pass_add_text(struct wlf_render_text_pass *pass,
		struct wlf_render_target_info *render_target_info,
		const struct wlf_render_text_options *options) {
	assert(options->font_size >= 0.0f);
	assert(options->padding_left >= 0.0f);
	assert(options->padding_top >= 0.0f);
	assert(options->padding_right >= 0.0f);
	assert(options->padding_bottom >= 0.0f);
	assert(options->box.width >= 0.0);
	assert(options->box.height >= 0.0);

	pass->impl->render(pass, render_target_info, options);
}

void wlf_render_text_options_get_layout_metrics(
		const struct wlf_render_text_options *options,
		struct wlf_text_layout_metrics *metrics) {
	assert(metrics != NULL);

	double char_width = options->font_size * 0.6;
	double line_height = text_line_height(options);
	size_t glyphs = text_codepoint_count(options->text);
	int explicit_lines = text_newline_count(options->text);

	double raw_width = glyphs * char_width;
	int line_count = explicit_lines > 0 ? explicit_lines : 1;
	bool truncated = false;

	int chars_per_line = text_estimated_chars_per_line(options, char_width);
	if (options->wrap_mode != WLF_TEXT_WRAP_NONE && chars_per_line > 0) {
		int wrapped_lines = (int)((glyphs + (size_t)chars_per_line - 1) / (size_t)chars_per_line);
		if (wrapped_lines < explicit_lines) {
			wrapped_lines = explicit_lines;
		}
		line_count = wrapped_lines > 0 ? wrapped_lines : 1;
	}

	if (options->maximum_line_count > 0 && line_count > options->maximum_line_count) {
		line_count = options->maximum_line_count;
		truncated = true;
	}

	double content_width = raw_width;
	double content_height = line_count * line_height;

	if (options->box.width > 0.0) {
		double inner_width = text_inner_width(options);
		if (options->wrap_mode != WLF_TEXT_WRAP_NONE && inner_width > 0.0) {
			if (content_width > inner_width) {
				content_width = inner_width;
			}
		} else if (options->elide != WLF_TEXT_ELIDE_NONE && inner_width > 0.0 &&
				raw_width > inner_width) {
			content_width = inner_width;
			truncated = true;
		}
	}

	metrics->content_width = content_width;
	metrics->content_height = content_height;
	metrics->line_count = line_count;
	metrics->truncated = truncated;
}

void wlf_render_text_options_get_box(const struct wlf_render_text_options *options,
		struct wlf_frect *box) {
	struct wlf_text_layout_metrics metrics;
	wlf_render_text_options_get_layout_metrics(options, &metrics);

	*box = options->box;
	if (wlf_frect_is_empty(box)) {
		box->width = metrics.content_width +
			options->padding_left + options->padding_right;
		box->height = metrics.content_height +
			options->padding_top + options->padding_bottom;
	}
}

float wlf_render_text_options_get_alpha(const struct wlf_render_text_options *options) {
	if (options->alpha == NULL) {
		return 1.0f;
	}

	return *options->alpha;
}
