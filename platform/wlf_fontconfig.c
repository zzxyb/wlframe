#include "wlf/platform/wlf_fontconfig.h"
#include "wlf/config.h"

#if WLF_HAS_MACOS_PLATFORM
#include "wlf/platform/macos/fontconfig.h"
#endif

#include "wlf/utils/wlf_compat.h"

#include <assert.h>
#include <stdlib.h>

const char *wlf_font_role_name(enum wlf_font_role role) {
	switch (role) {
	case WLF_FONT_ROLE_UI:
		return "ui";
	case WLF_FONT_ROLE_SANS_SERIF:
		return "sans-serif";
	case WLF_FONT_ROLE_SERIF:
		return "serif";
	case WLF_FONT_ROLE_MONOSPACE:
		return "monospace";
	case WLF_FONT_ROLE_ROUNDED:
		return "rounded";
	case WLF_FONT_ROLE_EMOJI:
		return "emoji";
	case WLF_FONT_ROLE_TITLE:
		return "title";
	case WLF_FONT_ROLE_COUNT:
	default:
		return "unknown";
	}
}

void wlf_fontconfig_init(struct wlf_fontconfig *config,
		const struct wlf_fontconfig_impl *impl) {
	assert(config != NULL);
	assert(impl != NULL);
	assert(impl->destroy != NULL);
	assert(impl->name != NULL);

	*config = (struct wlf_fontconfig){
		.impl = impl,
		.platform = impl->platform,
		.ui_scale = 1.0,
	};
}

struct wlf_fontconfig *wlf_fontconfig_autocreate(void) {
#if WLF_HAS_MACOS_PLATFORM
	struct wlf_macos_fontconfig *macos_fontconfig = wlf_macos_fontconfig_create();
	if (macos_fontconfig == NULL) {
		return NULL;
	}

	return &macos_fontconfig->base;
#endif

	return NULL;
}

void wlf_fontconfig_destroy(struct wlf_fontconfig *config) {
	if (config == NULL) {
		return;
	}

	if (config->impl != NULL && config->impl->destroy != NULL) {
		config->impl->destroy(config);
	} else {
		free(config);
	}
}

const char *wlf_fontconfig_primary_family(const struct wlf_fontconfig *config,
		enum wlf_font_role role) {
	if (config == NULL || role < 0 || role >= WLF_FONT_ROLE_COUNT ||
			config->counts[role] == 0) {
		return NULL;
	}

	return config->families[role][0];
}

size_t wlf_fontconfig_get_families(const struct wlf_fontconfig *config,
		enum wlf_font_role role, const char **out_families,
		size_t max_families) {
	if (config == NULL || out_families == NULL ||
			role < 0 || role >= WLF_FONT_ROLE_COUNT) {
		return 0;
	}

	size_t count = config->counts[role];
	if (count > max_families) {
		count = max_families;
	}

	for (size_t i = 0; i < count; ++i) {
		out_families[i] = config->families[role][i];
	}

	return count;
}

const char *wlf_fontconfig_resolve_generic(const struct wlf_fontconfig *config,
		const char *generic_family) {
	if (config == NULL || generic_family == NULL) {
		return NULL;
	}

	if (strcasecmp(generic_family, "system-ui") == 0 ||
			strcasecmp(generic_family, "ui") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_UI);
	}
	if (strcasecmp(generic_family, "sans-serif") == 0 ||
			strcasecmp(generic_family, "sans") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_SANS_SERIF);
	}
	if (strcasecmp(generic_family, "serif") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_SERIF);
	}
	if (strcasecmp(generic_family, "monospace") == 0 ||
			strcasecmp(generic_family, "mono") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_MONOSPACE);
	}
	if (strcasecmp(generic_family, "rounded") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_ROUNDED);
	}
	if (strcasecmp(generic_family, "emoji") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_EMOJI);
	}
	if (strcasecmp(generic_family, "title") == 0) {
		return wlf_fontconfig_primary_family(config, WLF_FONT_ROLE_TITLE);
	}

	return NULL;
}
