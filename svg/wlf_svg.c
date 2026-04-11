/**
 * @file        wlf_svg.c
 * @brief       SVG parser implementation used by wlframe.
 */

#include "wlf/svg/wlf_svg.h"
#include "wlf/utils/wlf_log.h"
#include "wlf/shapes/wlf_circle_shape.h"
#include "wlf/shapes/wlf_ellipse_shape.h"
#include "wlf/shapes/wlf_line_shape.h"
#include "wlf/shapes/wlf_path_shape.h"
#include "wlf/shapes/wlf_poly_shape.h"
#include "wlf/shapes/wlf_rect_shape.h"
#include "wlf/shapes/wlf_text_shape.h"
#include "wlf/shapes/wlf_shape.h"
#include "wlf/types/wlf_color.h"
#include "wlf/types/wlf_gradient.h"
#include "wlf/types/wlf_linear_gradient.h"
#include "wlf/types/wlf_radial_gradient.h"
#include "wlf/utils/wlf_linked_list.h"
#include "wlf/utils/wlf_utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Length proportional to radius of a cubic bezier handle for 90deg arcs.
#define WLF_SVG_KAPPA90 (0.5522847493f)

#define WLF_SVG_ALIGN_MIN 0
#define WLF_SVG_ALIGN_MID 1
#define WLF_SVG_ALIGN_MAX 2
#define WLF_SVG_ALIGN_NONE 0
#define WLF_SVG_ALIGN_MEET 1
#define WLF_SVG_ALIGN_SLICE 2

#define WLF_SVG_RGB(r, g, b) (((unsigned int)r) | ((unsigned int)g << 8) | ((unsigned int)b << 16))

#ifdef _MSC_VER
	#pragma warning (disable: 4996) // Switch off security warnings
	#pragma warning (disable: 4100) // Switch off unreferenced formal parameter warnings
	#ifdef __cplusplus
	#define WLF_SVG_INLINE inline
	#else
	#define WLF_SVG_INLINE
	#endif
#else
	#define WLF_SVG_INLINE inline
#endif

struct wlf_svg_named_color wlf_svg_colors[] = {

	{ "red", WLF_SVG_RGB(255, 0, 0) },
	{ "green", WLF_SVG_RGB( 0, 128, 0) },
	{ "blue", WLF_SVG_RGB( 0, 0, 255) },
	{ "yellow", WLF_SVG_RGB(255, 255, 0) },
	{ "cyan", WLF_SVG_RGB( 0, 255, 255) },
	{ "magenta", WLF_SVG_RGB(255, 0, 255) },
	{ "black", WLF_SVG_RGB( 0, 0, 0) },
	{ "grey", WLF_SVG_RGB(128, 128, 128) },
	{ "gray", WLF_SVG_RGB(128, 128, 128) },
	{ "white", WLF_SVG_RGB(255, 255, 255) },

#ifdef NANOSVG_ALL_COLOR_KEYWORDS
	{ "aliceblue", WLF_SVG_RGB(240, 248, 255) },
	{ "antiquewhite", WLF_SVG_RGB(250, 235, 215) },
	{ "aqua", WLF_SVG_RGB( 0, 255, 255) },
	{ "aquamarine", WLF_SVG_RGB(127, 255, 212) },
	{ "azure", WLF_SVG_RGB(240, 255, 255) },
	{ "beige", WLF_SVG_RGB(245, 245, 220) },
	{ "bisque", WLF_SVG_RGB(255, 228, 196) },
	{ "blanchedalmond", WLF_SVG_RGB(255, 235, 205) },
	{ "blueviolet", WLF_SVG_RGB(138, 43, 226) },
	{ "brown", WLF_SVG_RGB(165, 42, 42) },
	{ "burlywood", WLF_SVG_RGB(222, 184, 135) },
	{ "cadetblue", WLF_SVG_RGB( 95, 158, 160) },
	{ "chartreuse", WLF_SVG_RGB(127, 255, 0) },
	{ "chocolate", WLF_SVG_RGB(210, 105, 30) },
	{ "coral", WLF_SVG_RGB(255, 127, 80) },
	{ "cornflowerblue", WLF_SVG_RGB(100, 149, 237) },
	{ "cornsilk", WLF_SVG_RGB(255, 248, 220) },
	{ "crimson", WLF_SVG_RGB(220, 20, 60) },
	{ "darkblue", WLF_SVG_RGB( 0, 0, 139) },
	{ "darkcyan", WLF_SVG_RGB( 0, 139, 139) },
	{ "darkgoldenrod", WLF_SVG_RGB(184, 134, 11) },
	{ "darkgray", WLF_SVG_RGB(169, 169, 169) },
	{ "darkgreen", WLF_SVG_RGB( 0, 100, 0) },
	{ "darkgrey", WLF_SVG_RGB(169, 169, 169) },
	{ "darkkhaki", WLF_SVG_RGB(189, 183, 107) },
	{ "darkmagenta", WLF_SVG_RGB(139, 0, 139) },
	{ "darkolivegreen", WLF_SVG_RGB( 85, 107, 47) },
	{ "darkorange", WLF_SVG_RGB(255, 140, 0) },
	{ "darkorchid", WLF_SVG_RGB(153, 50, 204) },
	{ "darkred", WLF_SVG_RGB(139, 0, 0) },
	{ "darksalmon", WLF_SVG_RGB(233, 150, 122) },
	{ "darkseagreen", WLF_SVG_RGB(143, 188, 143) },
	{ "darkslateblue", WLF_SVG_RGB( 72, 61, 139) },
	{ "darkslategray", WLF_SVG_RGB( 47, 79, 79) },
	{ "darkslategrey", WLF_SVG_RGB( 47, 79, 79) },
	{ "darkturquoise", WLF_SVG_RGB( 0, 206, 209) },
	{ "darkviolet", WLF_SVG_RGB(148, 0, 211) },
	{ "deeppink", WLF_SVG_RGB(255, 20, 147) },
	{ "deepskyblue", WLF_SVG_RGB( 0, 191, 255) },
	{ "dimgray", WLF_SVG_RGB(105, 105, 105) },
	{ "dimgrey", WLF_SVG_RGB(105, 105, 105) },
	{ "dodgerblue", WLF_SVG_RGB( 30, 144, 255) },
	{ "firebrick", WLF_SVG_RGB(178, 34, 34) },
	{ "floralwhite", WLF_SVG_RGB(255, 250, 240) },
	{ "forestgreen", WLF_SVG_RGB( 34, 139, 34) },
	{ "fuchsia", WLF_SVG_RGB(255, 0, 255) },
	{ "gainsboro", WLF_SVG_RGB(220, 220, 220) },
	{ "ghostwhite", WLF_SVG_RGB(248, 248, 255) },
	{ "gold", WLF_SVG_RGB(255, 215, 0) },
	{ "goldenrod", WLF_SVG_RGB(218, 165, 32) },
	{ "greenyellow", WLF_SVG_RGB(173, 255, 47) },
	{ "honeydew", WLF_SVG_RGB(240, 255, 240) },
	{ "hotpink", WLF_SVG_RGB(255, 105, 180) },
	{ "indianred", WLF_SVG_RGB(205, 92, 92) },
	{ "indigo", WLF_SVG_RGB( 75, 0, 130) },
	{ "ivory", WLF_SVG_RGB(255, 255, 240) },
	{ "khaki", WLF_SVG_RGB(240, 230, 140) },
	{ "lavender", WLF_SVG_RGB(230, 230, 250) },
	{ "lavenderblush", WLF_SVG_RGB(255, 240, 245) },
	{ "lawngreen", WLF_SVG_RGB(124, 252, 0) },
	{ "lemonchiffon", WLF_SVG_RGB(255, 250, 205) },
	{ "lightblue", WLF_SVG_RGB(173, 216, 230) },
	{ "lightcoral", WLF_SVG_RGB(240, 128, 128) },
	{ "lightcyan", WLF_SVG_RGB(224, 255, 255) },
	{ "lightgoldenrodyellow", WLF_SVG_RGB(250, 250, 210) },
	{ "lightgray", WLF_SVG_RGB(211, 211, 211) },
	{ "lightgreen", WLF_SVG_RGB(144, 238, 144) },
	{ "lightgrey", WLF_SVG_RGB(211, 211, 211) },
	{ "lightpink", WLF_SVG_RGB(255, 182, 193) },
	{ "lightsalmon", WLF_SVG_RGB(255, 160, 122) },
	{ "lightseagreen", WLF_SVG_RGB( 32, 178, 170) },
	{ "lightskyblue", WLF_SVG_RGB(135, 206, 250) },
	{ "lightslategray", WLF_SVG_RGB(119, 136, 153) },
	{ "lightslategrey", WLF_SVG_RGB(119, 136, 153) },
	{ "lightsteelblue", WLF_SVG_RGB(176, 196, 222) },
	{ "lightyellow", WLF_SVG_RGB(255, 255, 224) },
	{ "lime", WLF_SVG_RGB( 0, 255, 0) },
	{ "limegreen", WLF_SVG_RGB( 50, 205, 50) },
	{ "linen", WLF_SVG_RGB(250, 240, 230) },
	{ "maroon", WLF_SVG_RGB(128, 0, 0) },
	{ "mediumaquamarine", WLF_SVG_RGB(102, 205, 170) },
	{ "mediumblue", WLF_SVG_RGB( 0, 0, 205) },
	{ "mediumorchid", WLF_SVG_RGB(186, 85, 211) },
	{ "mediumpurple", WLF_SVG_RGB(147, 112, 219) },
	{ "mediumseagreen", WLF_SVG_RGB( 60, 179, 113) },
	{ "mediumslateblue", WLF_SVG_RGB(123, 104, 238) },
	{ "mediumspringgreen", WLF_SVG_RGB( 0, 250, 154) },
	{ "mediumturquoise", WLF_SVG_RGB( 72, 209, 204) },
	{ "mediumvioletred", WLF_SVG_RGB(199, 21, 133) },
	{ "midnightblue", WLF_SVG_RGB( 25, 25, 112) },
	{ "mintcream", WLF_SVG_RGB(245, 255, 250) },
	{ "mistyrose", WLF_SVG_RGB(255, 228, 225) },
	{ "moccasin", WLF_SVG_RGB(255, 228, 181) },
	{ "navajowhite", WLF_SVG_RGB(255, 222, 173) },
	{ "navy", WLF_SVG_RGB( 0, 0, 128) },
	{ "oldlace", WLF_SVG_RGB(253, 245, 230) },
	{ "olive", WLF_SVG_RGB(128, 128, 0) },
	{ "olivedrab", WLF_SVG_RGB(107, 142, 35) },
	{ "orange", WLF_SVG_RGB(255, 165, 0) },
	{ "orangered", WLF_SVG_RGB(255, 69, 0) },
	{ "orchid", WLF_SVG_RGB(218, 112, 214) },
	{ "palegoldenrod", WLF_SVG_RGB(238, 232, 170) },
	{ "palegreen", WLF_SVG_RGB(152, 251, 152) },
	{ "paleturquoise", WLF_SVG_RGB(175, 238, 238) },
	{ "palevioletred", WLF_SVG_RGB(219, 112, 147) },
	{ "papayawhip", WLF_SVG_RGB(255, 239, 213) },
	{ "peachpuff", WLF_SVG_RGB(255, 218, 185) },
	{ "peru", WLF_SVG_RGB(205, 133, 63) },
	{ "pink", WLF_SVG_RGB(255, 192, 203) },
	{ "plum", WLF_SVG_RGB(221, 160, 221) },
	{ "powderblue", WLF_SVG_RGB(176, 224, 230) },
	{ "purple", WLF_SVG_RGB(128, 0, 128) },
	{ "rosybrown", WLF_SVG_RGB(188, 143, 143) },
	{ "royalblue", WLF_SVG_RGB( 65, 105, 225) },
	{ "saddlebrown", WLF_SVG_RGB(139, 69, 19) },
	{ "salmon", WLF_SVG_RGB(250, 128, 114) },
	{ "sandybrown", WLF_SVG_RGB(244, 164, 96) },
	{ "seagreen", WLF_SVG_RGB( 46, 139, 87) },
	{ "seashell", WLF_SVG_RGB(255, 245, 238) },
	{ "sienna", WLF_SVG_RGB(160, 82, 45) },
	{ "silver", WLF_SVG_RGB(192, 192, 192) },
	{ "skyblue", WLF_SVG_RGB(135, 206, 235) },
	{ "slateblue", WLF_SVG_RGB(106, 90, 205) },
	{ "slategray", WLF_SVG_RGB(112, 128, 144) },
	{ "slategrey", WLF_SVG_RGB(112, 128, 144) },
	{ "snow", WLF_SVG_RGB(255, 250, 250) },
	{ "springgreen", WLF_SVG_RGB( 0, 255, 127) },
	{ "steelblue", WLF_SVG_RGB( 70, 130, 180) },
	{ "tan", WLF_SVG_RGB(210, 180, 140) },
	{ "teal", WLF_SVG_RGB( 0, 128, 128) },
	{ "thistle", WLF_SVG_RGB(216, 191, 216) },
	{ "tomato", WLF_SVG_RGB(255, 99, 71) },
	{ "turquoise", WLF_SVG_RGB( 64, 224, 208) },
	{ "violet", WLF_SVG_RGB(238, 130, 238) },
	{ "wheat", WLF_SVG_RGB(245, 222, 179) },
	{ "whitesmoke", WLF_SVG_RGB(245, 245, 245) },
	{ "yellowgreen", WLF_SVG_RGB(154, 205, 50) },
#endif
};

static int wlf_svg_isspace(char c)
{
	return strchr(" \t\n\v\f\r", c) != 0;
}

static int wlf_svg_isdigit(char c)
{
	return c >= '0' && c <= '9';
}

static WLF_SVG_INLINE float wlf_svg_minf(float a, float b) { return a < b ? a : b; }
static WLF_SVG_INLINE float wlf_svg_maxf(float a, float b) { return a > b ? a : b; }

#define WLF_SVG_XML_TAG 1
#define WLF_SVG_XML_CONTENT 2
#define WLF_SVG_XML_MAX_ATTRIBS 256

static void wlf_svg_parse_content(char* s,
							   void (*contentCb)(void* ud, const char* s),
							   void* ud)
{
	// Trim start white spaces
	while (*s && wlf_svg_isspace(*s)) s++;
	if (!*s) return;

	if (contentCb)
		(*contentCb)(ud, s);
}

static void wlf_svg_parse_element(char* s,
							   void (*startelCb)(void* ud, const char* el, const char** attr),
							   void (*endelCb)(void* ud, const char* el),
							   void* ud)
{
	const char* attr[WLF_SVG_XML_MAX_ATTRIBS];
	int nattr = 0;
	char* name;
	int start = 0;
	int end = 0;
	char quote;

	// Skip white space after the '<'
	while (*s && wlf_svg_isspace(*s)) s++;

	// Check if the tag is end tag
	if (*s == '/') {
		s++;
		end = 1;
	} else {
		start = 1;
	}

	// Skip comments, data and preprocessor stuff.
	if (!*s || *s == '?' || *s == '!')
		return;

	// Get tag name
	name = s;
	while (*s && !wlf_svg_isspace(*s)) s++;
	if (*s) { *s++ = '\0'; }

	// Get attribs
	while (!end && *s && nattr < WLF_SVG_XML_MAX_ATTRIBS-3) {
		char* name = NULL;
		char* value = NULL;

		// Skip white space before the attrib name
		while (*s && wlf_svg_isspace(*s)) s++;
		if (!*s) break;
		if (*s == '/') {
			end = 1;
			break;
		}
		name = s;
		// Find end of the attrib name.
		while (*s && !wlf_svg_isspace(*s) && *s != '=') s++;
		if (*s) { *s++ = '\0'; }
		// Skip until the beginning of the value.
		while (*s && *s != '\"' && *s != '\'') s++;
		if (!*s) break;
		quote = *s;
		s++;
		// Store value and find the end of it.
		value = s;
		while (*s && *s != quote) s++;
		if (*s) { *s++ = '\0'; }

		// Store only well formed attributes
		if (name && value) {
			attr[nattr++] = name;
			attr[nattr++] = value;
		}
	}

	// List terminator
	attr[nattr++] = 0;
	attr[nattr++] = 0;

	// Call callbacks.
	if (start && startelCb)
		(*startelCb)(ud, name, attr);
	if (end && endelCb)
		(*endelCb)(ud, name);
}

static int wlf_svg_parse_xml(char* input,
				   void (*startelCb)(void* ud, const char* el, const char** attr),
				   void (*endelCb)(void* ud, const char* el),
				   void (*contentCb)(void* ud, const char* s),
				   void* ud)
{
	char* s = input;
	char* mark = s;
	int state = WLF_SVG_XML_CONTENT;
	while (*s) {
		if (*s == '<' && state == WLF_SVG_XML_CONTENT) {
			// Start of a tag
			*s++ = '\0';
			wlf_svg_parse_content(mark, contentCb, ud);
			mark = s;
			state = WLF_SVG_XML_TAG;
		} else if (*s == '>' && state == WLF_SVG_XML_TAG) {
			// Start of a content or new tag.
			*s++ = '\0';
			wlf_svg_parse_element(mark, startelCb, endelCb, ud);
			mark = s;
			state = WLF_SVG_XML_CONTENT;
		} else {
			s++;
		}
	}

	return 1;
}


static void wlf_svg_xform_identity(float* t)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void wlf_svg_xform_set_translation(float* t, float tx, float ty)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = tx; t[5] = ty;
}

