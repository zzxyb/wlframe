#include "wlf/utils/wlf_time.h"
#include "wlf/config.h"

#include <stdbool.h>

#if WLF_HAS_WINDOWS_PLATFORM
#include <windows.h>
#endif

static const long NSEC_PER_SEC = 1000000000;

void wlf_get_monotonic_time(struct timespec *ts) {
#if WLF_HAS_WINDOWS_PLATFORM
	static LARGE_INTEGER frequency;
	static bool initialized = false;
	LARGE_INTEGER counter;

	if (!initialized) {
		QueryPerformanceFrequency(&frequency);
		initialized = true;
	}

	QueryPerformanceCounter(&counter);
	ts->tv_sec = (time_t)(counter.QuadPart / frequency.QuadPart);
	ts->tv_nsec = (long)((counter.QuadPart % frequency.QuadPart) *
		NSEC_PER_SEC / frequency.QuadPart);
#else
	clock_gettime(CLOCK_MONOTONIC, ts);
#endif
}

int64_t timespec_to_msec(const struct timespec *a) {
	return (int64_t)a->tv_sec * 1000 + a->tv_nsec / 1000000;
}

int64_t timespec_to_nsec(const struct timespec *a) {
	return (int64_t)a->tv_sec * NSEC_PER_SEC + a->tv_nsec;
}

void timespec_from_nsec(struct timespec *r, int64_t nsec) {
	r->tv_sec = nsec / NSEC_PER_SEC;
	r->tv_nsec = nsec % NSEC_PER_SEC;
}

int64_t get_current_time_msec(void) {
	struct timespec now;
	wlf_get_monotonic_time(&now);
	return timespec_to_msec(&now);
}

void timespec_sub(struct timespec *r, const struct timespec *a,
		const struct timespec *b) {
	r->tv_sec = a->tv_sec - b->tv_sec;
	r->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (r->tv_nsec < 0) {
		r->tv_sec--;
		r->tv_nsec += NSEC_PER_SEC;
	}
}
