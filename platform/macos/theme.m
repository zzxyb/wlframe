#include "wlf/platform/macos/theme.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_env.h"

#import <AppKit/AppKit.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

@interface WLFMacOSThemeObserver : NSObject

- (instancetype)initWithTheme:(struct wlf_macos_theme *)theme;
- (void)appearanceChanged:(NSNotification *)notification;

@end

static bool macos_theme_parse_appearance(const char *value,
		enum wlf_theme_appearance *appearance) {
	if (value == NULL || appearance == NULL) {
		return false;
	}

	if (strcasecmp(value, "dark") == 0) {
		*appearance = WLF_THEME_APPEARANCE_DARK;
		return true;
	}

	if (strcasecmp(value, "light") == 0) {
		*appearance = WLF_THEME_APPEARANCE_LIGHT;
		return true;
	}

	return false;
}

static enum wlf_theme_appearance macos_theme_detect_appearance(void) {
	enum wlf_theme_appearance appearance;
	const char *env = wlf_get_env("WLF_THEME_APPEARANCE");

	if (macos_theme_parse_appearance(env, &appearance)) {
		return appearance;
	}

	@autoreleasepool {
		NSAppearance *effective_appearance;
		NSString *best_match;

		[NSApplication sharedApplication];
		effective_appearance = NSApp.effectiveAppearance;
		if (effective_appearance == nil) {
			return WLF_THEME_APPEARANCE_LIGHT;
		}

		best_match = [effective_appearance
			bestMatchFromAppearancesWithNames:@[
				NSAppearanceNameAqua,
				NSAppearanceNameDarkAqua,
			]];
		if ([best_match isEqualToString:NSAppearanceNameDarkAqua]) {
			return WLF_THEME_APPEARANCE_DARK;
		}
	}

	return WLF_THEME_APPEARANCE_LIGHT;
}

static void macos_theme_fill_palette(struct wlf_color palette[WLF_THEME_COLOR_COUNT],
		enum wlf_theme_appearance appearance) {
	static const struct wlf_color light_palette[WLF_THEME_COLOR_COUNT] = {
		[WLF_THEME_COLOR_WINDOW] = {1.0, 1.0, 1.0, 1.0},
		[WLF_THEME_COLOR_WINDOW_TEXT] = {0.11, 0.11, 0.12, 1.0},
		[WLF_THEME_COLOR_BASE] = {0.97, 0.97, 0.98, 1.0},
		[WLF_THEME_COLOR_ALTERNATE_BASE] = {0.92, 0.93, 0.95, 1.0},
		[WLF_THEME_COLOR_TEXT] = {0.11, 0.11, 0.12, 1.0},
		[WLF_THEME_COLOR_BUTTON] = {0.95, 0.95, 0.96, 1.0},
		[WLF_THEME_COLOR_BUTTON_TEXT] = {0.11, 0.11, 0.12, 1.0},
		[WLF_THEME_COLOR_BORDER] = {0.80, 0.80, 0.82, 1.0},
		[WLF_THEME_COLOR_SEPARATOR] = {0.84, 0.84, 0.86, 1.0},
		[WLF_THEME_COLOR_PLACEHOLDER_TEXT] = {0.45, 0.45, 0.48, 1.0},
		[WLF_THEME_COLOR_ACCENT] = {0.00, 0.48, 1.0, 1.0},
		[WLF_THEME_COLOR_HIGHLIGHT] = {0.00, 0.48, 1.0, 1.0},
		[WLF_THEME_COLOR_HIGHLIGHTED_TEXT] = {1.0, 1.0, 1.0, 1.0},
		[WLF_THEME_COLOR_LINK] = {0.00, 0.40, 0.87, 1.0},
		[WLF_THEME_COLOR_VISITED_LINK] = {0.44, 0.27, 0.68, 1.0},
		[WLF_THEME_COLOR_MARK] = {1.0, 0.93, 0.60, 1.0},
		[WLF_THEME_COLOR_SUCCESS] = {0.18, 0.69, 0.31, 1.0},
		[WLF_THEME_COLOR_WARNING] = {1.0, 0.62, 0.04, 1.0},
		[WLF_THEME_COLOR_ERROR] = {1.0, 0.23, 0.19, 1.0},
	};
	static const struct wlf_color dark_palette[WLF_THEME_COLOR_COUNT] = {
		[WLF_THEME_COLOR_WINDOW] = {0.11, 0.11, 0.12, 1.0},
		[WLF_THEME_COLOR_WINDOW_TEXT] = {0.93, 0.93, 0.94, 1.0},
		[WLF_THEME_COLOR_BASE] = {0.16, 0.16, 0.17, 1.0},
		[WLF_THEME_COLOR_ALTERNATE_BASE] = {0.20, 0.20, 0.22, 1.0},
		[WLF_THEME_COLOR_TEXT] = {0.93, 0.93, 0.94, 1.0},
		[WLF_THEME_COLOR_BUTTON] = {0.19, 0.19, 0.20, 1.0},
		[WLF_THEME_COLOR_BUTTON_TEXT] = {0.93, 0.93, 0.94, 1.0},
		[WLF_THEME_COLOR_BORDER] = {0.31, 0.31, 0.34, 1.0},
		[WLF_THEME_COLOR_SEPARATOR] = {0.27, 0.27, 0.29, 1.0},
		[WLF_THEME_COLOR_PLACEHOLDER_TEXT] = {0.58, 0.58, 0.61, 1.0},
		[WLF_THEME_COLOR_ACCENT] = {0.04, 0.52, 1.0, 1.0},
		[WLF_THEME_COLOR_HIGHLIGHT] = {0.04, 0.52, 1.0, 1.0},
		[WLF_THEME_COLOR_HIGHLIGHTED_TEXT] = {1.0, 1.0, 1.0, 1.0},
		[WLF_THEME_COLOR_LINK] = {0.35, 0.67, 1.0, 1.0},
		[WLF_THEME_COLOR_VISITED_LINK] = {0.67, 0.51, 0.96, 1.0},
		[WLF_THEME_COLOR_MARK] = {0.47, 0.38, 0.03, 1.0},
		[WLF_THEME_COLOR_SUCCESS] = {0.19, 0.82, 0.35, 1.0},
		[WLF_THEME_COLOR_WARNING] = {1.0, 0.74, 0.10, 1.0},
		[WLF_THEME_COLOR_ERROR] = {1.0, 0.41, 0.38, 1.0},
	};
	const struct wlf_color *source = appearance == WLF_THEME_APPEARANCE_DARK ?
		dark_palette : light_palette;

	memcpy(palette, source, sizeof(light_palette));
}

