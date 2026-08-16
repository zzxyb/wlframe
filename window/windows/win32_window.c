#include "wlf/window/windows/win32_window.h"

#include "wlf/platform/windows/backend.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

static const wchar_t win32_window_class_name[] = L"wlframe.window";

static struct wlf_win32_window *win32_from_hwnd(HWND hwnd) {
	return (struct wlf_win32_window *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
}

static struct wlf_win32_window *win32_from_window(struct wlf_window *base);

static void embedded_pointer_destroy(struct wlf_pointer *pointer) {
	WLF_UNUSED(pointer);
}

static void embedded_keyboard_destroy(struct wlf_keyboard *keyboard) {
	WLF_UNUSED(keyboard);
}

static void embedded_touch_destroy(struct wlf_touch *touch) {
	WLF_UNUSED(touch);
}

static void embedded_cursor_destroy(struct wlf_cursor *cursor) {
	WLF_UNUSED(cursor);
}

static LPCWSTR cursor_resource(enum wlf_cursor_shape shape) {
	switch (shape) {
	case WLF_CURSOR_SHAPE_HELP:
		return MAKEINTRESOURCEW(32651);
	case WLF_CURSOR_SHAPE_PROGRESS:
		return MAKEINTRESOURCEW(32650);
	case WLF_CURSOR_SHAPE_WAIT:
		return MAKEINTRESOURCEW(32514);
	case WLF_CURSOR_SHAPE_CROSSHAIR:
	case WLF_CURSOR_SHAPE_CELL:
		return MAKEINTRESOURCEW(32515);
	case WLF_CURSOR_SHAPE_TEXT:
	case WLF_CURSOR_SHAPE_VERTICAL_TEXT:
		return MAKEINTRESOURCEW(32513);
	case WLF_CURSOR_SHAPE_POINTER:
		return MAKEINTRESOURCEW(32649);
	case WLF_CURSOR_SHAPE_NOT_ALLOWED:
	case WLF_CURSOR_SHAPE_NO_DROP:
		return MAKEINTRESOURCEW(32648);
	case WLF_CURSOR_SHAPE_E_RESIZE:
	case WLF_CURSOR_SHAPE_W_RESIZE:
	case WLF_CURSOR_SHAPE_EW_RESIZE:
	case WLF_CURSOR_SHAPE_COL_RESIZE:
		return MAKEINTRESOURCEW(32644);
	case WLF_CURSOR_SHAPE_N_RESIZE:
	case WLF_CURSOR_SHAPE_S_RESIZE:
	case WLF_CURSOR_SHAPE_NS_RESIZE:
	case WLF_CURSOR_SHAPE_ROW_RESIZE:
		return MAKEINTRESOURCEW(32645);
	case WLF_CURSOR_SHAPE_NE_RESIZE:
	case WLF_CURSOR_SHAPE_SW_RESIZE:
	case WLF_CURSOR_SHAPE_NESW_RESIZE:
		return MAKEINTRESOURCEW(32643);
	case WLF_CURSOR_SHAPE_NW_RESIZE:
	case WLF_CURSOR_SHAPE_SE_RESIZE:
	case WLF_CURSOR_SHAPE_NWSE_RESIZE:
		return MAKEINTRESOURCEW(32642);
	case WLF_CURSOR_SHAPE_MOVE:
	case WLF_CURSOR_SHAPE_GRAB:
	case WLF_CURSOR_SHAPE_GRABBING:
	case WLF_CURSOR_SHAPE_ALL_SCROLL:
	case WLF_CURSOR_SHAPE_ALL_RESIZE:
		return MAKEINTRESOURCEW(32646);
	default:
		return MAKEINTRESOURCEW(32512);
	}
}

static bool win32_cursor_set_shape(struct wlf_cursor *cursor,
		uint32_t serial, enum wlf_cursor_shape shape) {
	WLF_UNUSED(serial);
	struct wlf_win32_window *window = wlf_container_of(cursor, window, cursor);
	HCURSOR handle = LoadCursorW(NULL, cursor_resource(shape));
	if (handle == NULL) {
		return false;
	}
	window->cursor_handle = handle;
	if (window->pointer_inside) {
		SetCursor(handle);
	}
	return true;
}

static const struct wlf_pointer_impl win32_pointer_impl = {
	.name = "Win32 pointer",
	.destroy = embedded_pointer_destroy,
};

static const struct wlf_keyboard_impl win32_keyboard_impl = {
	.name = "Win32 keyboard",
	.destroy = embedded_keyboard_destroy,
};

static const struct wlf_touch_impl win32_touch_impl = {
	.name = "Win32 touch",
	.destroy = embedded_touch_destroy,
};

static const struct wlf_cursor_impl win32_cursor_impl = {
	.destroy = embedded_cursor_destroy,
	.set_shape = win32_cursor_set_shape,
};

static uint32_t next_input_serial(struct wlf_win32_window *window) {
	window->input_serial++;
	if (window->input_serial == 0) {
		window->input_serial = 1;
	}
	return window->input_serial;
}

static uint32_t message_time(void) {
	return (uint32_t)GetMessageTime();
}

static int logical_from_physical(const struct wlf_win32_window *window,
		int value) {
	return (int)lround(value / window->base.state.scale);
}

static int physical_from_logical(const struct wlf_win32_window *window,
		int value) {
	return (int)lround(value * window->base.state.scale);
}

static void pointer_add_button(struct wlf_pointer *pointer, uint32_t button) {
	for (size_t i = 0; i < pointer->button_count; ++i) {
		if (pointer->buttons[i] == button) {
			return;
		}
	}
	if (pointer->button_count < WLF_POINTER_BUTTONS_CAP) {
		pointer->buttons[pointer->button_count++] = button;
	}
}

static void pointer_remove_button(struct wlf_pointer *pointer,
		uint32_t button) {
	for (size_t i = 0; i < pointer->button_count; ++i) {
		if (pointer->buttons[i] != button) {
			continue;
		}
		pointer->buttons[i] = pointer->buttons[pointer->button_count - 1];
		pointer->button_count--;
		return;
	}
}

static void handle_pointer_motion(struct wlf_win32_window *window,
		LPARAM lparam) {
	double x = logical_from_physical(window, (short)LOWORD(lparam));
	double y = logical_from_physical(window, (short)HIWORD(lparam));
	if (!window->pointer_inside) {
		TRACKMOUSEEVENT tracking = {
			.cbSize = sizeof(tracking),
			.dwFlags = TME_LEAVE,
			.hwndTrack = window->hwnd,
		};
		TrackMouseEvent(&tracking);
		window->pointer_inside = true;
		window->pointer.cursor_serial = next_input_serial(window);
		struct wlf_pointer_enter_event enter = {
			.pointer = &window->pointer,
			.serial = window->pointer.cursor_serial,
			.surface = window->hwnd,
			.x = x,
			.y = y,
		};
		wlf_window_pointer_enter(&window->base, &enter);
	}
	struct wlf_pointer_motion_absolute_event motion = {
		.pointer = &window->pointer,
		.surface = window->hwnd,
		.time_msec = message_time(),
		.x = x,
		.y = y,
	};
	wlf_window_pointer_motion(&window->base, &motion);
	wlf_window_pointer_frame(&window->base, &window->pointer);
}

static void handle_pointer_leave(struct wlf_win32_window *window) {
	if (!window->pointer_inside) {
		return;
	}
	window->pointer_inside = false;
	struct wlf_pointer_leave_event leave = {
		.pointer = &window->pointer,
		.serial = next_input_serial(window),
		.surface = window->hwnd,
	};
	wlf_window_pointer_leave(&window->base, &leave);
}

static uint32_t pointer_button_from_message(UINT message) {
	switch (message) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		return WLF_POINTER_BUTTON_LEFT;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		return WLF_POINTER_BUTTON_RIGHT;
	default:
		return WLF_POINTER_BUTTON_MIDDLE;
	}
}

