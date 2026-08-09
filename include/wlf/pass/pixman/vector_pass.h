/**
 * @file        vector_pass.h
 * @brief       Pixman vector rendering pass.
 * @details     Declares the Pixman implementation of the generic vector pass.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_PIXMAN_VECTOR_PASS_H
#define PASS_PIXMAN_VECTOR_PASS_H

#include "wlf/pass/wlf_vector_pass.h"

/**
 * @brief Creates a Pixman-backed vector pass.
 * @return Vector pass, or NULL when the pass cannot be created.
 */
struct wlf_vector_pass *wlf_pixman_vector_pass_create(void);

#endif // PASS_PIXMAN_VECTOR_PASS_H