static void wlf_svg_xform_set_scale(float* t, float sx, float sy)
{
	t[0] = sx; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = sy;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void wlf_svg_xform_set_skew_x(float* t, float a)
{
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = tanf(a); t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void wlf_svg_xform_set_skew_y(float* t, float a)
{
	t[0] = 1.0f; t[1] = tanf(a);
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void wlf_svg_xform_set_rotation(float* t, float a)
{
	float cs = cosf(a), sn = sinf(a);
	t[0] = cs; t[1] = sn;
	t[2] = -sn; t[3] = cs;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void wlf_svg_xform_multiply(float* t, float* s)
{
	float t0 = t[0] * s[0] + t[1] * s[2];
	float t2 = t[2] * s[0] + t[3] * s[2];
	float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
	t[1] = t[0] * s[1] + t[1] * s[3];
	t[3] = t[2] * s[1] + t[3] * s[3];
	t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
	t[0] = t0;
	t[2] = t2;
	t[4] = t4;
}

static void wlf_svg_xform_inverse(float* inv, float* t)
{
	double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
	if (det > -1e-6 && det < 1e-6) {
		wlf_svg_xform_identity(t);
		return;
	}
	invdet = 1.0 / det;
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static void wlf_svg_xform_premultiply(float* t, float* s)
{
	float s2[6];
	memcpy(s2, s, sizeof(float)*6);
	wlf_svg_xform_multiply(s2, t);
	memcpy(t, s2, sizeof(float)*6);
}

static void wlf_svg_xform_point(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2] + t[4];
	*dy = x*t[1] + y*t[3] + t[5];
}

static void wlf_svg_xform_vec(float* dx, float* dy, float x, float y, float* t)
{
	*dx = x*t[0] + y*t[2];
	*dy = x*t[1] + y*t[3];
}

#define WLF_SVG_EPSILON (1e-12)

static int wlf_svg_pt_in_bounds(float* pt, float* bounds)
{
	return pt[0] >= bounds[0] && pt[0] <= bounds[2] && pt[1] >= bounds[1] && pt[1] <= bounds[3];
}


static double wlf_svg_eval_bezier(double t, double p0, double p1, double p2, double p3)
{
	double it = 1.0-t;
	return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}

static void wlf_svg_curve_bounds(float* bounds, float* curve)
{
	int i, j, count;
	double roots[2], a, b, c, b2ac, t, v;
	float* v0 = &curve[0];
	float* v1 = &curve[2];
	float* v2 = &curve[4];
	float* v3 = &curve[6];

	// Start the bounding box by end points
	bounds[0] = wlf_svg_minf(v0[0], v3[0]);
	bounds[1] = wlf_svg_minf(v0[1], v3[1]);
	bounds[2] = wlf_svg_maxf(v0[0], v3[0]);
	bounds[3] = wlf_svg_maxf(v0[1], v3[1]);

	// Bezier curve fits inside the convex hull of it's control points.
	// If control points are inside the bounds, we're done.
	if (wlf_svg_pt_in_bounds(v1, bounds) && wlf_svg_pt_in_bounds(v2, bounds))
		return;

	// Add bezier curve inflection points in X and Y.
	for (i = 0; i < 2; i++) {
		a = -3.0 * v0[i] + 9.0 * v1[i] - 9.0 * v2[i] + 3.0 * v3[i];
		b = 6.0 * v0[i] - 12.0 * v1[i] + 6.0 * v2[i];
		c = 3.0 * v1[i] - 3.0 * v0[i];
		count = 0;
		if (fabs(a) < WLF_SVG_EPSILON) {
			if (fabs(b) > WLF_SVG_EPSILON) {
				t = -c / b;
				if (t > WLF_SVG_EPSILON && t < 1.0-WLF_SVG_EPSILON)
					roots[count++] = t;
			}
		} else {
			b2ac = b*b - 4.0*c*a;
			if (b2ac > WLF_SVG_EPSILON) {
				t = (-b + sqrt(b2ac)) / (2.0 * a);
				if (t > WLF_SVG_EPSILON && t < 1.0-WLF_SVG_EPSILON)
					roots[count++] = t;
				t = (-b - sqrt(b2ac)) / (2.0 * a);
				if (t > WLF_SVG_EPSILON && t < 1.0-WLF_SVG_EPSILON)
					roots[count++] = t;
			}
		}
		for (j = 0; j < count; j++) {
			v = wlf_svg_eval_bezier(roots[j], v0[i], v1[i], v2[i], v3[i]);
			bounds[0+i] = wlf_svg_minf(bounds[0+i], (float)v);
			bounds[2+i] = wlf_svg_maxf(bounds[2+i], (float)v);
		}
	}
}

static unsigned char wlf_svg_encode_paint_order(enum wlf_svg_paint_order a, enum wlf_svg_paint_order b, enum wlf_svg_paint_order c) {
    return (a & 0x03) | ((b & 0x03) << 2) | ((c & 0x03) << 4);
}

static float wlf_svg_shape_fill_alpha(const struct wlf_svg_shape *shape)
{
	if (shape == NULL) {
		return 1.0f;
	}
	if (shape->opacity < 0.0f || shape->fill_opacity < 0.0f) {
		return 0.0f;
	}
	if (shape->opacity > 1.0f || shape->fill_opacity > 1.0f) {
		float out = shape->opacity * shape->fill_opacity;
		return out > 1.0f ? 1.0f : out;
	}
	return shape->opacity * shape->fill_opacity;
}

static float wlf_svg_shape_stroke_alpha(const struct wlf_svg_shape *shape)
{
	if (shape == NULL) {
		return 1.0f;
	}
	if (shape->opacity < 0.0f || shape->stroke_opacity < 0.0f) {
		return 0.0f;
	}
	if (shape->opacity > 1.0f || shape->stroke_opacity > 1.0f) {
		float out = shape->opacity * shape->stroke_opacity;
		return out > 1.0f ? 1.0f : out;
	}
	return shape->opacity * shape->stroke_opacity;
}

static struct wlf_svg_parser *wlf_svg_create_parser(void)
{
	struct wlf_svg_parser *p = malloc(sizeof(struct wlf_svg_parser));
	if (p == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate wlf_svg_parser");
		goto error;
	}
	memset(p, 0, sizeof(struct wlf_svg_parser));

	p->image = (struct wlf_svg_image*)malloc(sizeof(struct wlf_svg_image));
	if (p->image == NULL) goto error;
	memset(p->image, 0, sizeof(struct wlf_svg_image));

	// Init style
	wlf_svg_xform_identity(p->attr[0].xform);
	memset(p->attr[0].id, 0, sizeof p->attr[0].id);
	p->attr[0].fillColor = WLF_SVG_RGB(0,0,0);
	p->attr[0].strokeColor = WLF_SVG_RGB(0,0,0);
	p->attr[0].opacity = 1;
	p->attr[0].fillOpacity = 1;
	p->attr[0].strokeOpacity = 1;
	p->attr[0].stopOpacity = 1;
	p->attr[0].strokeWidth = 1;
	p->attr[0].strokeLineJoin = WLF_SVG_JOIN_MITER;
	p->attr[0].strokeLineCap = WLF_SVG_CAP_BUTT;
	p->attr[0].miterLimit = 4;
	p->attr[0].fillRule = WLF_SVG_FILL_RULE_NONZERO;
	p->attr[0].hasFill = 1;
	p->attr[0].visible = 1;
	p->attr[0].paintOrder = wlf_svg_encode_paint_order(WLF_SVG_PAINT_FILL, WLF_SVG_PAINT_STROKE, WLF_SVG_PAINT_MARKERS);
	p->shape_geometry = NULL;
	strncpy(p->image->xml_version, "1.0", sizeof(p->image->xml_version) - 1);
	strncpy(p->image->xml_encoding, "UTF-8", sizeof(p->image->xml_encoding) - 1);
	strncpy(p->image->xmlns, "http://www.w3.org/2000/svg", sizeof(p->image->xmlns) - 1);

	return p;

error:
	if (p) {
		if (p->image) free(p->image);
		free(p);
	}
	return NULL;
}

static void wlf_svg_delete_paths(struct wlf_path* path)
{
	wlf_path_destroy_list(path);
}

static struct wlf_color wlf_svg_color_to_wlf(unsigned int color)
{
	uint8_t r = color & 0xFFu;
	uint8_t g = (color >> 8) & 0xFFu;
	uint8_t b = (color >> 16) & 0xFFu;
	uint8_t a = (color >> 24) & 0xFFu;
	if ((color & 0xFF000000u) == 0) {
		a = 255;
	}
	return wlf_color_from_rgba8(r, g, b, a);
}

static struct wlf_gradient *wlf_svg_create_solid_gradient(unsigned int color, float opacity_scale)
{
	struct wlf_color c = wlf_svg_color_to_wlf(color);
	c.a *= opacity_scale;
	if (c.a < 0.0) c.a = 0.0;
	if (c.a > 1.0) c.a = 1.0;

	struct wlf_gradient_stop stops[2] = {
		{ .offset = 0.0, .color = c },
		{ .offset = 1.0, .color = c },
	};
	struct wlf_fpoint start = { .x = 0.0, .y = 0.0 };
	struct wlf_fpoint end = { .x = 1.0, .y = 0.0 };
	struct wlf_linear_gradient *linear =
		wlf_linear_gradient_create(start, end, stops, 2);
	if (linear == NULL)
		return NULL;
	return &linear->base;
}

static struct wlf_gradient *wlf_svg_clone_gradient(struct wlf_gradient *grad)
{
	if (grad == NULL || grad->stop_count == 0)
		return NULL;

	struct wlf_gradient_stop *stops =
		calloc(grad->stop_count, sizeof(*stops));
	if (stops == NULL)
		return NULL;
	memcpy(stops, grad->stops, grad->stop_count * sizeof(*stops));

	struct wlf_gradient *copy = NULL;
	if (wlf_gradient_is_linear(grad)) {
		struct wlf_linear_gradient *lin = wlf_linear_gradient_from_gradient(grad);
		copy = (struct wlf_gradient *)wlf_linear_gradient_create(
			lin->start, lin->end, stops, grad->stop_count);
	} else if (wlf_gradient_is_radial(grad)) {
		struct wlf_radial_gradient *rad = wlf_radial_gradient_from_gradient(grad);
		copy = (struct wlf_gradient *)wlf_radial_gradient_create(
			rad->center, rad->focal, rad->radius, stops, grad->stop_count);
	}

	free(stops);

	if (copy && grad->has_xform) {
		wlf_gradient_set_transform(copy, grad->xform);
	}
	return copy;
}

static void wlf_svg_shape_destroy(struct wlf_shape *shape)
{
	struct wlf_svg_shape *svg = wlf_svg_shape_from_shape(shape);
	wlf_path_destroy_list(svg->paths);
	wlf_gradient_destroy(svg->fill);
	wlf_gradient_destroy(svg->stroke);
	wlf_shape_destroy(svg->geometry);
	free(svg);
}

static const struct wlf_shape_impl wlf_svg_shape_impl = {
	.destroy = wlf_svg_shape_destroy,
	.clone = NULL,
};

bool wlf_shape_is_svg(struct wlf_shape *shape)
{
	return shape != NULL && shape->impl == &wlf_svg_shape_impl;
}

struct wlf_svg_shape *wlf_svg_shape_from_shape(struct wlf_shape *shape)
{
	assert(shape->impl == &wlf_svg_shape_impl);

	struct wlf_svg_shape *svg_shape =
		wlf_container_of(shape, svg_shape, base);

	return svg_shape;
}

static void wlf_svg_delete_gradient_data(struct wlf_svg_gradient_data* grad)
{
	struct wlf_svg_gradient_data* next;
	while (grad != NULL) {
		next = grad->next;
		free(grad->stops);
		free(grad);
		grad = next;
	}
}

static void wlf_svg_delete_symbol_data(struct wlf_svg_symbol_data *sym)
{
	struct wlf_shape *shape, *snext;
	struct wlf_svg_symbol_data *next;
	while (sym) {
		next = sym->next;
		shape = sym->shapes;
		while (shape) {
			struct wlf_svg_shape *svg = wlf_svg_shape_from_shape(shape);
			snext = (struct wlf_shape *)svg->next;
			wlf_shape_destroy(shape);
			shape = snext;
		}
		free(sym);
		sym = next;
	}
}

static void wlf_svg_delete_use_data(struct wlf_svg_use_data *use)
{
	while (use) {
		struct wlf_svg_use_data *next = use->next;
		free(use);
		use = next;
	}
}

static void wlf_svg_delete_parser(struct wlf_svg_parser *p)
{
	if (p != NULL) {
		wlf_svg_delete_paths(p->plist);
		wlf_shape_destroy(p->shape_geometry);
		wlf_svg_delete_gradient_data(p->gradients);
		wlf_svg_delete_symbol_data(p->symbols);
		wlf_svg_delete_use_data(p->uses);
		wlf_svg_destroy(p->image);
		free(p->pts);
		free(p);
	}
}

static void wlf_svg_reset_path(struct wlf_svg_parser *p)
{
	p->npts = 0;
}

static void wlf_svg_add_point(struct wlf_svg_parser *p, float x, float y)
{
	if (p->npts+1 > p->cpts) {
		p->cpts = p->cpts ? p->cpts*2 : 8;
		p->pts = (float*)realloc(p->pts, p->cpts*2*sizeof(float));
		if (!p->pts) return;
	}
	p->pts[p->npts*2+0] = x;
	p->pts[p->npts*2+1] = y;
	p->npts++;
}

static void wlf_svg_move_to(struct wlf_svg_parser *p, float x, float y)
{
	if (p->npts > 0) {
		p->pts[(p->npts-1)*2+0] = x;
		p->pts[(p->npts-1)*2+1] = y;
	} else {
		wlf_svg_add_point(p, x, y);
	}
}

static void wlf_svg_line_to(struct wlf_svg_parser *p, float x, float y)
{
	float px,py, dx,dy;
	if (p->npts > 0) {
		px = p->pts[(p->npts-1)*2+0];
		py = p->pts[(p->npts-1)*2+1];
		dx = x - px;
		dy = y - py;
		wlf_svg_add_point(p, px + dx/3.0f, py + dy/3.0f);
		wlf_svg_add_point(p, x - dx/3.0f, y - dy/3.0f);
		wlf_svg_add_point(p, x, y);
	}
}

static void wlf_svg_cubic_bez_to(struct wlf_svg_parser *p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
	if (p->npts > 0) {
		wlf_svg_add_point(p, cpx1, cpy1);
		wlf_svg_add_point(p, cpx2, cpy2);
		wlf_svg_add_point(p, x, y);
	}
}

static struct wlf_svg_attrib *wlf_svg_get_attr(struct wlf_svg_parser *p)
{
	return &p->attr[p->attrHead];
}

static void wlf_svg_push_attr(struct wlf_svg_parser *p)
{
	if (p->attrHead < WLF_SVG_MAX_ATTR-1) {
		p->attrHead++;
		memcpy(&p->attr[p->attrHead], &p->attr[p->attrHead-1], sizeof(struct wlf_svg_attrib));
	}
}

static void wlf_svg_pop_attr(struct wlf_svg_parser *p)
{
	if (p->attrHead > 0)
		p->attrHead--;
}

static float wlf_svg_actual_orig_x(struct wlf_svg_parser *p)
{
	return p->viewMinx;
}

static float wlf_svg_actual_orig_y(struct wlf_svg_parser *p)
{
	return p->viewMiny;
}

static float wlf_svg_actual_width(struct wlf_svg_parser *p)
{
	return p->viewWidth;
}

static float wlf_svg_actual_height(struct wlf_svg_parser *p)
{
	return p->viewHeight;
}

static float wlf_svg_actual_length(struct wlf_svg_parser *p)
{
	float w = wlf_svg_actual_width(p), h = wlf_svg_actual_height(p);
	return sqrtf(w*w + h*h) / sqrtf(2.0f);
}

static float wlf_svg_convert_to_pixels(struct wlf_svg_parser *p, struct wlf_svg_coordinate c, float orig, float length)
{
	struct wlf_svg_attrib *attr = wlf_svg_get_attr(p);
	switch (c.units) {
		case WLF_SVG_UNITS_USER:		return c.value;
		case WLF_SVG_UNITS_PX:			return c.value;
		case WLF_SVG_UNITS_PT:			return c.value / 72.0f * p->dpi;
		case WLF_SVG_UNITS_PC:			return c.value / 6.0f * p->dpi;
		case WLF_SVG_UNITS_MM:			return c.value / 25.4f * p->dpi;
		case WLF_SVG_UNITS_CM:			return c.value / 2.54f * p->dpi;
		case WLF_SVG_UNITS_IN:			return c.value * p->dpi;
		case WLF_SVG_UNITS_EM:			return c.value * attr->fontSize;
		case WLF_SVG_UNITS_EX:			return c.value * attr->fontSize * 0.52f; // x-height of Helvetica.
		case WLF_SVG_UNITS_PERCENT:	return orig + c.value / 100.0f * length;
		default:					return c.value;
	}
	return c.value;
}

static struct wlf_svg_gradient_data* wlf_svg_find_gradient_data(struct wlf_svg_parser *p, const char* id)
{
	struct wlf_svg_gradient_data* grad = p->gradients;
	if (id == NULL || *id == '\0')
		return NULL;
	while (grad != NULL) {
		if (strcmp(grad->id, id) == 0)
			return grad;
		grad = grad->next;
	}
	return NULL;
}

static float wlf_svg_get_average_scale(float* t);

static struct wlf_gradient *wlf_svg_create_gradient(struct wlf_svg_parser *p, const char* id,
	const float* localBounds, const float *shape_xform, float opacity_scale)
{
	struct wlf_svg_gradient_data* data = NULL;
	struct wlf_svg_gradient_data* ref = NULL;
	struct wlf_svg_gradient_stop* stops = NULL;
	struct wlf_gradient_stop *grad_stops = NULL;
	struct wlf_gradient *grad = NULL;
	float ox, oy, sw, sh, sl;
	int nstops = 0;
	int refIter;

	data = wlf_svg_find_gradient_data(p, id);
	if (data == NULL) return NULL;

	// TODO: use ref to fill in all unset values too.
	ref = data;
	refIter = 0;
	while (ref != NULL) {
		struct wlf_svg_gradient_data* nextRef = NULL;
		if (stops == NULL && ref->stops != NULL) {
			stops = ref->stops;
			nstops = ref->nstops;
			break;
		}
		nextRef = wlf_svg_find_gradient_data(p, ref->ref);
		if (nextRef == ref) break; // prevent infite loops on malformed data
		ref = nextRef;
		refIter++;
		if (refIter > 32) break; // prevent infite loops on malformed data
	}
	if (stops == NULL) return NULL;

	grad_stops = calloc((size_t)nstops, sizeof(*grad_stops));
	if (grad_stops == NULL) return NULL;

	// The shape width and height.
	if (data->units == WLF_SVG_OBJECT_SPACE) {
		ox = localBounds[0];
		oy = localBounds[1];
		sw = localBounds[2] - localBounds[0];
		sh = localBounds[3] - localBounds[1];
	} else {
		ox = wlf_svg_actual_orig_x(p);
		oy = wlf_svg_actual_orig_y(p);
		sw = wlf_svg_actual_width(p);
		sh = wlf_svg_actual_height(p);
	}
	sl = sqrtf(sw*sw + sh*sh) / sqrtf(2.0f);

	if (data->type == WLF_SVG_PAINT_LINEAR_GRADIENT) {
		float x1, y1, x2, y2;
		float xform[6];
		struct wlf_fpoint start, end;

		x1 = wlf_svg_convert_to_pixels(p, data->linear.x1, ox, sw);
		y1 = wlf_svg_convert_to_pixels(p, data->linear.y1, oy, sh);
		x2 = wlf_svg_convert_to_pixels(p, data->linear.x2, ox, sw);
		y2 = wlf_svg_convert_to_pixels(p, data->linear.y2, oy, sh);

		memcpy(xform, data->xform, sizeof(xform));
		wlf_svg_xform_multiply(xform, (float *)shape_xform);
		wlf_svg_xform_point(&x1, &y1, x1, y1, xform);
		wlf_svg_xform_point(&x2, &y2, x2, y2, xform);
		start = (struct wlf_fpoint){ .x = x1, .y = y1 };
		end = (struct wlf_fpoint){ .x = x2, .y = y2 };

		for (int i = 0; i < nstops; i++) {
			struct wlf_color c = wlf_svg_color_to_wlf(stops[i].color);
			c.a *= opacity_scale;
			if (c.a < 0.0) c.a = 0.0;
			if (c.a > 1.0) c.a = 1.0;
			grad_stops[i].offset = stops[i].offset;
			grad_stops[i].color = c;
		}

		struct wlf_linear_gradient *linear =
			wlf_linear_gradient_create(start, end, grad_stops, (size_t)nstops);
		if (linear) {
			grad = &linear->base;
		}
	} else if (data->type == WLF_SVG_PAINT_RADIAL_GRADIENT) {
		float cx, cy, fx, fy, r;
		float xform[6];
		float avg_scale;
		struct wlf_fpoint center, focal;

		cx = wlf_svg_convert_to_pixels(p, data->radial.cx, ox, sw);
		cy = wlf_svg_convert_to_pixels(p, data->radial.cy, oy, sh);
		fx = wlf_svg_convert_to_pixels(p, data->radial.fx, ox, sw);
		fy = wlf_svg_convert_to_pixels(p, data->radial.fy, oy, sh);
		r = wlf_svg_convert_to_pixels(p, data->radial.r, 0, sl);

		memcpy(xform, data->xform, sizeof(xform));
		wlf_svg_xform_multiply(xform, (float *)shape_xform);
		avg_scale = wlf_svg_get_average_scale(xform);
		wlf_svg_xform_point(&cx, &cy, cx, cy, xform);
		wlf_svg_xform_point(&fx, &fy, fx, fy, xform);
		r *= avg_scale;

		center = (struct wlf_fpoint){ .x = cx, .y = cy };
		focal = (struct wlf_fpoint){ .x = fx, .y = fy };

		for (int i = 0; i < nstops; i++) {
			struct wlf_color c = wlf_svg_color_to_wlf(stops[i].color);
			c.a *= opacity_scale;
			if (c.a < 0.0) c.a = 0.0;
			if (c.a > 1.0) c.a = 1.0;
			grad_stops[i].offset = stops[i].offset;
			grad_stops[i].color = c;
		}

		struct wlf_radial_gradient *radial =
			wlf_radial_gradient_create(center, focal, r, grad_stops, (size_t)nstops);
		if (radial) {
			grad = &radial->base;
		}
	}

	free(grad_stops);
	return grad;
}

static float wlf_svg_get_average_scale(float* t)
{
	float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
	float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
	return (sx + sy) * 0.5f;
}

static void wlf_svg_get_local_bounds(float* bounds, struct wlf_svg_shape *shape, float* xform)
{
	struct wlf_path* path = NULL;
	float curve[4*2], curveBounds[4];
	int i, first = 1;
	for (path = shape->paths; path != NULL; path = path->next) {
		wlf_svg_xform_point(&curve[0], &curve[1], path->pts[0], path->pts[1], xform);
		for (i = 0; i < path->npts-1; i += 3) {
			wlf_svg_xform_point(&curve[2], &curve[3], path->pts[(i+1)*2], path->pts[(i+1)*2+1], xform);
			wlf_svg_xform_point(&curve[4], &curve[5], path->pts[(i+2)*2], path->pts[(i+2)*2+1], xform);
			wlf_svg_xform_point(&curve[6], &curve[7], path->pts[(i+3)*2], path->pts[(i+3)*2+1], xform);
			wlf_svg_curve_bounds(curveBounds, curve);
			if (first) {
				bounds[0] = curveBounds[0];
				bounds[1] = curveBounds[1];
				bounds[2] = curveBounds[2];
				bounds[3] = curveBounds[3];
				first = 0;
			} else {
				bounds[0] = wlf_svg_minf(bounds[0], curveBounds[0]);
				bounds[1] = wlf_svg_minf(bounds[1], curveBounds[1]);
				bounds[2] = wlf_svg_maxf(bounds[2], curveBounds[2]);
				bounds[3] = wlf_svg_maxf(bounds[3], curveBounds[3]);
			}
			curve[0] = curve[6];
			curve[1] = curve[7];
		}
	}
}

static void wlf_svg_apply_paint_to_geometry(struct wlf_shape *geometry,
	const struct wlf_svg_attrib *attr, float stroke_width,
	struct wlf_gradient *fill_gradient,
	struct wlf_gradient *stroke_gradient)
{
	struct wlf_shape_state *paint = NULL;

	if (geometry == NULL || attr == NULL) {
		return;
	}

	if (wlf_shape_is_rect(geometry)) {
		paint = &wlf_rect_shape_from_shape(geometry)->state;
	} else if (wlf_shape_is_circle(geometry)) {
		paint = &wlf_circle_shape_from_shape(geometry)->state;
	} else if (wlf_shape_is_ellipse(geometry)) {
		paint = &wlf_ellipse_shape_from_shape(geometry)->state;
	} else if (wlf_shape_is_line(geometry)) {
		paint = &wlf_line_shape_from_shape(geometry)->state;
	} else if (wlf_shape_is_poly(geometry)) {
		paint = &wlf_poly_shape_from_shape(geometry)->state;
	} else if (wlf_shape_is_path(geometry)) {
		paint = &wlf_path_shape_from_shape(geometry)->state;
	}

	if (paint == NULL) {
		return;
	}

	paint->fill_color = wlf_svg_color_to_wlf(attr->fillColor);
	paint->stroke_color = wlf_svg_color_to_wlf(attr->strokeColor);
	paint->stroke_width = stroke_width;
	paint->opacity = attr->opacity;
	paint->fill_opacity = attr->fillOpacity;
	paint->stroke_opacity = attr->strokeOpacity;
	paint->has_fill = attr->hasFill;
	paint->has_stroke = attr->hasStroke;
	paint->fill_gradient = fill_gradient;
	paint->stroke_gradient = stroke_gradient;
	paint->fill_color.a *= wlf_shape_state_fill_alpha(paint);
	paint->stroke_color.a *= wlf_shape_state_stroke_alpha(paint);
}

static void wlf_svg_add_shape(struct wlf_svg_parser *p)
{
	struct wlf_svg_attrib* attr = wlf_svg_get_attr(p);
	float scale = 1.0f;
	struct wlf_svg_shape* shape;
	struct wlf_path* path = NULL;
	int i;

	if (p->plist == NULL)
	{
		wlf_shape_destroy(p->shape_geometry);
		p->shape_geometry = NULL;
		return;
	}

	shape = (struct wlf_svg_shape*)malloc(sizeof(struct wlf_svg_shape));
	if (shape == NULL) goto error;
	memset(shape, 0, sizeof(struct wlf_svg_shape));
	wlf_shape_init(&shape->base, &wlf_svg_shape_impl);

	memcpy(shape->id, attr->id, sizeof shape->id);
	memcpy(shape->fill_gradient, attr->fillGradient, sizeof shape->fill_gradient);
	memcpy(shape->stroke_gradient, attr->strokeGradient, sizeof shape->stroke_gradient);
	memcpy(shape->xform, attr->xform, sizeof shape->xform);
	scale = wlf_svg_get_average_scale(attr->xform);
	shape->stroke_width = attr->strokeWidth * scale;
	shape->stroke_dash_offset = attr->strokeDashOffset * scale;
	shape->stroke_dash_count = (char)attr->strokeDashCount;
	for (i = 0; i < attr->strokeDashCount; i++)
		shape->stroke_dash_array[i] = attr->strokeDashArray[i] * scale;
	shape->stroke_line_join = attr->strokeLineJoin;
	shape->stroke_line_cap = attr->strokeLineCap;
	shape->miter_limit = attr->miterLimit;
	shape->fill_rule = attr->fillRule;
	shape->opacity = attr->opacity;
	shape->fill_opacity = attr->fillOpacity;
	shape->stroke_opacity = attr->strokeOpacity;
	shape->paint_order = attr->paintOrder;

	shape->paths = p->plist;
	p->plist = NULL;
	shape->geometry = p->shape_geometry;
	p->shape_geometry = NULL;

	// Calculate shape bounds
	shape->bounds[0] = shape->paths->bounds[0];
	shape->bounds[1] = shape->paths->bounds[1];
	shape->bounds[2] = shape->paths->bounds[2];
	shape->bounds[3] = shape->paths->bounds[3];
	for (path = shape->paths->next; path != NULL; path = path->next) {
		shape->bounds[0] = wlf_svg_minf(shape->bounds[0], path->bounds[0]);
		shape->bounds[1] = wlf_svg_minf(shape->bounds[1], path->bounds[1]);
		shape->bounds[2] = wlf_svg_maxf(shape->bounds[2], path->bounds[2]);
		shape->bounds[3] = wlf_svg_maxf(shape->bounds[3], path->bounds[3]);
	}

	// Set fill
	if (attr->hasFill == 1) {
		float scale_opacity = wlf_svg_shape_fill_alpha(shape);
		shape->fill = wlf_svg_create_solid_gradient(attr->fillColor, scale_opacity);
	} else if (attr->hasFill == 2) {
		shape->fill = NULL;
	} else {
		shape->fill = NULL;
	}

	// Set stroke
	if (attr->hasStroke == 1) {
		float scale_opacity = wlf_svg_shape_stroke_alpha(shape);
		shape->stroke = wlf_svg_create_solid_gradient(attr->strokeColor, scale_opacity);
	} else if (attr->hasStroke == 2) {
		shape->stroke = NULL;
	} else {
		shape->stroke = NULL;
	}

	wlf_svg_apply_paint_to_geometry(shape->geometry, attr, shape->stroke_width,
		shape->fill, shape->stroke);

	// Set flags
	shape->flags = (attr->visible ? WLF_SVG_FLAGS_VISIBLE : 0x00);

	// Add to tail — redirect into current_symbol when inside <symbol>
	if (p->current_symbol != NULL) {
		if (p->current_symbol->shapes == NULL)
			p->current_symbol->shapes = &shape->base;
		else
			wlf_svg_shape_from_shape(p->current_symbol->shapes_tail)->next = shape;
		p->current_symbol->shapes_tail = &shape->base;
	} else {
		if (p->image->shapes == NULL)
			p->image->shapes = &shape->base;
		else
			wlf_svg_shape_from_shape(p->shapesTail)->next = shape;
		p->shapesTail = &shape->base;
	}

	return;

error:
	if (shape) {
		wlf_shape_destroy(&shape->base);
	}
	wlf_shape_destroy(p->shape_geometry);
	p->shape_geometry = NULL;
}

static void wlf_svg_add_path(struct wlf_svg_parser *p, char closed)
{
	struct wlf_svg_attrib* attr = wlf_svg_get_attr(p);
	float bounds[4];
	float* curve;
	int i;

	if (p->npts < 4)
		return;

	if (closed)
		wlf_svg_line_to(p, p->pts[0], p->pts[1]);

	// Expect 1 + N*3 points (N = number of cubic bezier segments).
	if ((p->npts % 3) != 1)
		return;

	struct wlf_path* path = malloc(sizeof(struct wlf_path));
	if (path == NULL) goto error;
	memset(path, 0, sizeof(struct wlf_path));

	path->pts = (float*)malloc(p->npts*2*sizeof(float));
	if (path->pts == NULL) goto error;
	path->closed = closed;
	path->npts = p->npts;

	// Transform path.
	for (i = 0; i < p->npts; ++i)
		wlf_svg_xform_point(&path->pts[i*2], &path->pts[i*2+1], p->pts[i*2], p->pts[i*2+1], attr->xform);

	// Find bounds
	for (i = 0; i < path->npts-1; i += 3) {
		curve = &path->pts[i*2];
		wlf_svg_curve_bounds(bounds, curve);
		if (i == 0) {
			path->bounds[0] = bounds[0];
			path->bounds[1] = bounds[1];
			path->bounds[2] = bounds[2];
			path->bounds[3] = bounds[3];
		} else {
			path->bounds[0] = wlf_svg_minf(path->bounds[0], bounds[0]);
			path->bounds[1] = wlf_svg_minf(path->bounds[1], bounds[1]);
			path->bounds[2] = wlf_svg_maxf(path->bounds[2], bounds[2]);
			path->bounds[3] = wlf_svg_maxf(path->bounds[3], bounds[3]);
		}
	}

	path->next = p->plist;
	p->plist = path;

	return;

error:
	if (path != NULL) {
		if (path->pts != NULL) free(path->pts);
		free(path);
	}
}

// We roll our own string to float because the std library one uses locale and messes things up.
static double wlf_svg_atof(const char* s)
{
	char* cur = (char*)s;
	char* end = NULL;
	double res = 0.0, sign = 1.0;
	long long intPart = 0, fracPart = 0;
	char hasIntPart = 0, hasFracPart = 0;

	// Parse optional sign
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		sign = -1;
		cur++;
	}

	// Parse integer part
	if (wlf_svg_isdigit(*cur)) {
		// Parse digit sequence
		intPart = strtoll(cur, &end, 10);
		if (cur != end) {
			res = (double)intPart;
			hasIntPart = 1;
			cur = end;
		}
	}

	// Parse fractional part.
	if (*cur == '.') {
		cur++; // Skip '.'
		if (wlf_svg_isdigit(*cur)) {
			// Parse digit sequence
			fracPart = strtoll(cur, &end, 10);
			if (cur != end) {
				res += (double)fracPart / pow(10.0, (double)(end - cur));
				hasFracPart = 1;
				cur = end;
			}
		}
	}

	// A valid number should have integer or fractional part.
	if (!hasIntPart && !hasFracPart)
		return 0.0;

	// Parse optional exponent
	if (*cur == 'e' || *cur == 'E') {
		long expPart = 0;
		cur++; // skip 'E'
		expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
		if (cur != end) {
			res *= pow(10.0, (double)expPart);
		}
	}

	return res * sign;
}


static const char* wlf_svg_parse_number(const char* s, char* it, const int size)
{
	const int last = size-1;
	int i = 0;

	// sign
	if (*s == '-' || *s == '+') {
		if (i < last) it[i++] = *s;
		s++;
	}
	// integer part
	while (*s && wlf_svg_isdigit(*s)) {
		if (i < last) it[i++] = *s;
		s++;
	}
	if (*s == '.') {
		// decimal point
		if (i < last) it[i++] = *s;
		s++;
		// fraction part
		while (*s && wlf_svg_isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	// exponent
	if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
		if (i < last) it[i++] = *s;
		s++;
		if (*s == '-' || *s == '+') {
			if (i < last) it[i++] = *s;
			s++;
		}
		while (*s && wlf_svg_isdigit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	it[i] = '\0';

	return s;
}

static const char* wlf_svg_get_next_path_item_when_arc_flag(const char* s, char* it)
{
	it[0] = '\0';
	while (*s && (wlf_svg_isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '0' || *s == '1') {
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}
	return s;
}

static const char* wlf_svg_get_next_path_item(const char* s, char* it)
{
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (wlf_svg_isspace(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '-' || *s == '+' || *s == '.' || wlf_svg_isdigit(*s)) {
		s = wlf_svg_parse_number(s, it, 64);
	} else {
		// Parse command
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

static unsigned int wlf_svg_parse_color_hex(const char* str)
{
	unsigned int r=0, g=0, b=0;
	if (sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3 )		// 2 digit hex
		return WLF_SVG_RGB(r, g, b);
	if (sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3 )		// 1 digit hex, e.g. #abc -> 0xccbbaa
		return WLF_SVG_RGB(r*17, g*17, b*17);			// same effect as (r<<4|r), (g<<4|g), ..
	return WLF_SVG_RGB(128, 128, 128);
}

// Parse rgb color. The pointer 'str' must point at "rgb(" (4+ characters).
// This function returns gray (rgb(128, 128, 128) == '#808080') on parse errors
// for backwards compatibility. Note: other image viewers return black instead.

static unsigned int wlf_svg_parse_color_rgb(const char* str)
{
	int i;
	unsigned int rgbi[3];
	float rgbf[3];
	// try decimal integers first
	if (sscanf(str, "rgb(%u, %u, %u)", &rgbi[0], &rgbi[1], &rgbi[2]) != 3) {
		// integers failed, try percent values (float, locale independent)
		const char delimiter[3] = {',', ',', ')'};
		str += 4; // skip "rgb("
		for (i = 0; i < 3; i++) {
			while (*str && (wlf_svg_isspace(*str))) str++; 	// skip leading spaces
			if (*str == '+') str++;				// skip '+' (don't allow '-')
			if (!*str) break;
			rgbf[i] = wlf_svg_atof(str);

			// Note 1: it would be great if wlf_svg_atof() returned how many
			// bytes it consumed but it doesn't. We need to skip the number,
			// the '%' character, spaces, and the delimiter ',' or ')'.

			// Note 2: The following code does not allow values like "33.%",
			// i.e. a decimal point w/o fractional part, but this is consistent
			// with other image viewers, e.g. firefox, chrome, eog, gimp.

			while (*str && wlf_svg_isdigit(*str)) str++;		// skip integer part
			if (*str == '.') {
				str++;
				if (!wlf_svg_isdigit(*str)) break;		// error: no digit after '.'
				while (*str && wlf_svg_isdigit(*str)) str++;	// skip fractional part
			}
			if (*str == '%') str++; else break;
			while (*str && wlf_svg_isspace(*str)) str++;
			if (*str == delimiter[i]) str++;
			else break;
		}
		if (i == 3) {
			rgbi[0] = roundf(rgbf[0] * 2.55f);
			rgbi[1] = roundf(rgbf[1] * 2.55f);
			rgbi[2] = roundf(rgbf[2] * 2.55f);
		} else {
			rgbi[0] = rgbi[1] = rgbi[2] = 128;
		}
	}
	// clip values as the CSS spec requires
	for (i = 0; i < 3; i++) {
		if (rgbi[i] > 255) rgbi[i] = 255;
	}
	return WLF_SVG_RGB(rgbi[0], rgbi[1], rgbi[2]);
}

static unsigned int wlf_svg_parse_color(const char* str)
{
	size_t len = 0;
	while(*str == ' ') ++str;
	len = strlen(str);
	if (len >= 1 && *str == '#')
		return wlf_svg_parse_color_hex(str);
	else if (len >= 4 && str[0] == 'r' && str[1] == 'g' && str[2] == 'b' && str[3] == '(')
		return wlf_svg_parse_color_rgb(str);
	return wlf_svg_parse_color_name(str);
}

static float wlf_svg_parse_opacity(const char* str)
{
	float val = wlf_svg_atof(str);
	if (val < 0.0f) val = 0.0f;
	if (val > 1.0f) val = 1.0f;
	return val;
}

static float wlf_svg_parse_miter_limit(const char* str)
{
	float val = wlf_svg_atof(str);
	if (val < 0.0f) val = 0.0f;
	return val;
}

static int wlf_svg_parse_units(const char* units)
{
	if (units[0] == 'p' && units[1] == 'x')
		return WLF_SVG_UNITS_PX;
	else if (units[0] == 'p' && units[1] == 't')
		return WLF_SVG_UNITS_PT;
	else if (units[0] == 'p' && units[1] == 'c')
		return WLF_SVG_UNITS_PC;
	else if (units[0] == 'm' && units[1] == 'm')
		return WLF_SVG_UNITS_MM;
	else if (units[0] == 'c' && units[1] == 'm')
		return WLF_SVG_UNITS_CM;
	else if (units[0] == 'i' && units[1] == 'n')
		return WLF_SVG_UNITS_IN;
	else if (units[0] == '%')
		return WLF_SVG_UNITS_PERCENT;
	else if (units[0] == 'e' && units[1] == 'm')
		return WLF_SVG_UNITS_EM;
	else if (units[0] == 'e' && units[1] == 'x')
		return WLF_SVG_UNITS_EX;
	return WLF_SVG_UNITS_USER;
}

static int wlf_svg_is_coordinate(const char* s)
{
	// optional sign
	if (*s == '-' || *s == '+')
		s++;
	// must have at least one digit, or start by a dot
	return (wlf_svg_isdigit(*s) || *s == '.');
}

static struct wlf_svg_coordinate wlf_svg_parse_coordinate_raw(const char* str)
{
	struct wlf_svg_coordinate coord = {0, WLF_SVG_UNITS_USER};
	char buf[64];
	coord.units = wlf_svg_parse_units(wlf_svg_parse_number(str, buf, 64));
	coord.value = wlf_svg_atof(buf);
	return coord;
}

static struct wlf_svg_coordinate wlf_svg_coord(float v, int units)
{
	struct wlf_svg_coordinate coord = {v, units};
	return coord;
}

static float wlf_svg_parse_coordinate(struct wlf_svg_parser *p, const char* str, float orig, float length)
{
	struct wlf_svg_coordinate coord = wlf_svg_parse_coordinate_raw(str);
	return wlf_svg_convert_to_pixels(p, coord, orig, length);
}

static int wlf_svg_parse_transform_args(const char* str, float* args, int maxNa, int* na)
{
	const char* end;
	const char* ptr;
	char it[64];

	*na = 0;
	ptr = str;
	while (*ptr && *ptr != '(') ++ptr;
	if (*ptr == 0)
		return 1;
	end = ptr;
	while (*end && *end != ')') ++end;
	if (*end == 0)
		return 1;

	while (ptr < end) {
		if (*ptr == '-' || *ptr == '+' || *ptr == '.' || wlf_svg_isdigit(*ptr)) {
			if (*na >= maxNa) return 0;
			ptr = wlf_svg_parse_number(ptr, it, 64);
			args[(*na)++] = (float)wlf_svg_atof(it);
		} else {
			++ptr;
		}
	}
	return (int)(end - str);
}


static int wlf_svg_parse_matrix(float* xform, const char* str)
{
	float t[6];
	int na = 0;
	int len = wlf_svg_parse_transform_args(str, t, 6, &na);
	if (na != 6) return len;
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int wlf_svg_parse_translate(float* xform, const char* str)
{
	float args[2];
	float t[6];
	int na = 0;
	int len = wlf_svg_parse_transform_args(str, args, 2, &na);
	if (na == 1) args[1] = 0.0;

	wlf_svg_xform_set_translation(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int wlf_svg_parse_scale(float* xform, const char* str)
{
	float args[2];
	int na = 0;
	float t[6];
	int len = wlf_svg_parse_transform_args(str, args, 2, &na);
	if (na == 1) args[1] = args[0];
	wlf_svg_xform_set_scale(t, args[0], args[1]);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int wlf_svg_parse_skew_x(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = wlf_svg_parse_transform_args(str, args, 1, &na);
	wlf_svg_xform_set_skew_x(t, args[0]/180.0f*M_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int wlf_svg_parse_skew_y(float* xform, const char* str)
{
	float args[1];
	int na = 0;
	float t[6];
	int len = wlf_svg_parse_transform_args(str, args, 1, &na);
	wlf_svg_xform_set_skew_y(t, args[0]/180.0f*M_PI);
	memcpy(xform, t, sizeof(float)*6);
	return len;
}

static int wlf_svg_parse_rotate(float* xform, const char* str)
{
	float args[3];
	int na = 0;
	float m[6];
	float t[6];
	int len = wlf_svg_parse_transform_args(str, args, 3, &na);
	if (na == 1)
		args[1] = args[2] = 0.0f;
	wlf_svg_xform_identity(m);

	if (na > 1) {
		wlf_svg_xform_set_translation(t, -args[1], -args[2]);
		wlf_svg_xform_multiply(m, t);
	}

	wlf_svg_xform_set_rotation(t, args[0]/180.0f*M_PI);
	wlf_svg_xform_multiply(m, t);

	if (na > 1) {
		wlf_svg_xform_set_translation(t, args[1], args[2]);
		wlf_svg_xform_multiply(m, t);
	}

	memcpy(xform, m, sizeof(float)*6);

	return len;
}

static void wlf_svg_parse_transform(float* xform, const char* str)
{
	float t[6];
	int len;
	int i;
	struct wlf_svg_transform_cmd {
		const char* name;
		int len;
		int (*parse)(float*, const char*);
	};
	static const struct wlf_svg_transform_cmd commands[] = {
		{ "matrix", 6, wlf_svg_parse_matrix },
		{ "translate", 9, wlf_svg_parse_translate },
		{ "scale", 5, wlf_svg_parse_scale },
		{ "rotate", 6, wlf_svg_parse_rotate },
		{ "skewX", 5, wlf_svg_parse_skew_x },
		{ "skewY", 5, wlf_svg_parse_skew_y },
	};
	wlf_svg_xform_identity(xform);
	while (*str)
	{
		len = 0;
		for (i = 0; i < (int)(sizeof(commands) / sizeof(commands[0])); ++i) {
			if (strncmp(str, commands[i].name, commands[i].len) == 0) {
				len = commands[i].parse(t, str);
				break;
			}
		}
		if (len == 0) {
			++str;
			continue;
		}
		if (len != 0) {
			str += len;
		} else {
			++str;
			continue;
		}

		wlf_svg_xform_premultiply(xform, t);
	}
}

static void wlf_svg_parse_url(char* id, const char* str)
{
	int i = 0;
	str += 4; // "url(";
	if (*str && *str == '#')
		str++;
	while (i < 63 && *str && *str != ')') {
		id[i] = *str++;
		i++;
	}
	id[i] = '\0';
}

static const struct wlf_svg_name_map wlf_svg_attr_name_map[] = {
	{ "style", WLF_SVG_ATTR_STYLE },
	{ "display", WLF_SVG_ATTR_DISPLAY },
	{ "fill", WLF_SVG_ATTR_FILL },
	{ "opacity", WLF_SVG_ATTR_OPACITY },
	{ "fill-opacity", WLF_SVG_ATTR_FILL_OPACITY },
	{ "stroke", WLF_SVG_ATTR_STROKE },
	{ "stroke-width", WLF_SVG_ATTR_STROKE_WIDTH },
	{ "stroke-dasharray", WLF_SVG_ATTR_STROKE_DASHARRAY },
	{ "stroke-dashoffset", WLF_SVG_ATTR_STROKE_DASHOFFSET },
	{ "stroke-opacity", WLF_SVG_ATTR_STROKE_OPACITY },
	{ "stroke-linecap", WLF_SVG_ATTR_STROKE_LINECAP },
	{ "stroke-linejoin", WLF_SVG_ATTR_STROKE_LINEJOIN },
	{ "stroke-miterlimit", WLF_SVG_ATTR_STROKE_MITERLIMIT },
	{ "fill-rule", WLF_SVG_ATTR_FILL_RULE },
	{ "font-size", WLF_SVG_ATTR_FONT_SIZE },
	{ "transform", WLF_SVG_ATTR_TRANSFORM },
	{ "stop-color", WLF_SVG_ATTR_STOP_COLOR },
	{ "stop-opacity", WLF_SVG_ATTR_STOP_OPACITY },
	{ "offset", WLF_SVG_ATTR_OFFSET },
	{ "paint-order", WLF_SVG_ATTR_PAINT_ORDER },
	{ "id", WLF_SVG_ATTR_ID },
};

static const struct wlf_svg_name_map wlf_svg_element_name_map[] = {
	{ "g", WLF_SVG_EL_G },
	{ "path", WLF_SVG_EL_PATH },
	{ "rect", WLF_SVG_EL_RECT },
	{ "circle", WLF_SVG_EL_CIRCLE },
	{ "ellipse", WLF_SVG_EL_ELLIPSE },
	{ "line", WLF_SVG_EL_LINE },
	{ "polyline", WLF_SVG_EL_POLYLINE },
	{ "polygon", WLF_SVG_EL_POLYGON },
	{ "linearGradient", WLF_SVG_EL_LINEAR_GRADIENT },
	{ "radialGradient", WLF_SVG_EL_RADIAL_GRADIENT },
	{ "stop", WLF_SVG_EL_STOP },
	{ "defs", WLF_SVG_EL_DEFS },
	{ "svg", WLF_SVG_EL_SVG },
	{ "symbol", WLF_SVG_EL_SYMBOL },
	{ "use", WLF_SVG_EL_USE },
	{ "text", WLF_SVG_EL_TEXT },
};

static const struct wlf_svg_name_map wlf_svg_gradient_attr_name_map[] = {
	{ "id", WLF_SVG_GRADIENT_ATTR_ID },
	{ "gradientUnits", WLF_SVG_GRADIENT_ATTR_GRADIENT_UNITS },
	{ "gradientTransform", WLF_SVG_GRADIENT_ATTR_GRADIENT_TRANSFORM },
	{ "cx", WLF_SVG_GRADIENT_ATTR_CX },
	{ "cy", WLF_SVG_GRADIENT_ATTR_CY },
	{ "r", WLF_SVG_GRADIENT_ATTR_R },
	{ "fx", WLF_SVG_GRADIENT_ATTR_FX },
	{ "fy", WLF_SVG_GRADIENT_ATTR_FY },
	{ "x1", WLF_SVG_GRADIENT_ATTR_X1 },
	{ "y1", WLF_SVG_GRADIENT_ATTR_Y1 },
	{ "x2", WLF_SVG_GRADIENT_ATTR_X2 },
	{ "y2", WLF_SVG_GRADIENT_ATTR_Y2 },
	{ "spreadMethod", WLF_SVG_GRADIENT_ATTR_SPREAD_METHOD },
	{ "xlink:href", WLF_SVG_GRADIENT_ATTR_XLINK_HREF },
};

static const struct wlf_svg_name_map wlf_svg_root_attr_name_map[] = {
	{ "width", WLF_SVG_ROOT_ATTR_WIDTH },
	{ "height", WLF_SVG_ROOT_ATTR_HEIGHT },
	{ "viewBox", WLF_SVG_ROOT_ATTR_VIEWBOX },
	{ "preserveAspectRatio", WLF_SVG_ROOT_ATTR_PRESERVE_ASPECT_RATIO },
};

static int wlf_svg_lookup_name_value(const struct wlf_svg_name_map* map, int map_count, const char* name)
{
	int i;
	for (i = 0; i < map_count; ++i) {
		if (strcmp(map[i].name, name) == 0)
			return map[i].value;
	}
	return 0;
}

static int wlf_svg_lookup_char_value(const struct wlf_svg_char_name_map* map, int map_count, const char* name, char* out)
{
	int i;
	for (i = 0; i < map_count; ++i) {
		if (strcmp(map[i].name, name) == 0) {
			*out = map[i].value;
			return 1;
		}
	}
	return 0;
}

static int wlf_svg_lookup_uchar_value(const struct wlf_svg_uchar_name_map* map, int map_count, const char* name, unsigned char* out)
{
	int i;
	for (i = 0; i < map_count; ++i) {
		if (strcmp(map[i].name, name) == 0) {
			*out = map[i].value;
			return 1;
		}
	}
	return 0;
}

static enum wlf_svg_attr_name wlf_svg_lookup_attr_name(const char* name)
{
	return (enum wlf_svg_attr_name)wlf_svg_lookup_name_value(
		wlf_svg_attr_name_map,
		(int)(sizeof(wlf_svg_attr_name_map) / sizeof(wlf_svg_attr_name_map[0])),
		name);
}

static enum wlf_svg_element_name wlf_svg_lookup_element_name(const char* name)
{
	return (enum wlf_svg_element_name)wlf_svg_lookup_name_value(
		wlf_svg_element_name_map,
		(int)(sizeof(wlf_svg_element_name_map) / sizeof(wlf_svg_element_name_map[0])),
		name);
}

static enum wlf_svg_gradient_attr_name wlf_svg_lookup_gradient_attr_name(const char* name)
{
	return (enum wlf_svg_gradient_attr_name)wlf_svg_lookup_name_value(
		wlf_svg_gradient_attr_name_map,
		(int)(sizeof(wlf_svg_gradient_attr_name_map) / sizeof(wlf_svg_gradient_attr_name_map[0])),
		name);
}

static enum wlf_svg_root_attr_name wlf_svg_lookup_root_attr_name(const char* name)
{
	return (enum wlf_svg_root_attr_name)wlf_svg_lookup_name_value(
		wlf_svg_root_attr_name_map,
		(int)(sizeof(wlf_svg_root_attr_name_map) / sizeof(wlf_svg_root_attr_name_map[0])),
		name);
}

static char wlf_svg_parse_line_cap(const char* str)
{
	char value = WLF_SVG_CAP_BUTT;
	static const struct wlf_svg_char_name_map map[] = {
		{ "butt", WLF_SVG_CAP_BUTT },
		{ "round", WLF_SVG_CAP_ROUND },
		{ "square", WLF_SVG_CAP_SQUARE },
	};
	if (wlf_svg_lookup_char_value(map, (int)(sizeof(map) / sizeof(map[0])), str, &value))
		return value;
	// TODO: handle inherit.
	return WLF_SVG_CAP_BUTT;
}

static char wlf_svg_parse_line_join(const char* str)
{
	char value = WLF_SVG_JOIN_MITER;
	static const struct wlf_svg_char_name_map map[] = {
		{ "miter", WLF_SVG_JOIN_MITER },
		{ "round", WLF_SVG_JOIN_ROUND },
		{ "bevel", WLF_SVG_JOIN_BEVEL },
	};
	if (wlf_svg_lookup_char_value(map, (int)(sizeof(map) / sizeof(map[0])), str, &value))
		return value;
	// TODO: handle inherit.
	return WLF_SVG_JOIN_MITER;
}

static char wlf_svg_parse_fill_rule(const char* str)
{
	char value = WLF_SVG_FILL_RULE_NONZERO;
	static const struct wlf_svg_char_name_map map[] = {
		{ "nonzero", WLF_SVG_FILL_RULE_NONZERO },
		{ "evenodd", WLF_SVG_FILL_RULE_EVENODD },
	};
	if (wlf_svg_lookup_char_value(map, (int)(sizeof(map) / sizeof(map[0])), str, &value))
		return value;
	// TODO: handle inherit.
	return WLF_SVG_FILL_RULE_NONZERO;
}

static unsigned char wlf_svg_parse_paint_order(const char* str)
{
	unsigned char value = wlf_svg_encode_paint_order(WLF_SVG_PAINT_FILL, WLF_SVG_PAINT_STROKE, WLF_SVG_PAINT_MARKERS);
	static const struct wlf_svg_uchar_name_map map[] = {
		{ "normal", ((WLF_SVG_PAINT_FILL & 0x03) | ((WLF_SVG_PAINT_STROKE & 0x03) << 2) | ((WLF_SVG_PAINT_MARKERS & 0x03) << 4)) },
		{ "fill stroke markers", ((WLF_SVG_PAINT_FILL & 0x03) | ((WLF_SVG_PAINT_STROKE & 0x03) << 2) | ((WLF_SVG_PAINT_MARKERS & 0x03) << 4)) },
		{ "fill markers stroke", ((WLF_SVG_PAINT_FILL & 0x03) | ((WLF_SVG_PAINT_MARKERS & 0x03) << 2) | ((WLF_SVG_PAINT_STROKE & 0x03) << 4)) },
		{ "markers fill stroke", ((WLF_SVG_PAINT_MARKERS & 0x03) | ((WLF_SVG_PAINT_FILL & 0x03) << 2) | ((WLF_SVG_PAINT_STROKE & 0x03) << 4)) },
		{ "markers stroke fill", ((WLF_SVG_PAINT_MARKERS & 0x03) | ((WLF_SVG_PAINT_STROKE & 0x03) << 2) | ((WLF_SVG_PAINT_FILL & 0x03) << 4)) },
		{ "stroke fill markers", ((WLF_SVG_PAINT_STROKE & 0x03) | ((WLF_SVG_PAINT_FILL & 0x03) << 2) | ((WLF_SVG_PAINT_MARKERS & 0x03) << 4)) },
		{ "stroke markers fill", ((WLF_SVG_PAINT_STROKE & 0x03) | ((WLF_SVG_PAINT_MARKERS & 0x03) << 2) | ((WLF_SVG_PAINT_FILL & 0x03) << 4)) },
	};
	if (wlf_svg_lookup_uchar_value(map, (int)(sizeof(map) / sizeof(map[0])), str, &value))
		return value;
	// TODO: handle inherit.
	return wlf_svg_encode_paint_order(WLF_SVG_PAINT_FILL, WLF_SVG_PAINT_STROKE, WLF_SVG_PAINT_MARKERS);
}

static const char* wlf_svg_get_next_dash_item(const char* s, char* it)
{
	int n = 0;
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (wlf_svg_isspace(*s) || *s == ',')) s++;
	// Advance until whitespace, comma or end.
	while (*s && (!wlf_svg_isspace(*s) && *s != ',')) {
		if (n < 63)
			it[n++] = *s;
		s++;
	}
	it[n++] = '\0';
	return s;
}

static int wlf_svg_parse_stroke_dash_array(struct wlf_svg_parser *p, const char* str, float* strokeDashArray)
{
	char item[64];
	int count = 0, i;
	float sum = 0.0f;

	// Handle "none"
	if (str[0] == 'n')
		return 0;

	// Parse dashes
	while (*str) {
		str = wlf_svg_get_next_dash_item(str, item);
		if (!*item) break;
		if (count < WLF_SVG_MAX_DASHES)
			strokeDashArray[count++] = fabsf(wlf_svg_parse_coordinate(p, item, 0.0f, wlf_svg_actual_length(p)));
	}

	for (i = 0; i < count; i++)
		sum += strokeDashArray[i];
	if (sum <= 1e-6f)
		count = 0;

	return count;
}

static void wlf_svg_parse_style(struct wlf_svg_parser *p, const char* str);

static int wlf_svg_parse_attr(struct wlf_svg_parser *p, const char* name, const char* value)
{
	float xform[6];
	struct wlf_svg_attrib* attr = wlf_svg_get_attr(p);
	enum wlf_svg_attr_name attr_name;
	if (!attr) return 0;
	attr_name = wlf_svg_lookup_attr_name(name);

	switch (attr_name) {
		case WLF_SVG_ATTR_STYLE:
			wlf_svg_parse_style(p, value);
			break;
		case WLF_SVG_ATTR_DISPLAY:
			if (strcmp(value, "none") == 0)
				attr->visible = 0;
			break;
		case WLF_SVG_ATTR_FILL:
			if (strcmp(value, "none") == 0) {
				attr->hasFill = 0;
			} else if (strncmp(value, "url(", 4) == 0) {
				attr->hasFill = 2;
				wlf_svg_parse_url(attr->fillGradient, value);
			} else {
				attr->hasFill = 1;
				attr->fillColor = wlf_svg_parse_color(value);
			}
			break;
		case WLF_SVG_ATTR_OPACITY:
			attr->opacity = wlf_svg_parse_opacity(value);
			break;
		case WLF_SVG_ATTR_FILL_OPACITY:
			attr->fillOpacity = wlf_svg_parse_opacity(value);
			break;
		case WLF_SVG_ATTR_STROKE:
			if (strcmp(value, "none") == 0) {
				attr->hasStroke = 0;
			} else if (strncmp(value, "url(", 4) == 0) {
				attr->hasStroke = 2;
				wlf_svg_parse_url(attr->strokeGradient, value);
			} else {
				attr->hasStroke = 1;
				attr->strokeColor = wlf_svg_parse_color(value);
			}
			break;
		case WLF_SVG_ATTR_STROKE_WIDTH:
			attr->strokeWidth = wlf_svg_parse_coordinate(p, value, 0.0f, wlf_svg_actual_length(p));
			break;
		case WLF_SVG_ATTR_STROKE_DASHARRAY:
			attr->strokeDashCount = wlf_svg_parse_stroke_dash_array(p, value, attr->strokeDashArray);
			break;
		case WLF_SVG_ATTR_STROKE_DASHOFFSET:
			attr->strokeDashOffset = wlf_svg_parse_coordinate(p, value, 0.0f, wlf_svg_actual_length(p));
			break;
		case WLF_SVG_ATTR_STROKE_OPACITY:
			attr->strokeOpacity = wlf_svg_parse_opacity(value);
			break;
		case WLF_SVG_ATTR_STROKE_LINECAP:
			attr->strokeLineCap = wlf_svg_parse_line_cap(value);
			break;
		case WLF_SVG_ATTR_STROKE_LINEJOIN:
			attr->strokeLineJoin = wlf_svg_parse_line_join(value);
			break;
		case WLF_SVG_ATTR_STROKE_MITERLIMIT:
			attr->miterLimit = wlf_svg_parse_miter_limit(value);
			break;
		case WLF_SVG_ATTR_FILL_RULE:
			attr->fillRule = wlf_svg_parse_fill_rule(value);
			break;
		case WLF_SVG_ATTR_FONT_SIZE:
			attr->fontSize = wlf_svg_parse_coordinate(p, value, 0.0f, wlf_svg_actual_length(p));
			break;
		case WLF_SVG_ATTR_TRANSFORM:
			wlf_svg_parse_transform(xform, value);
			wlf_svg_xform_premultiply(attr->xform, xform);
			break;
		case WLF_SVG_ATTR_STOP_COLOR:
			attr->stopColor = wlf_svg_parse_color(value);
			break;
		case WLF_SVG_ATTR_STOP_OPACITY:
			attr->stopOpacity = wlf_svg_parse_opacity(value);
			break;
		case WLF_SVG_ATTR_OFFSET:
			attr->stopOffset = wlf_svg_parse_coordinate(p, value, 0.0f, 1.0f);
			break;
		case WLF_SVG_ATTR_PAINT_ORDER:
			attr->paintOrder = wlf_svg_parse_paint_order(value);
			break;
		case WLF_SVG_ATTR_ID:
			strncpy(attr->id, value, 63);
			attr->id[63] = '\0';
			break;
		default:
			return 0;
	}
	return 1;
}

static int wlf_svg_parse_name_value(struct wlf_svg_parser *p, const char* start, const char* end)
{
	const char* str;
	const char* val;
	char name[512];
	char value[512];
	int n;

	str = start;
	while (str < end && *str != ':') ++str;

	val = str;

	// Right Trim
	while (str > start &&  (*str == ':' || wlf_svg_isspace(*str))) --str;
	++str;

	n = (int)(str - start);
	if (n > 511) n = 511;
	if (n) memcpy(name, start, n);
	name[n] = 0;

	while (val < end && (*val == ':' || wlf_svg_isspace(*val))) ++val;

	n = (int)(end - val);
	if (n > 511) n = 511;
	if (n) memcpy(value, val, n);
	value[n] = 0;

	return wlf_svg_parse_attr(p, name, value);
}

static void wlf_svg_parse_style(struct wlf_svg_parser *p, const char* str)
{
	const char* start;
	const char* end;

	while (*str) {
		// Left Trim
		while(*str && wlf_svg_isspace(*str)) ++str;
		start = str;
		while(*str && *str != ';') ++str;
		end = str;

		// Right Trim
		while (end > start &&  (*end == ';' || wlf_svg_isspace(*end))) --end;
		++end;

		wlf_svg_parse_name_value(p, start, end);
		if (*str) ++str;
	}
}

static void wlf_svg_parse_attribs(struct wlf_svg_parser *p, const char** attr)
{
	int i;
	for (i = 0; attr[i]; i += 2)
	{
		if (strcmp(attr[i], "style") == 0)
			wlf_svg_parse_style(p, attr[i + 1]);
		else
			wlf_svg_parse_attr(p, attr[i], attr[i + 1]);
	}
}

static int wlf_svg_get_args_per_element(char cmd)
{
	switch (cmd) {
		case 'v':
		case 'V':
		case 'h':
		case 'H':
			return 1;
		case 'm':
		case 'M':
		case 'l':
		case 'L':
		case 't':
		case 'T':
			return 2;
		case 'q':
		case 'Q':
		case 's':
		case 'S':
			return 4;
		case 'c':
		case 'C':
			return 6;
		case 'a':
		case 'A':
			return 7;
		case 'z':
		case 'Z':
			return 0;
	}
	return -1;
}

static void wlf_svg_path_move_to(struct wlf_svg_parser *p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	wlf_svg_move_to(p, *cpx, *cpy);
}

static void wlf_svg_path_line_to(struct wlf_svg_parser *p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	wlf_svg_line_to(p, *cpx, *cpy);
}

static void wlf_svg_path_h_line_to(struct wlf_svg_parser *p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	wlf_svg_line_to(p, *cpx, *cpy);
}

static void wlf_svg_path_v_line_to(struct wlf_svg_parser *p, float* cpx, float* cpy, float* args, int rel)
{
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	wlf_svg_line_to(p, *cpx, *cpy);
}

static void wlf_svg_path_cubic_bez_to(struct wlf_svg_parser *p, float* cpx, float* cpy,
								 float* cpx2, float* cpy2, float* args, int rel)
{
	float x2, y2, cx1, cy1, cx2, cy2;

	if (rel) {
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	} else {
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}

	wlf_svg_cubic_bez_to(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void wlf_svg_path_cubic_bez_short_to(struct wlf_svg_parser *p, float* cpx, float* cpy,
									  float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	cx1 = 2*x1 - *cpx2;
	cy1 = 2*y1 - *cpy2;

	wlf_svg_cubic_bez_to(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void wlf_svg_path_quad_bez_to(struct wlf_svg_parser *p, float* cpx, float* cpy,
								float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	// Convert to cubic bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	wlf_svg_cubic_bez_to(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static void wlf_svg_path_quad_bez_short_to(struct wlf_svg_parser *p, float* cpx, float* cpy,
									 float* cpx2, float* cpy2, float* args, int rel)
{
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	} else {
		x2 = args[0];
		y2 = args[1];
	}

	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;

	// Convert to cubix bezier
	cx1 = x1 + 2.0f/3.0f*(cx - x1);
	cy1 = y1 + 2.0f/3.0f*(cy - y1);
	cx2 = x2 + 2.0f/3.0f*(cx - x2);
	cy2 = y2 + 2.0f/3.0f*(cy - y2);

	wlf_svg_cubic_bez_to(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static float wlf_svg_sqr(float x) { return x*x; }
static float wlf_svg_vmag(float x, float y) { return sqrtf(x*x + y*y); }

static float wlf_svg_vecrat(float ux, float uy, float vx, float vy)
{
	return (ux*vx + uy*vy) / (wlf_svg_vmag(ux,uy) * wlf_svg_vmag(vx,vy));
}

static float wlf_svg_vecang(float ux, float uy, float vx, float vy)
{
	float r = wlf_svg_vecrat(ux,uy, vx,vy);
	if (r < -1.0f) r = -1.0f;
	if (r > 1.0f) r = 1.0f;
	return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}

static void wlf_svg_path_arc_to(struct wlf_svg_parser *p, float* cpx, float* cpy, float* args, int rel)
{
	// Ported from canvg (https://code.google.com/p/canvg/)
	float rx, ry, rotx;
	float x1, y1, x2, y2, cx, cy, dx, dy, d;
	float x1p, y1p, cxp, cyp, s, sa, sb;
	float ux, uy, vx, vy, a1, da;
	float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
	float sinrx, cosrx;
	int fa, fs;
	int i, ndivs;
	float hda, kappa;

	rx = fabsf(args[0]);				// y radius
	ry = fabsf(args[1]);				// x radius
	rotx = args[2] / 180.0f * M_PI;		// x rotation angle
	fa = fabsf(args[3]) > 1e-6 ? 1 : 0;	// Large arc
	fs = fabsf(args[4]) > 1e-6 ? 1 : 0;	// Sweep direction
	x1 = *cpx;							// start point
	y1 = *cpy;
	if (rel) {							// end point
		x2 = *cpx + args[5];
		y2 = *cpy + args[6];
	} else {
		x2 = args[5];
		y2 = args[6];
	}

	dx = x1 - x2;
	dy = y1 - y2;
	d = sqrtf(dx*dx + dy*dy);
	if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
		// The arc degenerates to a line
		wlf_svg_line_to(p, x2, y2);
		*cpx = x2;
		*cpy = y2;
		return;
	}

	sinrx = sinf(rotx);
	cosrx = cosf(rotx);

	// Convert to center point parameterization.
	// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
	// 1) Compute x1', y1'
	x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
	y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
	d = wlf_svg_sqr(x1p)/wlf_svg_sqr(rx) + wlf_svg_sqr(y1p)/wlf_svg_sqr(ry);
	if (d > 1) {
		d = sqrtf(d);
		rx *= d;
		ry *= d;
	}
	// 2) Compute cx', cy'
	s = 0.0f;
	sa = wlf_svg_sqr(rx)*wlf_svg_sqr(ry) - wlf_svg_sqr(rx)*wlf_svg_sqr(y1p) - wlf_svg_sqr(ry)*wlf_svg_sqr(x1p);
	sb = wlf_svg_sqr(rx)*wlf_svg_sqr(y1p) + wlf_svg_sqr(ry)*wlf_svg_sqr(x1p);
	if (sa < 0.0f) sa = 0.0f;
	if (sb > 0.0f)
		s = sqrtf(sa / sb);
	if (fa == fs)
		s = -s;
	cxp = s * rx * y1p / ry;
	cyp = s * -ry * x1p / rx;

	// 3) Compute cx,cy from cx',cy'
	cx = (x1 + x2)/2.0f + cosrx*cxp - sinrx*cyp;
	cy = (y1 + y2)/2.0f + sinrx*cxp + cosrx*cyp;

	// 4) Calculate theta1, and delta theta.
	ux = (x1p - cxp) / rx;
	uy = (y1p - cyp) / ry;
	vx = (-x1p - cxp) / rx;
	vy = (-y1p - cyp) / ry;
	a1 = wlf_svg_vecang(1.0f,0.0f, ux,uy);	// Initial angle
	da = wlf_svg_vecang(ux,uy, vx,vy);		// Delta angle

//	if (vecrat(ux,uy,vx,vy) <= -1.0f) da = M_PI;
//	if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

	if (fs == 0 && da > 0)
		da -= 2 * M_PI;
	else if (fs == 1 && da < 0)
		da += 2 * M_PI;

	// Approximate the arc using cubic spline segments.
	t[0] = cosrx; t[1] = sinrx;
	t[2] = -sinrx; t[3] = cosrx;
	t[4] = cx; t[5] = cy;

	// Split arc into max 90 degree segments.
	// The loop assumes an iteration per end point (including start and end), this +1.
	ndivs = (int)(fabsf(da) / (M_PI*0.5f) + 1.0f);
	hda = (da / (float)ndivs) / 2.0f;
	// Fix for ticket #179: division by 0: avoid cotangens around 0 (infinite)
	if ((hda < 1e-3f) && (hda > -1e-3f))
		hda *= 0.5f;
	else
		hda = (1.0f - cosf(hda)) / sinf(hda);
	kappa = fabsf(4.0f / 3.0f * hda);
	if (da < 0.0f)
		kappa = -kappa;

	for (i = 0; i <= ndivs; i++) {
		a = a1 + da * ((float)i/(float)ndivs);
		dx = cosf(a);
		dy = sinf(a);
		wlf_svg_xform_point(&x, &y, dx*rx, dy*ry, t); // position
		wlf_svg_xform_vec(&tanx, &tany, -dy*rx * kappa, dx*ry * kappa, t); // tangent
		if (i > 0)
			wlf_svg_cubic_bez_to(p, px+ptanx,py+ptany, x-tanx, y-tany, x, y);
		px = x;
		py = y;
		ptanx = tanx;
		ptany = tany;
	}

	*cpx = x2;
	*cpy = y2;
}

static void wlf_svg_parse_path(struct wlf_svg_parser *p, const char** attr)
{
	const char* s = NULL;
	char cmd = '\0';
	float args[10];
	int nargs;
	int rargs = 0;
	char initPoint;
	float cpx, cpy, cpx2, cpy2;
	const char* tmp[4];
	char closedFlag;
	int i;
	char item[64];

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "d") == 0) {
			s = attr[i + 1];
		} else {
			tmp[0] = attr[i];
			tmp[1] = attr[i + 1];
			tmp[2] = 0;
			tmp[3] = 0;
			wlf_svg_parse_attribs(p, tmp);
		}
	}

	if (s) {
		wlf_svg_reset_path(p);
		cpx = 0; cpy = 0;
		cpx2 = 0; cpy2 = 0;
		initPoint = 0;
		closedFlag = 0;
		nargs = 0;

		while (*s) {
			item[0] = '\0';
			if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
				s = wlf_svg_get_next_path_item_when_arc_flag(s, item);
			if (!*item)
				s = wlf_svg_get_next_path_item(s, item);
			if (!*item) break;
			if (cmd != '\0' && wlf_svg_is_coordinate(item)) {
				if (nargs < 10)
					args[nargs++] = (float)wlf_svg_atof(item);
				if (nargs >= rargs) {
					switch (cmd) {
						case 'm':
						case 'M':
							wlf_svg_path_move_to(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = wlf_svg_get_args_per_element(cmd);
							cpx2 = cpx; cpy2 = cpy;
							initPoint = 1;
							break;
						case 'l':
						case 'L':
							wlf_svg_path_line_to(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							wlf_svg_path_h_line_to(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							wlf_svg_path_v_line_to(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							wlf_svg_path_cubic_bez_to(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							wlf_svg_path_cubic_bez_short_to(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							wlf_svg_path_quad_bez_to(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							wlf_svg_path_quad_bez_short_to(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							wlf_svg_path_arc_to(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						default:
							if (nargs >= 2) {
								cpx = args[nargs-2];
								cpy = args[nargs-1];
								cpx2 = cpx; cpy2 = cpy;
							}
							break;
					}
					nargs = 0;
				}
			} else {
				cmd = item[0];
				if (cmd == 'M' || cmd == 'm') {
					// Commit path.
					if (p->npts > 0)
						wlf_svg_add_path(p, closedFlag);
					// Start new subpath.
					wlf_svg_reset_path(p);
					closedFlag = 0;
					nargs = 0;
				} else if (initPoint == 0) {
					// Do not allow other commands until initial point has been set (moveTo called once).
					cmd = '\0';
				}
				if (cmd == 'Z' || cmd == 'z') {
					closedFlag = 1;
					// Commit path.
					if (p->npts > 0) {
						// Move current point to first point
						cpx = p->pts[0];
						cpy = p->pts[1];
						cpx2 = cpx; cpy2 = cpy;
						wlf_svg_add_path(p, closedFlag);
					}
					// Start new subpath.
					wlf_svg_reset_path(p);
					wlf_svg_move_to(p, cpx, cpy);
					closedFlag = 0;
					nargs = 0;
				}
				rargs = wlf_svg_get_args_per_element(cmd);
				if (rargs == -1) {
					// Command not recognized
					cmd = '\0';
					rargs = 0;
				}
			}
		}
		// Commit path.
		if (p->npts)
			wlf_svg_add_path(p, closedFlag);
	}

	if (p->plist != NULL) {
		p->shape_geometry = wlf_path_shape_create(p->plist, false);
	}
	wlf_svg_add_shape(p);
}

static void wlf_svg_parse_rect(struct wlf_svg_parser *p, const char** attr)
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 0.0f;
	float h = 0.0f;
	float rx = -1.0f; // marks not set
	float ry = -1.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "x") == 0) x = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
			if (strcmp(attr[i], "y") == 0) y = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
			if (strcmp(attr[i], "width") == 0) w = wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_width(p));
			if (strcmp(attr[i], "height") == 0) h = wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_height(p));
			if (strcmp(attr[i], "rx") == 0) rx = fabsf(wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_width(p)));
			if (strcmp(attr[i], "ry") == 0) ry = fabsf(wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_height(p)));
		}
	}

	if (rx < 0.0f && ry > 0.0f) rx = ry;
	if (ry < 0.0f && rx > 0.0f) ry = rx;
	if (rx < 0.0f) rx = 0.0f;
	if (ry < 0.0f) ry = 0.0f;
	if (rx > w/2.0f) rx = w/2.0f;
	if (ry > h/2.0f) ry = h/2.0f;

	if (w != 0.0f && h != 0.0f) {
		p->shape_geometry = wlf_rect_shape_create(x, y, w, h, rx, ry);
		wlf_svg_reset_path(p);

		if (rx < 0.00001f || ry < 0.0001f) {
			wlf_svg_move_to(p, x, y);
			wlf_svg_line_to(p, x+w, y);
			wlf_svg_line_to(p, x+w, y+h);
			wlf_svg_line_to(p, x, y+h);
		} else {
			// Rounded rectangle
			wlf_svg_move_to(p, x+rx, y);
			wlf_svg_line_to(p, x+w-rx, y);
			wlf_svg_cubic_bez_to(p, x+w-rx*(1-WLF_SVG_KAPPA90), y, x+w, y+ry*(1-WLF_SVG_KAPPA90), x+w, y+ry);
			wlf_svg_line_to(p, x+w, y+h-ry);
			wlf_svg_cubic_bez_to(p, x+w, y+h-ry*(1-WLF_SVG_KAPPA90), x+w-rx*(1-WLF_SVG_KAPPA90), y+h, x+w-rx, y+h);
			wlf_svg_line_to(p, x+rx, y+h);
			wlf_svg_cubic_bez_to(p, x+rx*(1-WLF_SVG_KAPPA90), y+h, x, y+h-ry*(1-WLF_SVG_KAPPA90), x, y+h-ry);
			wlf_svg_line_to(p, x, y+ry);
			wlf_svg_cubic_bez_to(p, x, y+ry*(1-WLF_SVG_KAPPA90), x+rx*(1-WLF_SVG_KAPPA90), y, x+rx, y);
		}

		wlf_svg_add_path(p, 1);

		wlf_svg_add_shape(p);
	}
}

static void wlf_svg_parse_circle(struct wlf_svg_parser *p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float r = 0.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "cx") == 0) cx = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
			if (strcmp(attr[i], "cy") == 0) cy = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
			if (strcmp(attr[i], "r") == 0) r = fabsf(wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_length(p)));
		}
	}

	if (r > 0.0f) {
		p->shape_geometry = wlf_circle_shape_create(cx, cy, r);
		wlf_svg_reset_path(p);

		wlf_svg_move_to(p, cx+r, cy);
		wlf_svg_cubic_bez_to(p, cx+r, cy+r*WLF_SVG_KAPPA90, cx+r*WLF_SVG_KAPPA90, cy+r, cx, cy+r);
		wlf_svg_cubic_bez_to(p, cx-r*WLF_SVG_KAPPA90, cy+r, cx-r, cy+r*WLF_SVG_KAPPA90, cx-r, cy);
		wlf_svg_cubic_bez_to(p, cx-r, cy-r*WLF_SVG_KAPPA90, cx-r*WLF_SVG_KAPPA90, cy-r, cx, cy-r);
		wlf_svg_cubic_bez_to(p, cx+r*WLF_SVG_KAPPA90, cy-r, cx+r, cy-r*WLF_SVG_KAPPA90, cx+r, cy);

		wlf_svg_add_path(p, 1);

		wlf_svg_add_shape(p);
	}
}

static void wlf_svg_parse_ellipse(struct wlf_svg_parser *p, const char** attr)
{
	float cx = 0.0f;
	float cy = 0.0f;
	float rx = 0.0f;
	float ry = 0.0f;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "cx") == 0) cx = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
			if (strcmp(attr[i], "cy") == 0) cy = wlf_svg_parse_coordinate(p, attr[i+1], wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
			if (strcmp(attr[i], "rx") == 0) rx = fabsf(wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_width(p)));
			if (strcmp(attr[i], "ry") == 0) ry = fabsf(wlf_svg_parse_coordinate(p, attr[i+1], 0.0f, wlf_svg_actual_height(p)));
		}
	}

	if (rx > 0.0f && ry > 0.0f) {
		p->shape_geometry = wlf_ellipse_shape_create(cx, cy, rx, ry);

		wlf_svg_reset_path(p);

		wlf_svg_move_to(p, cx+rx, cy);
		wlf_svg_cubic_bez_to(p, cx+rx, cy+ry*WLF_SVG_KAPPA90, cx+rx*WLF_SVG_KAPPA90, cy+ry, cx, cy+ry);
		wlf_svg_cubic_bez_to(p, cx-rx*WLF_SVG_KAPPA90, cy+ry, cx-rx, cy+ry*WLF_SVG_KAPPA90, cx-rx, cy);
		wlf_svg_cubic_bez_to(p, cx-rx, cy-ry*WLF_SVG_KAPPA90, cx-rx*WLF_SVG_KAPPA90, cy-ry, cx, cy-ry);
		wlf_svg_cubic_bez_to(p, cx+rx*WLF_SVG_KAPPA90, cy-ry, cx+rx, cy-ry*WLF_SVG_KAPPA90, cx+rx, cy);

		wlf_svg_add_path(p, 1);

		wlf_svg_add_shape(p);
	}
}

static void wlf_svg_parse_line(struct wlf_svg_parser *p, const char** attr)
{
	float x1 = 0.0;
	float y1 = 0.0;
	float x2 = 0.0;
	float y2 = 0.0;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "x1") == 0) x1 = wlf_svg_parse_coordinate(p, attr[i + 1], wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
			if (strcmp(attr[i], "y1") == 0) y1 = wlf_svg_parse_coordinate(p, attr[i + 1], wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
			if (strcmp(attr[i], "x2") == 0) x2 = wlf_svg_parse_coordinate(p, attr[i + 1], wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
			if (strcmp(attr[i], "y2") == 0) y2 = wlf_svg_parse_coordinate(p, attr[i + 1], wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
		}
	}

	wlf_svg_reset_path(p);
	p->shape_geometry = wlf_line_shape_create(x1, y1, x2, y2);

	wlf_svg_move_to(p, x1, y1);
	wlf_svg_line_to(p, x2, y2);

	wlf_svg_add_path(p, 0);

	wlf_svg_add_shape(p);
}

static void wlf_svg_parse_poly(struct wlf_svg_parser *p, const char** attr, int closeFlag)
{
	int i;
	const char* s;
	float args[2];
	int nargs, npts = 0;
	char item[64];
	float *points = NULL;
	int points_count = 0;
	int points_cap = 0;

	wlf_svg_reset_path(p);

	for (i = 0; attr[i]; i += 2) {
		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			if (strcmp(attr[i], "points") == 0) {
				s = attr[i + 1];
				nargs = 0;
				while (*s) {
					s = wlf_svg_get_next_path_item(s, item);
					args[nargs++] = (float)wlf_svg_atof(item);
					if (nargs >= 2) {
						if (points_count >= points_cap) {
							int new_cap = points_cap ? points_cap * 2 : 8;
							float *tmp = realloc(points, (size_t)new_cap * 2 * sizeof(float));
							if (!tmp) {
								free(points);
								points = NULL;
								points_cap = 0;
								points_count = 0;
								break;
							}
							points = tmp;
							points_cap = new_cap;
						}
						if (points) {
							points[points_count * 2 + 0] = args[0];
							points[points_count * 2 + 1] = args[1];
							points_count++;
						}
						if (npts == 0)
							wlf_svg_move_to(p, args[0], args[1]);
						else
							wlf_svg_line_to(p, args[0], args[1]);
						nargs = 0;
						npts++;
					}
				}
			}
		}
	}

	if (points && points_count > 0) {
		p->shape_geometry = wlf_poly_shape_create(points, points_count, closeFlag != 0);
	}
	free(points);

	wlf_svg_add_path(p, (char)closeFlag);

	wlf_svg_add_shape(p);
}

static enum wlf_text_anchor wlf_svg_parse_text_anchor(const char *value)
{
	if (value == NULL)
		return WLF_TEXT_ANCHOR_START;
	if (strcmp(value, "middle") == 0)
		return WLF_TEXT_ANCHOR_MIDDLE;
	if (strcmp(value, "end") == 0)
		return WLF_TEXT_ANCHOR_END;
	return WLF_TEXT_ANCHOR_START;
}

static void wlf_svg_parse_text_start(struct wlf_svg_parser *p, const char **attr)
{
	struct wlf_svg_attrib *cur = wlf_svg_get_attr(p);
	int i;

	wlf_svg_parse_attribs(p, attr);

	p->textFlag = 1;
	p->textX = 0.0f;
	p->textY = 0.0f;
	p->textFontSize = cur->fontSize > 0.0f ? cur->fontSize : 16.0f;
	p->textAnchor = WLF_TEXT_ANCHOR_START;
	strncpy(p->textFontFamily, "sans-serif", sizeof(p->textFontFamily) - 1);
	p->textFontFamily[sizeof(p->textFontFamily) - 1] = '\0';
	p->textContent[0] = '\0';

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "x") == 0) {
			p->textX = wlf_svg_parse_coordinate(p, attr[i + 1],
				wlf_svg_actual_orig_x(p), wlf_svg_actual_width(p));
		} else if (strcmp(attr[i], "y") == 0) {
			p->textY = wlf_svg_parse_coordinate(p, attr[i + 1],
				wlf_svg_actual_orig_y(p), wlf_svg_actual_height(p));
		} else if (strcmp(attr[i], "font-size") == 0) {
			p->textFontSize = wlf_svg_parse_coordinate(p, attr[i + 1],
				0.0f, wlf_svg_actual_length(p));
		} else if (strcmp(attr[i], "font-family") == 0) {
			strncpy(p->textFontFamily, attr[i + 1], sizeof(p->textFontFamily) - 1);
			p->textFontFamily[sizeof(p->textFontFamily) - 1] = '\0';
		} else if (strcmp(attr[i], "text-anchor") == 0) {
			p->textAnchor = (int)wlf_svg_parse_text_anchor(attr[i + 1]);
		}
	}

	if (p->textFontSize <= 0.0f)
		p->textFontSize = 16.0f;
}

static void wlf_svg_parse_text_end(struct wlf_svg_parser *p)
{
	float text_w;
	float text_h;
	float x0;
	float y0;

	if (!p->textFlag)
		return;
	p->textFlag = 0;

	if (p->textContent[0] == '\0')
		return;

	p->shape_geometry = wlf_text_shape_create(p->textX, p->textY,
		p->textContent, p->textFontFamily, p->textFontSize,
		(enum wlf_text_anchor)p->textAnchor);
	if (p->shape_geometry == NULL)
		return;

	text_w = (float)strlen(p->textContent) * p->textFontSize * 0.6f;
	text_h = p->textFontSize;
	x0 = p->textX;
	if (p->textAnchor == WLF_TEXT_ANCHOR_MIDDLE)
		x0 -= text_w * 0.5f;
	else if (p->textAnchor == WLF_TEXT_ANCHOR_END)
		x0 -= text_w;
	y0 = p->textY - text_h;

	wlf_svg_reset_path(p);
	wlf_svg_move_to(p, x0, y0);
	wlf_svg_line_to(p, x0 + text_w, y0);
	wlf_svg_line_to(p, x0 + text_w, y0 + text_h);
	wlf_svg_line_to(p, x0, y0 + text_h);
	wlf_svg_add_path(p, 1);
	wlf_svg_add_shape(p);
}

static void wlf_svg_parse_svg(struct wlf_svg_parser *p, const char** attr)
{
	int i;
	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "xmlns") == 0) {
			strncpy(p->image->xmlns, attr[i + 1], sizeof(p->image->xmlns) - 1);
			p->image->xmlns[sizeof(p->image->xmlns) - 1] = '\0';
			continue;
		}
		if (strcmp(attr[i], "xmlns:xlink") == 0) {
			strncpy(p->image->xmlns_xlink, attr[i + 1],
				sizeof(p->image->xmlns_xlink) - 1);
			p->image->xmlns_xlink[sizeof(p->image->xmlns_xlink) - 1] = '\0';
			continue;
		}

		if (!wlf_svg_parse_attr(p, attr[i], attr[i + 1])) {
			switch (wlf_svg_lookup_root_attr_name(attr[i])) {
			case WLF_SVG_ROOT_ATTR_WIDTH:
				strncpy(p->image->width_attr, attr[i + 1],
					sizeof(p->image->width_attr) - 1);
				p->image->width_attr[sizeof(p->image->width_attr) - 1] = '\0';
				p->image->width = wlf_svg_parse_coordinate(p, attr[i + 1], 0.0f, 0.0f);
				break;
			case WLF_SVG_ROOT_ATTR_HEIGHT:
				strncpy(p->image->height_attr, attr[i + 1],
					sizeof(p->image->height_attr) - 1);
				p->image->height_attr[sizeof(p->image->height_attr) - 1] = '\0';
				p->image->height = wlf_svg_parse_coordinate(p, attr[i + 1], 0.0f, 0.0f);
				break;
			case WLF_SVG_ROOT_ATTR_VIEWBOX: {
				const char *s = attr[i + 1];
				char buf[64];
				strncpy(p->image->view_box, attr[i + 1],
					sizeof(p->image->view_box) - 1);
				p->image->view_box[sizeof(p->image->view_box) - 1] = '\0';
				s = wlf_svg_parse_number(s, buf, 64);
				p->viewMinx = wlf_svg_atof(buf);
				while (*s && (wlf_svg_isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = wlf_svg_parse_number(s, buf, 64);
				p->viewMiny = wlf_svg_atof(buf);
				while (*s && (wlf_svg_isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = wlf_svg_parse_number(s, buf, 64);
				p->viewWidth = wlf_svg_atof(buf);
				while (*s && (wlf_svg_isspace(*s) || *s == '%' || *s == ',')) s++;
				if (!*s) return;
				s = wlf_svg_parse_number(s, buf, 64);
				p->viewHeight = wlf_svg_atof(buf);
				break;
			}
			case WLF_SVG_ROOT_ATTR_PRESERVE_ASPECT_RATIO:
				strncpy(p->image->preserve_aspect_ratio, attr[i + 1],
					sizeof(p->image->preserve_aspect_ratio) - 1);
				p->image->preserve_aspect_ratio[
					sizeof(p->image->preserve_aspect_ratio) - 1] = '\0';
				if (strstr(attr[i + 1], "none") != 0) {
					// No uniform scaling
					p->alignType = WLF_SVG_ALIGN_NONE;
				} else {
					// Parse X align
					if (strstr(attr[i + 1], "xMin") != 0)
						p->alignX = WLF_SVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "xMid") != 0)
						p->alignX = WLF_SVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "xMax") != 0)
						p->alignX = WLF_SVG_ALIGN_MAX;
					// Parse X align
					if (strstr(attr[i + 1], "yMin") != 0)
						p->alignY = WLF_SVG_ALIGN_MIN;
					else if (strstr(attr[i + 1], "yMid") != 0)
						p->alignY = WLF_SVG_ALIGN_MID;
					else if (strstr(attr[i + 1], "yMax") != 0)
						p->alignY = WLF_SVG_ALIGN_MAX;
					// Parse meet/slice
					p->alignType = WLF_SVG_ALIGN_MEET;
					if (strstr(attr[i + 1], "slice") != 0)
						p->alignType = WLF_SVG_ALIGN_SLICE;
				}
				break;
			default:
				break;
			}
		}
	}
}

