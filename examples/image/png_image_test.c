#include "wlf/image/wlf_image.h"
#include "wlf/utils/wlf_log.h"

#include <stdlib.h>

int main(int argc, char *argv[]) {
	int32_t log_level = WLF_DEBUG;
	wlf_log_init(log_level, NULL);

	if (argc >1) {
		const char *filename = argv[1];
		struct wlf_image *img = wlf_image_load(filename);
		if (img == NULL) {
			wlf_log(WLF_INFO, "Failed to load image: %s\n", filename);
			return EXIT_FAILURE;
		}

		wlf_image_save(img, "/tmp/wlf_image_test_save.png");
		wlf_image_finish(img);
	}

	return EXIT_SUCCESS;
}
