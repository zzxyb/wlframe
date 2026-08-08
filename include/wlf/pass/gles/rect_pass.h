/**
 * @file        rect_pass.h
 * @brief       GLES rectangle rendering pass.
 * @details     This file declares the GLES implementation of the generic
 *              rectangle rendering pass.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef PASS_GLES_RECT_PASS_H
#define PASS_GLES_RECT_PASS_H

#include "wlf/pass/wlf_rect_pass.h"

/**
 * @brief Creates a GLES-backed rectangle pass.
 * @return Rectangle pass, or NULL on allocation/GL failure.
 */
struct wlf_rect_pass *wlf_gles_rect_pass_create(void);

#endif // PASS_GLES_RECT_PASS_H
