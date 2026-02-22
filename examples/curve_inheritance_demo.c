/**
 * Example demonstrating the inheritance-based curve design
 * Each curve type has its own struct and factory function
 */

#define _DEFAULT_SOURCE
#include "wlf/animator/wlf_animator_curve.h"
#include <stdio.h>

int main(void) {
	printf("=== wlframe Animator Curve - Inheritance Design Demo ===\n\n");

	// Test different curve types
	printf("1. Linear Curve:\n");
	struct wlf_animator_curve *linear = wlf_animator_curve_linear_create();
	for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
		printf("   t=%.2f -> %.2f\n", t, wlf_animator_curve_value_at(linear, t));
	}
	wlf_animator_curve_destroy(linear);

	printf("\n2. Quadratic Ease-Out Curve:\n");
	struct wlf_animator_curve *quad_out = wlf_animator_curve_out_quad_create();
	for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
		printf("   t=%.2f -> %.2f\n", t, wlf_animator_curve_value_at(quad_out, t));
	}
	wlf_animator_curve_destroy(quad_out);

	printf("\n3. Cubic Ease-In-Out Curve:\n");
	struct wlf_animator_curve *cubic_in_out = wlf_animator_curve_in_out_cubic_create();
	for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
		printf("   t=%.2f -> %.2f\n", t, wlf_animator_curve_value_at(cubic_in_out, t));
	}
	wlf_animator_curve_destroy(cubic_in_out);

	printf("\n4. Elastic Curve (custom parameters):\n");
	struct wlf_animator_curve *elastic = wlf_animator_curve_out_elastic_create(1.5f, 0.4f);
	printf("   Amplitude: 1.5, Period: 0.4\n");
	for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
		printf("   t=%.2f -> %.3f\n", t, wlf_animator_curve_value_at(elastic, t));
	}
	wlf_animator_curve_destroy(elastic);

	printf("\n5. Back Curve (custom overshoot):\n");
	struct wlf_animator_curve *back = wlf_animator_curve_out_back_create(2.0f);
	printf("   Overshoot: 2.0\n");
	for (float t = 0.0f; t <= 1.0f; t += 0.25f) {
		printf("   t=%.2f -> %.3f\n", t, wlf_animator_curve_value_at(back, t));
	}
	wlf_animator_curve_destroy(back);

	printf("\n6. Bounce Curve:\n");
	struct wlf_animator_curve *bounce = wlf_animator_curve_out_bounce_create();
	for (float t = 0.0f; t <= 1.0f; t += 0.1f) {
		printf("   t=%.2f -> %.3f\n", t, wlf_animator_curve_value_at(bounce, t));
	}
	wlf_animator_curve_destroy(bounce);

	printf("\n=== Demonstration Complete ===\n");
	printf("\nKey Features of Inheritance-Based Design:\n");
	printf("- Each curve type has its own struct (e.g., wlf_animator_curve_elastic)\n");
	printf("- Factory functions create instances: wlf_animator_curve_*_create()\n");
	printf("- Polymorphic interface: wlf_animator_curve_value_at() works for all types\n");
	printf("- Parameters can be customized at creation (e.g., elastic amplitude/period)\n");
	printf("- Clean memory management with wlf_animator_curve_destroy()\n");

	return 0;
}
