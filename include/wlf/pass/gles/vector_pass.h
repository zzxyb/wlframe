/**
 * @file        vector_pass.h
 * @brief       GLES vector rendering pass.
 * @details     Declares the GLES implementation of the generic vector pass.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_GLES_VECTOR_PASS_H
#define PASS_GLES_VECTOR_PASS_H

#include "wlf/pass/wlf_vector_pass.h"

/**
 * @brief Creates a GLES-backed vector pass.
 * @return Vector pass, or NULL when the pass cannot be created.
 */
struct wlf_vector_pass *wlf_gles_vector_pass_create(void);

#endif // PASS_GLES_VECTOR_PASS_H
