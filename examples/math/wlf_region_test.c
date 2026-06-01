#include "wlf/math/wlf_region.h"
#include "wlf/math/wlf_rect.h"
#include "wlf/utils/wlf_log.h"

#include <stdint.h>

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG, NULL);
	struct wlf_region region;
	wlf_region_init(&region);

	struct wlf_frect r1 = {0, 0, 100, 100};
	struct wlf_frect r2 = {150, 150, 50, 50};
	wlf_region_add_rect(&region, &r1);
	wlf_region_add_rect(&region, &r2);

	char *str = wlf_region_to_str(&region);
	wlf_log(WLF_INFO, "Region to string:\n%s", str);

	struct wlf_region parsed;
	if (wlf_region_from_str(str, &parsed)) {
		char *parsed_str = wlf_region_to_str(&parsed);
		wlf_log(WLF_INFO, "Parsed region to string: \n%s", parsed_str);
		free(parsed_str);
		wlf_region_fini(&parsed);
	} else {
		wlf_log(WLF_ERROR, "Failed to parse region from string: %s", str);
	}
	free(str);

	// Test region contains point
	wlf_log(WLF_INFO, "Contains (10,10): %d", wlf_region_contains_point(&region, 10, 10));
	wlf_log(WLF_INFO, "Contains (199,199): %d", wlf_region_contains_point(&region, 199, 199));
	wlf_log(WLF_INFO, "Contains (200,200): %d", wlf_region_contains_point(&region, 200, 200));

	// Test intersects rect
	struct wlf_frect test_rect = {90, 90, 20, 20};
	struct wlf_region intersection_rect_result;
	wlf_region_init(&intersection_rect_result);
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
	wlf_region_init(&intersected_region_result);
	wlf_region_intersect(&region, &region, &intersected_region_result);
	if (!wlf_region_is_nil(&intersected_region_result)) {
		char *intersected_str = wlf_region_to_str(&intersected_region_result);
		wlf_log(WLF_INFO, "Intersected region: \n%s", intersected_str);
		free(intersected_str);
	} else {
		wlf_log(WLF_INFO, "No intersection found.");
	}

	wlf_log(WLF_INFO, "Region not empty: %d", wlf_region_not_empty(&region));
	wlf_log(WLF_INFO, "Region rect count: %ld", wlf_region_n_rects(&region));

	struct wlf_frect contained = {10, 10, 20, 20};
	struct wlf_frect partial = {90, 90, 80, 80};
	struct wlf_frect outside = {500, 500, 10, 10};
	wlf_log(WLF_INFO, "Contains rect [10,10,20,20]: %d",
		wlf_region_contains_rect(&region, &contained));
	wlf_log(WLF_INFO, "Contains rect [90,90,80,80]: %d",
		wlf_region_contains_rect(&region, &partial));
	wlf_log(WLF_INFO, "Contains rect [500,500,10,10]: %d",
		wlf_region_contains_rect(&region, &outside));

	struct wlf_region translated;
	wlf_region_init(&translated);
	wlf_region_copy(&translated, &region);
	wlf_region_translate(&translated, 5, 10);
	char *translated_str = wlf_region_to_str(&translated);
	wlf_log(WLF_INFO, "Translated region: \n%s", translated_str);
	free(translated_str);
	wlf_region_fini(&translated);

	struct wlf_region subtract_region;
	wlf_region_init_rect(&subtract_region, &(struct wlf_frect){20, 20, 40, 40});
	struct wlf_region subtracted;
	wlf_region_init(&subtracted);
	wlf_region_subtract(&subtracted, &region, &subtract_region);
	char *subtracted_str = wlf_region_to_str(&subtracted);
	wlf_log(WLF_INFO, "Subtracted region: \n%s", subtracted_str);
	free(subtracted_str);
	wlf_region_fini(&subtracted);
	wlf_region_fini(&subtract_region);

	wlf_region_fini(&region);
	wlf_region_fini(&intersection_rect_result);
	wlf_region_fini(&intersected_region_result);

	return 0;
}
