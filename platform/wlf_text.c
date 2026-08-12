#include "wlf/platform/wlf_text.h"

#include "wlf/config.h"

#if WLF_HAS_LINUX_PLATFORM
#include "wlf/platform/linux/text.h"
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void wlf_text_init(struct wlf_text *text,
		const struct wlf_text_impl *impl) {
	assert(text != NULL);
	assert(impl != NULL);
	assert(impl->name != NULL);
	assert(impl->rasterize != NULL);
	assert(impl->destroy_raster != NULL);
	assert(impl->destroy != NULL);

	text->impl = impl;
}

struct wlf_text *wlf_text_autocreate(void) {
#if WLF_HAS_LINUX_PLATFORM
	struct wlf_linux_text *text = wlf_linux_text_create();
	if (text == NULL) {
		return NULL;
	}

	return &text->base;
#else
	/* Core Text and DirectWrite implementations will be selected here once they are
	 * implemented. Keeping the selection in this module leaves scene code
	 * independent of platform text libraries. */
	return NULL;
#endif
}

void wlf_text_destroy(struct wlf_text *text) {
	if (text == NULL) {
		return;
	}

	if (text->impl != NULL && text->impl->destroy != NULL) {
		text->impl->destroy(text);
	} else {
		free(text);
	}
}

bool wlf_text_rasterize(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	if (raster == NULL) {
		return false;
	}
	*raster = (struct wlf_text_raster){0};

	if (text == NULL || text->impl == NULL || options == NULL ||
			options->text == NULL || options->font_size <= 0 ||
			!isfinite(options->font_size) || options->raster_scale <= 0 ||
			!isfinite(options->raster_scale) ||
			!wlf_text_is_valid_utf8(options->text)) {
		return false;
	}

	if (!text->impl->rasterize(text, options, raster)) {
		wlf_text_raster_destroy(text, raster);
		return false;
	}
	return true;
}

void wlf_text_raster_destroy(struct wlf_text *text,
		struct wlf_text_raster *raster) {
	if (raster == NULL) {
		return;
	}

	if (text != NULL && text->impl != NULL &&
			text->impl->destroy_raster != NULL) {
		text->impl->destroy_raster(text, raster);
	} else {
		*raster = (struct wlf_text_raster){0};
	}
}

static bool utf8_continuation(unsigned char byte) {
	return (byte & 0xc0) == 0x80;
}

bool wlf_text_is_valid_utf8(const char *text) {
	if (text == NULL) {
		return false;
	}

	const unsigned char *p = (const unsigned char *)text;
	size_t length = strlen(text);
	size_t offset = 0;
	while (offset < length) {
		unsigned char byte = p[offset];
		if (byte < 0x80) {
			offset++;
			continue;
		}

		if (byte >= 0xc2 && byte <= 0xdf) {
			if (length - offset < 2 ||
					!utf8_continuation(p[offset + 1])) {
				return false;
			}
			offset += 2;
			continue;
		}

		if (byte == 0xe0) {
			if (length - offset < 3 || p[offset + 1] < 0xa0 ||
					p[offset + 1] > 0xbf ||
					!utf8_continuation(p[offset + 2])) {
				return false;
			}
			offset += 3;
			continue;
		}
		if ((byte >= 0xe1 && byte <= 0xec) ||
				(byte >= 0xee && byte <= 0xef)) {
			if (length - offset < 3 ||
					!utf8_continuation(p[offset + 1]) ||
					!utf8_continuation(p[offset + 2])) {
				return false;
			}
			offset += 3;
			continue;
		}
		if (byte == 0xed) {
			if (length - offset < 3 || p[offset + 1] < 0x80 ||
					p[offset + 1] > 0x9f ||
					!utf8_continuation(p[offset + 2])) {
				return false;
			}
			offset += 3;
			continue;
		}

		if (byte == 0xf0) {
			if (length - offset < 4 || p[offset + 1] < 0x90 ||
					p[offset + 1] > 0xbf ||
					!utf8_continuation(p[offset + 2]) ||
					!utf8_continuation(p[offset + 3])) {
				return false;
			}
			offset += 4;
			continue;
		}
		if (byte >= 0xf1 && byte <= 0xf3) {
			if (length - offset < 4 ||
					!utf8_continuation(p[offset + 1]) ||
					!utf8_continuation(p[offset + 2]) ||
					!utf8_continuation(p[offset + 3])) {
				return false;
			}
			offset += 4;
			continue;
		}
		if (byte == 0xf4) {
			if (length - offset < 4 || p[offset + 1] < 0x80 ||
					p[offset + 1] > 0x8f ||
					!utf8_continuation(p[offset + 2]) ||
					!utf8_continuation(p[offset + 3])) {
				return false;
			}
			offset += 4;
			continue;
		}

		return false;
	}

	return true;
}
