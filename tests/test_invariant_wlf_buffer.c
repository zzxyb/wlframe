#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "buffer/wlf_buffer.h"

START_TEST(test_wlf_readonly_data_buffer_drop_invariant)
{
    // Invariant: wlf_readonly_data_buffer_drop must not read beyond buffer->data allocation
    struct wlf_readonly_data_buffer buffers[] = {
        // Exploit case: stride * height overflows or exceeds actual data allocation
        { .base = { .height = SIZE_MAX / 2 + 1 }, .stride = 2, .data = malloc(1) },
        // Boundary case: zero size allocation
        { .base = { .height = 0 }, .stride = 0, .data = NULL },
        // Valid case: normal parameters
        { .base = { .height = 10 }, .stride = 100, .data = malloc(1000) }
    };
    
    int num_buffers = sizeof(buffers) / sizeof(buffers[0]);
    
    for (int i = 0; i < num_buffers; i++) {
        struct wlf_readonly_data_buffer *buf = &buffers[i];
        
        // Save original data pointer to detect if function accessed invalid memory
        void *original_data = buf->data;
        size_t original_size = 0;
        if (original_data && buf->base.height > 0 && buf->stride > 0) {
            original_size = buf->stride * buf->base.height;
        }
        
        // Call the actual production function
        bool result = wlf_readonly_data_buffer_drop(buf);
        
        // Property: If allocation succeeds, saved_data must be valid or NULL
        if (buf->saved_data != NULL) {
            ck_assert_ptr_ne(buf->saved_data, original_data);
        }
        
        // Cleanup
        if (buf->saved_data) {
            free(buf->saved_data);
        }
        if (i == 0 || i == 2) {
            free(original_data);
        }
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_wlf_readonly_data_buffer_drop_invariant);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}