static void handle_pointer_button(struct wlf_win32_window *window,
		UINT message, bool pressed) {
	uint32_t button = pointer_button_from_message(message);
	if (pressed) {
		pointer_add_button(&window->pointer, button);
		SetCapture(window->hwnd);
	} else {
		pointer_remove_button(&window->pointer, button);
		if (window->pointer.button_count == 0 &&
				GetCapture() == window->hwnd) {
			ReleaseCapture();
		}
	}
	struct wlf_pointer_button_event event = {
		.pointer = &window->pointer,
		.serial = next_input_serial(window),
		.time_msec = message_time(),
		.button = button,
		.state = pressed ? WLF_POINTER_BUTTON_STATE_PRESSED :
			WLF_POINTER_BUTTON_STATE_RELEASED,
	};
	wlf_window_pointer_button(&window->base, &event);
	wlf_window_pointer_frame(&window->base, &window->pointer);
}

static void handle_pointer_axis(struct wlf_win32_window *window,
		UINT message, WPARAM wparam) {
	int delta = GET_WHEEL_DELTA_WPARAM(wparam);
	struct wlf_pointer_axis_event event = {
		.pointer = &window->pointer,
		.time_msec = message_time(),
		.source = WLF_POINTER_AXIS_SOURCE_WHEEL,
		.orientation = message == WM_MOUSEWHEEL ?
			WLF_POINTER_AXIS_VERTICAL_SCROLL :
			WLF_POINTER_AXIS_HORIZONTAL_SCROLL,
		.relative_direction = WLF_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL,
		.delta = -(double)delta / WHEEL_DELTA,
		.delta_discrete = -delta,
	};
	wlf_window_pointer_axis(&window->base, &event);
	wlf_window_pointer_frame(&window->base, &window->pointer);
}

