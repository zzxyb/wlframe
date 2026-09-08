#include "wlf/window/macos/window.h"

#include "wlf/platform/macos/backend.h"
#include "wlf/types/macos/keyboard.h"
#include "wlf/types/macos/pointer.h"
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
	NSTrackingArea *wlfTrackingArea;
}
@end

static uint32_t event_time_msec(NSEvent *event) {
	return (uint32_t)(event.timestamp * 1000.0);
}

static NSPoint event_location(WLFMetalView *view, NSEvent *event) {
	return [view convertPoint:event.locationInWindow fromView:nil];
}

static void emit_pointer_frame(struct wlf_macos_window *window) {
	wlf_signal_emit_mutable(&window->pointer->base.events.frame, NULL);
	wlf_window_pointer_frame(&window->base, NULL);
}

static void pointer_set_button(struct wlf_pointer *pointer, uint32_t button,
		bool pressed) {
	size_t index = 0;
	while (index < pointer->button_count &&
			pointer->buttons[index] != button) {
		index++;
	}
	if (pressed) {
		if (index == pointer->button_count &&
				pointer->button_count < WLF_POINTER_BUTTONS_CAP) {
			pointer->buttons[pointer->button_count++] = button;
		}
		return;
	}
	if (index == pointer->button_count) return;
	for (size_t i = index + 1; i < pointer->button_count; ++i) {
		pointer->buttons[i - 1] = pointer->buttons[i];
	}
	pointer->button_count--;
}

static uint32_t pointer_button_code(NSEvent *event) {
	switch (event.buttonNumber) {
	case 0: return WLF_POINTER_BUTTON_LEFT;
	case 1: return WLF_POINTER_BUTTON_RIGHT;
	case 2: return WLF_POINTER_BUTTON_MIDDLE;
	default: return WLF_POINTER_BUTTON_LEFT + (uint32_t)event.buttonNumber;
	}
}

static void emit_pointer_motion(WLFMetalView *view, NSEvent *event) {
	struct wlf_macos_window *window = view->wlfWindow;
	if (window == NULL) return;
	NSPoint point = event_location(view, event);
	struct wlf_pointer_motion_absolute_event motion = {
		.pointer = &window->pointer->base,
		.surface = window->view,
		.time_msec = event_time_msec(event),
		.x = point.x,
		.y = point.y,
	};
	wlf_signal_emit_mutable(&window->pointer->base.events.motion_absolute,
		&motion);
	wlf_window_pointer_motion(&window->base, &motion);
	emit_pointer_frame(window);
}

static void emit_pointer_button(WLFMetalView *view, NSEvent *event,
		enum wlf_pointer_button_state state) {
	struct wlf_macos_window *window = view->wlfWindow;
	if (window == NULL) return;
	uint32_t button = pointer_button_code(event);
	pointer_set_button(&window->pointer->base, button,
		state == WLF_POINTER_BUTTON_STATE_PRESSED);
	struct wlf_pointer_button_event button_event = {
		.pointer = &window->pointer->base,
		.serial = wlf_macos_pointer_next_serial(window->pointer),
		.time_msec = event_time_msec(event),
		.button = button,
		.state = state,
	};
	wlf_signal_emit_mutable(&window->pointer->base.events.button,
		&button_event);
	wlf_window_pointer_button(&window->base, &button_event);
	emit_pointer_frame(window);
}

static void emit_keyboard_key(WLFMetalView *view, NSEvent *event,
		enum wlf_keyboard_key_state state) {
	struct wlf_macos_window *window = view->wlfWindow;
	if (window == NULL) return;
	uint32_t key = event.keyCode;
	wlf_macos_keyboard_set_key_state(window->keyboard, key, state);
	struct wlf_keyboard_key_event key_event = {
		.keyboard = &window->keyboard->base,
		.serial = wlf_macos_keyboard_next_serial(window->keyboard),
		.time_msec = event_time_msec(event),
		.key = key,
		.state = state,
	};
	wlf_signal_emit_mutable(&window->keyboard->base.events.key, &key_event);
	wlf_window_keyboard_key(&window->base, &key_event);
}

@implementation WLFMetalView
- (CALayer *)makeBackingLayer {
	return [CAMetalLayer layer];
}

