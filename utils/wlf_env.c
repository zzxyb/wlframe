#include "wlf/utils/wlf_env.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>
#include <string.h>

const char* wlf_get_env(const char *name) {
	return getenv(name);
}

bool wlf_set_env(const char *name, const char *value) {
	return setenv(name, value, 1) == 0;
}

bool wlf_unset_env(const char *name) {
	return unsetenv(name) == 0;
}

bool wlf_env_parse_bool(const char *option) {
	const char *env = wlf_get_env(option);
	if (env) {
		wlf_log(WLF_INFO, "Loading %s option: %s", option, env);
	}

	if (env == NULL || strcmp(env, "0") == 0) {
		return false;
	} else if (strcmp(env, "1") == 0) {
		return true;
	}

	wlf_log(WLF_ERROR, "Unknown %s option: %s", option, env);
	return false;
}

size_t wlf_env_parse_switch(const char *option, const char **switches) {
	const char *env = wlf_get_env(option);
	if (env) {
		wlf_log(WLF_INFO, "Loading %s option: %s", option, env);
	} else {
		return 0;
	}

	for (ssize_t i = 0; switches[i]; i++) {
		if (strcmp(env, switches[i]) == 0) {
			return i;
		}
	}

	wlf_log(WLF_ERROR, "Unknown %s option: %s", option, env);
	return 0;
}
