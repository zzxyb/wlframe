#ifndef ANIMATOR_WLF_ANIMATOR_H
#define ANIMATOR_WLF_ANIMATOR_H

#include "wlf/animator/wlf_animator_curve.h"

#include <stdbool.h>

struct wlf_animator_impl {
	void (*start)(struct wlf_animator *animator); /**< Function to start the animation */
	void (*stop)(struct wlf_animator *animator); /**< Function to stop the animation */
	void (*update)(struct wlf_animator *animator); /**< Function to update the animation */
	bool (*write_back)(struct wlf_animator *animator); /**< Function to write back the current value */
	void (*pre_sync)(struct wlf_animator *animator); /**< Function to prepare for synchronization before the animation starts */
	void (*post_sync)(struct wlf_animator *animator); /**< Function to finalize synchronization after the animation ends */
	void (*destroy)(struct wlf_animator *animator); /**< Function to destroy the animator */
};

struct wlf_animator {
	const struct wlf_animator_impl *impl; /**< Pointer to the implementation */

	struct wlf_animator_curve curve; /**< Animation curve used for the animation */
	double duration; /**< Duration of the animation in seconds */
	double current; /**< Current value of the animation */
};

struct wlf_animator *wlf_animator_create(double duration, struct wlf_animator_curve curve);
void wlf_animator_destroy(struct wlf_animator *animator);

#endif // ANIMATOR_WLF_ANIMATOR_H