static void wlf_svg_parse_gradient(struct wlf_svg_parser *p, const char** attr, signed char type)
{
	int i;
	char spread_method;
	struct wlf_svg_gradient_data* grad = (struct wlf_svg_gradient_data*)malloc(sizeof(struct wlf_svg_gradient_data));
	static const struct wlf_svg_char_name_map spread_method_map[] = {
		{ "pad", WLF_SVG_SPREAD_PAD },
		{ "reflect", WLF_SVG_SPREAD_REFLECT },
		{ "repeat", WLF_SVG_SPREAD_REPEAT },
	};
	if (grad == NULL) return;
	memset(grad, 0, sizeof(struct wlf_svg_gradient_data));
	grad->units = WLF_SVG_OBJECT_SPACE;
	grad->type = type;
	if (grad->type == WLF_SVG_PAINT_LINEAR_GRADIENT) {
		grad->linear.x1 = wlf_svg_coord(0.0f, WLF_SVG_UNITS_PERCENT);
		grad->linear.y1 = wlf_svg_coord(0.0f, WLF_SVG_UNITS_PERCENT);
		grad->linear.x2 = wlf_svg_coord(100.0f, WLF_SVG_UNITS_PERCENT);
		grad->linear.y2 = wlf_svg_coord(0.0f, WLF_SVG_UNITS_PERCENT);
	} else if (grad->type == WLF_SVG_PAINT_RADIAL_GRADIENT) {
		grad->radial.cx = wlf_svg_coord(50.0f, WLF_SVG_UNITS_PERCENT);
		grad->radial.cy = wlf_svg_coord(50.0f, WLF_SVG_UNITS_PERCENT);
		grad->radial.r = wlf_svg_coord(50.0f, WLF_SVG_UNITS_PERCENT);
	}

	wlf_svg_xform_identity(grad->xform);

	for (i = 0; attr[i]; i += 2) {
		switch (wlf_svg_lookup_gradient_attr_name(attr[i])) {
			case WLF_SVG_GRADIENT_ATTR_ID:
				strncpy(grad->id, attr[i+1], 63);
				grad->id[63] = '\0';
				break;
			case WLF_SVG_GRADIENT_ATTR_GRADIENT_UNITS:
				if (strcmp(attr[i+1], "objectBoundingBox") == 0)
					grad->units = WLF_SVG_OBJECT_SPACE;
				else
					grad->units = WLF_SVG_USER_SPACE;
				break;
			case WLF_SVG_GRADIENT_ATTR_GRADIENT_TRANSFORM:
				wlf_svg_parse_transform(grad->xform, attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_CX:
				grad->radial.cx = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_CY:
				grad->radial.cy = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_R:
				grad->radial.r = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_FX:
				grad->radial.fx = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_FY:
				grad->radial.fy = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_X1:
				grad->linear.x1 = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_Y1:
				grad->linear.y1 = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_X2:
				grad->linear.x2 = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_Y2:
				grad->linear.y2 = wlf_svg_parse_coordinate_raw(attr[i + 1]);
				break;
			case WLF_SVG_GRADIENT_ATTR_SPREAD_METHOD:
				if (wlf_svg_lookup_char_value(spread_method_map,
					(int)(sizeof(spread_method_map) / sizeof(spread_method_map[0])),
					attr[i+1],
					&spread_method)) {
					grad->spread = spread_method;
				}
				break;
			case WLF_SVG_GRADIENT_ATTR_XLINK_HREF: {
				const char *href = attr[i+1];
				strncpy(grad->ref, href+1, 62);
				grad->ref[62] = '\0';
				break;
			}
			default:
				(void)wlf_svg_parse_attr(p, attr[i], attr[i + 1]);
				break;
		}
	}

	grad->next = p->gradients;
	p->gradients = grad;
}

static void wlf_svg_parse_gradient_stop(struct wlf_svg_parser *p, const char** attr)
{
	struct wlf_svg_attrib* curAttr = wlf_svg_get_attr(p);
	struct wlf_svg_gradient_data* grad;
	struct wlf_svg_gradient_stop* stop;
	int i, idx;

	curAttr->stopOffset = 0;
	curAttr->stopColor = 0;
	curAttr->stopOpacity = 1.0f;

	for (i = 0; attr[i]; i += 2) {
		wlf_svg_parse_attr(p, attr[i], attr[i + 1]);
	}

	// Add stop to the last gradient.
	grad = p->gradients;
	if (grad == NULL) return;

	grad->nstops++;
	grad->stops = (struct wlf_svg_gradient_stop*)realloc(grad->stops, sizeof(struct wlf_svg_gradient_stop)*grad->nstops);
	if (grad->stops == NULL) return;

	// Insert
	idx = grad->nstops-1;
	for (i = 0; i < grad->nstops-1; i++) {
		if (curAttr->stopOffset < grad->stops[i].offset) {
			idx = i;
			break;
		}
	}
	if (idx != grad->nstops-1) {
		for (i = grad->nstops-1; i > idx; i--)
			grad->stops[i] = grad->stops[i-1];
	}

	stop = &grad->stops[idx];
	stop->color = curAttr->stopColor;
	stop->color |= (unsigned int)(curAttr->stopOpacity*255) << 24;
	stop->offset = curAttr->stopOffset;
}

/* ---- <symbol> / <use> helpers ---- */

static struct wlf_path *wlf_svg_clone_path_translated(const struct wlf_path *src,
	float tx, float ty)
{
	int i;
	struct wlf_path *dst = malloc(sizeof(struct wlf_path));
	if (!dst) return NULL;
	memset(dst, 0, sizeof(*dst));
	dst->npts = src->npts;
	dst->closed = src->closed;
	dst->pts = (float *)malloc(src->npts * 2 * sizeof(float));
	if (!dst->pts) { free(dst); return NULL; }
	for (i = 0; i < src->npts; i++) {
		dst->pts[i * 2]     = src->pts[i * 2]     + tx;
		dst->pts[i * 2 + 1] = src->pts[i * 2 + 1] + ty;
	}
	dst->bounds[0] = src->bounds[0] + tx;
	dst->bounds[1] = src->bounds[1] + ty;
	dst->bounds[2] = src->bounds[2] + tx;
	dst->bounds[3] = src->bounds[3] + ty;
	return dst;
}

static void wlf_svg_translate_gradient(struct wlf_gradient *grad,
	float tx, float ty)
{
	if (grad == NULL)
		return;

	if (wlf_gradient_is_linear(grad)) {
		struct wlf_linear_gradient *linear =
			wlf_linear_gradient_from_gradient(grad);
		linear->start.x += tx;
		linear->start.y += ty;
		linear->end.x += tx;
		linear->end.y += ty;
	} else if (wlf_gradient_is_radial(grad)) {
		struct wlf_radial_gradient *radial =
			wlf_radial_gradient_from_gradient(grad);
		radial->center.x += tx;
		radial->center.y += ty;
		radial->focal.x += tx;
		radial->focal.y += ty;
	}
}

static struct wlf_shape *wlf_svg_clone_geometry_translated(
	struct wlf_shape *geometry, float tx, float ty)
{
	if (geometry == NULL)
		return NULL;

	if (wlf_shape_is_rect(geometry)) {
		struct wlf_rect_shape *rect = wlf_rect_shape_from_shape(geometry);
		return wlf_rect_shape_create(rect->x + tx, rect->y + ty,
			rect->width, rect->height, rect->rx, rect->ry);
	}
	if (wlf_shape_is_circle(geometry)) {
		struct wlf_circle_shape *circle = wlf_circle_shape_from_shape(geometry);
		return wlf_circle_shape_create(circle->cx + tx, circle->cy + ty, circle->r);
	}
	if (wlf_shape_is_ellipse(geometry)) {
		struct wlf_ellipse_shape *ellipse = wlf_ellipse_shape_from_shape(geometry);
		return wlf_ellipse_shape_create(ellipse->cx + tx, ellipse->cy + ty,
			ellipse->rx, ellipse->ry);
	}
	if (wlf_shape_is_line(geometry)) {
		struct wlf_line_shape *line = wlf_line_shape_from_shape(geometry);
		return wlf_line_shape_create(line->x1 + tx, line->y1 + ty,
			line->x2 + tx, line->y2 + ty);
	}
	if (wlf_shape_is_poly(geometry)) {
		struct wlf_poly_shape *poly = wlf_poly_shape_from_shape(geometry);
		float *points = calloc((size_t)poly->count * 2, sizeof(*points));
		if (points == NULL)
			return NULL;
		for (int i = 0; i < poly->count; i++) {
			points[i * 2] = poly->points[i * 2] + tx;
			points[i * 2 + 1] = poly->points[i * 2 + 1] + ty;
		}
		struct wlf_shape *out =
			wlf_poly_shape_create(points, poly->count, poly->closed);
		free(points);
		return out;
	}
	if (wlf_shape_is_path(geometry)) {
		struct wlf_path_shape *path_shape =
			wlf_path_shape_from_shape(geometry);
		struct wlf_path *paths = NULL;
		struct wlf_path *tail = NULL;
		for (struct wlf_path *p = path_shape->paths; p != NULL; p = p->next) {
			struct wlf_path *clone = wlf_svg_clone_path_translated(p, tx, ty);
			if (!clone) {
				wlf_path_destroy_list(paths);
				return NULL;
			}
			if (paths == NULL)
				paths = clone;
			else
				tail->next = clone;
			tail = clone;
		}
		return wlf_path_shape_create(paths, true);
	}
	if (wlf_shape_is_text(geometry)) {
		struct wlf_text_shape *text = wlf_text_shape_from_shape(geometry);
		return wlf_text_shape_create(text->x + tx, text->y + ty,
			text->text, text->font_family, text->font_size, text->text_anchor);
	}

	return NULL;
}

static struct wlf_svg_shape *wlf_svg_clone_shape_translated(const struct wlf_svg_shape *src,
	float tx, float ty)
{
	struct wlf_svg_shape *dst;
	struct wlf_path *sp, *dp, *dtail = NULL;

	dst = (struct wlf_svg_shape *)malloc(sizeof(struct wlf_svg_shape));
	if (!dst) return NULL;
	memset(dst, 0, sizeof(*dst));
	wlf_shape_init(&dst->base, &wlf_svg_shape_impl);
	memcpy(dst->id, src->id, sizeof dst->id);
	memcpy(dst->fill_gradient, src->fill_gradient, sizeof dst->fill_gradient);
	memcpy(dst->stroke_gradient, src->stroke_gradient, sizeof dst->stroke_gradient);
	memcpy(dst->xform, src->xform, sizeof dst->xform);
	dst->opacity = src->opacity;
	dst->fill_opacity = src->fill_opacity;
	dst->stroke_opacity = src->stroke_opacity;
	dst->stroke_width = src->stroke_width;
	dst->stroke_dash_offset = src->stroke_dash_offset;
	dst->stroke_dash_count = src->stroke_dash_count;
	memcpy(dst->stroke_dash_array, src->stroke_dash_array,
		sizeof dst->stroke_dash_array);
	dst->stroke_line_join = src->stroke_line_join;
	dst->stroke_line_cap = src->stroke_line_cap;
	dst->miter_limit = src->miter_limit;
	dst->fill_rule = src->fill_rule;
	dst->paint_order = src->paint_order;
	dst->flags = src->flags;
	dst->next = NULL;
	dst->paths = NULL;
	dst->geometry = NULL;

	dst->fill = wlf_svg_clone_gradient(src->fill);
	dst->stroke = wlf_svg_clone_gradient(src->stroke);
	if ((src->fill && !dst->fill) || (src->stroke && !dst->stroke)) {
		wlf_shape_destroy(&dst->base);
		return NULL;
	}
	wlf_svg_translate_gradient(dst->fill, tx, ty);
	wlf_svg_translate_gradient(dst->stroke, tx, ty);

	dst->geometry = wlf_svg_clone_geometry_translated(src->geometry, tx, ty);
	if (src->geometry && !dst->geometry) {
		wlf_shape_destroy(&dst->base);
		return NULL;
	}

	for (sp = src->paths; sp != NULL; sp = sp->next) {
		dp = wlf_svg_clone_path_translated(sp, tx, ty);
		if (!dp) {
			wlf_shape_destroy(&dst->base);
			return NULL;
		}
		if (!dst->paths) dst->paths = dp;
		else dtail->next = dp;
		dtail = dp;
	}

	dst->bounds[0] = src->bounds[0] + tx;
	dst->bounds[1] = src->bounds[1] + ty;
	dst->bounds[2] = src->bounds[2] + tx;
	dst->bounds[3] = src->bounds[3] + ty;
	return dst;
}

/* Start collecting shapes for a new <symbol>. */
static void wlf_svg_parse_symbol(struct wlf_svg_parser *p, const char **attr)
{
	struct wlf_svg_symbol_data *sym;
	int i;

	sym = (struct wlf_svg_symbol_data *)calloc(1, sizeof(struct wlf_svg_symbol_data));
	if (!sym) return;

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "id") == 0) {
			strncpy(sym->id, attr[i + 1], 63);
			sym->id[63] = '\0';
		}
	}

	sym->next = p->symbols;
	p->symbols = sym;
	p->current_symbol = sym;
	wlf_svg_push_attr(p);
}

