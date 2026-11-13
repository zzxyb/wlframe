#include "wlf/platform/x11/xresource.h"
#include "wlf/utils/wlf_log.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xcb/xcb_xrm.h>

static bool trailing_space(const char *text) {
	while (isspace((unsigned char)*text)) {
		text++;
	}
	return *text == '\0';
}

static char *resource_string(xcb_xrm_database_t *db,
		const char *name, const char *class_name) {
	char *value = NULL;
	if (xcb_xrm_resource_get_string(db, name, class_name, &value) < 0) {
		return NULL;
	}
	/* Xrm preserves trailing whitespace; typed values should tolerate it. */
	size_t length = strlen(value);
	while (length > 0 && isspace((unsigned char)value[length - 1])) {
		value[--length] = '\0';
	}
	return value;
}

static bool parse_bool(const char *value, bool *out) {
	if (strcasecmp(value, "true") == 0 || strcasecmp(value, "yes") == 0 ||
			strcasecmp(value, "on") == 0 || strcmp(value, "1") == 0) {
		*out = true;
		return true;
	}
	if (strcasecmp(value, "false") == 0 || strcasecmp(value, "no") == 0 ||
			strcasecmp(value, "off") == 0 || strcmp(value, "0") == 0) {
		*out = false;
		return true;
	}
	return false;
}

static bool parse_size(const char *value, int *out) {
	char *end;
	errno = 0;
	long size = strtol(value, &end, 10);
	if (end == value || !trailing_space(end) || errno != 0 || size < 1 || size > 1024) {
		return false;
	}
	*out = (int)size;
	return true;
}

static void invalid_resource(const char *name) {
	wlf_log(WLF_INFO, "Ignoring invalid X resource %s", name);
}

static void load_boolean(xcb_xrm_database_t *db, const char *name,
		const char *class_name, bool *out, uint32_t *present, uint32_t field) {
	char *value = resource_string(db, name, class_name);
	if (value == NULL) {
		return;
	}
	if (parse_bool(value, out)) {
		if (present != NULL) {
			*present |= field;
		}
	} else {
		invalid_resource(name);
	}
	free(value);
}

static void load_enum(xcb_xrm_database_t *db, const char *name,
		const char *class_name, const char *const *names, size_t count,
		int *out, uint32_t *present, uint32_t field) {
	char *value = resource_string(db, name, class_name);
	if (value == NULL) {
		return;
	}
	for (size_t i = 0; i < count; i++) {
		if (strcasecmp(value, names[i]) == 0) {
			*out = (int)i;
			*present |= field;
			free(value);
			return;
		}
	}
	invalid_resource(name);
	free(value);
}

static void load_settings(xcb_xrm_database_t *db,
		struct wlf_xft_settings *xft, struct wlf_xcursor_settings *cursor) {
	*xft = (struct wlf_xft_settings){
		.dpi = 96, .antialias = true, .hinting = true,
		.hintstyle = WLF_XFT_HINT_FULL, .rgba = WLF_XFT_RGBA_NONE,
		.lcdfilter = WLF_XFT_LCD_DEFAULT,
	};
	*cursor = (struct wlf_xcursor_settings){ .size = 24, .theme_core = true };
	char *value = resource_string(db, "Xft.dpi", "Xft.Dpi");
	if (value != NULL) {
		char *end;
		errno = 0;
		double dpi = strtod(value, &end);
		if (end != value && trailing_space(end) && errno == 0 &&
				isfinite(dpi) && dpi > 0 && dpi <= 1000) {
			xft->dpi = dpi;
			xft->present |= WLF_XFT_DPI;
		} else {
			invalid_resource("Xft.dpi");
		}
		free(value);
	}
	load_boolean(db, "Xft.antialias", "Xft.Antialias", &xft->antialias,
		&xft->present, WLF_XFT_ANTIALIAS);
	load_boolean(db, "Xft.hinting", "Xft.Hinting", &xft->hinting,
		&xft->present, WLF_XFT_HINTING);
	static const char *const hints[] = { "hintnone", "hintslight", "hintmedium", "hintfull" };
	static const char *const rgba[] = { "none", "rgb", "bgr", "vrgb", "vbgr" };
	static const char *const filters[] = { "lcdnone", "lcddefault", "lcdlight", "lcdlegacy" };
	int result = xft->hintstyle;
	load_enum(db, "Xft.hintstyle", "Xft.Hintstyle", hints, 4,
		&result, &xft->present, WLF_XFT_HINTSTYLE);
	xft->hintstyle = result;
	result = xft->rgba;
	load_enum(db, "Xft.rgba", "Xft.Rgba", rgba, 5,
		&result, &xft->present, WLF_XFT_RGBA);
	xft->rgba = result;
	result = xft->lcdfilter;
	load_enum(db, "Xft.lcdfilter", "Xft.Lcdfilter", filters, 4,
		&result, &xft->present, WLF_XFT_LCDFILTER);
	xft->lcdfilter = result;
	cursor->theme = resource_string(db, "Xcursor.theme", "Xcursor.Theme");
	cursor->size = (int)lround(24 * xft->dpi / 96);
	if (cursor->size < 1) {
		cursor->size = 1;
	}
	value = resource_string(db, "Xcursor.size", "Xcursor.Size");
	if (value != NULL) {
		if (!parse_size(value, &cursor->size)) {
			invalid_resource("Xcursor.size");
		}
		free(value);
	}
	load_boolean(db, "Xcursor.theme_core", "Xcursor.ThemeCore",
		&cursor->theme_core, NULL, 0);
}

