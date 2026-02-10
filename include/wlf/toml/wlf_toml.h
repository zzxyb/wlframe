#ifndef WLF_TOML_H
#define WLF_TOML_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief TOML value types
 */
enum wlf_toml_type {
	WLF_TOML_TYPE_NONE = 0,
	WLF_TOML_TYPE_STRING,
	WLF_TOML_TYPE_INT,
	WLF_TOML_TYPE_DOUBLE,
	WLF_TOML_TYPE_BOOL,
	WLF_TOML_TYPE_TIMESTAMP,
	WLF_TOML_TYPE_TABLE,
	WLF_TOML_TYPE_ARRAY,
};

/**
 * @brief Internal structures for TOML parsing
 */
struct wlf_toml_keyval {
	const char *key;
	char *val;
};

/**
 * @brief TOML timestamp structure
 */
struct wlf_toml_timestamp {
	struct {
		int year;
		int month;
		int day;
	} date;
	struct {
		int hour;
		int minute;
		int second;
		int millisec;
	} time;
	char z[10]; /**< Time zone offset (e.g., "+08:00", "Z") */
};

struct wlf_toml_table {
	const char *key; /**< Key of this table in parent */
	bool implicit; /**< True if table was created implicitly */
	
	int nkval; /**< Number of key-value pairs */
	struct wlf_toml_keyval **kval;
	
	int ntab; /**< Number of sub-tables */
	struct wlf_toml_table **tab;
	
	int narr; /**< Number of arrays */
	struct wlf_toml_array **arr;
};

struct wlf_toml_array {
	const char *key; /**< Key of this array in parent table */
	enum wlf_toml_type type; /**< Type of elements */
	int nelem; /**< Number of elements */
	char **val; /**< Raw string values */
	struct wlf_toml_table **tab; /**< Table elements */
	struct wlf_toml_array **arr; /**< Array elements */
};

/**
 * @brief TOML datum - represents a value in TOML
 */
struct wlf_toml_datum {
	bool ok; /**< Whether the value was successfully retrieved */
	union {
		char *s; /**< String value */
		int64_t i; /**< Integer value */
		double d; /**< Double value */
		bool b; /**< Boolean value */
		struct wlf_toml_timestamp ts; /**< Timestamp value */
	} u;
};

/**
 * @brief Parse TOML from a file
 * @param fp File pointer to read from
 * @param errbuf Buffer to store error message (at least 200 bytes)
 * @param errbuf_len Length of error buffer
 * @return Parsed TOML table, or NULL on error
 */
struct wlf_toml_table *wlf_toml_parse_file(FILE *fp, char *errbuf, int errbuf_len);

/**
 * @brief Parse TOML from a string
 * @param toml TOML string to parse
 * @param errbuf Buffer to store error message (at least 200 bytes)
 * @param errbuf_len Length of error buffer
 * @return Parsed TOML table, or NULL on error
 */
struct wlf_toml_table *wlf_toml_parse(const char *toml, char *errbuf, int errbuf_len);

/**
 * @brief Free a TOML table and all its contents
 * @param table Table to free
 */
void wlf_toml_free(struct wlf_toml_table *table);

/**
 * @brief Get the number of key-value pairs in a table
 * @param table TOML table
 * @return Number of pairs
 */
int wlf_toml_table_nkval(const struct wlf_toml_table *table);

/**
 * @brief Get the number of sub-tables in a table
 * @param table TOML table
 * @return Number of sub-tables
 */
int wlf_toml_table_ntab(const struct wlf_toml_table *table);

/**
 * @brief Get the number of arrays in a table
 * @param table TOML table
 * @return Number of arrays
 */
int wlf_toml_table_narr(const struct wlf_toml_table *table);

/**
 * @brief Get key by index in a table
 * @param table TOML table
 * @param idx Index
 * @return Key string, or NULL if out of bounds
 */
const char *wlf_toml_key_in(const struct wlf_toml_table *table, int idx);

/**
 * @brief Get string value from table
 * @param table TOML table
 * @param key Key to look up
 * @return Datum containing the string value
 */
struct wlf_toml_datum wlf_toml_string_in(const struct wlf_toml_table *table, 
                                          const char *key);

/**
 * @brief Get integer value from table
 * @param table TOML table
 * @param key Key to look up
 * @return Datum containing the integer value
 */
struct wlf_toml_datum wlf_toml_int_in(const struct wlf_toml_table *table, 
                                       const char *key);