- (BOOL)isFlipped {
	return YES;
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)updateTrackingAreas {
	[super updateTrackingAreas];
	if (wlfTrackingArea != nil) {
		[self removeTrackingArea:wlfTrackingArea];
		[wlfTrackingArea release];
	}
	NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
		NSTrackingMouseMoved | NSTrackingActiveInKeyWindow |
		NSTrackingInVisibleRect;
	wlfTrackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
		options:options owner:self userInfo:nil];
	[self addTrackingArea:wlfTrackingArea];
}

- (void)dealloc {
	if (wlfTrackingArea != nil) {
		[self removeTrackingArea:wlfTrackingArea];
		[wlfTrackingArea release];
	}
	[super dealloc];
}

- (void)mouseEntered:(NSEvent *)event {
	if (wlfWindow == NULL || wlfWindow->pointer_inside) return;
	wlfWindow->pointer_inside = true;
	NSPoint point = event_location(self, event);
	uint32_t serial = wlf_macos_pointer_next_serial(wlfWindow->pointer);
	wlfWindow->pointer->base.cursor_serial = serial;
	struct wlf_pointer_enter_event enter = {
		.pointer = &wlfWindow->pointer->base,
		.serial = serial,
		.surface = wlfWindow->view,
		.x = point.x,
		.y = point.y,
	};
	wlf_signal_emit_mutable(&wlfWindow->pointer->base.events.enter, &enter);
	wlf_window_pointer_enter(&wlfWindow->base, &enter);
	emit_pointer_frame(wlfWindow);
}

- (void)mouseExited:(NSEvent *)event {
	if (wlfWindow == NULL || !wlfWindow->pointer_inside) return;
	wlfWindow->pointer_inside = false;
	struct wlf_pointer_leave_event leave = {
		.pointer = &wlfWindow->pointer->base,
		.serial = wlf_macos_pointer_next_serial(wlfWindow->pointer),
		.surface = wlfWindow->view,
	};
	wlf_signal_emit_mutable(&wlfWindow->pointer->base.events.leave, &leave);
	wlf_window_pointer_leave(&wlfWindow->base, &leave);
	wlfWindow->pointer->base.cursor_serial = 0;
	emit_pointer_frame(wlfWindow);
	(void)event;
}

- (void)mouseMoved:(NSEvent *)event { emit_pointer_motion(self, event); }
- (void)mouseDragged:(NSEvent *)event { emit_pointer_motion(self, event); }
- (void)rightMouseDragged:(NSEvent *)event { emit_pointer_motion(self, event); }
- (void)otherMouseDragged:(NSEvent *)event { emit_pointer_motion(self, event); }
- (void)mouseDown:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_PRESSED);
}
- (void)mouseUp:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_RELEASED);
}
- (void)rightMouseDown:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_PRESSED);
}
- (void)rightMouseUp:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_RELEASED);
}
- (void)otherMouseDown:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_PRESSED);
}
- (void)otherMouseUp:(NSEvent *)event {
	emit_pointer_button(self, event, WLF_POINTER_BUTTON_STATE_RELEASED);
}

- (void)scrollWheel:(NSEvent *)event {
	if (wlfWindow == NULL) return;
	double deltas[2] = {-event.scrollingDeltaY, -event.scrollingDeltaX};
	for (int axis = 0; axis < 2; ++axis) {
		if (deltas[axis] == 0) continue;
		struct wlf_pointer_axis_event axis_event = {
			.pointer = &wlfWindow->pointer->base,
			.time_msec = event_time_msec(event),
			.source = event.hasPreciseScrollingDeltas ?
				WLF_POINTER_AXIS_SOURCE_FINGER : WLF_POINTER_AXIS_SOURCE_WHEEL,
			.orientation = (enum wlf_pointer_axis)axis,
			.relative_direction = event.directionInvertedFromDevice ?
				WLF_POINTER_AXIS_RELATIVE_DIRECTION_INVERTED :
				WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL,
			.delta = deltas[axis],
			.delta_discrete = event.hasPreciseScrollingDeltas ? 0 :
				(int32_t)llround(deltas[axis] * 120.0),
		};
		wlf_signal_emit_mutable(&wlfWindow->pointer->base.events.axis,
			&axis_event);
		wlf_window_pointer_axis(&wlfWindow->base, &axis_event);
	}
	emit_pointer_frame(wlfWindow);
}

