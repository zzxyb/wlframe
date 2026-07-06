#include "wlf/platform/wlf_fontconfig.h"

#include <stdio.h>

static const char *role_name(enum wlf_font_role role) {
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

int main(void) {
	static const enum wlf_font_role roles[] = {
		WLF_FONT_ROLE_UI,
		WLF_FONT_ROLE_SANS_SERIF,
		WLF_FONT_ROLE_SERIF,
		WLF_FONT_ROLE_MONOSPACE,
		WLF_FONT_ROLE_ROUNDED,
		WLF_FONT_ROLE_EMOJI,
		WLF_FONT_ROLE_TITLE,
	};
	struct wlf_fontconfig *config = wlf_fontconfig_autocreate();
	size_t i;

	if (config == NULL) {
		fprintf(stderr, "failed to create fontconfig\n");
		return 1;
	}

	printf("fontconfig implementation: %s\n", config->impl->name);
	printf("ui_scale: %.2f\n", config->ui_scale);

	for (i = 0; i < sizeof(roles) / sizeof(roles[0]); ++i) {
		const char *families[WLF_FONTCONFIG_MAX_FALLBACKS];
		size_t count = wlf_fontconfig_get_families(config, roles[i], families,
			WLF_FONTCONFIG_MAX_FALLBACKS);
		size_t j;

		printf("%-12s", role_name(roles[i]));
		for (j = 0; j < count; ++j) {
			printf("%s%s", j == 0 ? "" : ", ", families[j]);
		}
		printf("\n");
	}

	printf("resolve(sans-serif): %s\n",
		wlf_fontconfig_resolve_generic(config, "sans-serif"));
	printf("resolve(monospace): %s\n",
		wlf_fontconfig_resolve_generic(config, "monospace"));

	wlf_fontconfig_destroy(config);
	return 0;
}
