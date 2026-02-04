#include "wlf/window/macos/window.h"
#include "wlf/utils/wlf_log.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

#include <stdlib.h>
#include <string.h>

static const struct wlf_window_impl macos_window_impl;

/* -------------------------------------------------------------------------
 * WlfWindowDelegate — bridges NSWindow events into wlframe signals
 * ---------------------------------------------------------------------- */

@interface WlfWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) struct wlf_macos_window *wlf_win;
@end

@implementation WlfWindowDelegate

- (BOOL)windowShouldClose:(NSWindow *)sender {
	(void)sender;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.close, &self.wlf_win->base);
	return NO; /* let wlf_window_destroy() drive the actual teardown */
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.focused = true;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.focus_in, &self.wlf_win->base);
}

- (void)windowDidResignKey:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.focused = false;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.focus_out, &self.wlf_win->base);
}

- (void)windowDidResize:(NSNotification *)notification {
	NSWindow *win = notification.object;
	NSRect frame = win.contentView.frame;
	self.wlf_win->base.state.geometry.width  = (int)frame.size.width;
	self.wlf_win->base.state.geometry.height = (int)frame.size.height;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.resize, &self.wlf_win->base);
}

- (void)windowDidMove:(NSNotification *)notification {
	NSWindow *win = notification.object;
	NSRect frame = win.frame;
	self.wlf_win->base.state.geometry.x = (int)frame.origin.x;
	self.wlf_win->base.state.geometry.y = (int)frame.origin.y;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.move, &self.wlf_win->base);
}

- (void)windowDidMiniaturize:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.state |= WLF_WINDOW_MINIMIZED;
}

- (void)windowDidDeminiaturize:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.state &= ~WLF_WINDOW_MINIMIZED;
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.state |= WLF_WINDOW_FULLSCREEN;
}

- (void)windowDidExitFullScreen:(NSNotification *)notification {
	(void)notification;
	self.wlf_win->base.state.state &= ~WLF_WINDOW_FULLSCREEN;
}

- (void)windowDidExpose:(NSNotification *)notification {
	(void)notification;
	wlf_signal_emit_mutable(&self.wlf_win->base.events.expose, &self.wlf_win->base);
}

@end

/* -------------------------------------------------------------------------
 * wlf_window_impl callbacks
 * ---------------------------------------------------------------------- */

static void window_destroy(struct wlf_window *wlf_win) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);

		if (win->ns_window != NULL) {
			NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
			[nswin setDelegate:nil];
			[nswin close];
			[nswin release];
			win->ns_window = NULL;
		}

		if (win->ns_delegate != NULL) {
			WlfWindowDelegate *delegate =
				(__bridge WlfWindowDelegate *)win->ns_delegate;
			[delegate release];
			win->ns_delegate = NULL;
		}

		win->ns_view = NULL;
		free(win);
	}
}

static void window_close(struct wlf_window *wlf_win) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		[nswin close];
	}
}

static void window_show(struct wlf_window *wlf_win) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		[nswin makeKeyAndOrderFront:nil];
	}
}

static void window_hide(struct wlf_window *wlf_win) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		[nswin orderOut:nil];
	}
}

static void window_set_title(struct wlf_window *wlf_win, const char *title) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		nswin.title = [NSString stringWithUTF8String:title ? title : ""];
	}
}

static void window_set_geometry(struct wlf_window *wlf_win,
		const struct wlf_rect *geometry) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		NSRect frame = NSMakeRect(geometry->x, geometry->y,
			geometry->width, geometry->height);
		[nswin setFrame:frame display:YES];
	}
}

static void window_set_size(struct wlf_window *wlf_win, int width, int height) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		NSSize size = NSMakeSize(width, height);
		[nswin setContentSize:size];
	}
}

static void window_set_min_size(struct wlf_window *wlf_win,
		int width, int height) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		nswin.minSize = NSMakeSize(width, height);
	}
}

static void window_set_max_size(struct wlf_window *wlf_win,
		int width, int height) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		nswin.maxSize = NSMakeSize(width > 0 ? width : FLT_MAX,
		                           height > 0 ? height : FLT_MAX);
	}
}

static void window_set_position(struct wlf_window *wlf_win, int x, int y) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		[nswin setFrameOrigin:NSMakePoint(x, y)];
	}
}

static void window_set_state(struct wlf_window *wlf_win,
		enum wlf_window_state_flags state) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;

		if (state & WLF_WINDOW_MINIMIZED) {
			[nswin miniaturize:nil];
		} else if (state & WLF_WINDOW_FULLSCREEN) {
			if (!(wlf_win->state.state & WLF_WINDOW_FULLSCREEN)) {
				[nswin toggleFullScreen:nil];
			}
		} else if (state & WLF_WINDOW_MAXIMIZED) {
			if (!nswin.isZoomed) {
				[nswin zoom:nil];
			}
		} else if (state == WLF_WINDOW_NORMAL) {
			if (wlf_win->state.state & WLF_WINDOW_FULLSCREEN) {
				[nswin toggleFullScreen:nil];
			} else if (nswin.isMiniaturized) {
				[nswin deminiaturize:nil];
			} else if (nswin.isZoomed) {
				[nswin zoom:nil];
			}
		}
	}
}

