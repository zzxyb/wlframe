/**
 * @file        wlf_time.h
 * @brief       Time utility functions for wlframe.
 * @details     This file provides functions for time retrieval and conversion,
 *              including getting the current time in milliseconds, converting
 *              timespec structures to milliseconds or nanoseconds, and performing
 *              timespec arithmetic.
 * @author      YaoBing Xiao
 * @date        2024-05-20
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2024-05-20, initial version\n
 */

#ifndef UTILS_WLF_TIME_H
#define UTILS_WLF_TIME_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Gets the current time in milliseconds.
 * @return The current time in milliseconds since the epoch.
 */
int64_t get_current_time_msec(void);

/**
 * @brief Converts a timespec structure to milliseconds.
 * @param a The timespec structure to convert.
 * @return The equivalent time in milliseconds.
 */
int64_t timespec_to_msec(const struct timespec *a);

/**
 * @brief Converts a timespec structure to nanoseconds.
 * @param a The timespec structure to convert.
 * @return The equivalent time in nanoseconds.
 */
int64_t timespec_to_nsec(const struct timespec *a);

/**
 * @brief Sets a timespec structure from a given number of nanoseconds.
 * @param r The timespec structure to set.
 * @param nsec The number of nanoseconds to convert.
 */
void timespec_from_nsec(struct timespec *r, int64_t nsec);

/**
 * @brief Subtracts one timespec from another.
 * @param r The result timespec structure.
 * @param a The timespec to subtract from.
 * @param b The timespec to subtract.
 */
void timespec_sub(struct timespec *r, const struct timespec *a,
    const struct timespec *b);

#endif // UTILS_WLF_TIME_H
