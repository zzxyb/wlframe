#include "wlf/platform/linux/fontconfig.h"

#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_linked_list.h"

#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>
#include <string.h>

struct linux_fontconfig_role_query {
	enum wlf_font_role role;
	const char *generic_family;
	const char *fallbacks[4];
};

static void linux_fontconfig_set_family(struct wlf_fontconfig *config,
		enum wlf_font_role role, size_t index, const char *family) {
	assert(config != NULL);
	assert(role >= 0 && role < WLF_FONT_ROLE_COUNT);
	assert(index < WLF_FONTCONFIG_MAX_FALLBACKS);
	assert(family != NULL);

	strncpy(config->families[role][index], family,
		WLF_FONTCONFIG_FAMILY_NAME_MAX - 1);
	config->families[role][index][WLF_FONTCONFIG_FAMILY_NAME_MAX - 1] = '\0';

	if (index >= config->counts[role]) {
		config->counts[role] = index + 1;
	}
}

static bool linux_fontconfig_has_family(const struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *family) {
	for (size_t i = 0; i < config->counts[role]; ++i) {
		if (strcasecmp(config->families[role][i], family) == 0) {
			return true;
		}
	}

	return false;
}

static void linux_fontconfig_add_family(struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *family) {
	if (family == NULL || family[0] == '\0' ||
			config->counts[role] >= WLF_FONTCONFIG_MAX_FALLBACKS ||
			linux_fontconfig_has_family(config, role, family)) {
		return;
	}

	linux_fontconfig_set_family(config, role, config->counts[role], family);
}

static void linux_fontconfig_add_static_fallbacks(struct wlf_fontconfig *config,
		const struct linux_fontconfig_role_query *query) {
	for (size_t i = 0; i < sizeof(query->fallbacks) / sizeof(query->fallbacks[0]); ++i) {
		linux_fontconfig_add_family(config, query->role, query->fallbacks[i]);
	}
}

static void linux_fontconfig_collect_fontset(struct wlf_fontconfig *config,
		FcConfig *fc_config, const struct linux_fontconfig_role_query *query) {
	FcPattern *pattern;
	FcFontSet *font_set;
	FcResult result = FcResultNoMatch;

	pattern = FcPatternCreate();
	if (pattern == NULL) {
		return;
	}

	(void)FcPatternAddString(pattern, FC_FAMILY,
		(const FcChar8 *)query->generic_family);
	if (query->role == WLF_FONT_ROLE_TITLE) {
		(void)FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
	}

	(void)FcConfigSubstitute(fc_config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	font_set = FcFontSort(fc_config, pattern, FcTrue, NULL, &result);
	if (font_set != NULL && result == FcResultMatch) {
		for (int i = 0; i < font_set->nfont &&
				config->counts[query->role] < WLF_FONTCONFIG_MAX_FALLBACKS; ++i) {
			FcChar8 *family = NULL;
			if (FcPatternGetString(font_set->fonts[i], FC_FAMILY, 0,
					&family) == FcResultMatch) {
				linux_fontconfig_add_family(config, query->role,
					(const char *)family);
			}
		}
	}

	if (font_set != NULL) {
		FcFontSetDestroy(font_set);
	}
	FcPatternDestroy(pattern);
}

static void linux_fontconfig_resolve_role(struct wlf_fontconfig *config,
		FcConfig *fc_config, const struct linux_fontconfig_role_query *query) {
	config->counts[query->role] = 0;

	linux_fontconfig_collect_fontset(config, fc_config, query);
	linux_fontconfig_add_static_fallbacks(config, query);
}

static void linux_fontconfig_fill_defaults(struct wlf_fontconfig *config,
		FcConfig *fc_config) {
	static const struct linux_fontconfig_role_query queries[] = {
		{
			.role = WLF_FONT_ROLE_UI,
			.generic_family = "system-ui",
			.fallbacks = { "Noto Sans", "DejaVu Sans", "sans-serif", NULL },
		},
		{
			.role = WLF_FONT_ROLE_SANS_SERIF,
			.generic_family = "sans-serif",
			.fallbacks = { "Noto Sans", "DejaVu Sans", "Arial", NULL },
		},
		{
			.role = WLF_FONT_ROLE_SERIF,
			.generic_family = "serif",
			.fallbacks = { "Noto Serif", "DejaVu Serif", "Times New Roman", NULL },
		},
		{
			.role = WLF_FONT_ROLE_MONOSPACE,
			.generic_family = "monospace",
			.fallbacks = { "Noto Sans Mono", "DejaVu Sans Mono", "monospace", NULL },
		},
		{
			.role = WLF_FONT_ROLE_ROUNDED,
			.generic_family = "ui-rounded",
			.fallbacks = { "Noto Sans", "Cantarell", "DejaVu Sans", NULL },
		},
		{
			.role = WLF_FONT_ROLE_EMOJI,
			.generic_family = "emoji",
			.fallbacks = { "Noto Color Emoji", "Twemoji", "Segoe UI Emoji", NULL },
		},
		{
			.role = WLF_FONT_ROLE_TITLE,
			.generic_family = "system-ui",
			.fallbacks = { "Noto Sans", "Cantarell", "DejaVu Sans", NULL },
		},
	};

	for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i) {
		linux_fontconfig_resolve_role(config, fc_config, &queries[i]);
	}
}

static void linux_fontconfig_destroy(struct wlf_fontconfig *config) {
	struct wlf_linux_fontconfig *linux_fontconfig =
		wlf_linux_fontconfig_from_fontconfig(config);

	if (linux_fontconfig->fc_config != NULL) {
		FcConfigDestroy(linux_fontconfig->fc_config);
		linux_fontconfig->fc_config = NULL;
	}

	free(linux_fontconfig);
}

static const struct wlf_fontconfig_impl linux_fontconfig_impl = {
	.name = "linux",
	.destroy = linux_fontconfig_destroy,
};

struct wlf_linux_fontconfig *wlf_linux_fontconfig_create(void) {
	struct wlf_linux_fontconfig *config = calloc(1, sizeof(*config));
	if (config == NULL) {
		return NULL;
	}

	config->fc_config = FcInitLoadConfigAndFonts();
	if (config->fc_config == NULL) {
		free(config);
		return NULL;
	}

	wlf_fontconfig_init(&config->base, &linux_fontconfig_impl);
	linux_fontconfig_fill_defaults(&config->base, config->fc_config);

	return config;
}

bool wlf_fontconfig_is_linux(const struct wlf_fontconfig *config) {
	return config != NULL &&
		config->impl != NULL &&
		config->impl == &linux_fontconfig_impl;
}

struct wlf_linux_fontconfig *wlf_linux_fontconfig_from_fontconfig(
		struct wlf_fontconfig *config) {
	assert(config && config->impl == &linux_fontconfig_impl);

	struct wlf_linux_fontconfig *linux_fontconfig =
		wlf_container_of(config, linux_fontconfig, base);

	return linux_fontconfig;
}
