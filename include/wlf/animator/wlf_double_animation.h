/**
 * @file        wlf_double_animation.h
 * @brief       Generic double-valued property animation for wlframe.
 */

#ifndef ANIMATOR_WLF_DOUBLE_ANIMATION_H
#define ANIMATOR_WLF_DOUBLE_ANIMATION_H

#include "wlf/animator/wlf_animator_job.h"
#include "wlf/curve/wlf_curve.h"

#include <stdbool.h>

typedef double (*wlf_double_animation_getter_t)(void *target, void *user_data);
typedef void (*wlf_double_animation_setter_t)(void *target, double value,
	void *user_data);

struct wlf_double_animation_binding {
	void *target;
	void *user_data;
	wlf_double_animation_getter_t get;
	wlf_double_animation_setter_t set;
};

extern const struct wlf_animator_job_type WLF_ANIMATOR_JOB_TYPE_DOUBLE_ANIMATION;

struct wlf_double_animation {
	struct wlf_animator_job base;
	struct wlf_double_animation_binding binding;
	struct wlf_curve *curve;
	double from;
	double to;
	double value;
	bool has_explicit_from;
	bool owns_curve;
};

struct wlf_double_animation *wlf_double_animation_create(void);
void wlf_double_animation_init(struct wlf_double_animation *animation);
void wlf_double_animation_init_with_type(struct wlf_double_animation *animation,
	const struct wlf_animator_job_type *type);
void wlf_double_animation_fini(struct wlf_double_animation *animation);
void wlf_double_animation_destroy(struct wlf_double_animation *animation);

bool wlf_animator_job_is_double_animation(const struct wlf_animator_job *job);
struct wlf_double_animation *wlf_double_animation_from_job(
	struct wlf_animator_job *job);

void wlf_double_animation_set_binding(struct wlf_double_animation *animation,
	const struct wlf_double_animation_binding *binding);
void wlf_double_animation_set_curve(struct wlf_double_animation *animation,
	struct wlf_curve *curve, bool take_ownership);
void wlf_double_animation_set_from(struct wlf_double_animation *animation,
	double from);
void wlf_double_animation_set_to(struct wlf_double_animation *animation,
	double to);
void wlf_double_animation_clear_from(struct wlf_double_animation *animation);

#endif // ANIMATOR_WLF_DOUBLE_ANIMATION_H
