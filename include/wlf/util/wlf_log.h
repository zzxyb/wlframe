#ifndef WLF_LOG_H
#define WLF_LOG_H

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Enum for log importance levels.
 */
enum wlf_log_importance {
	WLF_SILENT = 0,  /**< No logging */
	WLF_ERROR = 1,   /**< Error messages */
	WLF_INFO = 2,    /**< Informational messages */
	WLF_DEBUG = 3,   /**< Debugging messages */
	WLF_LOG_IMPORTANCE_LAST, /**< Sentinel value for the last log importance */
};

/**
 * @brief Type definition for the log callback function.
 */
typedef void (*wlf_log_func_t)(enum wlf_log_importance importance,
	const char *fmt, va_list args);

/**
 * @brief Type definition for a termination callback function.
 * @param exit_code The exit code to use for termination.
 */
typedef void (*terminate_callback_t)(int exit_code);

/**
 * @brief Initializes logging with a specified verbosity level and callback.
 * @param verbosity The verbosity level for logging.
 * @param callback The callback function to handle log messages.
 */
void wlf_log_init(enum wlf_log_importance verbosity, wlf_log_func_t callback);

/**
 * @brief Gets the current verbosity level of logging.
 * @return The current verbosity level.
 */
enum wlf_log_importance wlf_log_get_verbosity(void);

#ifdef __GNUC__
/**
 * @brief Macro to specify printf-style format attributes for GCC.
 */
#define _WLF_ATTRIB_PRINTF(start, end) __attribute__((format(printf, start, end)))
#else
#define _WLF_ATTRIB_PRINTF(start, end) // No attributes for non-GCC compilers
#endif

/**
 * @brief Logs messages with variable arguments.
 * @param verbosity The verbosity level for the log message.
 * @param format The format string for the log message.
 */
void _wlf_log(enum wlf_log_importance verbosity, const char *format, ...) _WLF_ATTRIB_PRINTF(2, 3);

/**
 * @brief Logs messages with a va_list of arguments.
 * @param verbosity The verbosity level for the log message.
 * @param format The format string for the log message.
 * @param args The variable argument list.
 */
void _wlf_vlog(enum wlf_log_importance verbosity, const char *format, va_list args) _WLF_ATTRIB_PRINTF(2, 0);

/**
 * @brief Aborts the program with a formatted message.
 * @param filename The name of the file where the abort occurred.
 * @return The formatted abort message.
 */
void _wlf_abort(const char *filename, ...) _WLF_ATTRIB_PRINTF(1, 2);

/**
 * @brief Asserts a condition and logs a message if the condition is false.
 * @param condition The condition to check.
 * @param format The format string for the log message.
 * @return true if the condition is true, false otherwise.
 */
bool _wlf_assert(bool condition, const char* format, ...) _WLF_ATTRIB_PRINTF(2, 3);

#ifdef _WLF_REL_SRC_DIR
/**
 * @brief Macro to get the filename relative to the source directory.
 */
#define _WLF_FILENAME ((const char *)__FILE__ + sizeof(_WLF_REL_SRC_DIR) - 1)
#else
#define _WLF_FILENAME __FILE__ /**< Use the full filename if not in a relative source directory */
#endif

/**
 * @brief Macro to log a message with the filename and line number.
 * @param verb The verbosity level for the log message.
 * @param fmt The format string for the log message.
 * @param args The variable argument list.
 */
#define wlf_vlog(verb, fmt, args) \
	_wlf_vlog(verb, "[%s:%d] " fmt, _WLF_FILENAME, __LINE__, args)

#if __STDC_VERSION__ >= 202311L
/**
 * @brief Macros for logging with variadic arguments in C11 and later.
 */
#define wlf_log(verb, fmt, ...) \
	_wlf_log(verb, "[%s:%d] " fmt, _WLF_FILENAME, __LINE__ __VA_OPT__(,) __VA_ARGS__)
#define wlf_log_errno(verb, fmt, ...) \
	wlf_log(verb, fmt ": %s" __VA_OPT__(,) __VA_ARGS__, strerror(errno))

#else
/**
 * @brief Macros for logging with variadic arguments for older C standards.
 */
#define wlf_log(verb, fmt, ...) \
	_wlf_log(verb, "[%s:%d] " fmt, _WLF_FILENAME, __LINE__, ##__VA_ARGS__)
#define wlf_log_errno(verb, fmt, ...) \
	wlf_log(verb, fmt ": %s", ##__VA_ARGS__, strerror(errno))

#endif

/**
 * @brief Macro for aborting the program with a formatted message.
 * @param FMT The format string for the abort message.
 * @param ... Additional arguments for the format string.
 */
#define wlf_abort(FMT, ...) \
	_wlf_abort("[%s:%d] " FMT, _WLF_FILENAME, __LINE__, ##__VA_ARGS__)

/**
 * @brief Macro for asserting a condition with a formatted message.
 * @param COND The condition to check.
 * @param FMT The format string for the log message.
 * @param ... Additional arguments for the format string.
 */
#define wlf_assert(COND, FMT, ...) \
	_wlf_assert(COND, "[%s:%d] %s:" FMT, _WLF_FILENAME, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)

#endif
