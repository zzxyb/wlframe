#include "wlf/utils/wlf_double_list.h"
#include "wlf/utils/wlf_signal.h"

#include <stdint.h>

struct wlf_seat {
	char *name;
	uint32_t capabilities;
	uint32_t accumulated_capabilities;

	void *data;
};
