#include "wlf/math/wlf_region.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>

int main(int argc, char *argv[]) {
	int32_t log_level = WLF_DEBUG;
	wlf_log_init(log_level, NULL);
	struct wlf_region region;
	wlf_region_init(&region);

	struct wlf_frect r1 = {0, 0, 100, 100};
	struct wlf_frect r2 = {150, 150, 50, 50};
	wlf_region_add_rect(&region, &r1);
	wlf_region_add_rect(&region, &r2);

	char *str = wlf_region_to_str(&region);
	wlf_log(WLF_INFO, "Region to string:\n%s", str);
	free(str);

	const char *input = "{[0,0,100,100],[150,150,50,50],[0,200,100,100],[200,0,100,100]}";
	struct wlf_region parsed;
    if (wlf_region_from_str(input, &parsed)) {
		char *parsed_str = wlf_region_to_str(&parsed);
		wlf_log(WLF_INFO, "Parsed region to string: \n%s", parsed_str);
		free(parsed_str);
		// Test bounding rect
		// struct wlf_frect bounding_rect = wlf_region_bounding_rect(&parsed);
		// char *bounding_str = wlf_rect_to_str(&bounding_rect);
		// wlf_log(WLF_INFO, "Bounding rect: %s", bounding_str);
		// free(bounding_str);
		wlf_region_fini(&parsed);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse region from string: %s", input);
	}

	// Test region contains point
	wlf_log(WLF_INFO, "Contains (10,10): %d", wlf_region_contains_point(&region, 10, 10));
	wlf_log(WLF_INFO, "Contains (199,199): %d", wlf_region_contains_point(&region, 199, 199));
	wlf_log(WLF_INFO, "Contains (200,200): %d", wlf_region_contains_point(&region, 200, 200));

	// Test intersects rect
	struct wlf_frect test_rect = {90, 90, 20, 20};
	struct wlf_region intersection_rect_result;
	wlf_region_intersects_rect(&region, &test_rect, &intersection_rect_result);
	if (!wlf_region_is_nil(&intersection_rect_result)) {
		char *intersection_str = wlf_region_to_str(&intersection_rect_result);
		wlf_log(WLF_INFO, "Intersection with rect [90,90,20,20]: \n%s", intersection_str);
		free(intersection_str);
	} else {
		wlf_log(WLF_INFO, "No intersection with rect [90,90,20,20]");
	}

	// Test intersection
	struct wlf_region intersected_region_result;
	wlf_region_intersect(&region, &region, &intersected_region_result);
	if (!wlf_region_is_nil(&intersected_region_result)) {
		char *intersected_str = wlf_region_to_str(&intersected_region_result);
		wlf_log(WLF_INFO, "Intersected region: \n%s", intersected_str);
		free(intersected_str);
	} else {
		wlf_log(WLF_INFO, "No intersection found.");
	}

	wlf_region_fini(&region);

	return 0;
}