static void window_set_flags(struct wlf_window *wlf_win, uint32_t flags) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;

		NSWindowStyleMask mask = nswin.styleMask;

		if (flags & WLF_WINDOW_FLAG_RESIZABLE) {
			mask |= NSWindowStyleMaskResizable;
		} else {
			mask &= ~NSWindowStyleMaskResizable;
		}

		if (flags & WLF_WINDOW_FLAG_DECORATED) {
			mask |= NSWindowStyleMaskTitled |
			        NSWindowStyleMaskClosable |
			        NSWindowStyleMaskMiniaturizable;
		} else {
			mask &= ~(NSWindowStyleMaskTitled |
			          NSWindowStyleMaskClosable |
			          NSWindowStyleMaskMiniaturizable);
		}

		nswin.styleMask = mask;

		if (flags & WLF_WINDOW_FLAG_ALWAYS_ON_TOP) {
			nswin.level = NSFloatingWindowLevel;
		} else if (flags & WLF_WINDOW_FLAG_ALWAYS_ON_BOTTOM) {
			nswin.level = NSStatusWindowLevel - 1;
		} else {
			nswin.level = NSNormalWindowLevel;
		}
	}
}

static void window_set_opacity(struct wlf_window *wlf_win, float opacity) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		nswin.alphaValue = opacity;
	}
}

static void window_set_background_color(struct wlf_window *wlf_win,
		const struct wlf_color *color) {
	@autoreleasepool {
		struct wlf_macos_window *win =
			wlf_macos_window_from_window(wlf_win);
		NSWindow *nswin = (__bridge NSWindow *)win->ns_window;
		nswin.backgroundColor = [NSColor colorWithCalibratedRed:color->r
		                                                  green:color->g
		                                                   blue:color->b
		                                                  alpha:color->a];
	}
}

static const struct wlf_window_impl macos_window_impl = {
	.destroy             = window_destroy,
	.close               = window_close,
	.show                = window_show,
	.hide                = window_hide,
	.set_title           = window_set_title,
	.set_geometry        = window_set_geometry,
	.set_size            = window_set_size,
	.set_min_size        = window_set_min_size,
	.set_max_size        = window_set_max_size,
	.set_position        = window_set_position,
	.set_state           = window_set_state,
	.set_flags           = window_set_flags,
	.set_opacity         = window_set_opacity,
	.set_background_color = window_set_background_color,
};

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

struct wlf_window *wlf_macos_window_create(enum wlf_window_type type,
		uint32_t width, uint32_t height) {
	@autoreleasepool {
		struct wlf_macos_window *win = calloc(1, sizeof(*win));
		if (win == NULL) {
			wlf_log_errno(WLF_ERROR, "Failed to allocate wlf_macos_window");
			return NULL;
		}

		/* Ensure NSApplication is available */
		if (NSApp == nil) {
			[NSApplication sharedApplication];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
		}

		/* Build style mask from window type */
		NSWindowStyleMask style = NSWindowStyleMaskTitled |
		                          NSWindowStyleMaskClosable |
		                          NSWindowStyleMaskMiniaturizable |
		                          NSWindowStyleMaskResizable;
		if (type == WLF_WINDOW_TYPE_POPUP || type == WLF_WINDOW_TYPE_TOOLTIP) {
			style = NSWindowStyleMaskBorderless;
		}

		NSRect content = NSMakeRect(0, 0, width, height);
		NSWindow *nswin = [[NSWindow alloc]
			initWithContentRect:content
			          styleMask:style
			            backing:NSBackingStoreBuffered
			              defer:NO];
		if (nswin == nil) {
			wlf_log(WLF_ERROR, "Failed to create NSWindow");
			free(win);
			return NULL;
		}

		[nswin center];

		WlfWindowDelegate *delegate = [[WlfWindowDelegate alloc] init];
		delegate.wlf_win = win;
		nswin.delegate = delegate;

		win->ns_window   = (__bridge void *)nswin;
		win->ns_view     = (__bridge void *)nswin.contentView;
		win->ns_delegate = (__bridge void *)delegate;

		wlf_window_init(&win->base, type, &macos_window_impl, width, height);

		wlf_log(WLF_DEBUG, "Created macOS window %dx%d", width, height);

		return &win->base;
	}
}

bool wlf_window_is_macos(const struct wlf_window *window) {
	return window && window->impl == &macos_window_impl;
}

struct wlf_macos_window *wlf_macos_window_from_window(
		struct wlf_window *window) {
	if (!wlf_window_is_macos(window)) {
		return NULL;
	}
	return (struct wlf_macos_window *)window;
}
