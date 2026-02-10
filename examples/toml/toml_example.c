#include "wlf/toml/wlf_toml.h"
#include "wlf/utils/wlf_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file toml_example.c
 * @brief TOML parser example demonstrating reading and parsing TOML configuration files
 */

static void print_separator(const char *title) {
	printf("\n");
	printf("=== %s ===\n", title);
	printf("\n");
}

static void print_table_info(struct wlf_toml_table *table, const char *name, int indent) {
	for (int i = 0; i < indent; i++) {
		printf("  ");
	}
	
	int nkval = wlf_toml_table_nkval(table);
	int ntab = wlf_toml_table_ntab(table);
	int narr = wlf_toml_table_narr(table);
	
	printf("[%s] - %d key-values, %d sub-tables, %d arrays\n", 
	       name, nkval, ntab, narr);
}

static void print_all_keys(struct wlf_toml_table *table) {
	int nkval = wlf_toml_table_nkval(table);
	
	if (nkval == 0) {
		printf("  (no key-value pairs)\n");
		return;
	}
	
	for (int i = 0; i < nkval; i++) {
		const char *key = wlf_toml_key_in(table, i);
		if (key) {
			char *raw = wlf_toml_raw_in(table, key);
			if (raw) {
				printf("  %s = %s\n", key, raw);
				free(raw);
			}
		}
	}
}

static void demonstrate_string_values(struct wlf_toml_table *table) {
	print_separator("String Values");
	
	struct wlf_toml_datum title = wlf_toml_string_in(table, "title");
	if (title.ok) {
		printf("Title: %s\n", title.u.s);
		free(title.u.s);
	} else {
		printf("Title: (not found or invalid)\n");
	}
	
	struct wlf_toml_datum description = wlf_toml_string_in(table, "description");
	if (description.ok) {
		printf("Description: %s\n", description.u.s);
		free(description.u.s);
	}
}

static void demonstrate_numeric_values(struct wlf_toml_table *table) {
	print_separator("Numeric Values");
	
	struct wlf_toml_datum count = wlf_toml_int_in(table, "count");
	if (count.ok) {
		printf("Count: %lld\n", (long long)count.u.i);
	} else {
		printf("Count: (not found or invalid)\n");
	}
	
	struct wlf_toml_datum pi = wlf_toml_double_in(table, "pi");
	if (pi.ok) {
		printf("Pi: %.6f\n", pi.u.d);
	}
	
	struct wlf_toml_datum temperature = wlf_toml_double_in(table, "temperature");
	if (temperature.ok) {
		printf("Temperature: %.2f\n", temperature.u.d);
	}
}

static void demonstrate_boolean_values(struct wlf_toml_table *table) {
	print_separator("Boolean Values");
	
	struct wlf_toml_datum enabled = wlf_toml_bool_in(table, "enabled");
	if (enabled.ok) {
		printf("Enabled: %s\n", enabled.u.b ? "true" : "false");
	} else {
		printf("Enabled: (not found or invalid)\n");
	}
	
	struct wlf_toml_datum debug = wlf_toml_bool_in(table, "debug");
	if (debug.ok) {
		printf("Debug: %s\n", debug.u.b ? "true" : "false");
	}
}

static void demonstrate_nested_tables(struct wlf_toml_table *table) {
	print_separator("Nested Tables");
	
	struct wlf_toml_table *server = wlf_toml_table_in(table, "server");
	if (server) {
		printf("Found [server] table:\n");
		print_all_keys(server);
		
		// Access nested values
		struct wlf_toml_datum host = wlf_toml_string_in(server, "host");
		if (host.ok) {
			printf("\nServer host: %s\n", host.u.s);
			free(host.u.s);
		}
		
		struct wlf_toml_datum port = wlf_toml_int_in(server, "port");
		if (port.ok) {
			printf("Server port: %lld\n", (long long)port.u.i);
		}
	} else {
		printf("No [server] table found\n");
	}
	
	struct wlf_toml_table *database = wlf_toml_table_in(table, "database");
	if (database) {
		printf("\nFound [database] table:\n");
		print_all_keys(database);
	}
}

static void demonstrate_parsing_from_string(void) {
	print_separator("Parsing TOML from String");
	
	const char *toml_string = 
		"# Simple TOML example\n"
		"title = \"TOML Example\"\n"
		"count = 42\n"
		"pi = 3.14159\n"
		"enabled = true\n"
		"\n"
		"[server]\n"
		"host = \"localhost\"\n"
		"port = 8080\n"
		"timeout = 30\n";
	
	printf("Parsing TOML string:\n%s\n", toml_string);
	
	char errbuf[200];
	struct wlf_toml_table *conf = wlf_toml_parse(toml_string, errbuf, sizeof(errbuf));
	
	if (!conf) {
		printf("Error parsing TOML: %s\n", errbuf);
		return;
	}
	
	printf("Successfully parsed TOML!\n");
	
	// Display all root-level keys
	printf("\nRoot-level keys:\n");
	print_all_keys(conf);
	
	// Demonstrate accessing values
	demonstrate_string_values(conf);
	demonstrate_numeric_values(conf);
	demonstrate_boolean_values(conf);
	demonstrate_nested_tables(conf);
	
	wlf_toml_free(conf);
}