/* Clone symbol shapes into image->shapes with x/y offset from <use>. */
static void wlf_svg_parse_use(struct wlf_svg_parser *p, const char **attr)
{
	float x = 0.0f, y = 0.0f;
	const char *href = NULL;
	struct wlf_svg_symbol_data *sym;
	struct wlf_svg_shape *src, *clone;
	struct wlf_shape *base;
	int i;

	for (i = 0; attr[i]; i += 2) {
		if (strcmp(attr[i], "x") == 0)
			x = (float)wlf_svg_atof(attr[i + 1]);
		else if (strcmp(attr[i], "y") == 0)
			y = (float)wlf_svg_atof(attr[i + 1]);
		else if (strcmp(attr[i], "href") == 0 ||
				strcmp(attr[i], "xlink:href") == 0)
			href = attr[i + 1];
	}
	if (!href) return;
	if (href[0] == '#') href++;

	struct wlf_svg_use_data *use = calloc(1, sizeof(*use));
	if (use != NULL) {
		strncpy(use->href, href, sizeof(use->href) - 1);
		use->href[sizeof(use->href) - 1] = '\0';
		use->x = x;
		use->y = y;
		if (p->uses == NULL) {
			p->uses = use;
		} else {
			struct wlf_svg_use_data *tail = p->uses;
			while (tail->next != NULL) {
				tail = tail->next;
			}
			tail->next = use;
		}
	}

	for (sym = p->symbols; sym != NULL; sym = sym->next) {
		if (strcmp(sym->id, href) == 0) break;
	}
	if (!sym) return;

	for (base = sym->shapes; base != NULL;
		base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next) {
		src = wlf_svg_shape_from_shape(base);
		clone = wlf_svg_clone_shape_translated(src, x, y);
		if (!clone) continue;
		clone->flags |= WLF_SVG_FLAGS_FROM_USE;
		if (p->image->shapes == NULL)
			p->image->shapes = &clone->base;
		else
			wlf_svg_shape_from_shape(p->shapesTail)->next = clone;
		p->shapesTail = &clone->base;
	}
}

