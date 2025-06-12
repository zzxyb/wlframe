#include "wlf/utils/wlf_cmd_parser.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

struct test_config {
	bool verbose;           // Verbose output
	bool quiet;            // Quiet mode
	bool help;             // Show help
	char *test_filter;     // Test filter
	int repeat_count;      // Repeat count
	bool benchmark;        // Benchmark mode
};

struct test_result {
	int passed;
	int failed;
	int total;
};

static void init_test_config(struct test_config *config) {
	config->verbose = false;
	config->quiet = false;
	config->help = false;
	config->test_filter = NULL;
	config->repeat_count = 1;
	config->benchmark = false;
}

static void init_test_result(struct test_result *result) {
	result->passed = 0;
	result->failed = 0;
	result->total = 0;
}

static void print_test_result(const char *test_name, bool success, struct test_result *result, const struct test_config *config) {
	result->total++;
	if (success) {
		result->passed++;
		if (!config->quiet) {
			printf("[✓] %s\n", test_name);
		}
	} else {
		result->failed++;
		if (!config->quiet) {
			printf("[✗] %s\n", test_name);
		}
	}

	if (config->verbose && success) {
		printf("    └─ Test passed successfully\n");
	} else if (config->verbose && !success) {
		printf("    └─ Test failed - check implementation\n");
	}
}

static bool should_run_test(const char *test_name, const struct test_config *config) {
	if (config->test_filter == NULL) {
		return true;
	}
	return strstr(test_name, config->test_filter) != NULL;
}

static void test_integer_option(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("integer", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Integer Option Parsing ---\n");
	}

	int32_t int_value = 0;
	uint32_t uint_value = 0;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_INTEGER, "count", 'c', &int_value },
		{ WLF_OPTION_UNSIGNED_INTEGER, "size", 's', &uint_value }
	};

		// Test long option format
	char *argv1[] = { "test", "--count=42", "--size=100" };
	int argc1 = 3;
	int ret = wlf_cmd_parse_options(options, 2, &argc1, argv1);

	bool test1_passed = (ret == 1 && int_value == 42 && uint_value == 100);
	print_test_result("Integer options (long format)", test1_passed, result, config);

	if (config->verbose && test1_passed) {
		printf("    └─ Parsed values: count=%d, size=%u\n", int_value, uint_value);
	}

	// Reset values
	int_value = 0;
	uint_value = 0;

	// Test short option format
	char *argv2[] = { "test", "-c", "123", "-s", "456" };
	int argc2 = 5;
	ret = wlf_cmd_parse_options(options, 2, &argc2, argv2);

	bool test2_passed = (ret == 1 && int_value == 123 && uint_value == 456);
	print_test_result("Integer options (short format)", test2_passed, result, config);

	if (config->verbose && test2_passed) {
		printf("    └─ Parsed values: count=%d, size=%u\n", int_value, uint_value);
	}
}

static void test_string_option(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("string", config)) return;

	if (config->verbose) {
		printf("\n--- Testing String Option Parsing ---\n");
	}

	char *str_value = NULL;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_STRING, "file", 'f', &str_value }
	};

	// Test long option
	char *argv1[] = { "test", "--file=test.txt" };
	int argc1 = 2;
	int ret = wlf_cmd_parse_options(options, 1, &argc1, argv1);

	bool test1_passed = (ret == 1 && str_value != NULL && strcmp(str_value, "test.txt") == 0);
	print_test_result("String option (long format)", test1_passed, result, config);

	if (config->verbose && test1_passed) {
		printf("    └─ Parsed value: '%s'\n", str_value);
	}

	// Clean up memory
	free(str_value);
	str_value = NULL;

	// Test short option
	char *argv2[] = { "test", "-f", "config.conf" };
	int argc2 = 3;
	ret = wlf_cmd_parse_options(options, 1, &argc2, argv2);

	bool test2_passed = (ret == 1 && str_value != NULL && strcmp(str_value, "config.conf") == 0);
	print_test_result("String option (short format)", test2_passed, result, config);

	if (config->verbose && test2_passed) {
		printf("    └─ Parsed value: '%s'\n", str_value);
	}

	// Clean up memory
	free(str_value);
}

