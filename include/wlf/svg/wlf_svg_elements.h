/**
 * @file        wlf_svg_elements.h
 * @brief       SVG element type definitions for wlframe.
 * @details     This file defines the specific data structures for each SVG
 *              element type. Each structure contains the type-specific
 *              attributes and properties according to the SVG specification.
 *
 * @author      YaoBing Xiao
 * @date        2025-01-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-01-17, initial version\n
 */

#ifndef SVG_WLF_SVG_ELEMENTS_H
#define SVG_WLF_SVG_ELEMENTS_H

#include "wlf/svg/wlf_svg_node.h"

/**
 * @brief SVG root element data.
 */
struct wlf_svg_element_svg {
	struct wlf_svg_length width;          /**< SVG width */
	struct wlf_svg_length height;         /**< SVG height */
	struct wlf_svg_rect viewbox;          /**< ViewBox rectangle */
	bool viewbox_set;                     /**< Whether viewBox is specified */
	enum {
		WLF_SVG_ASPECT_RATIO_NONE,
		WLF_SVG_ASPECT_RATIO_XMINYMIN,
		WLF_SVG_ASPECT_RATIO_XMIDYMIN,
		WLF_SVG_ASPECT_RATIO_XMAXYMIN,
		WLF_SVG_ASPECT_RATIO_XMINYMID,
		WLF_SVG_ASPECT_RATIO_XMIDYMID,
		WLF_SVG_ASPECT_RATIO_XMAXYMID,
		WLF_SVG_ASPECT_RATIO_XMINYMAX,
		WLF_SVG_ASPECT_RATIO_XMIDYMAX,
		WLF_SVG_ASPECT_RATIO_XMAXYMAX
	} preserve_aspect_ratio;
	bool meet_or_slice;                   /**< true for meet, false for slice */
	char *version;                        /**< SVG version */
	char *xmlns;                          /**< XML namespace */
};

/**
 * @brief Group element data.
 */
struct wlf_svg_element_g {
	/* Groups primarily use common attributes from base node */
	char *title;                          /**< Optional title */
	char *desc;                           /**< Optional description */
};

/**
 * @brief Rectangle element data.
 */
struct wlf_svg_element_rect {
	struct wlf_svg_length x;              /**< X coordinate */
	struct wlf_svg_length y;              /**< Y coordinate */
	struct wlf_svg_length width;          /**< Width */
	struct wlf_svg_length height;         /**< Height */
	struct wlf_svg_length rx;             /**< X radius for rounded corners */
	struct wlf_svg_length ry;             /**< Y radius for rounded corners */
};

/**
 * @brief Circle element data.
 */
struct wlf_svg_element_circle {
	struct wlf_svg_length cx;             /**< Center X coordinate */
	struct wlf_svg_length cy;             /**< Center Y coordinate */
	struct wlf_svg_length r;              /**< Radius */
};

/**
 * @brief Ellipse element data.
 */
struct wlf_svg_element_ellipse {
	struct wlf_svg_length cx;             /**< Center X coordinate */
	struct wlf_svg_length cy;             /**< Center Y coordinate */
	struct wlf_svg_length rx;             /**< X radius */
	struct wlf_svg_length ry;             /**< Y radius */
};

/**
 * @brief Line element data.
 */
struct wlf_svg_element_line {
	struct wlf_svg_length x1;             /**< Start X coordinate */
	struct wlf_svg_length y1;             /**< Start Y coordinate */
	struct wlf_svg_length x2;             /**< End X coordinate */
	struct wlf_svg_length y2;             /**< End Y coordinate */
};

/**
 * @brief Polyline/Polygon element data.
 */
struct wlf_svg_element_poly {
	struct wlf_svg_point *points;         /**< Array of points */
	size_t point_count;                   /**< Number of points */
};

/**
 * @brief Path command types.
 */
enum wlf_svg_path_command {
	WLF_SVG_PATH_MOVETO = 'M',
	WLF_SVG_PATH_MOVETO_REL = 'm',
	WLF_SVG_PATH_LINETO = 'L',
	WLF_SVG_PATH_LINETO_REL = 'l',
	WLF_SVG_PATH_HORIZONTAL = 'H',
	WLF_SVG_PATH_HORIZONTAL_REL = 'h',
	WLF_SVG_PATH_VERTICAL = 'V',
	WLF_SVG_PATH_VERTICAL_REL = 'v',
	WLF_SVG_PATH_CURVETO = 'C',
	WLF_SVG_PATH_CURVETO_REL = 'c',
	WLF_SVG_PATH_SMOOTH_CURVETO = 'S',
	WLF_SVG_PATH_SMOOTH_CURVETO_REL = 's',
	WLF_SVG_PATH_QUADRATIC = 'Q',
	WLF_SVG_PATH_QUADRATIC_REL = 'q',
	WLF_SVG_PATH_SMOOTH_QUADRATIC = 'T',
	WLF_SVG_PATH_SMOOTH_QUADRATIC_REL = 't',
	WLF_SVG_PATH_ARC = 'A',
	WLF_SVG_PATH_ARC_REL = 'a',
	WLF_SVG_PATH_CLOSEPATH = 'Z',
	WLF_SVG_PATH_CLOSEPATH_REL = 'z',
};

/**
 * @brief Path segment structure.
 */
