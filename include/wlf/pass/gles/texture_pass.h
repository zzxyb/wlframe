/**
 * @file        texture_pass.h
 * @brief       GLES texture rendering pass.
 * @details     Declares the GLES implementation of the generic texture pass.
 * @author      YaoBing Xiao
 * @date        2026-08-05
 * @version     v1.0
 * @par Copyright(c):
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2026-08-05, initial version\n
 */

#ifndef PASS_GLES_TEXTURE_PASS_H
#define PASS_GLES_TEXTURE_PASS_H

#include "wlf/pass/wlf_texture_pass.h"

/**
 * @brief Creates a GLES-backed texture pass.
 * @return Texture pass, or NULL when the pass cannot be created.
 */
struct wlf_texture_pass *wlf_gles_texture_pass_create(void);

#endif // PASS_GLES_TEXTURE_PASS_H
