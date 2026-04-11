/**
 * @file        wlf_svg.h
 * @brief       SVG parser and data structures for wlframe.
 * @details     Provides a lightweight SVG parser that produces a linked-list
 *              representation of shapes and paths. Supports basic SVG elements
 *              (rect, circle, ellipse, line, polyline, polygon, path), fill/stroke
 *              paints (solid colour and linear/radial gradients), and viewBox scaling.
 *
 *              Typical usage:
 *                  - Load an SVG with wlf_svg_parse_from_file() or wlf_svg_parse().
 *                  - Walk image->shapes / shape->paths to inspect geometry.
 *                  - Query summary metadata with wlf_svg_get_info().
 *                  - Free the image with wlf_svg_destroy() when done.
 *
 * @author      YaoBing Xiao
 * @date        2025-06-08
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-06-08, initial version\n
 */

#ifndef SVG_WLF_SVG_H
#define SVG_WLF_SVG_H

#include "wlf/shapes/wlf_shape.h"
#include "wlf/shapes/wlf_path_shape.h"
#include "wlf/types/wlf_gradient.h"

enum wlf_svg_paint_type {
	WLF_SVG_PAINT_UNDEF = -1,
	WLF_SVG_PAINT_NONE = 0,
	WLF_SVG_PAINT_COLOR = 1,
	WLF_SVG_PAINT_LINEAR_GRADIENT = 2,
	WLF_SVG_PAINT_RADIAL_GRADIENT = 3
};

enum wlf_svg_spread_type {
	WLF_SVG_SPREAD_PAD = 0,
	WLF_SVG_SPREAD_REFLECT = 1,
	WLF_SVG_SPREAD_REPEAT = 2
};

enum wlf_svg_line_join {
	WLF_SVG_JOIN_MITER = 0,
	WLF_SVG_JOIN_ROUND = 1,
	WLF_SVG_JOIN_BEVEL = 2
};

enum wlf_svg_line_cap {
	WLF_SVG_CAP_BUTT = 0,
	WLF_SVG_CAP_ROUND = 1,
	WLF_SVG_CAP_SQUARE = 2
};

enum wlf_svg_fill_rule {
	WLF_SVG_FILL_RULE_NONZERO = 0,
	WLF_SVG_FILL_RULE_EVENODD = 1
};

enum wlf_svg_flags {
	WLF_SVG_FLAGS_VISIBLE = 0x01,
	WLF_SVG_FLAGS_FROM_USE = 0x02
};

enum wlf_svg_paint_order {
	WLF_SVG_PAINT_FILL = 0x00,
	WLF_SVG_PAINT_MARKERS = 0x01,
	WLF_SVG_PAINT_STROKE = 0x02,
};


enum wlf_svg_attr_name {
	WLF_SVG_ATTR_UNKNOWN = 0,
	WLF_SVG_ATTR_STYLE,
	WLF_SVG_ATTR_DISPLAY,
	WLF_SVG_ATTR_FILL,
	WLF_SVG_ATTR_OPACITY,
	WLF_SVG_ATTR_FILL_OPACITY,
	WLF_SVG_ATTR_STROKE,
	WLF_SVG_ATTR_STROKE_WIDTH,
	WLF_SVG_ATTR_STROKE_DASHARRAY,
	WLF_SVG_ATTR_STROKE_DASHOFFSET,
	WLF_SVG_ATTR_STROKE_OPACITY,
	WLF_SVG_ATTR_STROKE_LINECAP,
	WLF_SVG_ATTR_STROKE_LINEJOIN,
	WLF_SVG_ATTR_STROKE_MITERLIMIT,
	WLF_SVG_ATTR_FILL_RULE,
	WLF_SVG_ATTR_FONT_SIZE,
	WLF_SVG_ATTR_TRANSFORM,
	WLF_SVG_ATTR_STOP_COLOR,
	WLF_SVG_ATTR_STOP_OPACITY,
	WLF_SVG_ATTR_OFFSET,
	WLF_SVG_ATTR_PAINT_ORDER,
	WLF_SVG_ATTR_ID,
};

enum wlf_svg_element_name {
	WLF_SVG_EL_UNKNOWN = 0,
	WLF_SVG_EL_G,
	WLF_SVG_EL_PATH,
	WLF_SVG_EL_RECT,
	WLF_SVG_EL_CIRCLE,
	WLF_SVG_EL_ELLIPSE,
	WLF_SVG_EL_LINE,
	WLF_SVG_EL_POLYLINE,
	WLF_SVG_EL_POLYGON,
	WLF_SVG_EL_LINEAR_GRADIENT,
	WLF_SVG_EL_RADIAL_GRADIENT,
	WLF_SVG_EL_STOP,
	WLF_SVG_EL_DEFS,
	WLF_SVG_EL_SVG,
	WLF_SVG_EL_SYMBOL,
	WLF_SVG_EL_USE,
	WLF_SVG_EL_TEXT,
};

