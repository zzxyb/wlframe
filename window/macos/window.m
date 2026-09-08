#include "wlf/window/macos/window.h"

#include "wlf/platform/macos/backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include <assert.h>
#include <math.h>
#include <stdlib.h>

@interface WLFMetalView : NSView {
@public
	struct wlf_macos_window *wlfWindow;
}
@end

@implementation WLFMetalView
- (CALayer *)makeBackingLayer {
	return [CAMetalLayer layer];
}

- (BOOL)isFlipped {
	return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
	(void)dirtyRect;
	if (wlfWindow == NULL) {
		return;
	}
	wlfWindow->frame_pending = false;
	wlf_signal_emit_mutable(&wlfWindow->base.events.expose,
		&wlfWindow->base);
}
@end

@interface WLFWindowDelegate : NSObject <NSWindowDelegate> {
@public
	struct wlf_macos_window *wlfWindow;
}
@end

static void update_window_scale(struct wlf_macos_window *window) {
	NSWindow *native = (__bridge NSWindow *)window->ns_window;
	double scale = native.backingScaleFactor;
	if (scale <= 0) {
		scale = 1;
	}
	wlf_window_set_scale(&window->base, scale);
}

@implementation WLFWindowDelegate
- (void)windowDidResize:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow == NULL) return;
	NSView *view = (__bridge NSView *)wlfWindow->view;
	int width = (int)llround(view.bounds.size.width);
	int height = (int)llround(view.bounds.size.height);
	if (width == wlfWindow->base.state.geometry.width &&
			height == wlfWindow->base.state.geometry.height) {
		return;
	}
	wlfWindow->base.state.geometry.width = width;
	wlfWindow->base.state.geometry.height = height;
	update_window_scale(wlfWindow);
	wlf_signal_emit_mutable(&wlfWindow->base.events.resize,
		&wlfWindow->base);
}

- (void)windowDidMove:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow == NULL) return;
	NSWindow *native = (__bridge NSWindow *)wlfWindow->ns_window;
	wlfWindow->base.state.geometry.x = (int)llround(native.frame.origin.x);
	wlfWindow->base.state.geometry.y = (int)llround(native.frame.origin.y);
	wlf_signal_emit_mutable(&wlfWindow->base.events.move, &wlfWindow->base);
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow == NULL || wlfWindow->base.state.focused) return;
	wlfWindow->base.state.focused = true;
	wlfWindow->base.state.state |= WLF_WINDOW_ACTIVE;
	wlf_signal_emit_mutable(&wlfWindow->base.events.focus_in,
		&wlfWindow->base);
}

- (void)windowDidResignKey:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow == NULL || !wlfWindow->base.state.focused) return;
	wlfWindow->base.state.focused = false;
	wlfWindow->base.state.state &= ~WLF_WINDOW_ACTIVE;
	wlf_signal_emit_mutable(&wlfWindow->base.events.focus_out,
		&wlfWindow->base);
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow != NULL) update_window_scale(wlfWindow);
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
	if (wlfWindow == NULL) return YES;
	[sender orderOut:nil];
	wlfWindow->base.state.visible = false;
	wlf_signal_emit_mutable(&wlfWindow->base.events.close, &wlfWindow->base);
	return NO;
}
@end

static struct wlf_macos_window *macos_from_base(struct wlf_window *base);

static void macos_window_destroy(struct wlf_window *base) {
	struct wlf_macos_window *window = macos_from_base(base);
	WLFMetalView *view = (__bridge WLFMetalView *)window->view;
	WLFWindowDelegate *delegate =
		(__bridge WLFWindowDelegate *)window->delegate;
	NSWindow *native = (__bridge NSWindow *)window->ns_window;
	if (view != nil) view->wlfWindow = NULL;
	if (delegate != nil) delegate->wlfWindow = NULL;
	if (native != nil) {
		native.delegate = nil;
		[native orderOut:nil];
	}
	[delegate release];
	[native release];
	free(window);
}

static void macos_window_close(struct wlf_window *base) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	[native orderOut:nil];
}

static void macos_window_show(struct wlf_window *base) {
	struct wlf_macos_window *window = macos_from_base(base);
	NSWindow *native = (__bridge NSWindow *)window->ns_window;
	[NSApp activateIgnoringOtherApps:YES];
	[native makeKeyAndOrderFront:nil];
	if (!window->frame_pending) {
		window->frame_pending = true;
		[(__bridge NSView *)window->view setNeedsDisplay:YES];
	}
}

static void macos_window_hide(struct wlf_window *base) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	[native orderOut:nil];
}

static void macos_window_set_title(struct wlf_window *base,
		const char *title) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	NSString *value = title != NULL ? [NSString stringWithUTF8String:title] : @"";
	native.title = value != nil ? value : @"";
}

static void macos_window_set_geometry(struct wlf_window *base,
		const struct wlf_rect *geometry) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	[native setContentSize:NSMakeSize(geometry->width, geometry->height)];
	[native setFrameOrigin:NSMakePoint(geometry->x, geometry->y)];
}

static void macos_window_set_size(struct wlf_window *base, int width,
		int height) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	[native setContentSize:NSMakeSize(width, height)];
}

static void macos_window_set_min_size(struct wlf_window *base, int width,
		int height) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	native.contentMinSize = NSMakeSize(width, height);
}

static void macos_window_set_max_size(struct wlf_window *base, int width,
		int height) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	native.contentMaxSize = NSMakeSize(width, height);
}

static void macos_window_set_position(struct wlf_window *base, int x, int y) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	[native setFrameOrigin:NSMakePoint(x, y)];
}

