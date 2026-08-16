#include "wlf/platform/windows/output.h"

#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <shellscalingapi.h>

static char *wide_to_utf8(const wchar_t *wide) {
	int count = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0,
		NULL, NULL);
	if (count == 0) {
		return NULL;
	}
	char *text = calloc((size_t)count, sizeof(*text));
	if (text == NULL || WideCharToMultiByte(CP_UTF8, 0, wide, -1, text,
			count, NULL, NULL) == 0) {
		free(text);
		return NULL;
	}
	return text;
}

static enum wlf_output_transform output_transform(DWORD orientation) {
	switch (orientation) {
	case DMDO_90:
		return WLF_OUTPUT_TRANSFORM_90;
	case DMDO_180:
		return WLF_OUTPUT_TRANSFORM_180;
	case DMDO_270:
		return WLF_OUTPUT_TRANSFORM_270;
	default:
		return WLF_OUTPUT_TRANSFORM_NORMAL;
	}
}

static void windows_output_destroy(struct wlf_output *base) {
	struct wlf_windows_output *output =
		wlf_windows_output_from_output(base);
	if (output->base.link.next != NULL && output->base.link.prev != NULL) {
		wlf_linked_list_remove(&output->base.link);
	}
	free(output->base.name);
	free(output->base.model);
	free(output->base.manufacturer);
	free(output->base.description);
	free(output);
}

static const struct wlf_output_impl windows_output_impl = {
	.type = WLF_OUTPUT,
	.destroy = windows_output_destroy,
};

struct wlf_output *wlf_windows_output_create(HMONITOR monitor) {
	MONITORINFOEXW info = {.cbSize = sizeof(info)};
	if (!GetMonitorInfoW(monitor, (MONITORINFO *)&info)) {
		return NULL;
	}
	struct wlf_windows_output *output = calloc(1, sizeof(*output));
	if (output == NULL) {
		wlf_log_errno(WLF_ERROR, "Failed to allocate Windows output");
		return NULL;
	}
	wlf_output_init(&output->base, &windows_output_impl);
	output->monitor = monitor;
	output->base.name = wide_to_utf8(info.szDevice);
	output->base.description = output->base.name != NULL ?
		strdup(output->base.name) : NULL;
	output->base.geometry = (struct wlf_rect){
		.x = info.rcMonitor.left,
		.y = info.rcMonitor.top,
		.width = info.rcMonitor.right - info.rcMonitor.left,
		.height = info.rcMonitor.bottom - info.rcMonitor.top,
	};
	output->base.scale = 1;
	output->base.transform = WLF_OUTPUT_TRANSFORM_NORMAL;
	output->base.subpixel = WLF_OUTPUT_SUBPIXEL_UNKNOWN;

	DEVMODEW mode = {.dmSize = sizeof(mode)};
	if (EnumDisplaySettingsW(info.szDevice, ENUM_CURRENT_SETTINGS, &mode)) {
		output->base.refresh_rate = mode.dmDisplayFrequency > 1 ?
			(int)mode.dmDisplayFrequency * 1000 : 0;
		output->base.transform = output_transform(mode.dmDisplayOrientation);
	}

	UINT dpi_x = 96;
	UINT dpi_y = 96;
	if (SUCCEEDED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI,
			&dpi_x, &dpi_y))) {
		output->base.scale = (int)((dpi_x + 48) / 96);
		if (output->base.scale < 1) {
			output->base.scale = 1;
		}
	}

	HDC dc = CreateDCW(L"DISPLAY", info.szDevice, NULL, NULL);
	if (dc != NULL) {
		output->base.physical_size.width = GetDeviceCaps(dc, HORZSIZE);
		output->base.physical_size.height = GetDeviceCaps(dc, VERTSIZE);
		DeleteDC(dc);
	}
	return &output->base;
}

bool wlf_output_is_windows(const struct wlf_output *output) {
	return output != NULL && output->impl == &windows_output_impl;
}

struct wlf_windows_output *wlf_windows_output_from_output(
		struct wlf_output *base) {
	assert(wlf_output_is_windows(base));
	struct wlf_windows_output *output = NULL;
	return wlf_container_of(base, output, base);
}
