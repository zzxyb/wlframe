#include "wlf/fontconfig/macos/fontconfig.h"

#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_env.h"

#import <AppKit/AppKit.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void macos_fontconfig_set_family(struct wlf_fontconfig *config,
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

static void macos_fontconfig_set_role_families(struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *family0, const char *family1,
		const char *family2, const char *family3) {
	config->counts[role] = 0;
	if (family0 != NULL) {
		macos_fontconfig_set_family(config, role, config->counts[role], family0);
	}
	if (family1 != NULL) {
		macos_fontconfig_set_family(config, role, config->counts[role], family1);
	}
	if (family2 != NULL) {
		macos_fontconfig_set_family(config, role, config->counts[role], family2);
	}
	if (family3 != NULL) {
		macos_fontconfig_set_family(config, role, config->counts[role], family3);
	}
}

static void macos_fontconfig_apply_env_override(struct wlf_fontconfig *config,
		enum wlf_font_role role, const char *env_name) {
	const char *override = wlf_get_env(env_name);
	if (override == NULL || override[0] == '\0') {
		return;
	}

	config->counts[role] = 0;
	macos_fontconfig_set_family(config, role, 0, override);
}

static const char *macos_fontconfig_nsfont_family(NSFont *font,
		const char *fallback_family) {
	if (font == nil) {
		return fallback_family;
	}

	NSString *family = font.familyName;
	if (family == nil || family.length == 0) {
		return fallback_family;
	}

	return family.UTF8String;
}

static void macos_fontconfig_fill_defaults(struct wlf_fontconfig *config) {
	const char *ui_family = ".AppleSystemUIFont";
	const char *sans_family = ".AppleSystemUIFont";
	const char *serif_family = "Times New Roman";
	const char *mono_family = "SF Mono";
	const char *rounded_family = "SF Pro Rounded";
	const char *emoji_family = "Apple Color Emoji";
	const char *title_family = "SF Pro Display";

	@autoreleasepool {
		[NSApplication sharedApplication];

		ui_family = macos_fontconfig_nsfont_family(
			[NSFont systemFontOfSize:0], ui_family);
		sans_family = macos_fontconfig_nsfont_family(
			[NSFont messageFontOfSize:0], sans_family);
		serif_family = macos_fontconfig_nsfont_family(
			[NSFont fontWithName:@"Times New Roman" size:0], serif_family);
		mono_family = macos_fontconfig_nsfont_family(
			[NSFont monospacedSystemFontOfSize:0 weight:NSFontWeightRegular],
			mono_family);
		emoji_family = macos_fontconfig_nsfont_family(
			[NSFont fontWithName:@"Apple Color Emoji" size:0], emoji_family);
		title_family = macos_fontconfig_nsfont_family(
			[NSFont boldSystemFontOfSize:0], title_family);
		rounded_family = macos_fontconfig_nsfont_family(
			[NSFont fontWithName:@"SF Pro Rounded" size:0], rounded_family);
	}

	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_UI,
		ui_family, "Helvetica Neue", "PingFang SC", NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_SANS_SERIF,
		sans_family, "Helvetica Neue", "Arial", NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_SERIF,
		serif_family, "STSong", "Songti SC", NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_MONOSPACE,
		mono_family, "Menlo", "Monaco", NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_ROUNDED,
		rounded_family, ui_family, "PingFang SC", NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_EMOJI,
		emoji_family, ui_family, NULL, NULL);
	macos_fontconfig_set_role_families(config, WLF_FONT_ROLE_TITLE,
		title_family, ui_family, "PingFang SC", NULL);

	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_UI, "WLF_FONT_UI");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_SANS_SERIF,
		"WLF_FONT_SANS");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_SERIF,
		"WLF_FONT_SERIF");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_MONOSPACE,
		"WLF_FONT_MONO");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_ROUNDED,
		"WLF_FONT_ROUNDED");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_EMOJI,
		"WLF_FONT_EMOJI");
	macos_fontconfig_apply_env_override(config, WLF_FONT_ROLE_TITLE,
		"WLF_FONT_TITLE");
}

static void macos_fontconfig_destroy(struct wlf_fontconfig *config) {
	free(config);
}

static const struct wlf_fontconfig_impl macos_fontconfig_impl = {
	.name = "macos",
	.platform = WLF_FONTCONFIG_PLATFORM_MACOS,
	.destroy = macos_fontconfig_destroy,
};

struct wlf_macos_fontconfig *wlf_macos_fontconfig_create(void) {
	struct wlf_macos_fontconfig *config = calloc(1, sizeof(*config));
	if (config == NULL) {
		return NULL;
	}

	wlf_fontconfig_init(&config->base, &macos_fontconfig_impl);
	macos_fontconfig_fill_defaults(&config->base);

	return config;
}

bool wlf_fontconfig_is_macos(const struct wlf_fontconfig *config) {
	return config != NULL &&
		config->impl != NULL &&
		config->impl == &macos_fontconfig_impl;
}

struct wlf_macos_fontconfig *wlf_macos_fontconfig_from_fontconfig(
		struct wlf_fontconfig *config) {
	assert(config && config->impl == &macos_fontconfig_impl);

	struct wlf_macos_fontconfig *macos_fontconfig =
		wlf_container_of(config, macos_fontconfig, base);

	return macos_fontconfig;
}
