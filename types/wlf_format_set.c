#include "wlf/types/wlf_format_set.h"

#include <stdlib.h>
#include <string.h>

static bool checked_mul_size(size_t a, size_t b, size_t *out) {
	if (a != 0 && b > SIZE_MAX / a) {
		return false;
	}

	*out = a * b;
	return true;
}

static struct wlf_format *wlf_format_set_get_mut(
		struct wlf_format_set *set, uint32_t format) {
	if (set == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < set->len; i++) {
		if (set->formats[i].format == format) {
			return &set->formats[i];
		}
	}

	return NULL;
}

static bool wlf_format_modifier_index(const struct wlf_format *format,
		uint64_t modifier, size_t *index) {
	if (format == NULL) {
		return false;
	}

	for (size_t i = 0; i < format->len; i++) {
		if (format->modifiers[i] == modifier) {
			if (index != NULL) {
				*index = i;
			}
			return true;
		}
	}

	return false;
}

static bool wlf_format_ensure_capacity(struct wlf_format *format, size_t needed) {
	if (needed <= format->capacity) {
		return true;
	}

	size_t new_capacity = format->capacity == 0 ? 4 : format->capacity;
	while (new_capacity < needed) {
		if (new_capacity > SIZE_MAX / 2) {
			new_capacity = needed;
			break;
		}
		new_capacity *= 2;
	}

	size_t bytes = 0;
	if (!checked_mul_size(new_capacity, sizeof(*format->modifiers), &bytes)) {
		return false;
	}

	uint64_t *modifiers = realloc(format->modifiers, bytes);
	if (modifiers == NULL) {
		return false;
	}

	format->modifiers = modifiers;
	format->capacity = new_capacity;
	return true;
}

static bool wlf_format_set_ensure_capacity(struct wlf_format_set *set, size_t needed) {
	if (needed <= set->capacity) {
		return true;
	}

	size_t new_capacity = set->capacity == 0 ? 4 : set->capacity;
	while (new_capacity < needed) {
		if (new_capacity > SIZE_MAX / 2) {
			new_capacity = needed;
			break;
		}
		new_capacity *= 2;
	}

	size_t bytes = 0;
	if (!checked_mul_size(new_capacity, sizeof(*set->formats), &bytes)) {
		return false;
	}

	struct wlf_format *formats = realloc(set->formats, bytes);
	if (formats == NULL) {
		return false;
	}

	set->formats = formats;
	set->capacity = new_capacity;
	return true;
}

static struct wlf_format *wlf_format_set_insert_format(
		struct wlf_format_set *set, uint32_t format) {
	if (!wlf_format_set_ensure_capacity(set, set->len + 1)) {
		return NULL;
	}

	struct wlf_format *entry = &set->formats[set->len++];
	*entry = (struct wlf_format) {
		.format = format,
		.len = 0,
		.capacity = 0,
		.modifiers = NULL,
	};

	return entry;
}

void wlf_format_finish(struct wlf_format *format) {
	if (format == NULL) {
		return;
	}

	free(format->modifiers);
	format->modifiers = NULL;
	format->len = 0;
	format->capacity = 0;
	format->format = 0;
}

void wlf_format_set_finish(struct wlf_format_set *set) {
	if (set == NULL) {
		return;
	}

	for (size_t i = 0; i < set->len; i++) {
		wlf_format_finish(&set->formats[i]);
	}

	free(set->formats);
	set->formats = NULL;
	set->len = 0;
	set->capacity = 0;
}

const struct wlf_format *wlf_format_set_get(
		const struct wlf_format_set *set, uint32_t format) {
	if (set == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < set->len; i++) {
		if (set->formats[i].format == format) {
			return &set->formats[i];
		}
	}

	return NULL;
}

bool wlf_format_set_remove(struct wlf_format_set *set, uint32_t format,
		uint64_t modifier) {
	struct wlf_format *entry = wlf_format_set_get_mut(set, format);
	if (entry == NULL) {
		return false;
	}

	size_t index = 0;
	if (!wlf_format_modifier_index(entry, modifier, &index)) {
		return false;
	}

	size_t idx = index;
	if (idx + 1 < entry->len) {
		memmove(&entry->modifiers[idx], &entry->modifiers[idx + 1],
			(entry->len - idx - 1) * sizeof(*entry->modifiers));
	}
	entry->len--;

	if (entry->len > 0) {
		return true;
	}

	for (size_t i = 0; i < set->len; i++) {
		if (&set->formats[i] != entry) {
			continue;
		}

		wlf_format_finish(&set->formats[i]);
		if (i + 1 < set->len) {
			memmove(&set->formats[i], &set->formats[i + 1],
				(set->len - i - 1) * sizeof(*set->formats));
		}
		set->len--;
		break;
	}

	return true;
}

bool wlf_format_set_has(const struct wlf_format_set *set,
		uint32_t format, uint64_t modifier) {
	const struct wlf_format *entry = wlf_format_set_get(set, format);
	if (entry == NULL) {
		return false;
	}

	return wlf_format_modifier_index(entry, modifier, NULL);
}

bool wlf_format_set_add(struct wlf_format_set *set, uint32_t format,
		uint64_t modifier) {
	if (set == NULL) {
		return false;
	}

	struct wlf_format *entry = wlf_format_set_get_mut(set, format);
	if (entry == NULL) {
		entry = wlf_format_set_insert_format(set, format);
		if (entry == NULL) {
			return false;
		}
	}

	if (wlf_format_modifier_index(entry, modifier, NULL)) {
		return true;
	}

	if (!wlf_format_ensure_capacity(entry, entry->len + 1)) {
		return false;
	}

	entry->modifiers[entry->len++] = modifier;
	return true;
}

bool wlf_format_set_intersect(struct wlf_format_set *dst,
		const struct wlf_format_set *a, const struct wlf_format_set *b) {
	if (dst == NULL || a == NULL || b == NULL) {
		return false;
	}

	struct wlf_format_set out = {0};
	for (size_t i = 0; i < a->len; i++) {
		const struct wlf_format *a_fmt = &a->formats[i];
		const struct wlf_format *b_fmt = wlf_format_set_get(b, a_fmt->format);
		if (b_fmt == NULL) {
			continue;
		}

		for (size_t j = 0; j < a_fmt->len; j++) {
			uint64_t modifier = a_fmt->modifiers[j];
			if (!wlf_format_modifier_index(b_fmt, modifier, NULL)) {
				continue;
			}

			if (!wlf_format_set_add(&out, a_fmt->format, modifier)) {
				wlf_format_set_finish(&out);
				return false;
			}
		}
	}

	wlf_format_set_finish(dst);
	*dst = out;

	return dst->len > 0;
}

bool wlf_format_set_union(struct wlf_format_set *dst,
		const struct wlf_format_set *a, const struct wlf_format_set *b) {
	if (dst == NULL || a == NULL || b == NULL) {
		return false;
	}

	struct wlf_format_set out = {0};
	const struct wlf_format_set *sources[] = {a, b};

	for (size_t s = 0; s < sizeof(sources) / sizeof(sources[0]); s++) {
		for (size_t i = 0; i < sources[s]->len; i++) {
			const struct wlf_format *fmt = &sources[s]->formats[i];
			for (size_t j = 0; j < fmt->len; j++) {
				if (!wlf_format_set_add(&out, fmt->format, fmt->modifiers[j])) {
					wlf_format_set_finish(&out);
					return false;
				}
			}
		}
	}

	wlf_format_set_finish(dst);
	*dst = out;
	return true;
}