static void demonstrate_parsing_from_file(const char *filename) {
	print_separator("Parsing TOML from File");
	
	printf("Reading from file: %s\n", filename);
	
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		printf("Error: Cannot open file '%s'\n", filename);
		printf("Make sure the file exists in the current directory.\n");
		return;
	}
	
	char errbuf[200];
	struct wlf_toml_table *conf = wlf_toml_parse_file(fp, errbuf, sizeof(errbuf));
	fclose(fp);
	
	if (!conf) {
		printf("Error parsing TOML file: %s\n", errbuf);
		return;
	}
	
	printf("Successfully parsed TOML file!\n");
	
	// Display structure information
	printf("\nConfiguration structure:\n");
	print_table_info(conf, "root", 0);
	
	// List all root keys
	printf("\nRoot-level configuration:\n");
	print_all_keys(conf);
	
	// Check for nested tables
	int ntab = wlf_toml_table_ntab(conf);
	if (ntab > 0) {
		printf("\nSub-tables found: %d\n", ntab);
		
		// Iterate through common table names
		const char *common_tables[] = {"server", "database", "client", "logging", NULL};
		for (int i = 0; common_tables[i] != NULL; i++) {
			struct wlf_toml_table *subtab = wlf_toml_table_in(conf, common_tables[i]);
			if (subtab) {
				printf("\n[%s]\n", common_tables[i]);
				print_all_keys(subtab);
			}
		}
	}
	
	wlf_toml_free(conf);
}

static void demonstrate_error_handling(void) {
	print_separator("Error Handling Examples");
	
	// Test 1: Invalid syntax - commented out for now due to parser limitations
	// TODO: Improve parser error handling for malformed input
	printf("Test 1: Parser error handling needs improvement\n");
	printf("(Some edge cases may cause issues)\n");
	
	// Test 2: Duplicate key
	const char *duplicate_key = 
		"name = \"first\"\n"
		"name = \"second\"\n";
	
	printf("\nTest 2: Duplicate key\n");
	char errbuf[200];
	struct wlf_toml_table *conf = wlf_toml_parse(duplicate_key, errbuf, sizeof(errbuf));
	if (!conf) {
		printf("Expected error: %s\n", errbuf);
	} else {
		printf("Unexpectedly succeeded\n");
		wlf_toml_free(conf);
	}
	
	// Test 3: Accessing non-existent key
	const char *valid_toml = "title = \"Test\"\n";
	printf("\nTest 3: Accessing non-existent key\n");
	conf = wlf_toml_parse(valid_toml, errbuf, sizeof(errbuf));
	if (conf) {
		struct wlf_toml_datum result = wlf_toml_string_in(conf, "nonexistent");
		if (!result.ok) {
			printf("Correctly handled missing key\n");
		}
		wlf_toml_free(conf);
	}
}

static void demonstrate_type_conversions(void) {
	print_separator("Type Conversion Examples");
	
	const char *toml_string = 
		"int_value = 42\n"
		"float_value = 3.14\n"
		"bool_value = true\n"
		"string_value = \"Hello\"\n";
	
	char errbuf[200];
	struct wlf_toml_table *conf = wlf_toml_parse(toml_string, errbuf, sizeof(errbuf));
	
	if (!conf) {
		printf("Error: %s\n", errbuf);
		return;
	}
	
	// Try to read int as different types
	printf("Reading int_value=42:\n");
	struct wlf_toml_datum int_as_int = wlf_toml_int_in(conf, "int_value");
	printf("  As integer: %s (value: %lld)\n", 
	       int_as_int.ok ? "success" : "failed",
	       int_as_int.ok ? (long long)int_as_int.u.i : 0);
	
	struct wlf_toml_datum int_as_string = wlf_toml_string_in(conf, "int_value");
	printf("  As string: %s (value: %s)\n",
	       int_as_string.ok ? "success" : "failed",
	       int_as_string.ok ? int_as_string.u.s : "N/A");
	if (int_as_string.ok) free(int_as_string.u.s);
	
	// Try to read string as different types
	printf("\nReading string_value=\"Hello\":\n");
	struct wlf_toml_datum string_as_string = wlf_toml_string_in(conf, "string_value");
	printf("  As string: %s (value: %s)\n",
	       string_as_string.ok ? "success" : "failed",
	       string_as_string.ok ? string_as_string.u.s : "N/A");
	if (string_as_string.ok) free(string_as_string.u.s);
	
	struct wlf_toml_datum string_as_int = wlf_toml_int_in(conf, "string_value");
	printf("  As integer: %s\n",
	       string_as_int.ok ? "success" : "failed (expected)");
	
	wlf_toml_free(conf);
}

static void print_usage(const char *program_name) {
	printf("WLF TOML Parser Example\n\n");
	printf("This example demonstrates how to use the wlf_toml library to parse\n");
	printf("TOML configuration files.\n\n");
	printf("Usage: %s [filename]\n\n", program_name);
	printf("  filename    Optional TOML file to parse\n\n");
	printf("If no filename is provided, built-in examples will be used.\n\n");
	printf("Example:\n");
	printf("  %s config.toml\n", program_name);
}

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_INFO, NULL);
	
	printf("==============================================\n");
	printf("       WLF TOML Parser Example Program       \n");
	printf("==============================================\n");
	
	if (argc > 2) {
		print_usage(argv[0]);
		return 1;
	}
	
	// If a filename is provided, parse it
	if (argc == 2) {
		demonstrate_parsing_from_file(argv[1]);
	} else {
		// Otherwise, demonstrate various features
		demonstrate_parsing_from_string();
		demonstrate_error_handling();
		demonstrate_type_conversions();
		
		// Try to parse example.toml if it exists
		printf("\n");
		demonstrate_parsing_from_file("example.toml");
	}
	
	print_separator("End of Example");
	printf("For more information, see the wlf_toml.h header file.\n");
	
	return 0;
}
