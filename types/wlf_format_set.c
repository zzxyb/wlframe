#include "wlf/types/wlf_format_set.h"
#include "wlf/types/wlf_pixel_format.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void wlf_render_format_finish(struct wlf_render_format *format) {
	if (!format) {
		return;
	}

	free(format->modifiers);
}

void wlf_render_format_set_finish(struct wlf_render_format_set *set) {
	for (size_t i = 0; i < set->len; ++i) {
		wlf_render_format_finish(&set->formats[i]);
	}
	free(set->formats);

	set->len = 0;
	set->capacity = 0;
	set->formats = NULL;
}

static struct wlf_render_format *format_set_get(const struct wlf_render_format_set *set,
		uint32_t format) {
	for (size_t i = 0; i < set->len; ++i) {
		if (set->formats[i].format == format) {
			return &set->formats[i];
		}
	}

	return NULL;
}

const struct wlf_render_format *wlf_render_format_set_get(
		const struct wlf_render_format_set *set, uint32_t format) {
	return format_set_get(set, format);
}

bool wlf_render_format_set_has(const struct wlf_render_format_set *set,
		uint32_t format, uint64_t modifier) {
	const struct wlf_render_format *fmt = wlf_render_format_set_get(set, format);
	if (!fmt) {
		return false;
	}
	return wlf_render_format_has(fmt, modifier);
}

bool wlf_render_format_set_add(struct wlf_render_format_set *set, uint32_t format,
		uint64_t modifier) {
	assert(format != WLF_FORMAT_INVALID);

	struct wlf_render_format *existing = format_set_get(set, format);
	if (existing) {
		return wlf_render_format_add(existing, modifier);
	}

	struct wlf_render_format fmt;
	wlf_render_format_init(&fmt, format);
	if (!wlf_render_format_add(&fmt, modifier)) {
		wlf_render_format_finish(&fmt);
		return false;
	}

	if (set->len == set->capacity) {
		size_t capacity = set->capacity ? set->capacity * 2 : 4;

		struct wlf_render_format *fmts = realloc(set->formats, sizeof(*fmts) * capacity);
		if (!fmts) {
			wlf_log_errno(WLF_ERROR, "Allocation failed");
			wlf_render_format_finish(&fmt);
			return false;
		}

		set->capacity = capacity;
		set->formats = fmts;
	}

	set->formats[set->len++] = fmt;
	return true;
}

bool wlf_render_format_set_remove(struct wlf_render_format_set *set, uint32_t format,
		uint64_t modifier) {
	struct wlf_render_format *fmt = format_set_get(set, format);
	if (fmt == NULL) {
		return false;
	}

	for (size_t idx = 0; idx < fmt->len; idx++) {
		if (fmt->modifiers[idx] == modifier) {
			memmove(&fmt->modifiers[idx], &fmt->modifiers[idx+1], (fmt->len - idx - 1) * sizeof(fmt->modifiers[0]));
			fmt->len--;
			return true;
		}
	}
	return false;
}

void wlf_render_format_init(struct wlf_render_format *fmt, uint32_t format) {
	*fmt = (struct wlf_render_format){
		.format = format,
	};
}

bool wlf_render_format_has(const struct wlf_render_format *fmt, uint64_t modifier) {
	for (size_t i = 0; i < fmt->len; ++i) {
		if (fmt->modifiers[i] == modifier) {
			return true;
		}
	}
	return false;
}

bool wlf_render_format_add(struct wlf_render_format *fmt, uint64_t modifier) {
	if (wlf_render_format_has(fmt, modifier)) {
		return true;
	}

	if (fmt->len == fmt->capacity) {
		size_t capacity = fmt->capacity ? fmt->capacity * 2 : 4;

		uint64_t *new_modifiers = realloc(fmt->modifiers, sizeof(*fmt->modifiers) * capacity);
		if (!new_modifiers) {
			wlf_log_errno(WLF_ERROR, "Allocation failed");
			return false;
		}

		fmt->capacity = capacity;
		fmt->modifiers = new_modifiers;
	}

	fmt->modifiers[fmt->len++] = modifier;
	return true;
}

bool wlf_render_format_copy(struct wlf_render_format *dst, const struct wlf_render_format *src) {
	assert(src->len <= src->capacity);

	uint64_t *modifiers = malloc(sizeof(*modifiers) * src->len);
	if (!modifiers) {
		return false;
	}

	memcpy(modifiers, src->modifiers, sizeof(*modifiers) * src->len);

	wlf_render_format_finish(dst);
	dst->capacity = src->len;
	dst->len = src->len;
	dst->format = src->format;
	dst->modifiers = modifiers;
	return true;
}