static void wlf_svg_start_element(void* ud, const char* el, const char** attr)
{
	struct wlf_svg_parser *p = (struct wlf_svg_parser  *)ud;
	enum wlf_svg_element_name element_name = wlf_svg_lookup_element_name(el);

	/* <symbol> can appear inside <defs> or standalone — handle before defsFlag. */
	if (element_name == WLF_SVG_EL_SYMBOL) {
		wlf_svg_parse_symbol(p, attr);
		return;
	}

	/* Shape elements inside a <symbol> bypass defsFlag and collect into the symbol. */
	if (p->current_symbol != NULL) {
		switch (element_name) {
			case WLF_SVG_EL_G:
				wlf_svg_push_attr(p);
				wlf_svg_parse_attribs(p, attr);
				break;
			case WLF_SVG_EL_PATH:
				wlf_svg_push_attr(p);
				wlf_svg_parse_path(p, attr);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_RECT:
				wlf_svg_push_attr(p);
				wlf_svg_parse_rect(p, attr);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_CIRCLE:
				wlf_svg_push_attr(p);
				wlf_svg_parse_circle(p, attr);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_ELLIPSE:
				wlf_svg_push_attr(p);
				wlf_svg_parse_ellipse(p, attr);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_LINE:
				wlf_svg_push_attr(p);
				wlf_svg_parse_line(p, attr);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_POLYLINE:
				wlf_svg_push_attr(p);
				wlf_svg_parse_poly(p, attr, 0);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_POLYGON:
				wlf_svg_push_attr(p);
				wlf_svg_parse_poly(p, attr, 1);
				wlf_svg_pop_attr(p);
				break;
			case WLF_SVG_EL_TEXT:
				wlf_svg_push_attr(p);
				wlf_svg_parse_text_start(p, attr);
				break;
			case WLF_SVG_EL_LINEAR_GRADIENT:
				wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_LINEAR_GRADIENT);
				break;
			case WLF_SVG_EL_RADIAL_GRADIENT:
				wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_RADIAL_GRADIENT);
				break;
			case WLF_SVG_EL_STOP:
				wlf_svg_parse_gradient_stop(p, attr);
				break;
			default:
				break;
		}
		return;
	}

	if (p->defsFlag) {
		// Skip everything but gradients in defs
		if (element_name == WLF_SVG_EL_LINEAR_GRADIENT) {
			wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_LINEAR_GRADIENT);
		} else if (element_name == WLF_SVG_EL_RADIAL_GRADIENT) {
			wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_RADIAL_GRADIENT);
		} else if (element_name == WLF_SVG_EL_STOP) {
			wlf_svg_parse_gradient_stop(p, attr);
		}
		return;
	}

	switch (element_name) {
		case WLF_SVG_EL_G:
			wlf_svg_push_attr(p);
			wlf_svg_parse_attribs(p, attr);
			break;
		case WLF_SVG_EL_PATH:
			if (p->pathFlag)
				return;
			wlf_svg_push_attr(p);
			wlf_svg_parse_path(p, attr);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_RECT:
			wlf_svg_push_attr(p);
			wlf_svg_parse_rect(p, attr);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_CIRCLE:
			wlf_svg_push_attr(p);
			wlf_svg_parse_circle(p, attr);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_ELLIPSE:
			wlf_svg_push_attr(p);
			wlf_svg_parse_ellipse(p, attr);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_LINE:
			wlf_svg_push_attr(p);
			wlf_svg_parse_line(p, attr);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_POLYLINE:
			wlf_svg_push_attr(p);
			wlf_svg_parse_poly(p, attr, 0);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_POLYGON:
			wlf_svg_push_attr(p);
			wlf_svg_parse_poly(p, attr, 1);
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_TEXT:
			wlf_svg_push_attr(p);
			wlf_svg_parse_text_start(p, attr);
			break;
		case WLF_SVG_EL_LINEAR_GRADIENT:
			wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_LINEAR_GRADIENT);
			break;
		case WLF_SVG_EL_RADIAL_GRADIENT:
			wlf_svg_parse_gradient(p, attr, WLF_SVG_PAINT_RADIAL_GRADIENT);
			break;
		case WLF_SVG_EL_STOP:
			wlf_svg_parse_gradient_stop(p, attr);
			break;
		case WLF_SVG_EL_DEFS:
			p->defsFlag = 1;
			break;
		case WLF_SVG_EL_SVG:
			wlf_svg_parse_svg(p, attr);
			break;
		case WLF_SVG_EL_USE:
			wlf_svg_push_attr(p);
			wlf_svg_parse_use(p, attr);
			wlf_svg_pop_attr(p);
			break;
		default:
			break;
	}
}