static void macos_window_set_state(struct wlf_window *base,
		enum wlf_window_state_flags state) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	if ((state & WLF_WINDOW_MINIMIZED) && !native.miniaturized) {
		[native miniaturize:nil];
	} else if (!(state & WLF_WINDOW_MINIMIZED) && native.miniaturized) {
		[native deminiaturize:nil];
	}
	bool fullscreen = (native.styleMask & NSWindowStyleMaskFullScreen) != 0;
	if (!!(state & WLF_WINDOW_FULLSCREEN) != fullscreen) {
		[native toggleFullScreen:nil];
	}
	if ((state & WLF_WINDOW_MAXIMIZED) && !native.zoomed) {
		[native zoom:nil];
	} else if (!(state & WLF_WINDOW_MAXIMIZED) && native.zoomed) {
		[native zoom:nil];
	}
}

static void macos_window_set_flags(struct wlf_window *base, uint32_t flags) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	NSWindowStyleMask style = native.styleMask;
	if (flags & WLF_WINDOW_FLAG_RESIZABLE) style |= NSWindowStyleMaskResizable;
	else style &= ~NSWindowStyleMaskResizable;
	if (flags & WLF_WINDOW_FLAG_DECORATED) style |= NSWindowStyleMaskTitled;
	else style &= ~NSWindowStyleMaskTitled;
	native.styleMask = style;
	if (flags & WLF_WINDOW_FLAG_ALWAYS_ON_TOP) {
		native.level = NSFloatingWindowLevel;
	} else if (flags & WLF_WINDOW_FLAG_ALWAYS_ON_BOTTOM) {
		native.level = kCGDesktopWindowLevel;
	} else {
		native.level = NSNormalWindowLevel;
	}
}

static void macos_window_set_opacity(struct wlf_window *base, float opacity) {
	NSWindow *native = (__bridge NSWindow *)macos_from_base(base)->ns_window;
	native.alphaValue = opacity;
}

static void *macos_window_native_handle(struct wlf_window *base) {
	return macos_from_base(base)->view;
}

static void macos_window_schedule_frame(struct wlf_window *base) {
	struct wlf_macos_window *window = macos_from_base(base);
	if (window->frame_pending) return;
	window->frame_pending = true;
	[(__bridge NSView *)window->view setNeedsDisplay:YES];
}

static void macos_window_arm_frame(struct wlf_window *base) {
	WLF_UNUSED(base);
}

static const struct wlf_window_impl macos_window_impl = {
	.destroy = macos_window_destroy,
	.close = macos_window_close,
	.show = macos_window_show,
	.hide = macos_window_hide,
	.set_title = macos_window_set_title,
	.set_geometry = macos_window_set_geometry,
	.set_size = macos_window_set_size,
	.set_min_size = macos_window_set_min_size,
	.set_max_size = macos_window_set_max_size,
	.set_position = macos_window_set_position,
	.set_state = macos_window_set_state,
	.set_flags = macos_window_set_flags,
	.set_opacity = macos_window_set_opacity,
	.native_handle = macos_window_native_handle,
	.arm_frame = macos_window_arm_frame,
	.schedule_frame = macos_window_schedule_frame,
};

static struct wlf_macos_window *macos_from_base(struct wlf_window *base) {
	assert(wlf_window_is_macos(base));
	struct wlf_macos_window *window = NULL;
	return wlf_container_of(base, window, base);
}

struct wlf_macos_window *wlf_macos_window_create_from_backend(
		struct wlf_backend *backend, uint32_t width, uint32_t height) {
	if (backend == NULL || !wlf_backend_is_macos(backend) ||
			width == 0 || height == 0) {
		return NULL;
	}
	if (![NSThread isMainThread]) {
		wlf_log(WLF_ERROR, "macOS windows must be created on the main thread");
		return NULL;
	}

	@autoreleasepool {
		if (NSApp == nil) [NSApplication sharedApplication];
		struct wlf_macos_window *window = calloc(1, sizeof(*window));
		if (window == NULL) return NULL;
		wlf_window_init(&window->base, WLF_WINDOW_TYPE_TOPLEVEL,
			&macos_window_impl, backend, width, height);
		window->backend = backend;

		NSWindowStyleMask style = NSWindowStyleMaskTitled |
			NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |
			NSWindowStyleMaskResizable;
		NSWindow *native = [[NSWindow alloc]
			initWithContentRect:NSMakeRect(0, 0, width, height)
			styleMask:style backing:NSBackingStoreBuffered defer:NO];
		if (native == nil) {
			wlf_window_destroy(&window->base);
			return NULL;
		}
		native.releasedWhenClosed = NO;
		window->ns_window = (__bridge void *)native;
		WLFMetalView *view = [[WLFMetalView alloc]
			initWithFrame:NSMakeRect(0, 0, width, height)];
		WLFWindowDelegate *delegate = [[WLFWindowDelegate alloc] init];
		if (view == nil || delegate == nil) {
			[view release];
			[delegate release];
			wlf_window_destroy(&window->base);
			return NULL;
		}
		window->view = (__bridge void *)view;
		window->delegate = (__bridge void *)delegate;
		view->wlfWindow = window;
		delegate->wlfWindow = window;
		view.wantsLayer = YES;
		native.contentView = view;
		native.delegate = delegate;
		[native center];
		[view release];
		update_window_scale(window);
		return window;
	}
}

bool wlf_window_is_macos(const struct wlf_window *window) {
	return window != NULL && window->impl == &macos_window_impl;
}

struct wlf_macos_window *wlf_macos_window_from_window(
		struct wlf_window *window) {
	return macos_from_base(window);
}

void *wlf_macos_window_get_nswindow(const struct wlf_macos_window *window) {
	return window != NULL ? window->ns_window : NULL;
}

void *wlf_macos_window_get_view(const struct wlf_macos_window *window) {
	return window != NULL ? window->view : NULL;
}