static void macos_theme_reload(struct wlf_macos_theme *theme) {
	enum wlf_theme_appearance appearance;

	if (theme == NULL) {
		return;
	}

	appearance = macos_theme_detect_appearance();
	if (appearance == theme->base.appearance) {
		return;
	}

	theme->base.appearance = appearance;
	macos_theme_fill_palette(theme->palette, appearance);
	wlf_signal_emit_mutable(&theme->base.events.theme_changed, &theme->base);
}

@implementation WLFMacOSThemeObserver {
	struct wlf_macos_theme *_theme;
}

- (instancetype)initWithTheme:(struct wlf_macos_theme *)theme {
	self = [super init];
	if (self != nil) {
		_theme = theme;
	}
	return self;
}

- (void)appearanceChanged:(NSNotification *)notification {
	(void)notification;

	if (_theme == NULL) {
		return;
	}

	macos_theme_reload(_theme);
}

@end

static struct wlf_color macos_theme_palette_color(struct wlf_theme *theme,
		enum wlf_theme_color_role role) {
	struct wlf_macos_theme *macos_theme = (struct wlf_macos_theme *)theme;

	if (role >= WLF_THEME_COLOR_COUNT) {
		return WLF_COLOR_TRANSPARENT;
	}

	return macos_theme->palette[role];
}

static void macos_theme_unregister_observer(struct wlf_macos_theme *theme) {
	if (theme == NULL || !theme->observer_registered) {
		return;
	}

	@autoreleasepool {
		id observer = (__bridge id)theme->observer;
		NSDistributedNotificationCenter *center =
			[NSDistributedNotificationCenter defaultCenter];

		[center removeObserver:observer
			name:@"AppleInterfaceThemeChangedNotification"
			object:nil];
		[observer release];
	}

	theme->observer = NULL;
	theme->observer_registered = false;
}

static void macos_theme_destroy(struct wlf_theme *theme) {
	struct wlf_macos_theme *macos_theme = (struct wlf_macos_theme *)theme;

	macos_theme_unregister_observer(macos_theme);
	free(theme);
}

static void macos_theme_register_observer(struct wlf_macos_theme *theme) {
	if (theme == NULL || theme->observer_registered) {
		return;
	}

	@autoreleasepool {
		WLFMacOSThemeObserver *observer =
			[[WLFMacOSThemeObserver alloc] initWithTheme:theme];
		NSDistributedNotificationCenter *center =
			[NSDistributedNotificationCenter defaultCenter];

		[center addObserver:observer
			selector:@selector(appearanceChanged:)
			name:@"AppleInterfaceThemeChangedNotification"
			object:nil
			suspensionBehavior:NSNotificationSuspensionBehaviorDeliverImmediately];

		theme->observer = (__bridge void *)observer;
		theme->observer_registered = true;
	}
}

static const struct wlf_theme_impl macos_theme_impl = {
	.name = "macos",
	.destroy = macos_theme_destroy,
	.theme_palette_color = macos_theme_palette_color,
};

struct wlf_macos_theme *wlf_macos_theme_create(void) {
	struct wlf_macos_theme *theme = calloc(1, sizeof(*theme));
	if (theme == NULL) {
		return NULL;
	}

	wlf_theme_init(&theme->base, &macos_theme_impl);
	theme->base.appearance = macos_theme_detect_appearance();
	macos_theme_fill_palette(theme->palette, theme->base.appearance);
	macos_theme_register_observer(theme);

	return theme;
}

bool wlf_theme_is_macos(const struct wlf_theme *theme) {
	return theme != NULL &&
		theme->impl != NULL &&
		theme->impl == &macos_theme_impl;
}

struct wlf_macos_theme *wlf_macos_theme_from_theme(struct wlf_theme *theme) {
	assert(theme && theme->impl == &macos_theme_impl);

	struct wlf_macos_theme *macos_theme =
		wlf_container_of(theme, macos_theme, base);

	return macos_theme;
}