enum wlf_svg_gradient_attr_name {
	WLF_SVG_GRADIENT_ATTR_UNKNOWN = 0,
	WLF_SVG_GRADIENT_ATTR_ID,
	WLF_SVG_GRADIENT_ATTR_GRADIENT_UNITS,
	WLF_SVG_GRADIENT_ATTR_GRADIENT_TRANSFORM,
	WLF_SVG_GRADIENT_ATTR_CX,
	WLF_SVG_GRADIENT_ATTR_CY,
	WLF_SVG_GRADIENT_ATTR_R,
	WLF_SVG_GRADIENT_ATTR_FX,
	WLF_SVG_GRADIENT_ATTR_FY,
	WLF_SVG_GRADIENT_ATTR_X1,
	WLF_SVG_GRADIENT_ATTR_Y1,
	WLF_SVG_GRADIENT_ATTR_X2,
	WLF_SVG_GRADIENT_ATTR_Y2,
	WLF_SVG_GRADIENT_ATTR_SPREAD_METHOD,
	WLF_SVG_GRADIENT_ATTR_XLINK_HREF,
};

enum wlf_svg_root_attr_name {
	WLF_SVG_ROOT_ATTR_UNKNOWN = 0,
	WLF_SVG_ROOT_ATTR_WIDTH,
	WLF_SVG_ROOT_ATTR_HEIGHT,
	WLF_SVG_ROOT_ATTR_VIEWBOX,
	WLF_SVG_ROOT_ATTR_PRESERVE_ASPECT_RATIO,
};

enum wlf_svg_gradient_units {
	WLF_SVG_USER_SPACE   = 0, /**< Gradient coords in user (document) space. */
	WLF_SVG_OBJECT_SPACE = 1, /**< Gradient coords relative to the object bounding box. */
};

enum wlf_svg_units {
	WLF_SVG_UNITS_USER,
	WLF_SVG_UNITS_PX,
	WLF_SVG_UNITS_PT,
	WLF_SVG_UNITS_PC,
	WLF_SVG_UNITS_MM,
	WLF_SVG_UNITS_CM,
	WLF_SVG_UNITS_IN,
	WLF_SVG_UNITS_PERCENT,
	WLF_SVG_UNITS_EM,
	WLF_SVG_UNITS_EX,
};

struct wlf_svg_named_color {
	const char* name;
	unsigned int color;
};

struct wlf_svg_name_map {
	const char* name;
	int value;
};

struct wlf_svg_char_name_map {
	const char* name;
	char value;
};

struct wlf_svg_uchar_name_map {
	const char* name;
	unsigned char value;
};

struct wlf_svg_gradient_stop {
	unsigned int color;
	float offset;
};

struct wlf_svg_shape {
	struct wlf_shape base;         /**< Base shape for SVG-specific payloads. */
	char id[64];                   /**< Optional 'id' attribute of the shape or its group. */
	struct wlf_shape *geometry;    /**< Geometry payload (from wlf/shapes). */
	struct wlf_path *paths;           /**< Linked list of paths making up this shape. */
	struct wlf_gradient *fill;     /**< Fill paint (NULL = none). */
	struct wlf_gradient *stroke;   /**< Stroke paint (NULL = none). */
	float opacity;                 /**< Shape opacity [0..1]. */
	float fill_opacity;            /**< Fill opacity [0..1]. */
	float stroke_opacity;          /**< Stroke opacity [0..1]. */
	float stroke_width;            /**< Stroke width, scaled to pixel units. */
	float stroke_dash_offset;      /**< Stroke dash offset, scaled to pixel units. */
	float stroke_dash_array[8];    /**< Stroke dash array values, scaled to pixel units. */
	char stroke_dash_count;        /**< Number of valid entries in stroke_dash_array. */
	char stroke_line_join;         /**< Stroke join type (see wlf_svg_line_join). */
	char stroke_line_cap;          /**< Stroke cap type (see wlf_svg_line_cap). */
	float miter_limit;             /**< Miter limit for miter joins. */
	char fill_rule;                /**< Fill rule (see wlf_svg_fill_rule). */
	unsigned char paint_order;     /**< Encoded paint order: 3×2-bit fields (fill/stroke/markers). */
	unsigned char flags;           /**< Logical OR of wlf_svg_flags values. */
	float bounds[4];               /**< Tight bounding box [minx, miny, maxx, maxy]. */
	char fill_gradient[64];        /**< Optional 'id' of the fill gradient definition. */
	char stroke_gradient[64];      /**< Optional 'id' of the stroke gradient definition. */
	float xform[6];                /**< Root transform matrix for fill/stroke gradient. */
	struct wlf_svg_shape *next;    /**< Next shape in the linked list, or NULL. */
};