static void emit_keyboard_repeat_info(struct wlf_win32_window *window) {
	UINT speed = 31;
	UINT delay = 1;
	SystemParametersInfoW(SPI_GETKEYBOARDSPEED, 0, &speed, 0);
	SystemParametersInfoW(SPI_GETKEYBOARDDELAY, 0, &delay, 0);
	struct wlf_keyboard_repeat_info_event event = {
		.keyboard = &window->keyboard,
		.rate = (int32_t)lround(2.5 + 27.5 * speed / 31.0),
		.delay = (int32_t)((delay + 1) * 250),
	};
	wlf_window_keyboard_repeat_info(&window->base, &event);
}

static void handle_keyboard_focus(struct wlf_win32_window *window,
		bool focused) {
	if (focused) {
		struct wlf_keyboard_enter_event event = {
			.keyboard = &window->keyboard,
			.serial = next_input_serial(window),
			.window = &window->base,
		};
		wlf_window_keyboard_enter(&window->base, &event);
		emit_keyboard_repeat_info(window);
	} else {
		struct wlf_keyboard_leave_event event = {
			.keyboard = &window->keyboard,
			.serial = next_input_serial(window),
			.window = &window->base,
		};
		wlf_window_keyboard_leave(&window->base, &event);
	}
}

static void handle_keyboard_key(struct wlf_win32_window *window,
		LPARAM lparam, bool pressed) {
	uint32_t scan_code = ((uint32_t)lparam >> 16) & 0xff;
	if (((uint32_t)lparam & (1u << 24)) != 0) {
		scan_code |= 0x100;
	}
	struct wlf_keyboard_key_event event = {
		.keyboard = &window->keyboard,
		.serial = next_input_serial(window),
		.time_msec = message_time(),
		.key = scan_code,
		.state = pressed ? WLF_KEYBOARD_KEY_STATE_PRESSED :
			WLF_KEYBOARD_KEY_STATE_RELEASED,
	};
	wlf_window_keyboard_key(&window->base, &event);
}

static bool get_touch_info(struct wlf_win32_window *window, WPARAM wparam,
		POINTER_TOUCH_INFO *touch, double *x, double *y) {
	UINT32 pointer_id = GET_POINTERID_WPARAM(wparam);
	if (!GetPointerTouchInfo(pointer_id, touch)) {
		return false;
	}
	POINT point = touch->pointerInfo.ptPixelLocation;
	if (!ScreenToClient(window->hwnd, &point)) {
		return false;
	}
	*x = logical_from_physical(window, point.x);
	*y = logical_from_physical(window, point.y);
	return true;
}

static void emit_touch_shape(struct wlf_win32_window *window,
		const POINTER_TOUCH_INFO *touch) {
	if ((touch->touchMask & TOUCH_MASK_CONTACTAREA) == 0) {
		return;
	}
	double width = logical_from_physical(window,
		touch->rcContact.right - touch->rcContact.left);
	double height = logical_from_physical(window,
		touch->rcContact.bottom - touch->rcContact.top);
	struct wlf_touch_shape_event event = {
		.touch = &window->touch,
		.touch_id = (int32_t)touch->pointerInfo.pointerId,
		.major = width > height ? width : height,
		.minor = width > height ? height : width,
	};
	wlf_window_touch_shape(&window->base, &event);
}