static void test_boolean_option(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("boolean", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Boolean Option Parsing ---\n");
	}

	bool verbose = false;
	bool debug = false;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose },
		{ WLF_OPTION_BOOLEAN, "debug", 'd', &debug }
	};

		// Test long options
	char *argv1[] = { "test", "--verbose", "--debug" };
	int argc1 = 3;
	int ret = wlf_cmd_parse_options(options, 2, &argc1, argv1);

	bool test1_passed = (ret == 1 && verbose == true && debug == true);
	print_test_result("Boolean options (long format)", test1_passed, result, config);

	if (config->verbose && test1_passed) {
		printf("    └─ Parsed values: verbose=%s, debug=%s\n",
		       verbose ? "true" : "false", debug ? "true" : "false");
	}

	// Reset values
	verbose = false;
	debug = false;

	// Test short options
	char *argv2[] = { "test", "-v", "-d" };
	int argc2 = 3;
	ret = wlf_cmd_parse_options(options, 2, &argc2, argv2);

	bool test2_passed = (ret == 1 && verbose == true && debug == true);
	print_test_result("Boolean options (short format)", test2_passed, result, config);

	if (config->verbose && test2_passed) {
		printf("    └─ Parsed values: verbose=%s, debug=%s\n",
		       verbose ? "true" : "false", debug ? "true" : "false");
	}
}

static void test_mixed_options(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("mixed", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Mixed Option Parsing ---\n");
	}

	int32_t count = 0;
	char *filename = NULL;
	bool verbose = false;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_INTEGER, "count", 'c', &count },
		{ WLF_OPTION_STRING, "file", 'f', &filename },
		{ WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose }
	};

	char *argv[] = { "test", "--count=10", "-f", "data.txt", "--verbose", "remaining_arg" };
	int argc = 6;
	int ret = wlf_cmd_parse_options(options, 3, &argc, argv);

	bool test_passed = (ret == 2 && count == 10 && filename != NULL &&
			   strcmp(filename, "data.txt") == 0 && verbose == true &&
			   argc == 2 && strcmp(argv[1], "remaining_arg") == 0);

	print_test_result("Mixed options with remaining arguments", test_passed, result, config);

	if (config->verbose && test_passed) {
		printf("    └─ Parsed values: count=%d, file='%s', verbose=%s\n",
		       count, filename, verbose ? "true" : "false");
		printf("    └─ Remaining args: argc=%d, argv[1]='%s'\n", argc, argv[1]);
	}

	free(filename);
}

static void test_options_help(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("help", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Options Help Output ---\n");
	}

	int32_t count = 0;
	char *filename = NULL;
	bool verbose = false;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_INTEGER, "count", 'c', &count },
		{ WLF_OPTION_STRING, "file", 'f', &filename },
		{ WLF_OPTION_BOOLEAN, "verbose", 'v', &verbose },
		{ WLF_OPTION_BOOLEAN, "quiet", 0, NULL },  // Long option only
		{ WLF_OPTION_INTEGER, NULL, 'n', &count }   // Short option only
	};

	if (!config->quiet) {
		printf("\n--- Options Help Output ---\n");
		wlf_print_options_help(options, 5, "utils_parser_test");
		printf("--- End of Help Output ---\n\n");
	}

	// This test is mainly for visual verification, always passes
	print_test_result("Options help output", true, result, config);
}

