#include "wlf/macos/wlf_macos_output.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_linked_list.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Forward declarations */
static const struct wlf_output_impl macos_output_impl;
static const struct wlf_output_manager_impl macos_output_manager_impl;

/* -------------------------------------------------------------------------
 * macOS output manager
 * ---------------------------------------------------------------------- */

/**
 * @brief macOS-specific output manager.
 *
 * Holds the notification observer for screen-configuration changes so it can
 * be unregistered on destroy.
 */
struct wlf_macos_output_manager {
	struct wlf_output_manager base;  /**< Generic output manager base (must be first). */
	void *observer;                  /**< NSNotificationCenter observer token (opaque). */
};

/* -------------------------------------------------------------------------
 * Helper: populate wlf_output fields from NSScreen
 * ---------------------------------------------------------------------- */

static void fill_output_from_screen(struct wlf_macos_output *output,
		NSScreen *screen) {
	/* Geometry — main screen has origin (0,0); others relative to it */
	NSRect frame = screen.frame;
	output->base.geometry.x      = (int)frame.origin.x;
	output->base.geometry.y      = (int)frame.origin.y;
	output->base.geometry.width  = (int)frame.size.width;
	output->base.geometry.height = (int)frame.size.height;

	/* Scale factor — macOS uses backing scale for HiDPI */
	output->base.scale = (int)screen.backingScaleFactor;

	/* Physical size — derived from backing pixels and a nominal 96 DPI.
	 * NSScreen does not expose physical mm directly; use deviceDescription
	 * for resolution in DPI to compute mm. */
	NSDictionary *desc = screen.deviceDescription;
	NSValue *resValue = desc[NSDeviceResolution];
	double dpi_x = 96.0;
	double dpi_y = 96.0;
	if (resValue != nil) {
		NSSize res = [resValue sizeValue];
		if (res.width  > 0) dpi_x = res.width;
		if (res.height > 0) dpi_y = res.height;
	}
	/* Backing pixel size */
	double scale = screen.backingScaleFactor;
	double backing_w = frame.size.width  * scale;
	double backing_h = frame.size.height * scale;
	output->base.physical_size.width  = (int)round(backing_w / dpi_x * 25.4);
	output->base.physical_size.height = (int)round(backing_h / dpi_y * 25.4);

	/* Refresh rate */
	CGDirectDisplayID display_id = 0;
	NSNumber *display_id_num = desc[@"NSScreenNumber"];
	if (display_id_num != nil) {
		display_id = (CGDirectDisplayID)[display_id_num unsignedIntValue];
	}
	int refresh = 60;
	if (display_id != 0) {
		CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display_id);
		if (mode != NULL) {
			double hz = CGDisplayModeGetRefreshRate(mode);
			if (hz > 0) refresh = (int)(hz * 1000 + 0.5);
			CGDisplayModeRelease(mode);
		}
	}
	output->base.refresh_rate = refresh;

	/* Name — use NSScreen.localizedName when available (macOS 10.15+) */
	const char *name = NULL;
	if (@available(macOS 10.15, *)) {
		name = screen.localizedName.UTF8String;
	}
	if (name == NULL && display_id_num != nil) {
		/* Fallback: build a generic name from display ID */
		char buf[32];
		snprintf(buf, sizeof(buf), "display%u", (unsigned)display_id);
		name = buf;
	}
	if (name == NULL) name = "display";

	free(output->base.name);
	output->base.name = strdup(name);

	/* Subpixel — macOS does not expose this; report UNKNOWN */
	output->base.subpixel = WLF_OUTPUT_SUBPIXEL_UNKNOWN;

	/* Transform — always normal on macOS (rotation is handled by the OS) */
	output->base.transform = WLF_OUTPUT_TRANSFORM_NORMAL;
}

/* -------------------------------------------------------------------------
 * wlf_output_impl
 * ---------------------------------------------------------------------- */

static void output_destroy(struct wlf_output *wlf_out) {
	struct wlf_macos_output *output =
		wlf_macos_output_from_output(wlf_out);
	if (output == NULL) {
		return;
	}

	if (output->ns_screen != NULL) {
		NSScreen *screen = (__bridge NSScreen *)output->ns_screen;
		[screen release];
		output->ns_screen = NULL;
	}

	free(output->base.name);
	free(output->base.model);
	free(output->base.manufacturer);
	free(output->base.description);
	output->base.name = NULL;
	output->base.model = NULL;
	output->base.manufacturer = NULL;
	output->base.description = NULL;

	wlf_linked_list_remove(&output->base.link);
	free(output);
}

static const struct wlf_output_impl macos_output_impl = {
	.type    = WLF_OUTPUT,
	.destroy = output_destroy,
};

/* -------------------------------------------------------------------------
 * Internal: create one wlf_macos_output from an NSScreen
 * ---------------------------------------------------------------------- */

