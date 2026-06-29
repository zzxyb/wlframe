/**
 * @file        rect_pass.h
 * @brief       GLES rectangle rendering pass.
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