bool wlf_render_format_set_copy(struct wlf_render_format_set *dst, const struct wlf_render_format_set *src) {
	struct wlf_render_format *formats = malloc(src->len * sizeof(formats[0]));
	if (formats == NULL) {
		return false;
	}

	struct wlf_render_format_set out = {
		.len = 0,
		.capacity = src->len,
		.formats = formats,
	};

	size_t i;
	for (i = 0; i < src->len; i++) {
		out.formats[out.len] = (struct wlf_render_format){0};
		if (!wlf_render_format_copy(&out.formats[out.len], &src->formats[i])) {
			wlf_render_format_set_finish(&out);
			return false;
		}

		out.len++;
	}

	*dst = out;

	return true;
}

bool wlf_render_format_intersect(struct wlf_render_format *dst,
		const struct wlf_render_format *a, const struct wlf_render_format *b) {
	assert(a->format == b->format);

	size_t capacity = a->len < b->len ? a->len : b->len;
	uint64_t *modifiers = malloc(sizeof(*modifiers) * capacity);
	if (!modifiers) {
		return false;
	}

	struct wlf_render_format fmt = {
		.capacity = capacity,
		.len = 0,
		.modifiers = modifiers,
		.format = a->format,
	};

	for (size_t i = 0; i < a->len; i++) {
		for (size_t j = 0; j < b->len; j++) {
			if (a->modifiers[i] == b->modifiers[j]) {
				assert(fmt.len < fmt.capacity);
				fmt.modifiers[fmt.len++] = a->modifiers[i];
				break;
			}
		}
	}

	wlf_render_format_finish(dst);
	*dst = fmt;
	return true;
}

bool wlf_render_format_set_intersect(struct wlf_render_format_set *dst,
		const struct wlf_render_format_set *a, const struct wlf_render_format_set *b) {
	struct wlf_render_format_set out = {0};
	out.capacity = a->len < b->len ? a->len : b->len;
	out.formats = malloc(sizeof(*out.formats) * out.capacity);
	if (out.formats == NULL) {
		wlf_log_errno(WLF_ERROR, "Allocation failed");
		return false;
	}

	for (size_t i = 0; i < a->len; i++) {
		for (size_t j = 0; j < b->len; j++) {
			if (a->formats[i].format == b->formats[j].format) {
				// When the two formats have no common modifier, keep
				// intersecting the rest of the formats: they may be compatible
				// with each other
				out.formats[out.len] = (struct wlf_render_format){0};
				if (!wlf_render_format_intersect(&out.formats[out.len],
						&a->formats[i], &b->formats[j])) {
					wlf_render_format_set_finish(&out);
					return false;
				}

				if (out.formats[out.len].len == 0) {
					wlf_render_format_finish(&out.formats[out.len]);
				} else {
					out.len++;
				}

				break;
			}
		}
	}

	if (out.len == 0) {
		wlf_render_format_set_finish(&out);
		return false;
	}

	wlf_render_format_set_finish(dst);
	*dst = out;
	return true;
}

static bool drm_format_set_extend(struct wlf_render_format_set *dst,
		const struct wlf_render_format_set *src) {
	for (size_t i = 0; i < src->len; i++) {
		struct wlf_render_format *format = &src->formats[i];
		for (size_t j = 0; j < format->len; j++) {
			if (!wlf_render_format_set_add(dst, format->format, format->modifiers[j])) {
				wlf_log_errno(WLF_ERROR, "Adding format/modifier to set failed");
				return false;
			}
		}
	}

	return true;
}

bool wlf_render_format_set_union(struct wlf_render_format_set *dst,
		const struct wlf_render_format_set *a, const struct wlf_render_format_set *b) {
	struct wlf_render_format_set out = {0};
	out.capacity = a->len + b->len;
	out.formats = malloc(sizeof(*out.formats) * out.capacity);
	if (out.formats == NULL) {
		wlf_log_errno(WLF_ERROR, "Allocation failed");
		return false;
	}

	if (!drm_format_set_extend(&out, a) ||
		!drm_format_set_extend(&out, b)) {
		wlf_render_format_set_finish(&out);
		return false;
	}

	wlf_render_format_set_finish(dst);
	*dst = out;

	return true;
}