static void emit_touch_orientation(struct wlf_win32_window *window,
		const POINTER_TOUCH_INFO *touch) {
	if ((touch->touchMask & TOUCH_MASK_ORIENTATION) == 0) {
		return;
	}
	struct wlf_touch_orientation_event event = {
		.touch = &window->touch,
		.touch_id = (int32_t)touch->pointerInfo.pointerId,
		.orientation = touch->orientation,
	};
	wlf_window_touch_orientation(&window->base, &event);
}

static void handle_touch_pointer(struct wlf_win32_window *window,
		UINT message, WPARAM wparam) {
	POINTER_TOUCH_INFO touch = {0};
	double x = 0;
	double y = 0;
	if (!get_touch_info(window, wparam, &touch, &x, &y) ||
			touch.pointerInfo.pointerType != PT_TOUCH) {
		return;
	}
	int32_t touch_id = (int32_t)touch.pointerInfo.pointerId;
	uint32_t time = touch.pointerInfo.dwTime;
	if (message == WM_POINTERDOWN) {
		struct wlf_touch_down_event event = {
			.touch = &window->touch,
			.surface = window->hwnd,
			.time_msec = time,
			.touch_id = touch_id,
			.x = x,
			.y = y,
		};
		wlf_window_touch_down(&window->base, &event);
	} else if (message == WM_POINTERUPDATE) {
		struct wlf_touch_motion_event event = {
			.touch = &window->touch,
			.time_msec = time,
			.touch_id = touch_id,
			.x = x,
			.y = y,
		};
		wlf_window_touch_motion(&window->base, &event);
	} else {
		struct wlf_touch_up_event event = {
			.touch = &window->touch,
			.time_msec = time,
			.touch_id = touch_id,
		};
		wlf_window_touch_up(&window->base, &event);
	}
	emit_touch_shape(window, &touch);
	emit_touch_orientation(window, &touch);
	wlf_window_touch_frame(&window->base, &window->touch);
}

static void handle_touch_cancel(struct wlf_win32_window *window,
		WPARAM wparam) {
	struct wlf_touch_cancel_event event = {
		.touch = &window->touch,
		.time_msec = message_time(),
		.touch_id = (int32_t)GET_POINTERID_WPARAM(wparam),
	};
	wlf_window_touch_cancel(&window->base, &event);
	wlf_window_touch_frame(&window->base, &window->touch);
}

static wchar_t *utf8_to_wide(const char *text) {
	int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1,
		NULL, 0);
	if (count == 0) {
		return NULL;
	}
	wchar_t *wide = calloc((size_t)count, sizeof(*wide));
	if (wide == NULL || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
			text, -1, wide, count) == 0) {
		free(wide);
		return NULL;
	}
	return wide;
}

static DWORD window_style(uint32_t flags) {
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if (flags & WLF_WINDOW_FLAG_DECORATED) {
		style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	}
	if (flags & WLF_WINDOW_FLAG_RESIZABLE) {
		style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
	}
	return style;
}

static DWORD window_ex_style(uint32_t flags) {
	DWORD style = WS_EX_APPWINDOW;
	if (flags & WLF_WINDOW_FLAG_ALWAYS_ON_TOP) {
		style |= WS_EX_TOPMOST;
	}
	return style;
}

static RECT client_geometry_to_window(struct wlf_win32_window *window,
		const struct wlf_rect *geometry) {
	RECT rect = {0, 0, physical_from_logical(window, geometry->width),
		physical_from_logical(window, geometry->height)};
	DWORD style = (DWORD)GetWindowLongPtrW(window->hwnd, GWL_STYLE);
	DWORD ex_style = (DWORD)GetWindowLongPtrW(window->hwnd, GWL_EXSTYLE);
	AdjustWindowRectEx(&rect, style, FALSE, ex_style);
	rect.right -= rect.left;
	rect.bottom -= rect.top;
	rect.left = geometry->x;
	rect.top = geometry->y;
	return rect;
}

