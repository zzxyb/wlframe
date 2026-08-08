#include "wlf/texture/gles/texture.h"
#include "wlf/renderer/gles/renderer.h"
#include "wlf/types/wlf_pixel_format.h"

#include <GLES2/gl2ext.h>
#include <stddef.h>

static const struct wlf_gles_pixel_format formats[] = {
	{
		.format = WLF_FORMAT_ARGB8888,
		.gl_format = GL_BGRA_EXT,
		.gl_type = GL_UNSIGNED_BYTE,
	},
	{
		.format = WLF_FORMAT_XRGB8888,
		.gl_format = GL_BGRA_EXT,
		.gl_type = GL_UNSIGNED_BYTE,
	},
	{
		.format = WLF_FORMAT_XBGR8888,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_BYTE,
	},
	{
		.format = WLF_FORMAT_ABGR8888,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_BYTE,
	},
	{
		/* RGB byte data is represented by BGR888 in DRM notation. */
		.format = WLF_FORMAT_BGR888,
		.gl_format = GL_RGB,
		.gl_type = GL_UNSIGNED_BYTE,
	},
	{
		.format = WLF_FORMAT_R8,
		.gl_format = GL_LUMINANCE,
		.gl_type = GL_UNSIGNED_BYTE,
	},
#if WLF_LITTLE_ENDIAN
	{
		.format = WLF_FORMAT_RGBX4444,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_SHORT_4_4_4_4,
	},
	{
		.format = WLF_FORMAT_RGBA4444,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_SHORT_4_4_4_4,
	},
	{
		.format = WLF_FORMAT_RGB565,
		.gl_format = GL_RGB,
		.gl_type = GL_UNSIGNED_SHORT_5_6_5,
	},
	{
		.format = WLF_FORMAT_XBGR2101010,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_INT_2_10_10_10_REV_EXT,
	},
	{
		.format = WLF_FORMAT_ABGR2101010,
		.gl_format = GL_RGBA,
		.gl_type = GL_UNSIGNED_INT_2_10_10_10_REV_EXT,
	},
#endif
};

const struct wlf_gles_pixel_format *wlf_gles_pixel_format_from_wlf(
		uint32_t format) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); i++) {
		if (formats[i].format == format) {
			return &formats[i];
		}
	}

	return NULL;
}

bool wlf_gles_pixel_format_is_supported(
		const struct wlf_gles_renderer *renderer,
		const struct wlf_gles_pixel_format *format) {
	if (renderer == NULL || format == NULL) {
		return false;
	}

	if (format->gl_type == GL_UNSIGNED_INT_2_10_10_10_REV_EXT &&
			!renderer->exts.EXT_texture_type_2_10_10_10_REV) {
		return false;
	}
	if (format->gl_type == GL_HALF_FLOAT_OES &&
			!renderer->exts.OES_texture_half_float_linear) {
		return false;
	}
	if (format->gl_type == GL_UNSIGNED_SHORT &&
			!renderer->exts.EXT_texture_norm16) {
		return false;
	}

	return true;
}

const struct wlf_gles_pixel_format *wlf_gles_pixel_format_from_gl(
		GLint gl_format, GLint gl_type, bool alpha) {
	for (size_t i = 0; i < sizeof(formats) / sizeof(*formats); i++) {
		if (formats[i].gl_format != gl_format ||
				formats[i].gl_type != gl_type ||
				pixel_format_has_alpha(formats[i].format) != alpha) {
			continue;
		}

		return &formats[i];
	}

	return NULL;
}