static void wlf_svg_end_element(void* ud, const char* el)
{
	struct wlf_svg_parser *p = (struct wlf_svg_parser *)ud;
	enum wlf_svg_element_name element_name = wlf_svg_lookup_element_name(el);

	switch (element_name) {
		case WLF_SVG_EL_G:
			wlf_svg_pop_attr(p);
			break;
		case WLF_SVG_EL_PATH:
			p->pathFlag = 0;
			break;
		case WLF_SVG_EL_DEFS:
			p->defsFlag = 0;
			break;
		case WLF_SVG_EL_SYMBOL:
			wlf_svg_pop_attr(p);
			p->current_symbol = NULL;
			break;
		case WLF_SVG_EL_TEXT:
			wlf_svg_parse_text_end(p);
			wlf_svg_pop_attr(p);
			break;
		default:
			break;
	}
}

static void wlf_svg_content(void* ud, const char* s)
{
	struct wlf_svg_parser *p = (struct wlf_svg_parser *)ud;
	size_t n, cap;

	if (!p->textFlag || s == NULL || *s == '\0')
		return;

	n = strlen(p->textContent);
	if (n >= sizeof(p->textContent) - 1)
		return;
	cap = sizeof(p->textContent) - 1 - n;
	strncat(p->textContent, s, cap);
}