- (void)keyDown:(NSEvent *)event {
	emit_keyboard_key(self, event, WLF_KEYBOARD_KEY_STATE_PRESSED);
}

- (void)keyUp:(NSEvent *)event {
	emit_keyboard_key(self, event, WLF_KEYBOARD_KEY_STATE_RELEASED);
}

- (void)flagsChanged:(NSEvent *)event {
	if (wlfWindow == NULL) return;
	uint32_t modifiers = (uint32_t)(event.modifierFlags &
		NSEventModifierFlagDeviceIndependentFlagsMask);
	wlfWindow->keyboard->modifiers = modifiers;
	struct wlf_keyboard_modifiers_event modifiers_event = {
		.keyboard = &wlfWindow->keyboard->base,
		.serial = wlf_macos_keyboard_next_serial(wlfWindow->keyboard),
		.mods_depressed = modifiers,
	};
	wlf_signal_emit_mutable(&wlfWindow->keyboard->base.events.modifiers,
		&modifiers_event);
	wlf_window_keyboard_modifiers(&wlfWindow->base, &modifiers_event);
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
	struct wlf_keyboard_keymap_event keymap = {
		.keyboard = &wlfWindow->keyboard->base,
		.format = WLF_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP,
		.fd = -1,
	};
	wlf_signal_emit_mutable(&wlfWindow->keyboard->base.events.keymap, &keymap);
	wlf_window_keyboard_keymap(&wlfWindow->base, &keymap);
	struct wlf_keyboard_repeat_info_event repeat = {
		.keyboard = &wlfWindow->keyboard->base,
		.rate = 30,
		.delay = 500,
	};
	wlf_signal_emit_mutable(&wlfWindow->keyboard->base.events.repeat_info,
		&repeat);
	wlf_window_keyboard_repeat_info(&wlfWindow->base, &repeat);
	struct wlf_keyboard_enter_event enter = {
		.keyboard = &wlfWindow->keyboard->base,
		.serial = wlf_macos_keyboard_next_serial(wlfWindow->keyboard),
		.window = &wlfWindow->base,
		.keys = wlfWindow->keyboard->keys,
		.keys_count = wlfWindow->keyboard->key_count,
	};
	wlf_signal_emit_mutable(&wlfWindow->keyboard->base.events.enter, &enter);
	wlf_window_keyboard_enter(&wlfWindow->base, &enter);
}

- (void)windowDidResignKey:(NSNotification *)notification {
	(void)notification;
	if (wlfWindow == NULL || !wlfWindow->base.state.focused) return;
	wlfWindow->base.state.focused = false;
	wlfWindow->base.state.state &= ~WLF_WINDOW_ACTIVE;
	wlf_signal_emit_mutable(&wlfWindow->base.events.focus_out,
		&wlfWindow->base);
	struct wlf_keyboard_leave_event leave = {
		.keyboard = &wlfWindow->keyboard->base,
		.serial = wlf_macos_keyboard_next_serial(wlfWindow->keyboard),
		.window = &wlfWindow->base,
	};
	wlf_signal_emit_mutable(&wlfWindow->keyboard->base.events.leave, &leave);
	wlf_window_keyboard_leave(&wlfWindow->base, &leave);
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
	wlf_pointer_destroy(window->pointer != NULL ? &window->pointer->base : NULL);
	wlf_keyboard_destroy(window->keyboard != NULL ? &window->keyboard->base : NULL);
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
		window->pointer = wlf_macos_pointer_create();
		window->keyboard = wlf_macos_keyboard_create();
		if (window->pointer == NULL || window->keyboard == NULL) {
			wlf_window_destroy(&window->base);
			return NULL;
		}

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
		native.acceptsMouseMovedEvents = YES;
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

struct wlf_pointer *wlf_macos_window_get_pointer(
		const struct wlf_macos_window *window) {
	return window != NULL && window->pointer != NULL ?
		&window->pointer->base : NULL;
}

struct wlf_keyboard *wlf_macos_window_get_keyboard(
		const struct wlf_macos_window *window) {
	return window != NULL && window->keyboard != NULL ?
		&window->keyboard->base : NULL;
}
