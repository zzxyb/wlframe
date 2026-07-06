#include "wlf/platform/linux/theme.h"
#include "wlf/utils/wlf_compat.h"
#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_log.h"

#include <gio/gio.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define WLF_PORTAL_BUS_NAME "org.freedesktop.portal.Desktop"
#define WLF_PORTAL_OBJECT_PATH "/org/freedesktop/portal/desktop"
#define WLF_PORTAL_SETTINGS_IFACE "org.freedesktop.portal.Settings"
#define WLF_PORTAL_APPEARANCE_NS "org.freedesktop.appearance"
#define WLF_PORTAL_COLOR_SCHEME_KEY "color-scheme"
#define WLF_PORTAL_ACCENT_COLOR_KEY "accent-color"
#define WLF_GNOME_INTERFACE_SCHEMA "org.gnome.desktop.interface"

static GDBusProxy *linux_theme_create_portal_proxy(void) {
	return g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES, NULL,
		WLF_PORTAL_BUS_NAME, WLF_PORTAL_OBJECT_PATH,
		WLF_PORTAL_SETTINGS_IFACE, NULL, NULL);
}

static GVariant *linux_theme_portal_read(GDBusProxy *proxy, const char *key) {
	GVariant *result;
	GVariant *value = NULL;
	GError *error = NULL;

	if (proxy == NULL || key == NULL) {
		return NULL;
	}

	result = g_dbus_proxy_call_sync(proxy, "Read",
		g_variant_new("(ss)", WLF_PORTAL_APPEARANCE_NS, key),
		G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
	if (result == NULL) {
		if (error != NULL) {
			wlf_log(WLF_DEBUG, "Portal settings read failed for %s: %s",
				key, error->message);
			g_error_free(error);
		}
		return NULL;
	}

	g_variant_get(result, "(v)", &value);
	g_variant_unref(result);

	return value;
}

static bool linux_theme_portal_available(void) {
	GDBusProxy *proxy = linux_theme_create_portal_proxy();
	GVariant *value;

	if (proxy == NULL) {
		return false;
	}

	value = linux_theme_portal_read(proxy, WLF_PORTAL_COLOR_SCHEME_KEY);
	g_object_unref(proxy);
	if (value == NULL) {
		return false;
	}

	g_variant_unref(value);
	return true;
}

static bool linux_theme_portal_appearance(
		enum wlf_theme_appearance *appearance) {
	GDBusProxy *proxy;
	GVariant *value;
	guint32 color_scheme;
	bool detected = false;

	if (appearance == NULL) {
		return false;
	}

	proxy = linux_theme_create_portal_proxy();
	value = linux_theme_portal_read(proxy, WLF_PORTAL_COLOR_SCHEME_KEY);
	if (proxy != NULL) {
		g_object_unref(proxy);
	}
	if (value == NULL) {
		return false;
	}

	if (g_variant_is_of_type(value, G_VARIANT_TYPE_UINT32)) {
		color_scheme = g_variant_get_uint32(value);
		if (color_scheme == 1) {
			*appearance = WLF_THEME_APPEARANCE_DARK;
			detected = true;
		} else if (color_scheme == 0 || color_scheme == 2) {
			*appearance = WLF_THEME_APPEARANCE_LIGHT;
			detected = true;
		}
	}
	g_variant_unref(value);

	return detected;
}

static bool linux_theme_portal_accent(struct wlf_color *accent) {
	GDBusProxy *proxy;
	GVariant *value;
	double r;
	double g;
	double b;
	bool detected = false;

	if (accent == NULL) {
		return false;
	}

	proxy = linux_theme_create_portal_proxy();
	value = linux_theme_portal_read(proxy, WLF_PORTAL_ACCENT_COLOR_KEY);
	if (proxy != NULL) {
		g_object_unref(proxy);
	}
	if (value == NULL) {
		return false;
	}

	if (g_variant_is_of_type(value, G_VARIANT_TYPE("(ddd)"))) {
		g_variant_get(value, "(ddd)", &r, &g, &b);
		*accent = (struct wlf_color){r, g, b, 1.0};
		detected = true;
	}
	g_variant_unref(value);

	return detected;
}

static GSettings *linux_theme_create_interface_settings(void) {
	GSettingsSchemaSource *source = g_settings_schema_source_get_default();
	GSettingsSchema *schema;
	GSettings *settings;

	if (source == NULL) {
		return NULL;
	}

	schema = g_settings_schema_source_lookup(source,
		WLF_GNOME_INTERFACE_SCHEMA, TRUE);
	if (schema == NULL) {
		return NULL;
	}

	settings = g_settings_new(WLF_GNOME_INTERFACE_SCHEMA);
	g_settings_schema_unref(schema);

	return settings;
}

static bool linux_theme_interface_has_key(const char *key) {
	GSettingsSchemaSource *source = g_settings_schema_source_get_default();
	GSettingsSchema *schema;
	bool has_key;

	if (source == NULL || key == NULL) {
		return false;
	}

	schema = g_settings_schema_source_lookup(source,
		WLF_GNOME_INTERFACE_SCHEMA, TRUE);
	if (schema == NULL) {
		return false;
	}

	has_key = g_settings_schema_has_key(schema, key);
	g_settings_schema_unref(schema);

	return has_key;
}

static bool linux_theme_gsettings_appearance(
		enum wlf_theme_appearance *appearance) {
	GSettings *settings = linux_theme_create_interface_settings();
	char *color_scheme;
	bool detected = false;

	if (settings == NULL || appearance == NULL ||
			!linux_theme_interface_has_key("color-scheme")) {
		if (settings != NULL) {
			g_object_unref(settings);
		}
		return false;
	}

	color_scheme = g_settings_get_string(settings, "color-scheme");
	if (color_scheme != NULL) {
		if (strcmp(color_scheme, "prefer-dark") == 0) {
			*appearance = WLF_THEME_APPEARANCE_DARK;
			detected = true;
		} else if (strcmp(color_scheme, "prefer-light") == 0 ||
				strcmp(color_scheme, "default") == 0) {
			*appearance = WLF_THEME_APPEARANCE_LIGHT;
			detected = true;
		}
		g_free(color_scheme);
	}

	g_object_unref(settings);

	return detected;
}

static bool linux_theme_gsettings_accent(struct wlf_color *accent) {
	static const struct {
		const char *name;
		struct wlf_color color;
	} colors[] = {
		{ "blue", {0.21, 0.52, 0.89, 1.0} },
		{ "teal", {0.13, 0.56, 0.64, 1.0} },
		{ "green", {0.23, 0.58, 0.29, 1.0} },
		{ "yellow", {0.78, 0.53, 0.00, 1.0} },
		{ "orange", {0.93, 0.36, 0.00, 1.0} },
		{ "red", {0.88, 0.20, 0.14, 1.0} },
		{ "pink", {0.83, 0.34, 0.60, 1.0} },
		{ "purple", {0.57, 0.25, 0.67, 1.0} },
		{ "slate", {0.43, 0.51, 0.59, 1.0} },
	};
	GSettings *settings = linux_theme_create_interface_settings();
	char *accent_name;
	bool detected = false;

	if (settings == NULL || accent == NULL ||
			!linux_theme_interface_has_key("accent-color")) {
		if (settings != NULL) {
			g_object_unref(settings);
		}
		return false;
	}

	accent_name = g_settings_get_string(settings, "accent-color");
	if (accent_name != NULL) {
		for (size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
			if (strcmp(accent_name, colors[i].name) == 0) {
				*accent = colors[i].color;
				detected = true;
				break;
			}
		}
		g_free(accent_name);
	}

	g_object_unref(settings);

	return detected;
}

static enum wlf_theme_appearance linux_theme_detect_appearance(
		const struct wlf_linux_theme *theme) {
	enum wlf_theme_appearance appearance;
	if (theme != NULL && theme->use_portal &&
			linux_theme_portal_appearance(&appearance)) {
		return appearance;
	}
	if (linux_theme_gsettings_appearance(&appearance)) {
		return appearance;
	}

	return WLF_THEME_APPEARANCE_LIGHT;
}

static struct wlf_color linux_theme_default_accent(
		enum wlf_theme_appearance appearance) {
	if (appearance == WLF_THEME_APPEARANCE_DARK) {
		return (struct wlf_color){0.35, 0.67, 1.0, 1.0};
	}

	return (struct wlf_color){0.00, 0.40, 0.87, 1.0};
}

static void linux_theme_fill_palette(struct wlf_linux_theme *theme,
		struct wlf_color palette[WLF_THEME_COLOR_COUNT],
		enum wlf_theme_appearance appearance) {
	palette[WLF_THEME_COLOR_HIGHLIGHT] = linux_theme_default_accent(appearance);

	if (theme != NULL && theme->use_portal &&
			linux_theme_portal_accent(&palette[WLF_THEME_COLOR_HIGHLIGHT])) {
		return;
	}

	(void)linux_theme_gsettings_accent(&palette[WLF_THEME_COLOR_HIGHLIGHT]);
}

void wlf_linux_theme_reload(struct wlf_linux_theme *theme) {
	enum wlf_theme_appearance appearance;
	struct wlf_color palette[WLF_THEME_COLOR_COUNT];
	bool appearance_changed;
	bool highlight_changed;

	if (theme == NULL) {
		return;
	}

	appearance = linux_theme_detect_appearance(theme);
	linux_theme_fill_palette(theme, palette, appearance);
	if (appearance == theme->base.appearance &&
			memcmp(theme->base.palette, palette, sizeof(palette)) == 0) {
		return;
	}

	appearance_changed = appearance != theme->base.appearance;
	highlight_changed = memcmp(
		&theme->base.palette[WLF_THEME_COLOR_HIGHLIGHT],
		&palette[WLF_THEME_COLOR_HIGHLIGHT],
		sizeof(struct wlf_color)) != 0;

	theme->base.appearance = appearance;
	memcpy(theme->base.palette, palette, sizeof(palette));

	if (appearance_changed) {
		wlf_signal_emit_mutable(&theme->base.events.theme_changed,
			&theme->base);
	}
	if (highlight_changed) {
		wlf_signal_emit_mutable(&theme->base.events.highlight_changed,
			&theme->base);
	}
}

static void linux_theme_portal_signal(GDBusProxy *proxy,
		char *sender_name, char *signal_name, GVariant *parameters,
		gpointer data) {
	struct wlf_linux_theme *theme = data;
	const char *namespace;
	const char *key;
	GVariant *value;

	(void)proxy;
	(void)sender_name;

	if (strcmp(signal_name, "SettingChanged") != 0) {
		return;
	}

	g_variant_get(parameters, "(&s&sv)", &namespace, &key, &value);
	if (strcmp(namespace, WLF_PORTAL_APPEARANCE_NS) == 0 &&
			(strcmp(key, WLF_PORTAL_COLOR_SCHEME_KEY) == 0 ||
				strcmp(key, WLF_PORTAL_ACCENT_COLOR_KEY) == 0)) {
		wlf_linux_theme_reload(theme);
	}
	g_variant_unref(value);
}

static gpointer linux_theme_monitor_thread(gpointer data) {
	struct wlf_linux_theme *theme = data;

	if (theme->monitor_context == NULL || theme->monitor_loop == NULL) {
		return NULL;
	}

	g_main_context_push_thread_default(theme->monitor_context);
	if (theme->use_portal) {
		theme->portal_settings = linux_theme_create_portal_proxy();
		if (theme->portal_settings != NULL) {
			theme->change_handler_id = g_signal_connect(
				theme->portal_settings, "g-signal",
				G_CALLBACK(linux_theme_portal_signal), theme);
			g_main_loop_run(theme->monitor_loop);
			if (theme->change_handler_id != 0) {
				g_signal_handler_disconnect(theme->portal_settings,
					theme->change_handler_id);
				theme->change_handler_id = 0;
			}
			g_object_unref(theme->portal_settings);
			theme->portal_settings = NULL;
		}
	}
	g_main_context_pop_thread_default(theme->monitor_context);

	return NULL;
}

static gboolean linux_theme_quit_monitor_loop(gpointer data) {
	GMainLoop *loop = data;

	g_main_loop_quit(loop);

	return G_SOURCE_REMOVE;
}

static void linux_theme_destroy(struct wlf_theme *theme) {
	struct wlf_linux_theme *linux_theme = wlf_linux_theme_from_theme(theme);

	if (linux_theme->monitor_thread != NULL) {
		if (linux_theme->monitor_context != NULL &&
				linux_theme->monitor_loop != NULL) {
			g_main_context_invoke(linux_theme->monitor_context,
				linux_theme_quit_monitor_loop,
				linux_theme->monitor_loop);
		}
		g_thread_join(linux_theme->monitor_thread);
		linux_theme->monitor_thread = NULL;
	}
	if (linux_theme->monitor_loop != NULL) {
		g_main_loop_unref(linux_theme->monitor_loop);
		linux_theme->monitor_loop = NULL;
	}
	if (linux_theme->monitor_context != NULL) {
		g_main_context_unref(linux_theme->monitor_context);
		linux_theme->monitor_context = NULL;
	}

	free(linux_theme);
}

static const struct wlf_theme_impl linux_theme_impl = {
	.name = "linux",
	.destroy = linux_theme_destroy,
};

struct wlf_linux_theme *wlf_linux_theme_create(void) {
	struct wlf_linux_theme *theme = calloc(1, sizeof(*theme));

	if (theme == NULL) {
		return NULL;
	}

	wlf_theme_init(&theme->base, &linux_theme_impl);
	theme->use_portal = linux_theme_portal_available();
	theme->base.appearance = linux_theme_detect_appearance(theme);
	linux_theme_fill_palette(theme, theme->base.palette, theme->base.appearance);
	if (theme->use_portal) {
		theme->monitor_context = g_main_context_new();
		theme->monitor_loop = g_main_loop_new(theme->monitor_context, FALSE);
		theme->monitor_thread = g_thread_new("wlf-linux-theme-monitor",
			linux_theme_monitor_thread, theme);
	}

	return theme;
}

bool wlf_theme_is_linux(const struct wlf_theme *theme) {
	return theme != NULL && theme->impl == &linux_theme_impl;
}

struct wlf_linux_theme *wlf_linux_theme_from_theme(struct wlf_theme *theme) {
	assert(theme && theme->impl == &linux_theme_impl);

	struct wlf_linux_theme *linux_theme =
		wlf_container_of(theme, linux_theme, base);

	return linux_theme;
}
