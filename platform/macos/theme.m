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

static void macos_theme_fill_palette(
	struct wlf_color palette[WLF_THEME_COLOR_COUNT]);

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

static struct wlf_color macos_theme_color_from_nscolor_current(NSColor *color,
		struct wlf_color fallback) {
	if (color == nil) {
		return fallback;
	}

	@autoreleasepool {
		NSColor *resolved = [color colorUsingColorSpace:NSColorSpace.sRGBColorSpace];

		if (resolved == nil) {
			return fallback;
		}

		return (struct wlf_color){
			.r = resolved.redComponent,
			.g = resolved.greenComponent,
			.b = resolved.blueComponent,
			.a = resolved.alphaComponent,
		};
	}
}

static void macos_theme_apply_system_colors(
		struct wlf_color palette[WLF_THEME_COLOR_COUNT]) {
	@autoreleasepool {
		[NSApplication sharedApplication];

		palette[WLF_THEME_COLOR_HIGHLIGHT] =
			macos_theme_color_from_nscolor_current(
				NSColor.selectedContentBackgroundColor,
				palette[WLF_THEME_COLOR_HIGHLIGHT]);
	}
}

static bool macos_theme_highlight_changed(
		const struct wlf_color old_palette[WLF_THEME_COLOR_COUNT],
		const struct wlf_color new_palette[WLF_THEME_COLOR_COUNT]) {
	return memcmp(&old_palette[WLF_THEME_COLOR_HIGHLIGHT],
			&new_palette[WLF_THEME_COLOR_HIGHLIGHT],
			sizeof(struct wlf_color)) != 0;
}

void wlf_macos_theme_reload(struct wlf_macos_theme *theme) {
	enum wlf_theme_appearance appearance;
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];
	bool appearance_changed;
	bool highlight_changed;

	if (theme == NULL) {
		return;
	}

	appearance = macos_theme_detect_appearance();
	macos_theme_fill_palette(palette);
	if (appearance == theme->base.appearance &&
			memcmp(theme->palette, palette, sizeof(palette)) == 0) {
		return;
	}

	appearance_changed = appearance != theme->base.appearance;
	highlight_changed = macos_theme_highlight_changed(theme->palette, palette);
	theme->base.appearance = appearance;
	memcpy(theme->palette, palette, sizeof(palette));
	if (appearance_changed) {
		wlf_signal_emit_mutable(&theme->base.events.theme_changed,
			&theme->base);
	}
	if (highlight_changed) {
		wlf_signal_emit_mutable(&theme->base.events.highlight_changed,
			&theme->base);
	}
}

static void macos_theme_fill_palette(
		struct wlf_color palette[WLF_THEME_COLOR_COUNT]) {
	static const struct wlf_color fallback_palette[WLF_THEME_COLOR_COUNT] = {
		[WLF_THEME_COLOR_HIGHLIGHT] = {0.00, 0.35, 0.82, 1.0},
	};

	memcpy(palette, fallback_palette, sizeof(fallback_palette));
	macos_theme_apply_system_colors(palette);
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

	wlf_macos_theme_reload(_theme);
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
		[[NSNotificationCenter defaultCenter] removeObserver:observer
			name:NSSystemColorsDidChangeNotification
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
		[[NSNotificationCenter defaultCenter] addObserver:observer
			selector:@selector(appearanceChanged:)
			name:NSSystemColorsDidChangeNotification
			object:nil];

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
	macos_theme_fill_palette(theme->palette);
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