static void test_invalid_options(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("invalid", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Invalid Option Handling ---\n");
	}

	int32_t count = 0;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_INTEGER, "count", 'c', &count }
	};

	// Test invalid long option
	char *argv1[] = { "test", "--invalid-option" };
	int argc1 = 2;
	int ret1 = wlf_cmd_parse_options(options, 1, &argc1, argv1);

	bool test1_passed = (ret1 == 2 && argc1 == 2 && strcmp(argv1[1], "--invalid-option") == 0);
	print_test_result("Invalid long option handling", test1_passed, result, config);

	// Test invalid short option
	char *argv2[] = { "test", "-x" };
	int argc2 = 2;
	int ret2 = wlf_cmd_parse_options(options, 1, &argc2, argv2);

	bool test2_passed = (ret2 == 2 && argc2 == 2 && strcmp(argv2[1], "-x") == 0);
	print_test_result("Invalid short option handling", test2_passed, result, config);
}

static void test_edge_cases(struct test_result *result, const struct test_config *config) {
	if (!should_run_test("edge", config)) return;

	if (config->verbose) {
		printf("\n--- Testing Edge Cases ---\n");
	}

	int32_t count = 0;

	struct wlf_cmd_option options[] = {
		{ WLF_OPTION_INTEGER, "count", 'c', &count }
	};

	// Test empty argument list
	char *argv1[] = { "test" };
	int argc1 = 1;
	int ret1 = wlf_cmd_parse_options(options, 1, &argc1, argv1);

	bool test1_passed = (ret1 == 1 && argc1 == 1);
	print_test_result("Empty arguments handling", test1_passed, result, config);

	// Test case with only program name
	char *argv2[] = { "test", "non_option_arg1", "non_option_arg2" };
	int argc2 = 3;
	int ret2 = wlf_cmd_parse_options(options, 1, &argc2, argv2);

	bool test2_passed = (ret2 == 3 && argc2 == 3 &&
				strcmp(argv2[1], "non_option_arg1") == 0 &&
				strcmp(argv2[2], "non_option_arg2") == 0);
	print_test_result("Non-option arguments preservation", test2_passed, result, config);
}

static void run_benchmark(const struct test_config *config) {
	if (!config->benchmark) return;

	printf("\n=== Benchmark Tests ===\n");

	// Benchmark test: parsing many options
	int32_t values[1000];
	struct wlf_cmd_option options[1000];

	// Initialize options array
	for (int i = 0; i < 1000; i++) {
		values[i] = 0;
		options[i].type = WLF_OPTION_INTEGER;
		options[i].name = NULL;  // Use short options only for simplicity
		options[i].short_name = 'a' + (i % 26);  // Cycle through letters
		options[i].data = &values[i];
	}

	// Create argument array
	char *argv[2001];  // Program name + 1000 options and values
	argv[0] = "benchmark";

	for (int i = 0; i < 1000; i++) {
		argv[1 + i*2] = malloc(3);
		sprintf(argv[1 + i*2], "-%c", 'a' + (i % 26));
		argv[1 + i*2 + 1] = malloc(10);
		sprintf(argv[1 + i*2 + 1], "%d", i);
	}

	int argc = 2001;

	clock_t start = clock();
	for (int repeat = 0; repeat < config->repeat_count; repeat++) {
		int temp_argc = argc;
		wlf_cmd_parse_options(options, 1000, &temp_argc, argv);
	}
	clock_t end = clock();

	double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
	printf("Benchmark: Parsed 1000 options %d times in %.3f seconds\n",
			config->repeat_count, elapsed);
	printf("Average time per parse: %.6f seconds\n", elapsed / config->repeat_count);

	for (int i = 0; i < 1000; i++) {
		free(argv[1 + i*2]);
		free(argv[1 + i*2 + 1]);
	}
}