bool wlf_xresource_manager_reload(struct wlf_xresource_manager *manager) {
	if (manager == NULL || xcb_connection_has_error(manager->xcb)) {
		return false;
	}
	xcb_xrm_database_t *db = xcb_xrm_database_from_default(manager->xcb);
	if (db == NULL) {
		/* A valid empty database is different from a failed allocation. */
		db = xcb_xrm_database_from_string("");
	}
	if (db == NULL || xcb_connection_has_error(manager->xcb)) {
		xcb_xrm_database_free(db);
		return false;
	}
	struct wlf_xft_settings xft;
	struct wlf_xcursor_settings cursor;
	load_settings(db, &xft, &cursor);
	const char *theme = getenv("XCURSOR_THEME");
	if (theme != NULL && *theme != '\0') {
		char *copy = strdup(theme);
		if (copy == NULL) {
			free(cursor.theme);
			xcb_xrm_database_free(db);
			return false;
		}
		free(cursor.theme);
		cursor.theme = copy;
	}
	const char *size = getenv("XCURSOR_SIZE");
	if (size != NULL && !parse_size(size, &cursor.size)) {
		invalid_resource("XCURSOR_SIZE");
	}
	xcb_xrm_database_free(manager->database);
	free(manager->xcursor.theme);
	manager->database = db;
	manager->xft = xft;
	manager->xcursor = cursor;
	wlf_signal_emit_mutable(&manager->events.changed, manager);
	return true;
}

struct wlf_xresource_manager *wlf_xresource_manager_create(xcb_connection_t *xcb) {
	if (xcb == NULL || xcb_connection_has_error(xcb)) {
		return NULL;
	}
	xcb_screen_iterator_t screen = xcb_setup_roots_iterator(xcb_get_setup(xcb));
	if (!screen.rem) {
		return NULL;
	}
	struct wlf_xresource_manager *manager = calloc(1, sizeof(*manager));
	if (manager == NULL) {
		return NULL;
	}
	manager->xcb = xcb;
	manager->root = screen.data->root;
	wlf_signal_init(&manager->events.changed);
	xcb_get_window_attributes_reply_t *attrs = xcb_get_window_attributes_reply(xcb,
		xcb_get_window_attributes(xcb, manager->root), NULL);
	if (attrs == NULL) {
		free(manager);
		return NULL;
	}
	uint32_t mask = attrs->your_event_mask | XCB_EVENT_MASK_PROPERTY_CHANGE;
	free(attrs);
	xcb_generic_error_t *error = xcb_request_check(xcb,
		xcb_change_window_attributes_checked(xcb, manager->root, XCB_CW_EVENT_MASK, &mask));
	if (error != NULL) {
		free(error);
		free(manager);
		return NULL;
	}
	if (!wlf_xresource_manager_reload(manager)) {
		wlf_xresource_manager_destroy(manager);
		return NULL;
	}
	return manager;
}

void wlf_xresource_manager_destroy(struct wlf_xresource_manager *manager) {
	if (manager == NULL) {
		return;
	}
	assert(wlf_linked_list_empty(&manager->events.changed.listener_list));
	xcb_xrm_database_free(manager->database);
	free(manager->xcursor.theme);
	free(manager);
}

bool wlf_xresource_manager_handle_event(struct wlf_xresource_manager *manager,
		const xcb_generic_event_t *event) {
	if (manager == NULL || event == NULL || (event->response_type & 0x7f) != XCB_PROPERTY_NOTIFY) {
		return false;
	}
	const xcb_property_notify_event_t *property = (const void *)event;
	if (property->window != manager->root || property->atom != XCB_ATOM_RESOURCE_MANAGER) {
		return false;
	}
	if (!wlf_xresource_manager_reload(manager)) {
		wlf_log(WLF_ERROR, "Failed to reload X resources; retaining previous settings");
	}
	return true;
}

bool wlf_xresource_manager_get_string(struct wlf_xresource_manager *manager,
		const char *name, const char *class_name, char **value) {
	if (value == NULL) {
		return false;
	}
	*value = NULL;
	return manager != NULL && name != NULL &&
		xcb_xrm_resource_get_string(manager->database, name, class_name, value) == 0;
}