struct wlf_svg_use_data {
	char href[64];
	float x;
	float y;
	struct wlf_svg_use_data *next;
};

struct wlf_svg_image {
	float width;                   /**< SVG canvas width in pixel units. */
	float height;                  /**< SVG canvas height in pixel units. */
	char xml_version[16];          /**< XML version string (e.g. "1.0"). */
	char xml_encoding[16];         /**< XML encoding string (e.g. "UTF-8"). */
	char xmlns[128];               /**< SVG namespace URI from root element. */
	char xmlns_xlink[128];         /**< xlink namespace URI from root element. */
	char width_attr[64];           /**< Raw width attribute text from root element. */
	char height_attr[64];          /**< Raw height attribute text from root element. */
	char view_box[128];            /**< Raw viewBox attribute text from root element. */
	char preserve_aspect_ratio[64];/**< Raw preserveAspectRatio text from root element. */
	struct wlf_shape *shapes;      /**< Linked list of shapes parsed from the SVG. */
	struct wlf_svg_symbol_data *symbols; /**< Parsed <symbol> definitions. */
	struct wlf_svg_use_data *uses; /**< Parsed <use> elements. */
};

/**
 * @brief Summary information extracted from a parsed SVG image.
 */
struct wlf_svg_info {
	float width;     /**< SVG canvas width in pixel units. */
	float height;    /**< SVG canvas height in pixel units. */
	int n_shapes;  /**< Total number of shapes in the image. */
	int n_paths;   /**< Total number of paths across all shapes. */
	float bounds[4]; /**< Overall geometry bounding box [minx, miny, maxx, maxy]. */
};

/**
 * @brief Checks if a shape is an SVG shape.
 * @param shape Shape to test.
 * @return true if shape is an SVG shape, false otherwise.
 */
bool wlf_shape_is_svg(struct wlf_shape *shape);

/**
 * @brief Casts a base shape to an SVG shape.
 * @param shape Base shape pointer.
 * @return SVG shape pointer (asserts if not an SVG shape).
 */
struct wlf_svg_shape *wlf_svg_shape_from_shape(struct wlf_shape *shape);

/* -------------------------------------------------------------------------
 * Internal parser types
 * These types represent intermediate state used by the SVG parser and are
 * exposed here so that translation units can see the full definitions without
 * requiring a separate private header.
 * ---------------------------------------------------------------------- */

/** Maximum nesting depth of the attribute stack. */
#define WLF_SVG_MAX_ATTR   128
/** Maximum number of entries in a stroke dash array. */
#define WLF_SVG_MAX_DASHES 8

/** A length / coordinate value together with its unit. */
struct wlf_svg_coordinate {
	float value; /**< Numeric magnitude. */
	int units; /**< Unit type (enum wlf_svg_units). */
};

/** Endpoint coordinates for a linear gradient. */
struct wlf_svg_linear_data {
	struct wlf_svg_coordinate x1, y1, x2, y2;
};

/** Center / radius / focal-point coordinates for a radial gradient. */
struct wlf_svg_radial_data {
	struct wlf_svg_coordinate cx, cy, r, fx, fy;
};

/**
 * @brief Unresolved gradient definition collected during parsing.
 *
 * After the document is fully parsed, each shape's fill/stroke gradient
 * reference is resolved into a wlf_gradient (the public type) by
 * wlf_svg_create_gradients().
 */
struct wlf_svg_gradient_data {
	char id[64];                   /**< Id attribute of the gradient element. */
	char ref[64];                  /**< xlink:href reference to another gradient definition. */
	signed char type;              /**< Gradient type (wlf_svg_paint_type). */
	union {
		struct wlf_svg_linear_data linear;
		struct wlf_svg_radial_data radial;
	};
	char                           spread; /**< Spread method (wlf_svg_spread_type). */
	char                           units;  /**< Coordinate space (wlf_svg_gradient_units). */
	float                          xform[6];
	int                            nstops;
	struct wlf_svg_gradient_stop  *stops;
	struct wlf_svg_gradient_data  *next;
};