static void show_help(const char *program_name) {
	printf("WLF Option Parser Test Program\n\n");
	printf("This program tests the wlf_cmd_option parsing functionality and demonstrates\n");
	printf("how to use command line options in your own programs.\n\n");

	printf("Usage: %s [options]\n\n", program_name);
	printf("Test Control Options:\n");
	printf("  -v, --verbose      Enable verbose output with detailed test information\n");
	printf("  -q, --quiet        Quiet mode - only show summary results\n");
	printf("  -f, --filter TEXT  Run only tests containing TEXT in their name\n");
	printf("  -r, --repeat NUM   Repeat tests NUM times (default: 1)\n");
	printf("  -b, --benchmark    Run benchmark tests\n");
	printf("  -h, --help         Show this help message and exit\n\n");

	printf("Examples:\n");
	printf("  %s                    # Run all tests with normal output\n", program_name);
	printf("  %s -v                 # Run with verbose output\n", program_name);
	printf("  %s -q                 # Run in quiet mode\n", program_name);
	printf("  %s -f integer         # Run only integer-related tests\n", program_name);
	printf("  %s -r 10 -b           # Run benchmark 10 times\n", program_name);
	printf("  %s --help             # Show this help\n", program_name);
}

int main(int argc, char *argv[]) {
	struct test_config config;
	struct test_result result;

	init_test_config(&config);
	init_test_result(&result);

	struct wlf_cmd_option program_options[] = {
		{ WLF_OPTION_BOOLEAN, "verbose", 'v', &config.verbose },
		{ WLF_OPTION_BOOLEAN, "quiet", 'q', &config.quiet },
		{ WLF_OPTION_BOOLEAN, "help", 'h', &config.help },
		{ WLF_OPTION_STRING, "filter", 'f', &config.test_filter },
		{ WLF_OPTION_INTEGER, "repeat", 'r', &config.repeat_count },
		{ WLF_OPTION_BOOLEAN, "benchmark", 'b', &config.benchmark }
	};

	int ret = wlf_cmd_parse_options(program_options, 6, &argc, argv);
	if (ret < 0) {
		fprintf(stderr, "Error parsing command line options\n");
		return 1;
	}

	if (config.help) {
		show_help(argv[0]);
		return 0;
	}

	if (config.verbose && config.quiet) {
		fprintf(stderr, "Error: Cannot use both --verbose and --quiet options\n");
		return 1;
	}

	if (config.repeat_count < 1) {
		fprintf(stderr, "Error: Repeat count must be at least 1\n");
		return 1;
	}

	if (!config.quiet) {
		wlf_log(WLF_INFO, "Starting wlf_cmd_option tests...");
		printf("\n=== WLF Option Parser Tests ===\n");

		if (config.verbose) {
			printf("Configuration:\n");
			printf("  Verbose: %s\n", config.verbose ? "enabled" : "disabled");
			printf("  Filter: %s\n", config.test_filter ? config.test_filter : "none");
			printf("  Repeat count: %d\n", config.repeat_count);
			printf("  Benchmark: %s\n", config.benchmark ? "enabled" : "disabled");
		}
		printf("\n");
	}

	for (int i = 0; i < config.repeat_count; i++) {
		if (config.repeat_count > 1 && !config.quiet) {
			printf("=== Run %d/%d ===\n", i + 1, config.repeat_count);
		}

		test_integer_option(&result, &config);
		test_string_option(&result, &config);
		test_boolean_option(&result, &config);
		test_mixed_options(&result, &config);
		test_options_help(&result, &config);
		test_invalid_options(&result, &config);
		test_edge_cases(&result, &config);
	}

	run_benchmark(&config);

	if (!config.quiet) {
		printf("\n=== Test Summary ===\n");
		printf("Total tests: %d\n", result.total);
		printf("Passed: %d\n", result.passed);
		printf("Failed: %d\n", result.failed);
		printf("Success rate: %.1f%%\n", result.total > 0 ? (100.0 * result.passed / result.total) : 0.0);

		if (config.repeat_count > 1) {
			printf("Tests repeated: %d times\n", config.repeat_count);
		}
	}

	if (config.test_filter) {
		free(config.test_filter);
	}

	if (result.failed == 0) {
		if (!config.quiet) {
			wlf_log(WLF_INFO, "All tests passed!");
		}
		return 0;
	} else {
		if (!config.quiet) {
			wlf_log(WLF_ERROR, "%d tests failed", result.failed);
		}
		return 1;
	}
}