static void wlf_svg_image_bounds(struct wlf_svg_parser *p, float* bounds)
{
	struct wlf_shape *base = p->image->shapes;
	struct wlf_svg_shape *shape;
	if (base == NULL) {
		bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0;
		return;
	}
	shape = wlf_svg_shape_from_shape(base);
	bounds[0] = shape->bounds[0];
	bounds[1] = shape->bounds[1];
	bounds[2] = shape->bounds[2];
	bounds[3] = shape->bounds[3];
	for (shape = shape->next; shape != NULL; shape = shape->next) {
		bounds[0] = wlf_svg_minf(bounds[0], shape->bounds[0]);
		bounds[1] = wlf_svg_minf(bounds[1], shape->bounds[1]);
		bounds[2] = wlf_svg_maxf(bounds[2], shape->bounds[2]);
		bounds[3] = wlf_svg_maxf(bounds[3], shape->bounds[3]);
	}
}

static float wlf_svg_view_align(float content, float container, int type)
{
	if (type == WLF_SVG_ALIGN_MIN)
		return 0;
	else if (type == WLF_SVG_ALIGN_MAX)
		return container - content;
	// mid
	return (container - content) * 0.5f;
}

static void wlf_svg_scale_gradient(struct wlf_gradient* grad, float tx, float ty,
	float sx, float sy)
{
	if (grad == NULL)
		return;

	if (wlf_gradient_is_linear(grad)) {
		struct wlf_linear_gradient *linear =
			wlf_linear_gradient_from_gradient(grad);
		linear->start.x = (linear->start.x + tx) * sx;
		linear->start.y = (linear->start.y + ty) * sy;
		linear->end.x = (linear->end.x + tx) * sx;
		linear->end.y = (linear->end.y + ty) * sy;
	} else if (wlf_gradient_is_radial(grad)) {
		struct wlf_radial_gradient *radial =
			wlf_radial_gradient_from_gradient(grad);
		float avg = (sx + sy) * 0.5f;
		radial->center.x = (radial->center.x + tx) * sx;
		radial->center.y = (radial->center.y + ty) * sy;
		radial->focal.x = (radial->focal.x + tx) * sx;
		radial->focal.y = (radial->focal.y + ty) * sy;
		radial->radius *= avg;
	}
}

static void wlf_svg_transform_geometry(struct wlf_shape *geometry,
	float tx, float ty, float sx, float sy)
{
	if (geometry == NULL)
		return;

	if (wlf_shape_is_rect(geometry)) {
		struct wlf_rect_shape *rect = wlf_rect_shape_from_shape(geometry);
		rect->x = (rect->x + tx) * sx;
		rect->y = (rect->y + ty) * sy;
		rect->width *= sx;
		rect->height *= sy;
		rect->rx *= sx;
		rect->ry *= sy;
		return;
	}
	if (wlf_shape_is_circle(geometry)) {
		struct wlf_circle_shape *circle = wlf_circle_shape_from_shape(geometry);
		float avg = (sx + sy) * 0.5f;
		circle->cx = (circle->cx + tx) * sx;
		circle->cy = (circle->cy + ty) * sy;
		circle->r *= avg;
		return;
	}
	if (wlf_shape_is_ellipse(geometry)) {
		struct wlf_ellipse_shape *ellipse = wlf_ellipse_shape_from_shape(geometry);
		ellipse->cx = (ellipse->cx + tx) * sx;
		ellipse->cy = (ellipse->cy + ty) * sy;
		ellipse->rx *= sx;
		ellipse->ry *= sy;
		return;
	}
	if (wlf_shape_is_line(geometry)) {
		struct wlf_line_shape *line = wlf_line_shape_from_shape(geometry);
		line->x1 = (line->x1 + tx) * sx;
		line->y1 = (line->y1 + ty) * sy;
		line->x2 = (line->x2 + tx) * sx;
		line->y2 = (line->y2 + ty) * sy;
		return;
	}
	if (wlf_shape_is_poly(geometry)) {
		struct wlf_poly_shape *poly = wlf_poly_shape_from_shape(geometry);
		for (int i = 0; i < poly->count; i++) {
			poly->points[i * 2] = (poly->points[i * 2] + tx) * sx;
			poly->points[i * 2 + 1] = (poly->points[i * 2 + 1] + ty) * sy;
		}
		return;
	}
	if (wlf_shape_is_path(geometry)) {
		struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(geometry);
		if (!path_shape->owns_paths)
			return;
		for (struct wlf_path *path = path_shape->paths; path != NULL; path = path->next) {
			path->bounds[0] = (path->bounds[0] + tx) * sx;
			path->bounds[1] = (path->bounds[1] + ty) * sy;
			path->bounds[2] = (path->bounds[2] + tx) * sx;
			path->bounds[3] = (path->bounds[3] + ty) * sy;
			for (int i = 0; i < path->npts; i++) {
				float *pt = &path->pts[i * 2];
				pt[0] = (pt[0] + tx) * sx;
				pt[1] = (pt[1] + ty) * sy;
			}
		}
		return;
	}
	if (wlf_shape_is_text(geometry)) {
		struct wlf_text_shape *text = wlf_text_shape_from_shape(geometry);
		text->x = (text->x + tx) * sx;
		text->y = (text->y + ty) * sy;
		text->font_size *= (sx + sy) * 0.5f;
		return;
	}
}

