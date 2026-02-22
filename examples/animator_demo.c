/**
 * Example demonstrating the wlframe animator system
 * Based on Qt Quick's animation framework
 */

#define _DEFAULT_SOURCE
#include "wlf/animator/wlf_animator.h"
#include "wlf/animator/wlf_opacity_animator.h"
#include "wlf/animator/wlf_x_animator.h"
#include "wlf/animator/wlf_y_animator.h"
#include "wlf/animator/wlf_rotation_animator.h"
#include "wlf/animator/wlf_scale_animator.h"
#include "wlf/utils/wlf_linked_list.h"
#include <stdio.h>
#include <unistd.h>

/* Custom listener with user data */
struct animator_listener {
	struct wlf_listener listener;
	const char *name;
};

/* Animation signal listeners */
static void on_animation_started(struct wlf_listener *listener, void *data) {
	struct wlf_animator *animator = data;
	(void)animator;
	struct animator_listener *anim_listener = wlf_container_of(listener, anim_listener, listener);
	printf("Animation started: %s\n", anim_listener->name);
}

static void on_animation_finished(struct wlf_listener *listener, void *data) {
	struct wlf_animator *animator = data;
	(void)animator;
	struct animator_listener *anim_listener = wlf_container_of(listener, anim_listener, listener);
	printf("Animation finished: %s\n", anim_listener->name);
}

static void on_animation_updated(struct wlf_listener *listener, void *data) {
	struct wlf_animator *animator = data;
	(void)listener;
	float progress = wlf_animator_get_progress(animator);
	printf("  Progress: %.2f\n", progress);
}

/* Example 1: Simple opacity animation */
static void example_opacity_animation(void) {
	printf("\n=== Example 1: Opacity Animation ===\n");

	float opacity = 0.0f;
	struct wlf_opacity_animator *animator = wlf_opacity_animator_create(
		1000,  // 1 second duration
		0.0f,  // from fully transparent
		1.0f,  // to fully opaque
		&opacity
	);

	struct wlf_animator *base = wlf_opacity_animator_get_base(animator);

	// Set easing curve
	wlf_animator_set_curve(base, wlf_animator_curve_out_cubic_create());

	// Add signal listeners
	struct animator_listener started_listener = {{.notify = on_animation_started}, .name = "Opacity"};
	struct animator_listener finished_listener = {{.notify = on_animation_finished}, .name = "Opacity"};
	struct wlf_listener updated_listener = {.notify = on_animation_updated};
	wlf_signal_add(&base->events.started, &started_listener.listener);
	wlf_signal_add(&base->events.finished, &finished_listener.listener);
	wlf_signal_add(&base->events.updated, &updated_listener);

	// Start animation
	wlf_animator_start(base);

	// Simulate animation loop (update every 16ms for ~60fps)
	while (wlf_animator_is_running(base)) {
		wlf_animator_update(base, 16);
		printf("Opacity: %.3f\n", opacity);
		usleep(16000);  // 16ms
	}

	wlf_opacity_animator_destroy(animator);
}

/* Example 2: Position animation with bounce effect */
static void example_position_animation(void) {
	printf("\n=== Example 2: Position Animation (Bounce) ===\n");

	float x = 0.0f;
	float y = 0.0f;

	struct wlf_x_animator *x_animator = wlf_x_animator_create(
		1500,  // 1.5 seconds
		0.0f,
		800.0f,
		&x
	);

	struct wlf_y_animator *y_animator = wlf_y_animator_create(
		1500,
		0.0f,
		600.0f,
		&y
	);

	struct wlf_animator *x_base = wlf_x_animator_get_base(x_animator);
	struct wlf_animator *y_base = wlf_y_animator_get_base(y_animator);

	// Use bounce easing for fun effect
	wlf_animator_set_curve(x_base, wlf_animator_curve_out_bounce_create());
	wlf_animator_set_curve(y_base, wlf_animator_curve_out_bounce_create());

	// Start both animations
	wlf_animator_start(x_base);
	wlf_animator_start(y_base);

	// Run animation loop
	int frame = 0;
	while (wlf_animator_is_running(x_base) || wlf_animator_is_running(y_base)) {
		wlf_animator_update(x_base, 16);
		wlf_animator_update(y_base, 16);

		if (frame % 10 == 0) {  // Print every 10 frames
			printf("Position: (%.2f, %.2f)\n", x, y);
		}
		frame++;
		usleep(16000);
	}

	wlf_x_animator_destroy(x_animator);
	wlf_y_animator_destroy(y_animator);
}

/* Example 3: Rotation animation with looping */
static void example_rotation_animation(void) {
	printf("\n=== Example 3: Rotation Animation (Looping) ===\n");

	float rotation = 0.0f;
	struct wlf_rotation_animator *animator = wlf_rotation_animator_create(
		2000,  // 2 seconds per rotation
		0.0f,
		360.0f,
		&rotation
	);

	struct wlf_animator *base = wlf_rotation_animator_get_base(animator);

	// Set smooth easing
	wlf_animator_set_curve(base, wlf_animator_curve_in_out_sine_create());

	// Loop 3 times
	wlf_animator_set_loop_count(base, 3);

	// Start animation
	wlf_animator_start(base);

	int frame = 0;
	while (wlf_animator_is_running(base)) {
		wlf_animator_update(base, 16);

		if (frame % 20 == 0) {
			printf("Rotation: %.2f degrees\n", rotation);
		}
		frame++;
		usleep(16000);
	}

	wlf_rotation_animator_destroy(animator);
}

/* Example 4: Scale animation with elastic effect */
static void example_scale_animation(void) {
	printf("\n=== Example 4: Scale Animation (Elastic) ===\n");

	float scale_x = 1.0f;
	float scale_y = 1.0f;

	struct wlf_scale_animator *animator = wlf_scale_animator_create(
		1500,
		1.0f,  // from normal size
		2.0f,  // to double size
		&scale_x,
		&scale_y
	);

	struct wlf_animator *base = wlf_scale_animator_get_base(animator);

	// Use elastic easing for bouncy effect (amplitude=1.0, period=0.3)
	wlf_animator_set_curve(base, wlf_animator_curve_out_elastic_create(1.0f, 0.3f));

	// Add signal listeners
	struct animator_listener started_listener = {{.notify = on_animation_started}, .name = "Scale"};
	struct animator_listener finished_listener = {{.notify = on_animation_finished}, .name = "Scale"};
	wlf_signal_add(&base->events.started, &started_listener.listener);
	wlf_signal_add(&base->events.finished, &finished_listener.listener);

	wlf_animator_start(base);

	int frame = 0;
	while (wlf_animator_is_running(base)) {
		wlf_animator_update(base, 16);

		if (frame % 10 == 0) {
			printf("Scale: (%.3f, %.3f)\n", scale_x, scale_y);
		}
		frame++;
		usleep(16000);
	}

	wlf_scale_animator_destroy(animator);
}

int main(void) {
	printf("wlframe Animator Examples\n");
	printf("Based on Qt Quick Animation Framework\n");
	printf("=====================================\n");

	example_opacity_animation();
	example_position_animation();
	example_rotation_animation();
	example_scale_animation();

	printf("\n=== All examples completed ===\n");
	return 0;
}