static struct wlf_macos_output *macos_output_create(
		struct wlf_macos_output_manager *manager, NSScreen *screen) {
	struct wlf_macos_output *output = calloc(1, sizeof(*output));
	if (output == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_macos_output");
		return NULL;
	}

	output->manager   = manager;
	output->ns_screen = (__bridge void *)[screen retain];

	wlf_output_init(&output->base, &macos_output_impl);
	fill_output_from_screen(output, screen);

	wlf_log(WLF_DEBUG, "Created macOS output '%s' %dx%d @%dx",
		output->base.name ? output->base.name : "?",
		output->base.geometry.width, output->base.geometry.height,
		output->base.scale);

	return output;
}

/* -------------------------------------------------------------------------
 * Screen-change notification handler
 * ---------------------------------------------------------------------- */

/* Forward declaration for the notification helper */
static void handle_screen_change(struct wlf_macos_output_manager *manager);

static void setup_screen_change_observer(struct wlf_macos_output_manager *manager) {
	id token = [[NSNotificationCenter defaultCenter]
		addObserverForName:NSApplicationDidChangeScreenParametersNotification
		            object:nil
		             queue:[NSOperationQueue mainQueue]
		        usingBlock:^(NSNotification *note __attribute__((unused))) {
			handle_screen_change(manager);
		}];
	[token retain];
	manager->observer = (__bridge void *)token;
}

static void handle_screen_change(struct wlf_macos_output_manager *manager) {
	@autoreleasepool {
		NSArray<NSScreen *> *current_screens = [NSScreen screens];

		/* Remove outputs whose NSScreen no longer exists */
		struct wlf_output *out;
		struct wlf_output *tmp;
		wlf_linked_list_for_each_safe(out, tmp, &manager->base.outputs, link) {
			struct wlf_macos_output *macos_out =
				wlf_macos_output_from_output(out);
			if (macos_out == NULL) continue;

			NSScreen *screen = (__bridge NSScreen *)macos_out->ns_screen;
			bool found = false;
			for (NSScreen *s in current_screens) {
				if (s == screen) { found = true; break; }
			}
			if (!found) {
				wlf_signal_emit_mutable(&manager->base.events.output_removed, out);
				wlf_output_destroy(out);
			}
		}

		/* Add newly attached screens */
		for (NSScreen *screen in current_screens) {
			bool already = false;
			wlf_linked_list_for_each(out, &manager->base.outputs, link) {
				struct wlf_macos_output *mo =
					wlf_macos_output_from_output(out);
				if (mo && (__bridge NSScreen *)mo->ns_screen == screen) {
					already = true;
					break;
				}
			}
			if (!already) {
				struct wlf_macos_output *new_out =
					macos_output_create(manager, screen);
				if (new_out != NULL) {
					wlf_linked_list_insert(&manager->base.outputs,
						&new_out->base.link);
					wlf_signal_emit_mutable(&manager->base.events.output_added,
						&new_out->base);
				}
			}
		}
	}
}

/* -------------------------------------------------------------------------
 * wlf_output_manager_impl
 * ---------------------------------------------------------------------- */

static void output_manager_destroy(struct wlf_output_manager *wlf_mgr) {
	struct wlf_macos_output_manager *manager =
		(struct wlf_macos_output_manager *)wlf_mgr;

	/* Remove notification observer */
	if (manager->observer != NULL) {
		id token = (__bridge id)manager->observer;
		[[NSNotificationCenter defaultCenter] removeObserver:token];
		[token release];
		manager->observer = NULL;
	}

	/* Destroy all registered outputs */
	struct wlf_output *out;
	struct wlf_output *tmp;
	wlf_linked_list_for_each_safe(out, tmp, &manager->base.outputs, link) {
		wlf_output_destroy(out);
	}

	free(manager);
}

static const struct wlf_output_manager_impl macos_output_manager_impl = {
	.destroy = output_manager_destroy,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_output_manager *wlf_macos_output_manager_create(void) {
	@autoreleasepool {
		struct wlf_macos_output_manager *manager =
			calloc(1, sizeof(*manager));
		if (manager == NULL) {
			wlf_log_errno(WLF_ERROR,
				"Failed to allocate wlf_macos_output_manager");
			return NULL;
		}

		wlf_output_manager_init(&manager->base, &macos_output_manager_impl);

		/* Enumerate current screens */
		for (NSScreen *screen in [NSScreen screens]) {
			struct wlf_macos_output *output =
				macos_output_create(manager, screen);
			if (output == NULL) {
				output_manager_destroy(&manager->base);
				return NULL;
			}
			wlf_linked_list_insert(&manager->base.outputs, &output->base.link);
		}

		/* Listen for future screen changes */
		setup_screen_change_observer(manager);

		wlf_log(WLF_DEBUG, "Created macOS output manager");
		return &manager->base;
	}
}

bool wlf_output_is_macos(const struct wlf_output *output) {
	return output && output->impl == &macos_output_impl;
}

struct wlf_macos_output *wlf_macos_output_from_output(
		struct wlf_output *output) {
	if (!wlf_output_is_macos(output)) {
		return NULL;
	}
	return (struct wlf_macos_output *)output;
}
