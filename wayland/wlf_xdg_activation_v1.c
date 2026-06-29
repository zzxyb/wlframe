#include "wlf/wayland/wlf_xdg_activation_v1.h"
#include "wayland/protocols/xdg-activation-v1-client-protocol.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/utils/wlf_signal.h"
#include "wlf/utils/wlf_utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void token_handle_done(void *data, struct xdg_activation_token_v1 *base,
		const char *token) {
	WLF_UNUSED(base);

	struct wlf_xdg_activation_token_v1 *tok = data;
	tok->done = true;
	wlf_signal_emit_mutable(&tok->events.done, (void *)token);
}

static const struct xdg_activation_token_v1_listener token_listener = {
	.done = token_handle_done,
};

struct wlf_xdg_activation_v1 *wlf_xdg_activation_v1_create(
		struct wl_registry *wl_registry,
		uint32_t name,
		uint32_t version) {
	assert(wl_registry);

	uint32_t bind_ver = (uint32_t)xdg_activation_v1_interface.version;
	if (version < bind_ver) {
		bind_ver = version;
	}

	struct wlf_xdg_activation_v1 *activation =
		calloc(1, sizeof(*activation));
	if (activation == NULL) {
		wlf_log_errno(
			WLF_ERROR, "failed to allocate wlf_xdg_activation_v1");
		return NULL;
	}

	activation->base = wl_registry_bind(
		wl_registry, name, &xdg_activation_v1_interface, bind_ver);
	if (activation->base == NULL) {
		wlf_log(WLF_ERROR,
			"wl_registry_bind failed for xdg_activation_v1 (name: "
			"%u)",
			name);
		free(activation);
		return NULL;
	}
	activation->version = bind_ver;

	wlf_signal_init(&activation->events.destroy);

	wlf_log(WLF_DEBUG, "bound xdg_activation_v1 (name: %u, version: %u)",
		name, bind_ver);

	return activation;
}

void wlf_xdg_activation_v1_destroy(struct wlf_xdg_activation_v1 *activation) {
	if (activation == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&activation->events.destroy, activation);
	assert(wlf_linked_list_empty(&activation->events.destroy.listener_list));
	if (activation->base != NULL) {
		xdg_activation_v1_destroy(activation->base);
	}
	free(activation);
}

struct wlf_xdg_activation_token_v1 *wlf_xdg_activation_v1_get_activation_token(
		struct wlf_xdg_activation_v1 *activation) {
	assert(activation);
	assert(activation->base);

	struct wlf_xdg_activation_token_v1 *tok = calloc(1, sizeof(*tok));
	if (tok == NULL) {
		wlf_log_errno(WLF_ERROR,
			"failed to allocate wlf_xdg_activation_token_v1");
		return NULL;
	}

	tok->base = xdg_activation_v1_get_activation_token(activation->base);
	if (tok->base == NULL) {
		wlf_log(WLF_ERROR,
			"xdg_activation_v1_get_activation_token() returned "
			"NULL");
		free(tok);
		return NULL;
	}
	tok->version = wl_proxy_get_version((struct wl_proxy *)tok->base);

	wlf_signal_init(&tok->events.done);
	wlf_signal_init(&tok->events.destroy);

	xdg_activation_token_v1_add_listener(tok->base, &token_listener, tok);
	return tok;
}

void wlf_xdg_activation_v1_activate(struct wlf_xdg_activation_v1 *activation,
		const char *token,
		struct wl_surface *surface) {
	assert(activation);
	assert(activation->base);
	assert(token);
	assert(surface);

	xdg_activation_v1_activate(activation->base, token, surface);
}

void wlf_xdg_activation_token_v1_set_serial(
		struct wlf_xdg_activation_token_v1 *token,
		uint32_t serial,
		struct wl_seat *seat) {
	assert(token);
	assert(token->base);
	assert(seat);
	assert(!token->committed);

	xdg_activation_token_v1_set_serial(token->base, serial, seat);
}

void wlf_xdg_activation_token_v1_set_app_id(
		struct wlf_xdg_activation_token_v1 *token,
		const char *app_id) {
	assert(token);
	assert(token->base);
	assert(app_id);
	assert(!token->committed);

	xdg_activation_token_v1_set_app_id(token->base, app_id);
}

void wlf_xdg_activation_token_v1_set_surface(
		struct wlf_xdg_activation_token_v1 *token,
		struct wl_surface *surface) {
	assert(token);
	assert(token->base);
	assert(surface);
	assert(!token->committed);

	xdg_activation_token_v1_set_surface(token->base, surface);
}

void wlf_xdg_activation_token_v1_commit(struct wlf_xdg_activation_token_v1 *token) {
	assert(token);
	assert(token->base);
	assert(!token->committed);

	token->committed = true;
	xdg_activation_token_v1_commit(token->base);
}

void wlf_xdg_activation_token_v1_destroy(struct wlf_xdg_activation_token_v1 *token) {
	if (token == NULL) {
		return;
	}

	wlf_signal_emit_mutable(&token->events.destroy, token);
	assert(wlf_linked_list_empty(&token->events.done.listener_list));
	assert(wlf_linked_list_empty(&token->events.destroy.listener_list));
	if (token->base != NULL) {
		xdg_activation_token_v1_destroy(token->base);
	}
	free(token);
}
