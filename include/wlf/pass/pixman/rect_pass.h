/**
 * @file        rect_pass.h
 * @brief       Pixman rectangle rendering pass.
 * @details     This file declares the Pixman implementation of the generic
 *              rectangle rendering pass.
 * @author      YaoBing Xiao
 * @date        2026-08-09
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-09, initial version\n
 */

#ifndef PASS_PIXMAN_RECT_PASS_H
#define PASS_PIXMAN_RECT_PASS_H

#include "wlf/pass/wlf_rect_pass.h"

/**
 * @brief Creates a pixman-backed rectangle pass.
 * @return Rectangle pass, or NULL on allocation failure.
 */
struct wlf_rect_pass *wlf_pixman_rect_pass_create(void);

#endif // PASS_PIXMAN_RECT_PASS_H