static void win32_window_destroy(struct wlf_window *base) {
	struct wlf_win32_window *window = win32_from_window(base);
	if (window->hwnd != NULL) {
		DestroyWindow(window->hwnd);
	}
	wlf_pointer_destroy(&window->pointer);
	wlf_keyboard_destroy(&window->keyboard);
	wlf_touch_destroy(&window->touch);
	wlf_cursor_destroy(&window->cursor);
	free(window);
}

static void win32_window_close(struct wlf_window *base) {
	struct wlf_win32_window *window = win32_from_window(base);
	if (window->hwnd != NULL) {
		DestroyWindow(window->hwnd);
	}
}

static void win32_window_show(struct wlf_window *base) {
	struct wlf_win32_window *window = win32_from_window(base);
	ShowWindow(window->hwnd, SW_SHOWNORMAL);
	UpdateWindow(window->hwnd);
}

static void win32_window_hide(struct wlf_window *base) {
	ShowWindow(win32_from_window(base)->hwnd, SW_HIDE);
}

static void win32_window_set_title(struct wlf_window *base,
		const char *title) {
	struct wlf_win32_window *window = win32_from_window(base);
	wchar_t *wide = utf8_to_wide(title);
	if (wide == NULL) {
		wlf_log(WLF_ERROR, "Failed to convert Win32 window title to UTF-16");
		return;
	}
	SetWindowTextW(window->hwnd, wide);
	free(wide);
}

static void win32_window_set_geometry(struct wlf_window *base,
		const struct wlf_rect *geometry) {
	struct wlf_win32_window *window = win32_from_window(base);
	RECT rect = client_geometry_to_window(window, geometry);
	SetWindowPos(window->hwnd, NULL, rect.left, rect.top, rect.right,
		rect.bottom, SWP_NOACTIVATE | SWP_NOZORDER);
}

static void win32_window_set_size(struct wlf_window *base,
		int width, int height) {
	struct wlf_rect geometry = base->state.geometry;
	geometry.width = width;
	geometry.height = height;
	win32_window_set_geometry(base, &geometry);
}

