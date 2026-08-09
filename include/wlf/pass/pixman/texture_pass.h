/**
 * @file        texture_pass.h
 * @brief       Pixman texture rendering pass.
 * @details     Declares the Pixman implementation of the generic texture pass.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_PIXMAN_TEXTURE_PASS_H
#define PASS_PIXMAN_TEXTURE_PASS_H

#include "wlf/pass/wlf_texture_pass.h"

/**
 * @brief Creates a Pixman-backed texture pass.
 * @return Texture pass, or NULL when the pass cannot be created.
 */
struct wlf_texture_pass *wlf_pixman_texture_pass_create(void);

#endif // PASS_PIXMAN_TEXTURE_PASS_H