/**
 * @brief Get double value from table
 * @param table TOML table
 * @param key Key to look up
 * @return Datum containing the double value
 */
struct wlf_toml_datum wlf_toml_double_in(const struct wlf_toml_table *table, 
                                          const char *key);

/**
 * @brief Get boolean value from table
 * @param table TOML table
 * @param key Key to look up
 * @return Datum containing the boolean value
 */
struct wlf_toml_datum wlf_toml_bool_in(const struct wlf_toml_table *table, 
                                        const char *key);

/**
 * @brief Get timestamp value from table
 * @param table TOML table
 * @param key Key to look up
 * @return Datum containing the timestamp value
 */
struct wlf_toml_datum wlf_toml_timestamp_in(const struct wlf_toml_table *table, 
                                             const char *key);

/**
 * @brief Get sub-table by key
 * @param table TOML table
 * @param key Key to look up
 * @return Sub-table, or NULL if not found
 */
struct wlf_toml_table *wlf_toml_table_in(const struct wlf_toml_table *table, 
                                          const char *key);

/**
 * @brief Get array by key
 * @param table TOML table
 * @param key Key to look up
 * @return Array, or NULL if not found
 */
struct wlf_toml_array *wlf_toml_array_in(const struct wlf_toml_table *table, 
                                          const char *key);

/**
 * @brief Get the length of an array
 * @param array TOML array
 * @return Number of elements
 */
int wlf_toml_array_nelem(const struct wlf_toml_array *array);

/**
 * @brief Get the type of array elements
 * @param array TOML array
 * @return Type of elements
 */
enum wlf_toml_type wlf_toml_array_type(const struct wlf_toml_array *array);

/**
 * @brief Get string value from array
 * @param array TOML array
 * @param idx Index
 * @return Datum containing the string value
 */
struct wlf_toml_datum wlf_toml_string_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get integer value from array
 * @param array TOML array
 * @param idx Index
 * @return Datum containing the integer value
 */
struct wlf_toml_datum wlf_toml_int_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get double value from array
 * @param array TOML array
 * @param idx Index
 * @return Datum containing the double value
 */
struct wlf_toml_datum wlf_toml_double_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get boolean value from array
 * @param array TOML array
 * @param idx Index
 * @return Datum containing the boolean value
 */
struct wlf_toml_datum wlf_toml_bool_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get timestamp value from array
 * @param array TOML array
 * @param idx Index
 * @return Datum containing the timestamp value
 */
struct wlf_toml_datum wlf_toml_timestamp_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get sub-table from array
 * @param array TOML array
 * @param idx Index
 * @return Sub-table, or NULL if out of bounds
 */
struct wlf_toml_table *wlf_toml_table_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get sub-array from array
 * @param array TOML array
 * @param idx Index
 * @return Sub-array, or NULL if out of bounds
 */
struct wlf_toml_array *wlf_toml_array_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Get raw string representation of a value in a table
 * @param table TOML table
 * @param key Key to look up
 * @return Raw string, or NULL if not found. Caller must free.
 */
char *wlf_toml_raw_in(const struct wlf_toml_table *table, const char *key);

/**
 * @brief Get raw string representation of a value in an array
 * @param array TOML array
 * @param idx Index
 * @return Raw string, or NULL if out of bounds. Caller must free.
 */
char *wlf_toml_raw_at(const struct wlf_toml_array *array, int idx);

/**
 * @brief Convert raw string to string value
 * @param raw Raw string
 * @param ret Buffer to store result
 * @return 0 on success, -1 on error
 */
int wlf_toml_rtos(const char *raw, char **ret);

/**
 * @brief Convert raw string to boolean value
 * @param raw Raw string
 * @param ret Pointer to store result
 * @return 0 on success, -1 on error
 */
int wlf_toml_rtob(const char *raw, bool *ret);

/**
 * @brief Convert raw string to integer value
 * @param raw Raw string
 * @param ret Pointer to store result
 * @return 0 on success, -1 on error
 */
int wlf_toml_rtoi(const char *raw, int64_t *ret);

/**
 * @brief Convert raw string to double value
 * @param raw Raw string
 * @param ret Pointer to store result
 * @return 0 on success, -1 on error
 */
int wlf_toml_rtod(const char *raw, double *ret);

/**
 * @brief Convert raw string to timestamp value
 * @param raw Raw string
 * @param ret Pointer to store result
 * @return 0 on success, -1 on error
 */
int wlf_toml_rtots(const char *raw, struct wlf_toml_timestamp *ret);

#endif // WLF_TOML_H