struct wlf_svg_path_segment {
	enum wlf_svg_path_command command;
	float *params;                        /**< Parameter array */
	size_t param_count;                   /**< Number of parameters */
};

/**
 * @brief Path element data.
 */
struct wlf_svg_element_path {
	char *d;                              /**< Path data string */
	struct wlf_svg_path_segment *segments; /**< Parsed path segments */
	size_t segment_count;                 /**< Number of segments */
	struct wlf_svg_length path_length;    /**< Total path length */
};

/**
 * @brief Text element data.
 */
struct wlf_svg_element_text {
	struct wlf_svg_length x;              /**< X coordinate */
	struct wlf_svg_length y;              /**< Y coordinate */
	struct wlf_svg_length dx;             /**< X offset */
	struct wlf_svg_length dy;             /**< Y offset */
	char *text_content;                   /**< Text content */
	char *font_family;                    /**< Font family */
	struct wlf_svg_length font_size;      /**< Font size */
	enum {
		WLF_SVG_FONT_STYLE_NORMAL,
		WLF_SVG_FONT_STYLE_ITALIC,
		WLF_SVG_FONT_STYLE_OBLIQUE
	} font_style;
	enum {
		WLF_SVG_FONT_WEIGHT_NORMAL = 400,
		WLF_SVG_FONT_WEIGHT_BOLD = 700,
	} font_weight;
	enum {
		WLF_SVG_TEXT_ANCHOR_START,
		WLF_SVG_TEXT_ANCHOR_MIDDLE,
		WLF_SVG_TEXT_ANCHOR_END
	} text_anchor;
};

/**
 * @brief Linear gradient element data.
 */
struct wlf_svg_element_linear_gradient {
	struct wlf_svg_length x1;             /**< Start X coordinate */
	struct wlf_svg_length y1;             /**< Start Y coordinate */
	struct wlf_svg_length x2;             /**< End X coordinate */
	struct wlf_svg_length y2;             /**< End Y coordinate */
	enum {
		WLF_SVG_GRADIENT_UNITS_OBJECT_BBOX,
		WLF_SVG_GRADIENT_UNITS_USER_SPACE
	} gradient_units;
	enum {
		WLF_SVG_SPREAD_PAD,
		WLF_SVG_SPREAD_REFLECT,
		WLF_SVG_SPREAD_REPEAT
	} spread_method;
	char *href;                           /**< Reference to another gradient */
};

/**
 * @brief Radial gradient element data.
 */
struct wlf_svg_element_radial_gradient {
	struct wlf_svg_length cx;             /**< Center X coordinate */
	struct wlf_svg_length cy;             /**< Center Y coordinate */
	struct wlf_svg_length r;              /**< Radius */
	struct wlf_svg_length fx;             /**< Focal X coordinate */
	struct wlf_svg_length fy;             /**< Focal Y coordinate */
	struct wlf_svg_length fr;             /**< Focal radius */
	enum {
		WLF_SVG_GRADIENT_UNITS_OBJECT_BBOX,
		WLF_SVG_GRADIENT_UNITS_USER_SPACE
	} gradient_units;
	enum {
		WLF_SVG_SPREAD_PAD,
		WLF_SVG_SPREAD_REFLECT,
		WLF_SVG_SPREAD_REPEAT
	} spread_method;
	char *href;                           /**< Reference to another gradient */
};

/**
 * @brief Gradient stop element data.
 */
struct wlf_svg_element_stop {
	struct wlf_svg_length offset;         /**< Stop offset [0-1] */
	struct wlf_svg_color stop_color;      /**< Stop color */
	float stop_opacity;                   /**< Stop opacity [0-1] */
};

/**
 * @brief Image element data.
 */
struct wlf_svg_element_image {
	struct wlf_svg_length x;              /**< X coordinate */
	struct wlf_svg_length y;              /**< Y coordinate */
	struct wlf_svg_length width;          /**< Width */
	struct wlf_svg_length height;         /**< Height */
	char *href;                           /**< Image URL or data URI */
	enum {
		WLF_SVG_ASPECT_RATIO_NONE,
		WLF_SVG_ASPECT_RATIO_XMINYMIN,
		WLF_SVG_ASPECT_RATIO_XMIDYMIN,
		WLF_SVG_ASPECT_RATIO_XMAXYMIN,
		WLF_SVG_ASPECT_RATIO_XMINYMID,
		WLF_SVG_ASPECT_RATIO_XMIDYMID,
		WLF_SVG_ASPECT_RATIO_XMAXYMID,
		WLF_SVG_ASPECT_RATIO_XMINYMAX,
		WLF_SVG_ASPECT_RATIO_XMIDYMAX,
		WLF_SVG_ASPECT_RATIO_XMAXYMAX
	} preserve_aspect_ratio;
};

/**
 * @brief Use element data.
 */
struct wlf_svg_element_use {
	struct wlf_svg_length x;              /**< X coordinate */
	struct wlf_svg_length y;              /**< Y coordinate */
	struct wlf_svg_length width;          /**< Width (optional) */
	struct wlf_svg_length height;         /**< Height (optional) */
	char *href;                           /**< Reference to element */
};

/**
 * @brief Text content node data.
 */
struct wlf_svg_element_text_content {
	char *content;                        /**< Text content */
	size_t length;                        /**< Content length */
};

#endif // SVG_WLF_SVG_ELEMENTS_H
