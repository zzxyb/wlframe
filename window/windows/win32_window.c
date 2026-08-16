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
	RECT rect = {0, 0, geometry->width, geometry->height};
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
	window->base.state.geometry.width = client.right;
	window->base.state.geometry.height = client.bottom;
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
	case WM_SETFOCUS:
		window->base.state.focused = true;
		window->base.state.state |= WLF_WINDOW_ACTIVE;
		wlf_signal_emit_mutable(&window->base.events.focus_in, &window->base);
		return 0;
	case WM_KILLFOCUS:
		window->base.state.focused = false;
		window->base.state.state &= ~WLF_WINDOW_ACTIVE;
		wlf_signal_emit_mutable(&window->base.events.focus_out, &window->base);
		return 0;
	case WM_GETMINMAXINFO: {
		MINMAXINFO *info = (MINMAXINFO *)lparam;
		if (window->base.state.min_size.width > 0) {
			info->ptMinTrackSize.x = window->base.state.min_size.width;
		}
		if (window->base.state.min_size.height > 0) {
			info->ptMinTrackSize.y = window->base.state.min_size.height;
		}
		if (window->base.state.max_size.width > 0) {
			info->ptMaxTrackSize.x = window->base.state.max_size.width;
		}
		if (window->base.state.max_size.height > 0) {
			info->ptMaxTrackSize.y = window->base.state.max_size.height;
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
	RECT rect = {0, 0, (LONG)width, (LONG)height};
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
