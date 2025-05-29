#ifndef ANIMATOR_WLF_X_ANIMATOR_H
#define ANIMATOR_WLF_X_ANIMATOR_H

struct wlf_animator;

struct wlf_x_animator {
	struct wlf_animator *base; /**< Pointer to the base animator */

	float x; /**< Current X position of the animator */
	float from; /**< Starting X position */
	float to; /**< Ending X position */
};

struct wlf_x_animator *wlf_x_animator_create(double duration, float from, float to);
void wlf_x_animator_destroy(struct wlf_x_animator *animator);

#endif // ANIMATOR_WLF_X_ANIMATOR_H
