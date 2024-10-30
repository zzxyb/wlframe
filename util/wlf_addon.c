#include "wlf/util/wlf_addon.h"
#include "wlf/util/wlf_log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>

void wlf_addon_set_init(struct wlf_addon_set *set) {
	*set = (struct wlf_addon_set){0};
	wl_list_init(&set->addons);
}

void wlf_addon_set_finish(struct wlf_addon_set *set) {
	while (!wl_list_empty(&set->addons)) {
		struct wl_list *link = set->addons.next;
		struct wlf_addon *addon = wl_container_of(link, addon, link);
		const struct wlf_addon_interface *impl = addon->impl;
		addon->impl->destroy(addon);
		if (set->addons.next == link) {
			wlf_log(WLF_ERROR, "Dangling addon: %s", impl->name);
			abort();
		}
	}
}

void wlf_addon_init(struct wlf_addon *addon, struct wlf_addon_set *set,
		const void *owner, const struct wlf_addon_interface *impl) {
	assert(impl);
	*addon = (struct wlf_addon){
		.impl = impl,
		.owner = owner,
	};
	struct wlf_addon *iter;
	wl_list_for_each(iter, &set->addons, link) {
		if (iter->owner == addon->owner && iter->impl == addon->impl) {
			assert(0 && "Can't have two addons of the same type with the same owner");
		}
	}
	wl_list_insert(&set->addons, &addon->link);
}

void wlf_addon_finish(struct wlf_addon *addon) {
	wl_list_remove(&addon->link);
}

struct wlf_addon *wlf_addon_find(struct wlf_addon_set *set, const void *owner,
		const struct wlf_addon_interface *impl) {
	struct wlf_addon *addon;
	wl_list_for_each(addon, &set->addons, link) {
		if (addon->owner == owner && addon->impl == impl) {
			return addon;
		}
	}
	return NULL;
}