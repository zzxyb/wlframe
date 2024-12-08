#include "wlf/utils/wlf_utils.h"
#include "wlf/utils/wlf_log.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

bool generate_token(char out[static TOKEN_SIZE]) {
	static FILE *urandom = NULL;
	uint64_t data[2];

	if (!urandom) {
		int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (fd < 0) {
			wlf_log(WLF_ERROR, "Failed to open random device");
			return false;
		}
		if (!(urandom = fdopen(fd, "r"))) {
			wlf_log(WLF_ERROR, "fdopen failed");
			close(fd);
			return false;
		}
	}
	if (fread(data, sizeof(data), 1, urandom) != 1) {
		wlf_log(WLF_ERROR, "Failed to read from random device");
		return false;
	}
	if (snprintf(out, TOKEN_SIZE, "%016" PRIx64 "%016" PRIx64, data[0], data[1]) != TOKEN_SIZE - 1) {
		wlf_log(WLF_ERROR, "Failed to format hex string token");
		return false;
	}
	return true;
}

static bool in_range(char x, uint8_t low, uint8_t high) {
	uint8_t v = (uint8_t)x;
	return low <= v && v <= high;
}

bool is_utf8(const char *string) {
	/* Returns true iff the string is 'well-formed', as defined by
	 * Unicode Standard 15.0.0. See Chapter 3, D92 and Table 3.7.
	 *
	 * UTF-8 strings are sequences of code points encoded in one of the
	 * following ways. The first byte determines the pattern.
	 *
	 * 00..7F
	 * C2..DF 80..BF
	 * E0     A0..BF 80..BF
	 * E1..EC 80..BF 80..BF
	 * ED     80..9F 80..BF
	 * EE..EF 80..BF 80..BF
	 * F0     90..BF 80..BF 80..BF
	 * F1..F3 80..BF 80..BF 80..BF
	 * F4     80..8F 80..BF 80..BF
	 */
	uint8_t range_table[9][8] = {
		{0x00, 0x7F},
		{0xC2, 0xDF, 0x80, 0xBF},
		{0xE0, 0xE0, 0xA0, 0xBF, 0x80, 0xBF},
		{0xE1, 0xEC, 0x80, 0xBF, 0x80, 0xBF},
		{0xED, 0xED, 0x80, 0x9F, 0x80, 0xBF},
		{0xEE, 0xEF, 0x80, 0xBF, 0x80, 0xBF},
		{0xF0, 0xF0, 0x90, 0xBF, 0x80, 0xBF, 0x80, 0xBF},
		{0xF1, 0xF3, 0x80, 0xBF, 0x80, 0xBF, 0x80, 0xBF},
		{0xF4, 0xF4, 0x80, 0x8F, 0x80, 0xBF, 0x80, 0xBF},
	};
	int lengths[9] = {
		1, 2, 3, 3, 3, 3, 4, 4, 4
	};

	while (string[0]) {
		bool accept = false;
		for (int i = 0; i < 9; i++) {
			if (!in_range(string[0], range_table[i][0],
					range_table[i][1])) {
				continue;
			}
			for (int j = 1; j < lengths[i]; j++) {
				if (!in_range(string[j], range_table[i][2 * j],
						range_table[i][2 * j + 1])) {
					// Early exit is necessary to avoid
					// reading past the null terminator
					return false;
				}
			}
			string += lengths[i];
			accept = true;
			break;
		}
		if (!accept) {
			return false;
		}
	}

	return true;
}

ssize_t set_add(uint32_t values[], size_t *len, size_t cap, uint32_t target) {
	for (uint32_t i = 0; i < *len; ++i) {
		if (values[i] == target) {
			return i;
		}
	}
	if (*len == cap) {
		return -1;
	}
	values[*len] = target;
	return (*len)++;
}

ssize_t set_remove(uint32_t values[], size_t *len, size_t cap, uint32_t target) {
	for (uint32_t i = 0; i < *len; ++i) {
		if (values[i] == target) {
			--(*len);
			values[i] = values[*len];
			return i;
		}
	}
	return -1;
}