static void wlf_svg_scale_to_viewbox(struct wlf_svg_parser *p, const char* units)
{
	struct wlf_shape *base;
	struct wlf_svg_shape* shape;
	struct wlf_path* path;
	float tx, ty, sx, sy, us, bounds[4], avgs;
	int i;
	float* pt;

	// Guess image size if not set completely.
	wlf_svg_image_bounds(p, bounds);

	if (p->viewWidth == 0) {
		if (p->image->width > 0) {
			p->viewWidth = p->image->width;
		} else {
			p->viewMinx = bounds[0];
			p->viewWidth = bounds[2] - bounds[0];
		}
	}
	if (p->viewHeight == 0) {
		if (p->image->height > 0) {
			p->viewHeight = p->image->height;
		} else {
			p->viewMiny = bounds[1];
			p->viewHeight = bounds[3] - bounds[1];
		}
	}
	if (p->image->width == 0)
		p->image->width = p->viewWidth;
	if (p->image->height == 0)
		p->image->height = p->viewHeight;

	tx = -p->viewMinx;
	ty = -p->viewMiny;
	sx = p->viewWidth > 0 ? p->image->width / p->viewWidth : 0;
	sy = p->viewHeight > 0 ? p->image->height / p->viewHeight : 0;
	// Unit scaling
	us = 1.0f / wlf_svg_convert_to_pixels(p, wlf_svg_coord(1.0f, wlf_svg_parse_units(units)), 0.0f, 1.0f);

	// Fix aspect ratio
	if (p->alignType == WLF_SVG_ALIGN_MEET) {
		// fit whole image into viewbox
		sx = sy = wlf_svg_minf(sx, sy);
		tx += wlf_svg_view_align(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += wlf_svg_view_align(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	} else if (p->alignType == WLF_SVG_ALIGN_SLICE) {
		// fill whole viewbox with image
		sx = sy = wlf_svg_maxf(sx, sy);
		tx += wlf_svg_view_align(p->viewWidth*sx, p->image->width, p->alignX) / sx;
		ty += wlf_svg_view_align(p->viewHeight*sy, p->image->height, p->alignY) / sy;
	}

	// Transform
	sx *= us;
	sy *= us;
	avgs = (sx+sy) / 2.0f;
	for (base = p->image->shapes; base != NULL;
		base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next) {
		shape = wlf_svg_shape_from_shape(base);
		shape->bounds[0] = (shape->bounds[0] + tx) * sx;
		shape->bounds[1] = (shape->bounds[1] + ty) * sy;
		shape->bounds[2] = (shape->bounds[2] + tx) * sx;
		shape->bounds[3] = (shape->bounds[3] + ty) * sy;
		for (path = shape->paths; path != NULL; path = path->next) {
			path->bounds[0] = (path->bounds[0] + tx) * sx;
			path->bounds[1] = (path->bounds[1] + ty) * sy;
			path->bounds[2] = (path->bounds[2] + tx) * sx;
			path->bounds[3] = (path->bounds[3] + ty) * sy;
			for (i =0; i < path->npts; i++) {
				pt = &path->pts[i*2];
				pt[0] = (pt[0] + tx) * sx;
				pt[1] = (pt[1] + ty) * sy;
			}
		}

		wlf_svg_transform_geometry(shape->geometry, tx, ty, sx, sy);
		wlf_svg_scale_gradient(shape->fill, tx, ty, sx, sy);
		wlf_svg_scale_gradient(shape->stroke, tx, ty, sx, sy);

		shape->stroke_width *= avgs;
		shape->stroke_dash_offset *= avgs;
		for (i = 0; i < shape->stroke_dash_count; i++)
			shape->stroke_dash_array[i] *= avgs;
	}
}

static void wlf_svg_create_gradients(struct wlf_svg_parser *p)
{
	struct wlf_shape *base;
	struct wlf_svg_shape *shape;

	for (base = p->image->shapes; base != NULL;
		base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next) {
		shape = wlf_svg_shape_from_shape(base);

		if (shape->fill_gradient[0] != '\0') {
			float inv[6], localBounds[4];
			float opacity_scale = wlf_svg_shape_fill_alpha(shape);
			wlf_svg_xform_inverse(inv, shape->xform);
			wlf_svg_get_local_bounds(localBounds, shape, inv);
			struct wlf_gradient *grad =
				wlf_svg_create_gradient(p, shape->fill_gradient,
					localBounds, shape->xform, opacity_scale);
			if (grad != NULL) {
				wlf_gradient_destroy(shape->fill);
				shape->fill = grad;
			}
		}

		if (shape->stroke_gradient[0] != '\0') {
			float inv[6], localBounds[4];
			float opacity_scale = wlf_svg_shape_stroke_alpha(shape);
			wlf_svg_xform_inverse(inv, shape->xform);
			wlf_svg_get_local_bounds(localBounds, shape, inv);
			struct wlf_gradient *grad =
				wlf_svg_create_gradient(p, shape->stroke_gradient,
					localBounds, shape->xform, opacity_scale);
			if (grad != NULL) {
				wlf_gradient_destroy(shape->stroke);
				shape->stroke = grad;
			}
		}
	}
}

struct wlf_svg_image *wlf_svg_parse_from_file(const char *filename,
		const char *units, float dpi) {
	FILE* fp = NULL;
	size_t size;
	char* data = NULL;
	struct wlf_svg_image* image = NULL;

	fp = fopen(filename, "rb");
	if (!fp) goto error;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (char*)malloc(size+1);
	if (data == NULL) goto error;
	if (fread(data, 1, size, fp) != size) goto error;
	data[size] = '\0';	// Must be null terminated.
	fclose(fp);
	image = wlf_svg_parse(data, units, dpi);
	free(data);

	return image;

error:
	if (fp) fclose(fp);
	if (data) free(data);
	if (image) wlf_svg_destroy(image);
	return NULL;
}

struct wlf_svg_image *wlf_svg_parse(char *input, const char *units, float dpi) {
	struct wlf_svg_parser *p;
	struct wlf_svg_image* ret = 0;

	p = wlf_svg_create_parser();
	if (p == NULL) {
		return NULL;
	}
	p->dpi = dpi;

	wlf_svg_parse_xml(input, wlf_svg_start_element, wlf_svg_end_element, wlf_svg_content, p);

	// Create gradients after all definitions have been parsed
	wlf_svg_create_gradients(p);

	// Scale to viewBox
	wlf_svg_scale_to_viewbox(p, units);

	ret = p->image;
	p->image = NULL;

	if (ret != NULL) {
		ret->symbols = p->symbols;
		ret->uses = p->uses;
		p->symbols = NULL;
		p->uses = NULL;
	}

	wlf_svg_delete_parser(p);

	return ret;
}

struct wlf_path *wlf_svg_path_duplicate(struct wlf_path *path) {
	return wlf_path_duplicate(path);
}

static const char *wlf_svg_cap_to_str(char cap)
{
	switch (cap) {
	case WLF_SVG_CAP_BUTT:
		return "butt";
	case WLF_SVG_CAP_ROUND:
		return "round";
	case WLF_SVG_CAP_SQUARE:
		return "square";
	default:
		return "butt";
	}
}

static const char *wlf_svg_join_to_str(char join)
{
	switch (join) {
	case WLF_SVG_JOIN_MITER:
		return "miter";
	case WLF_SVG_JOIN_ROUND:
		return "round";
	case WLF_SVG_JOIN_BEVEL:
		return "bevel";
	default:
		return "miter";
	}
}

static bool wlf_svg_gradient_is_solid(const struct wlf_gradient *grad,
	struct wlf_color *out_color)
{
	if (grad == NULL || grad->stop_count == 0) {
		return false;
	}

	struct wlf_color color = grad->stops[0].color;
	for (size_t i = 1; i < grad->stop_count; i++) {
		if (!wlf_color_equal(&color, &grad->stops[i].color)) {
			return false;
		}
	}

	if (out_color != NULL) {
		*out_color = color;
	}
	return true;
}

static struct wlf_color wlf_svg_gradient_fallback_color(
	const struct wlf_gradient *grad)
{
	if (grad == NULL || grad->stop_count == 0) {
		return WLF_COLOR_TRANSPARENT;
	}
	if (wlf_svg_gradient_is_solid(grad, NULL)) {
		return grad->stops[0].color;
	}
	return wlf_gradient_sample_stops((struct wlf_gradient *)grad, 0.5);
}

static void wlf_svg_write_color(FILE *fp, const struct wlf_color *color,
	const char *attr)
{
	struct wlf_color c = wlf_color_clamp(color);
	unsigned int r = (unsigned int)(c.r * 255.0 + 0.5);
	unsigned int g = (unsigned int)(c.g * 255.0 + 0.5);
	unsigned int b = (unsigned int)(c.b * 255.0 + 0.5);
	float a = (float)c.a;

	fprintf(fp, "%s=\"rgb(%u,%u,%u)\"", attr, r, g, b);
	if (a < 0.999f) {
		fprintf(fp, " %s-opacity=\"%.3f\"", attr, a);
	}
}

static void wlf_svg_write_style(FILE *fp, const struct wlf_svg_shape *shape)
{
	fprintf(fp, " ");
	if (shape->fill == NULL) {
		fprintf(fp, "fill=\"none\" ");
	} else {
		struct wlf_color solid;
		if (wlf_svg_gradient_is_solid(shape->fill, &solid)) {
			wlf_svg_write_color(fp, &solid, "fill");
			fprintf(fp, " ");
		} else {
			struct wlf_color fallback =
				wlf_svg_gradient_fallback_color(shape->fill);
			wlf_svg_write_color(fp, &fallback, "fill");
			fprintf(fp, " ");
		}
	}

	if (shape->stroke == NULL) {
		fprintf(fp, "stroke=\"none\" ");
	} else {
		struct wlf_color solid;
		if (wlf_svg_gradient_is_solid(shape->stroke, &solid)) {
			wlf_svg_write_color(fp, &solid, "stroke");
			fprintf(fp, " ");
		} else {
			struct wlf_color fallback =
				wlf_svg_gradient_fallback_color(shape->stroke);
			wlf_svg_write_color(fp, &fallback, "stroke");
			fprintf(fp, " ");
		}
	}

	if (shape->stroke != NULL) {
		fprintf(fp, "stroke-width=\"%.3f\" ", shape->stroke_width);
		fprintf(fp, "stroke-linecap=\"%s\" ",
			wlf_svg_cap_to_str(shape->stroke_line_cap));
		fprintf(fp, "stroke-linejoin=\"%s\" ",
			wlf_svg_join_to_str(shape->stroke_line_join));
	}
}

static void wlf_svg_write_float(FILE *fp, float value)
{
	if (fabsf(value) < 1e-6f) {
		fprintf(fp, "0");
		return;
	}

	if (fabsf(value - roundf(value)) < 1e-6f) {
		fprintf(fp, "%.0f", value);
		return;
	}

	char buf[32];
	snprintf(buf, sizeof(buf), "%.6f", value);
	char *end = buf + strlen(buf) - 1;
	while (end > buf && *end == '0') {
		*end-- = '\0';
	}
	if (end > buf && *end == '.') {
		*end = '\0';
	}
	fprintf(fp, "%s", buf);
}

static void wlf_svg_write_attr_float(FILE *fp, const char *attr, float value)
{
	fprintf(fp, " %s=\"", attr);
	wlf_svg_write_float(fp, value);
	fprintf(fp, "\"");
}

static void wlf_svg_write_path_data(FILE *fp, const struct wlf_path *path)
{
	fprintf(fp, "M %.3f %.3f ", path->pts[0], path->pts[1]);
	for (int i = 0; i < path->npts - 1; i += 3) {
		float cpx1 = path->pts[(i + 1) * 2];
		float cpy1 = path->pts[(i + 1) * 2 + 1];
		float cpx2 = path->pts[(i + 2) * 2];
		float cpy2 = path->pts[(i + 2) * 2 + 1];
		float x = path->pts[(i + 3) * 2];
		float y = path->pts[(i + 3) * 2 + 1];
		fprintf(fp, "C %.3f %.3f %.3f %.3f %.3f %.3f ",
			cpx1, cpy1, cpx2, cpy2, x, y);
	}
	if (path->closed) {
		fprintf(fp, "Z");
	}
}

static void wlf_svg_write_poly_points(FILE *fp, const float *points, int count)
{
	for (int i = 0; i < count; i++) {
		wlf_svg_write_float(fp, points[i * 2]);
		fprintf(fp, ",");
		wlf_svg_write_float(fp, points[i * 2 + 1]);
		if (i != count - 1) {
			fprintf(fp, " ");
		}
	}
}

static void wlf_svg_write_indent(FILE *fp, int indent)
{
	for (int i = 0; i < indent; i++) {
		fputc(' ', fp);
	}
}

static void wlf_svg_write_shape(FILE *fp, const struct wlf_svg_shape *s,
	int indent)
{
	struct wlf_shape *geom = s->geometry;

	if (geom && wlf_shape_is_rect(geom)) {
		const struct wlf_rect_shape *rect = wlf_rect_shape_from_shape(geom);
		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<rect");
		wlf_svg_write_attr_float(fp, "x", rect->x);
		wlf_svg_write_attr_float(fp, "y", rect->y);
		wlf_svg_write_attr_float(fp, "width", rect->width);
		wlf_svg_write_attr_float(fp, "height", rect->height);
		if (rect->rx > 0.0f || rect->ry > 0.0f) {
			wlf_svg_write_attr_float(fp, "rx", rect->rx);
			wlf_svg_write_attr_float(fp, "ry", rect->ry);
			fprintf(fp, " ");
		}
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
		return;
	}

	if (geom && wlf_shape_is_circle(geom)) {
		const struct wlf_circle_shape *circle = wlf_circle_shape_from_shape(geom);
		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<circle");
		wlf_svg_write_attr_float(fp, "cx", circle->cx);
		wlf_svg_write_attr_float(fp, "cy", circle->cy);
		wlf_svg_write_attr_float(fp, "r", circle->r);
		fprintf(fp, " ");
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
		return;
	}

	if (geom && wlf_shape_is_ellipse(geom)) {
		const struct wlf_ellipse_shape *ellipse =
			wlf_ellipse_shape_from_shape(geom);
		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<ellipse");
		wlf_svg_write_attr_float(fp, "cx", ellipse->cx);
		wlf_svg_write_attr_float(fp, "cy", ellipse->cy);
		wlf_svg_write_attr_float(fp, "rx", ellipse->rx);
		wlf_svg_write_attr_float(fp, "ry", ellipse->ry);
		fprintf(fp, " ");
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
		return;
	}

	if (geom && wlf_shape_is_line(geom)) {
		const struct wlf_line_shape *line = wlf_line_shape_from_shape(geom);
		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<line");
		wlf_svg_write_attr_float(fp, "x1", line->x1);
		wlf_svg_write_attr_float(fp, "y1", line->y1);
		wlf_svg_write_attr_float(fp, "x2", line->x2);
		wlf_svg_write_attr_float(fp, "y2", line->y2);
		fprintf(fp, " ");
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
		return;
	}

	if (geom && wlf_shape_is_poly(geom)) {
		const struct wlf_poly_shape *poly = wlf_poly_shape_from_shape(geom);
		wlf_svg_write_indent(fp, indent);
		if (poly->closed) {
			fprintf(fp, "<polygon points=\"");
		} else {
			fprintf(fp, "<polyline points=\"");
		}
		wlf_svg_write_poly_points(fp, poly->points, poly->count);
		fprintf(fp, "\" ");
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
		return;
	}

	if (geom && wlf_shape_is_text(geom)) {
		const struct wlf_text_shape *text = wlf_text_shape_from_shape(geom);
		const char *anchor = "start";
		if (text->text_anchor == WLF_TEXT_ANCHOR_MIDDLE)
			anchor = "middle";
		else if (text->text_anchor == WLF_TEXT_ANCHOR_END)
			anchor = "end";

		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<text");
		wlf_svg_write_attr_float(fp, "x", text->x);
		wlf_svg_write_attr_float(fp, "y", text->y);
		wlf_svg_write_attr_float(fp, "font-size", text->font_size);
		fprintf(fp, " font-family=\"%s\"", text->font_family);
		fprintf(fp, " text-anchor=\"%s\"", anchor);
		wlf_svg_write_style(fp, s);
		fprintf(fp, ">%s</text>\n", text->text);
		return;
	}

	const struct wlf_path *paths = s->paths;
	if (geom && wlf_shape_is_path(geom)) {
		const struct wlf_path_shape *path_shape = wlf_path_shape_from_shape(geom);
		paths = path_shape->paths;
	}

	for (const struct wlf_path *p = paths; p != NULL; p = p->next) {
		wlf_svg_write_indent(fp, indent);
		fprintf(fp, "<path d=\"");
		wlf_svg_write_path_data(fp, p);
		fprintf(fp, "\" ");
		wlf_svg_write_style(fp, s);
		fprintf(fp, "/>\n");
	}
}

static void wlf_svg_write_shape_list(FILE *fp, const struct wlf_shape *list,
	int indent, bool skip_from_use)
{
	for (const struct wlf_shape *base = list; base != NULL;
		base = (const struct wlf_shape *)wlf_svg_shape_from_shape(
			(struct wlf_shape *)base)->next) {
		const struct wlf_svg_shape *s =
			wlf_svg_shape_from_shape((struct wlf_shape *)base);
		if (skip_from_use && (s->flags & WLF_SVG_FLAGS_FROM_USE)) {
			continue;
		}
		wlf_svg_write_shape(fp, s, indent);
	}
}

static bool wlf_svg_shape_list_bounds(const struct wlf_shape *list,
	float bounds[4])
{
	bool first = true;
	for (const struct wlf_shape *base = list; base != NULL;
		base = (const struct wlf_shape *)wlf_svg_shape_from_shape(
			(struct wlf_shape *)base)->next) {
		const struct wlf_svg_shape *s =
			wlf_svg_shape_from_shape((struct wlf_shape *)base);
		if (first) {
			memcpy(bounds, s->bounds, sizeof(float) * 4);
			first = false;
		} else {
			if (s->bounds[0] < bounds[0]) bounds[0] = s->bounds[0];
			if (s->bounds[1] < bounds[1]) bounds[1] = s->bounds[1];
			if (s->bounds[2] > bounds[2]) bounds[2] = s->bounds[2];
			if (s->bounds[3] > bounds[3]) bounds[3] = s->bounds[3];
		}
	}
	return !first;
}

bool wlf_svg_save(const struct wlf_svg_image *image, const char *filename)
{
	if (image == NULL || filename == NULL) {
		return false;
	}

	FILE *fp = fopen(filename, "wb");
	if (fp == NULL) {
		return false;
	}

	struct wlf_svg_info info = {0};
	wlf_svg_get_info(image, &info);
	float width = image->width;
	float height = image->height;
	if (width <= 0.0f || height <= 0.0f) {
		width = info.bounds[2] - info.bounds[0];
		height = info.bounds[3] - info.bounds[1];
		if (width <= 0.0f) width = 1.0f;
		if (height <= 0.0f) height = 1.0f;
	}

	const char *xml_version = image->xml_version[0] ? image->xml_version : "1.0";
	const char *xml_encoding = image->xml_encoding[0] ? image->xml_encoding : "UTF-8";
	const char *xmlns = image->xmlns[0] ? image->xmlns : "http://www.w3.org/2000/svg";
	bool need_xlink = image->uses != NULL || image->xmlns_xlink[0] != '\0';

	fprintf(fp, "<?xml version=\"%s\" encoding=\"%s\"?>\n",
		xml_version, xml_encoding);
	fprintf(fp, "<svg xmlns=\"%s\"", xmlns);
	if (need_xlink) {
		const char *xmlns_xlink = image->xmlns_xlink[0] ? image->xmlns_xlink
			: "http://www.w3.org/1999/xlink";
		fprintf(fp, " xmlns:xlink=\"%s\"", xmlns_xlink);
	}

	if (image->width_attr[0] != '\0') {
		fprintf(fp, " width=\"%s\"", image->width_attr);
	} else {
		fprintf(fp, " width=\"");
		wlf_svg_write_float(fp, width);
		fprintf(fp, "\"");
	}

	if (image->height_attr[0] != '\0') {
		fprintf(fp, " height=\"%s\"", image->height_attr);
	} else {
		fprintf(fp, " height=\"");
		wlf_svg_write_float(fp, height);
		fprintf(fp, "\"");
	}

	if (image->view_box[0] != '\0') {
		fprintf(fp, " viewBox=\"%s\"", image->view_box);
	} else {
		fprintf(fp, " viewBox=\"0 0 ");
		wlf_svg_write_float(fp, width);
		fprintf(fp, " ");
		wlf_svg_write_float(fp, height);
		fprintf(fp, "\"");
	}

	if (image->preserve_aspect_ratio[0] != '\0') {
		fprintf(fp, " preserveAspectRatio=\"%s\"",
			image->preserve_aspect_ratio);
	}
	fprintf(fp, ">\n");

	if (image->symbols != NULL) {
		fprintf(fp, "  <defs>\n");
		for (struct wlf_svg_symbol_data *sym = image->symbols; sym != NULL;
			sym = sym->next) {
			float bounds[4];
			bool has_bounds = wlf_svg_shape_list_bounds(sym->shapes, bounds);
			wlf_svg_write_indent(fp, 4);
			fprintf(fp, "<symbol id=\"%s\"", sym->id[0] ? sym->id : "");
			if (has_bounds) {
				fprintf(fp, " viewBox=\"");
				wlf_svg_write_float(fp, bounds[0]);
				fprintf(fp, " ");
				wlf_svg_write_float(fp, bounds[1]);
				fprintf(fp, " ");
				wlf_svg_write_float(fp, bounds[2] - bounds[0]);
				fprintf(fp, " ");
				wlf_svg_write_float(fp, bounds[3] - bounds[1]);
				fprintf(fp, "\"");
			}
			fprintf(fp, ">\n");
			wlf_svg_write_shape_list(fp, sym->shapes, 6, false);
			wlf_svg_write_indent(fp, 4);
			fprintf(fp, "</symbol>\n");
		}
		fprintf(fp, "  </defs>\n");
	}

	wlf_svg_write_shape_list(fp, image->shapes, 2, true);
	for (struct wlf_svg_use_data *use = image->uses; use != NULL;
		use = use->next) {
		wlf_svg_write_indent(fp, 2);
		fprintf(fp, "<use href=\"#%s\" xlink:href=\"#%s\"",
			use->href, use->href);
		wlf_svg_write_attr_float(fp, "x", use->x);
		wlf_svg_write_attr_float(fp, "y", use->y);
		fprintf(fp, "/>\n");
	}

	fprintf(fp, "</svg>\n");
	fclose(fp);
	return true;
}

void wlf_svg_destroy(struct wlf_svg_image *image) {
	struct wlf_shape *shape, *snext;
	if (image == NULL) return;
	shape = image->shapes;
	while (shape != NULL) {
		struct wlf_svg_shape *svg = wlf_svg_shape_from_shape(shape);
		snext = (struct wlf_shape *)svg->next;
		wlf_shape_destroy(shape);
		shape = snext;
	}
	wlf_svg_delete_symbol_data(image->symbols);
	wlf_svg_delete_use_data(image->uses);
	free(image);
}

void wlf_svg_get_info(const struct wlf_svg_image *image, struct wlf_svg_info *info) {
	struct wlf_shape *base;
	struct wlf_svg_shape *shape;
	struct wlf_path *path;
	int first = 1;

	if (!image || !info)
		return;

	*info = (struct wlf_svg_info){0};
	info->width = image->width;
	info->height = image->height;

	for (base = image->shapes; base != NULL;
		base = (struct wlf_shape *)wlf_svg_shape_from_shape(base)->next) {
		shape = wlf_svg_shape_from_shape(base);
		info->n_shapes++;
		for (path = shape->paths; path != NULL; path = path->next) {
			info->n_paths++;
			if (first) {
				info->bounds[0] = path->bounds[0];
				info->bounds[1] = path->bounds[1];
				info->bounds[2] = path->bounds[2];
				info->bounds[3] = path->bounds[3];
				first = 0;
			} else {
				if (path->bounds[0] < info->bounds[0]) info->bounds[0] = path->bounds[0];
				if (path->bounds[1] < info->bounds[1]) info->bounds[1] = path->bounds[1];
				if (path->bounds[2] > info->bounds[2]) info->bounds[2] = path->bounds[2];
				if (path->bounds[3] > info->bounds[3]) info->bounds[3] = path->bounds[3];
			}
		}
	}
}

unsigned int wlf_svg_parse_color_name(const char* str)
{
	int i, ncolors = sizeof(wlf_svg_colors) / sizeof(struct wlf_svg_named_color);

	for (i = 0; i < ncolors; i++) {
		if (strcmp(wlf_svg_colors[i].name, str) == 0) {
			return wlf_svg_colors[i].color;
		}
	}

	return WLF_SVG_RGB(128, 128, 128);
}