/**
 * @brief Accumulated style / attribute state for the current element.
 *
 * The parser maintains a stack of these (depth WLF_SVG_MAX_ATTR) so that
 * nested groups can inherit and override presentation attributes.
 */
struct wlf_svg_attrib {
	char         id[64];
	float        xform[6];
	unsigned int fillColor;
	unsigned int strokeColor;
	float        opacity;
	float        fillOpacity;
	float        strokeOpacity;
	char         fillGradient[64];
	char         strokeGradient[64];
	float        strokeWidth;
	float        strokeDashOffset;
	float        strokeDashArray[WLF_SVG_MAX_DASHES];
	int          strokeDashCount;
	char         strokeLineJoin;
	char         strokeLineCap;
	float        miterLimit;
	char         fillRule;
	float        fontSize;
	unsigned int stopColor;
	float        stopOpacity;
	float        stopOffset;
	char         hasFill;
	char         hasStroke;
	char         visible;
	unsigned char paintOrder;
};

/** Shapes collected while parsing a \<symbol\> element. */
struct wlf_svg_symbol_data {
	char                       id[64];
	struct wlf_shape          *shapes;      /**< Head of the shape linked list. */
	struct wlf_shape          *shapes_tail; /**< Tail pointer for O(1) append. */
	struct wlf_svg_symbol_data *next;       /**< Next symbol definition. */
};

/** Full parser state. Allocated and freed by wlf_svg_parse_from_file() / wlf_svg_parse(). */
struct wlf_svg_parser {
	struct wlf_svg_attrib              attr[WLF_SVG_MAX_ATTR];
	int                         attrHead;
	float                      *pts;
	int                         npts;
	int                         cpts;
	struct wlf_path *plist;
	struct wlf_svg_image       *image;
	struct wlf_svg_gradient_data *gradients;
	struct wlf_shape           *shapesTail;
	struct wlf_shape           *shape_geometry;
	struct wlf_svg_symbol_data *symbols;        /**< All parsed \<symbol\> definitions. */
	struct wlf_svg_symbol_data *current_symbol; /**< Non-NULL while inside \<symbol\>. */
	struct wlf_svg_use_data    *uses;           /**< All parsed \<use\> elements. */
	float viewMinx, viewMiny, viewWidth, viewHeight;
	int   alignX, alignY, alignType;
	float dpi;
	char  pathFlag;
	char  defsFlag;
	char  textFlag;
	float textX;
	float textY;
	float textFontSize;
	int   textAnchor;
	char  textFontFamily[64];
	char  textContent[256];
};

/**
 * @brief Parse an SVG file into wlframe SVG image data.
 * @param filename SVG file path.
 * @param units Target units (`px`, `pt`, `pc`, `mm`, `cm`, `in`).
 * @param dpi Dots per inch used for unit conversion.
 * @return Parsed image, or NULL on failure.
 */
struct wlf_svg_image *wlf_svg_parse_from_file(const char *filename,
	const char *units, float dpi);

/**
 * @brief Parse an SVG buffer into wlframe SVG image data.
 * @param input Null-terminated SVG text buffer. This function modifies the buffer.
 * @param units Target units (`px`, `pt`, `pc`, `mm`, `cm`, `in`).
 * @param dpi Dots per inch used for unit conversion.
 * @return Parsed image, or NULL on failure.
 */
struct wlf_svg_image *wlf_svg_parse(char *input, const char *units, float dpi);

/**
 * @brief Save a parsed SVG image back to an SVG file.
 * @param image Parsed SVG image.
 * @param filename Output file path.
 * @return true on success, false on failure.
 */
bool wlf_svg_save(const struct wlf_svg_image *image, const char *filename);

/**
 * @brief Duplicate a wlframe SVG path.
 * @param path Source path.
 * @return Duplicated path, or NULL on failure.
 */
struct wlf_path *wlf_svg_path_duplicate(struct wlf_path *path);

/**
 * @brief Destroy a wlframe SVG image.
 * @param image Parsed image to destroy.
 */
void wlf_svg_destroy(struct wlf_svg_image *image);

/**
 * @brief Fill a wlf_svg_info with summary metadata from a parsed SVG image.
 * @param image Parsed SVG image. Must not be NULL.
 * @param info  Output info structure to populate. Must not be NULL.
 */
void wlf_svg_get_info(const struct wlf_svg_image *image, struct wlf_svg_info *info);

/**
 * @brief Parse a named SVG color into packed RGBA value.
 * @param str Color name string (for example: "red", "steelblue").
 * @return Packed color value. Returns 0 when name is unknown.
 */
unsigned int wlf_svg_parse_color_name(const char* str);

#endif // SVG_WLF_SVG_H