static void win32_window_set_position(struct wlf_window *base, int x, int y) {
	SetWindowPos(win32_from_window(base)->hwnd, NULL, x, y, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

static void win32_window_set_size_limits(struct wlf_window *base,
		int width, int height) {
	WLF_UNUSED(width);
	WLF_UNUSED(height);
	SetWindowPos(win32_from_window(base)->hwnd, NULL, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
		SWP_NOACTIVATE | SWP_NOZORDER);
}

static void win32_window_set_state(struct wlf_window *base,
		enum wlf_window_state_flags state) {
	struct wlf_win32_window *window = win32_from_window(base);
	if ((state & WLF_WINDOW_FULLSCREEN) && !window->fullscreen) {
		MONITORINFO monitor = {.cbSize = sizeof(monitor)};
		window->restore_style = (DWORD)GetWindowLongPtrW(window->hwnd,
			GWL_STYLE);
		window->restore_placement.length = sizeof(window->restore_placement);
		GetWindowPlacement(window->hwnd, &window->restore_placement);
		GetMonitorInfoW(MonitorFromWindow(window->hwnd,
			MONITOR_DEFAULTTONEAREST), &monitor);
		SetWindowLongPtrW(window->hwnd, GWL_STYLE,
			(LONG_PTR)(window->restore_style & ~WS_OVERLAPPEDWINDOW));
		SetWindowPos(window->hwnd, HWND_TOP, monitor.rcMonitor.left,
			monitor.rcMonitor.top,
			monitor.rcMonitor.right - monitor.rcMonitor.left,
			monitor.rcMonitor.bottom - monitor.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
		window->fullscreen = true;
		return;
	}
	if (!(state & WLF_WINDOW_FULLSCREEN) && window->fullscreen) {
		SetWindowLongPtrW(window->hwnd, GWL_STYLE,
			(LONG_PTR)window->restore_style);
		SetWindowPlacement(window->hwnd, &window->restore_placement);
		SetWindowPos(window->hwnd, NULL, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
			SWP_NOOWNERZORDER | SWP_NOZORDER);
		window->fullscreen = false;
	}
	ShowWindow(window->hwnd, state & WLF_WINDOW_MINIMIZED ? SW_MINIMIZE :
		state & WLF_WINDOW_MAXIMIZED ? SW_MAXIMIZE : SW_RESTORE);
}

static void win32_window_set_flags(struct wlf_window *base, uint32_t flags) {
	struct wlf_win32_window *window = win32_from_window(base);
	SetWindowLongPtrW(window->hwnd, GWL_STYLE,
		(LONG_PTR)window_style(flags));
	SetWindowLongPtrW(window->hwnd, GWL_EXSTYLE,
		(LONG_PTR)window_ex_style(flags));
	HWND insert_after = flags & WLF_WINDOW_FLAG_ALWAYS_ON_TOP ? HWND_TOPMOST :
		flags & WLF_WINDOW_FLAG_ALWAYS_ON_BOTTOM ? HWND_BOTTOM : HWND_NOTOPMOST;
	SetWindowPos(window->hwnd, insert_after, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

static void win32_window_set_opacity(struct wlf_window *base, float opacity) {
	struct wlf_win32_window *window = win32_from_window(base);
	float clamped = fmaxf(0.0f, fminf(1.0f, opacity));
	LONG_PTR ex_style = GetWindowLongPtrW(window->hwnd, GWL_EXSTYLE);
	SetWindowLongPtrW(window->hwnd, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
	SetLayeredWindowAttributes(window->hwnd, 0,
		(BYTE)lroundf(clamped * 255.0f), LWA_ALPHA);
}

static void win32_window_set_background_color(struct wlf_window *base,
		const struct wlf_color *color) {
	WLF_UNUSED(color);
	InvalidateRect(win32_from_window(base)->hwnd, NULL, TRUE);
}

static void *win32_window_native_handle(struct wlf_window *base) {
	return win32_from_window(base)->hwnd;
}

static void win32_window_schedule_frame(struct wlf_window *base) {
	InvalidateRect(win32_from_window(base)->hwnd, NULL, FALSE);
}

static const struct wlf_window_impl win32_window_impl = {
	.destroy = win32_window_destroy,
	.close = win32_window_close,
	.show = win32_window_show,
	.hide = win32_window_hide,
	.set_title = win32_window_set_title,
	.set_geometry = win32_window_set_geometry,
	.set_size = win32_window_set_size,
	.set_min_size = win32_window_set_size_limits,
	.set_max_size = win32_window_set_size_limits,
	.set_position = win32_window_set_position,
	.set_state = win32_window_set_state,
	.set_flags = win32_window_set_flags,
	.set_opacity = win32_window_set_opacity,
	.set_background_color = win32_window_set_background_color,
	.native_handle = win32_window_native_handle,
	.schedule_frame = win32_window_schedule_frame,
};

static void emit_geometry_changes(struct wlf_win32_window *window,
		bool moved, bool resized) {
	RECT client;
	POINT origin = {0, 0};
	if (!GetClientRect(window->hwnd, &client) ||
			!ClientToScreen(window->hwnd, &origin)) {
		return;
	}
	window->base.state.geometry.x = origin.x;
	window->base.state.geometry.y = origin.y;
	window->base.state.geometry.width = logical_from_physical(window,
		client.right);
	window->base.state.geometry.height = logical_from_physical(window,
		client.bottom);
	if (moved) {
		wlf_signal_emit_mutable(&window->base.events.move, &window->base);
	}
	if (resized && client.right > 0 && client.bottom > 0) {
		wlf_signal_emit_mutable(&window->base.events.resize, &window->base);
	}
}

static LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT message,
		WPARAM wparam, LPARAM lparam) {
	struct wlf_win32_window *window = win32_from_hwnd(hwnd);
	if (message == WM_NCCREATE) {
		CREATESTRUCTW *create = (CREATESTRUCTW *)lparam;
		window = create->lpCreateParams;
		window->hwnd = hwnd;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)window);
	}
	if (window == NULL) {
		return DefWindowProcW(hwnd, message, wparam, lparam);
	}

	switch (message) {
	case WM_CLOSE:
		wlf_window_close(&window->base);
		return 0;
	case WM_DESTROY: {
		struct wlf_windows_backend *backend = wlf_windows_backend_from_backend(
			window->base.state.backend);
		window->hwnd = NULL;
		if (window->counted) {
			window->counted = false;
			if (backend->window_count > 0) {
				backend->window_count--;
			}
			if (backend->window_count == 0) {
				PostQuitMessage(0);
			}
		}
		return 0;
	}
	case WM_MOVE:
		emit_geometry_changes(window, true, false);
		return 0;
	case WM_SIZE:
		if (wparam == SIZE_MINIMIZED) {
			window->base.state.state |= WLF_WINDOW_MINIMIZED;
		} else {
			window->base.state.state &= ~WLF_WINDOW_MINIMIZED;
		}
		emit_geometry_changes(window, false, true);
		return 0;
	case WM_DPICHANGED: {
		UINT dpi = HIWORD(wparam);
		RECT *suggested = (RECT *)lparam;
		wlf_window_set_scale(&window->base, (double)dpi / 96.0);
		SetWindowPos(hwnd, NULL, suggested->left, suggested->top,
			suggested->right - suggested->left,
			suggested->bottom - suggested->top,
			SWP_NOACTIVATE | SWP_NOZORDER);
		return 0;
	}
	case WM_DISPLAYCHANGE: {
		struct wlf_windows_backend *backend = wlf_windows_backend_from_backend(
			window->base.state.backend);
		DWORD time = (DWORD)GetMessageTime();
		if (backend->display_change_time != time) {
			backend->display_change_time = time;
			wlf_windows_backend_refresh_outputs(backend);
		}
		return 0;
	}
	case WM_SETFOCUS:
		window->base.state.focused = true;
		window->base.state.state |= WLF_WINDOW_ACTIVE;
		wlf_signal_emit_mutable(&window->base.events.focus_in, &window->base);
		handle_keyboard_focus(window, true);
		return 0;
	case WM_KILLFOCUS:
		window->base.state.focused = false;
		window->base.state.state &= ~WLF_WINDOW_ACTIVE;
		wlf_signal_emit_mutable(&window->base.events.focus_out, &window->base);
		handle_keyboard_focus(window, false);
		return 0;
	case WM_MOUSEMOVE:
		handle_pointer_motion(window, lparam);
		return 0;
	case WM_MOUSELEAVE:
		handle_pointer_leave(window);
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		handle_pointer_button(window, message, true);
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		handle_pointer_button(window, message, false);
		return 0;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		handle_pointer_axis(window, message, wparam);
		return 0;
	case WM_KEYDOWN:
		handle_keyboard_key(window, lparam, true);
		return 0;
	case WM_SYSKEYDOWN:
		handle_keyboard_key(window, lparam, true);
		return DefWindowProcW(hwnd, message, wparam, lparam);
	case WM_KEYUP:
		handle_keyboard_key(window, lparam, false);
		return 0;
	case WM_SYSKEYUP:
		handle_keyboard_key(window, lparam, false);
		return DefWindowProcW(hwnd, message, wparam, lparam);
	case WM_SETCURSOR:
		if (LOWORD(lparam) == HTCLIENT && window->cursor_handle != NULL) {
			SetCursor(window->cursor_handle);
			return TRUE;
		}
		return DefWindowProcW(hwnd, message, wparam, lparam);
	case WM_POINTERDOWN:
	case WM_POINTERUPDATE:
	case WM_POINTERUP:
		handle_touch_pointer(window, message, wparam);
		return 0;
	case WM_POINTERCAPTURECHANGED:
		handle_touch_cancel(window, wparam);
		return 0;
	case WM_GETMINMAXINFO: {
		MINMAXINFO *info = (MINMAXINFO *)lparam;
		if (window->base.state.min_size.width > 0) {
			info->ptMinTrackSize.x = physical_from_logical(window,
				window->base.state.min_size.width);
		}
		if (window->base.state.min_size.height > 0) {
			info->ptMinTrackSize.y = physical_from_logical(window,
				window->base.state.min_size.height);
		}
		if (window->base.state.max_size.width > 0) {
			info->ptMaxTrackSize.x = physical_from_logical(window,
				window->base.state.max_size.width);
		}
		if (window->base.state.max_size.height > 0) {
			info->ptMaxTrackSize.y = physical_from_logical(window,
				window->base.state.max_size.height);
		}
		return 0;
	}
	case WM_ERASEBKGND: {
		const struct wlf_color *color = &window->base.state.background_color;
		HBRUSH brush = CreateSolidBrush(RGB((BYTE)(color->r * 255.0),
			(BYTE)(color->g * 255.0), (BYTE)(color->b * 255.0)));
		RECT rect;
		GetClientRect(hwnd, &rect);
		FillRect((HDC)wparam, &rect, brush);
		DeleteObject(brush);
		return 1;
	}
	case WM_PAINT: {
		PAINTSTRUCT paint;
		BeginPaint(hwnd, &paint);
		EndPaint(hwnd, &paint);
		wlf_signal_emit_mutable(&window->base.events.expose, &window->base);
		return 0;
	}
	default:
		return DefWindowProcW(hwnd, message, wparam, lparam);
	}
}

static bool ensure_window_class(struct wlf_windows_backend *backend) {
	WNDCLASSEXW existing;
	if (GetClassInfoExW(backend->instance, win32_window_class_name,
			&existing)) {
		return true;
	}
	WNDCLASSEXW window_class = {
		.cbSize = sizeof(window_class),
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = win32_window_proc,
		.hInstance = backend->instance,
		.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512)),
		.lpszClassName = win32_window_class_name,
	};
	if (RegisterClassExW(&window_class) == 0) {
		wlf_log(WLF_ERROR, "RegisterClassExW failed: %lu",
			(unsigned long)GetLastError());
		return false;
	}
	return true;
}

struct wlf_window *wlf_win32_window_create_from_backend(
		struct wlf_backend *backend, uint32_t width, uint32_t height) {
	if (!wlf_backend_is_windows(backend) || width == 0 || height == 0) {
		return NULL;
	}
	struct wlf_windows_backend *windows =
		wlf_windows_backend_from_backend(backend);
	if (windows->thread_id != GetCurrentThreadId()) {
		wlf_log(WLF_ERROR, "Win32 windows must be created on the backend thread");
		return NULL;
	}
	if (!ensure_window_class(windows)) {
		return NULL;
	}
	struct wlf_win32_window *window = calloc(1, sizeof(*window));
	if (window == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Win32 window");
		return NULL;
	}
	wlf_window_init(&window->base, WLF_WINDOW_TYPE_TOPLEVEL,
		&win32_window_impl, backend, width, height);
	wlf_pointer_init(&window->pointer, &win32_pointer_impl);
	wlf_keyboard_init(&window->keyboard, &win32_keyboard_impl);
	wlf_touch_init(&window->touch, &win32_touch_impl);
	wlf_cursor_init(&window->cursor, &win32_cursor_impl);
	window->pointer.cursor = &window->cursor;
	window->cursor_handle = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
	UINT initial_dpi = GetDpiForSystem();
	RECT rect = {0, 0,
		MulDiv((int)width, (int)initial_dpi, 96),
		MulDiv((int)height, (int)initial_dpi, 96)};
	DWORD style = window_style(window->base.state.flags);
	DWORD ex_style = window_ex_style(window->base.state.flags);
	AdjustWindowRectEx(&rect, style, FALSE, ex_style);
	window->hwnd = CreateWindowExW(ex_style, win32_window_class_name,
		L"wlframe", style, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL,
		windows->instance, window);
	if (window->hwnd == NULL) {
		wlf_log(WLF_ERROR, "CreateWindowExW failed: %lu",
			(unsigned long)GetLastError());
		window->hwnd = NULL;
		wlf_window_destroy(&window->base);
		return NULL;
	}
	window->counted = true;
	windows->window_count++;
	window->base.state.geometry.width = (int)width;
	window->base.state.geometry.height = (int)height;
	wlf_window_set_scale(&window->base,
		(double)GetDpiForWindow(window->hwnd) / 96.0);
	return &window->base;
}

bool wlf_window_is_win32(const struct wlf_window *window) {
	return window != NULL && window->impl == &win32_window_impl;
}

static struct wlf_win32_window *win32_from_window(struct wlf_window *base) {
	assert(wlf_window_is_win32(base));
	struct wlf_win32_window *window = NULL;
	return wlf_container_of(base, window, base);
}

struct wlf_win32_window *wlf_win32_window_from_window(
		struct wlf_window *window) {
	return wlf_window_is_win32(window) ? win32_from_window(window) : NULL;
}
