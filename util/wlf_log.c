#include "wlf/util/wlf_log.h"
#include "wlf/util/wlf_time.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <wayland-server-core.h>

static bool colored = true;
static enum wlf_log_importance log_importance = WLF_ERROR;
static struct timespec start_time = {-1};

static const char *verbosity_colors[] = {
	[WLF_SILENT] = "",
	[WLF_ERROR] = "\x1B[1;31m",
	[WLF_INFO] = "\x1B[1;34m",
	[WLF_DEBUG] = "\x1B[1;90m",
};

static const char *verbosity_headers[] = {
	[WLF_SILENT] = "",
	[WLF_ERROR] = "[ERROR]",
	[WLF_INFO] = "[INFO]",
	[WLF_DEBUG] = "[DEBUG]",
};

static void init_start_time(void) {
	if (start_time.tv_sec >= 0) {
		return;
	}
	clock_gettime(CLOCK_MONOTONIC, &start_time);
}

static void log_stderr(enum wlf_log_importance verbosity, const char *fmt,
		va_list args) {
	init_start_time();

	if (verbosity > log_importance) {
		return;
	}

	struct timespec ts = {0};
	clock_gettime(CLOCK_MONOTONIC, &ts);
	timespec_sub(&ts, &ts, &start_time);

	fprintf(stderr, "%02d:%02d:%02d.%03ld ", (int)(ts.tv_sec / 60 / 60),
		(int)(ts.tv_sec / 60 % 60), (int)(ts.tv_sec % 60),
		ts.tv_nsec / 1000000);

	unsigned c = (verbosity < WLF_LOG_IMPORTANCE_LAST) ? verbosity : WLF_LOG_IMPORTANCE_LAST - 1;

	if (colored && isatty(STDERR_FILENO)) {
		fprintf(stderr, "%s", verbosity_colors[c]);
	} else {
		fprintf(stderr, "%s ", verbosity_headers[c]);
	}

	vfprintf(stderr, fmt, args);

	if (colored && isatty(STDERR_FILENO)) {
		fprintf(stderr, "\x1B[0m");
	}
	fprintf(stderr, "\n");
}

static wlf_log_func_t log_callback = log_stderr;

static void log_wl(const char *fmt, va_list args) {
	static char wlf_fmt[1024];
	int n = snprintf(wlf_fmt, sizeof(wlf_fmt), "[wayland] %s", fmt);
	size_t len = strlen(wlf_fmt);
	if (n > 0 && wlf_fmt[len - 1] == '\n') {
		wlf_fmt[len - 1] = '\0';
	}
	_wlf_vlog(WLF_INFO, wlf_fmt, args);
}

void wlf_log_init(enum wlf_log_importance verbosity, wlf_log_func_t callback) {
	init_start_time();

	if (verbosity < WLF_LOG_IMPORTANCE_LAST) {
		log_importance = verbosity;
	}
	if (callback) {
		log_callback = callback;
	}

	wl_log_set_handler_server(log_wl);
}

void _wlf_vlog(enum wlf_log_importance verbosity, const char *fmt, va_list args) {
	log_callback(verbosity, fmt, args);
}

void _wlf_log(enum wlf_log_importance verbosity, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	log_callback(verbosity, fmt, args);
	va_end(args);
}

enum wlf_log_importance wlf_log_get_verbosity(void) {
	return log_importance;
}
