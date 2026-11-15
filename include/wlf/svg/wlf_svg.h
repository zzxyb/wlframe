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
#include "wlf/effect/wlf_filter.h"
#include "wlf/types/wlf_gradient.h"

/**
 * @brief Kinds of SVG paint supported by the parser.
 */
enum wlf_svg_paint_type {
	WLF_SVG_PAINT_UNDEF = -1, /**< Paint has not been specified. */
	WLF_SVG_PAINT_NONE = 0, /**< No paint. */
	WLF_SVG_PAINT_COLOR = 1, /**< Solid color paint. */
	WLF_SVG_PAINT_LINEAR_GRADIENT = 2, /**< Linear gradient paint. */
	WLF_SVG_PAINT_RADIAL_GRADIENT = 3 /**< Radial gradient paint. */
};

/**
 * @brief Gradient behavior outside the defined stop range.
 */
enum wlf_svg_spread_type {
	WLF_SVG_SPREAD_PAD = 0, /**< Extend the endpoint stop colors. */
	WLF_SVG_SPREAD_REFLECT = 1, /**< Mirror the gradient on alternate repetitions. */
	WLF_SVG_SPREAD_REPEAT = 2 /**< Repeat the gradient. */
};

/**
 * @brief Styles used to join adjacent stroke segments.
 */
enum wlf_svg_line_join {
	WLF_SVG_JOIN_MITER = 0, /**< Stroke join: miter. */
	WLF_SVG_JOIN_ROUND = 1, /**< Stroke join: round. */
	WLF_SVG_JOIN_BEVEL = 2 /**< Stroke join: bevel. */
};

/**
 * @brief Styles used at the ends of open stroked paths.
 */
enum wlf_svg_line_cap {
	WLF_SVG_CAP_BUTT = 0, /**< Stroke endpoint cap: butt. */
	WLF_SVG_CAP_ROUND = 1, /**< Stroke endpoint cap: round. */
	WLF_SVG_CAP_SQUARE = 2 /**< Stroke endpoint cap: square. */
};

/**
 * @brief Rules for determining the interior of a path.
 */
enum wlf_svg_fill_rule {
	WLF_SVG_FILL_RULE_NONZERO = 0, /**< Use the nonzero winding rule. */
	WLF_SVG_FILL_RULE_EVENODD = 1 /**< Use the even-odd crossing rule. */
};

/**
 * @brief Visibility and origin flags for parsed SVG shapes.
 */
enum wlf_svg_flags {
	WLF_SVG_FLAGS_VISIBLE = 0x01, /**< Shape participates in rendering. */
	WLF_SVG_FLAGS_FROM_USE = 0x02 /**< Shape was instantiated from a use reference. */
};

/**
 * @brief Paint operation identifiers encoded in an SVG paint order.
 */
enum wlf_svg_paint_order {
	WLF_SVG_PAINT_FILL = 0x00, /**< Paint operation: fill. */
	WLF_SVG_PAINT_MARKERS = 0x01, /**< Paint operation: markers. */
	WLF_SVG_PAINT_STROKE = 0x02, /**< Paint operation: stroke. */
};


/**
 * @brief SVG presentation attribute identifiers recognized by the parser.
 */
enum wlf_svg_attr_name {
	WLF_SVG_ATTR_UNKNOWN = 0, /**< SVG presentation attribute: unknown. */
	WLF_SVG_ATTR_STYLE, /**< SVG presentation attribute: style. */
	WLF_SVG_ATTR_DISPLAY, /**< SVG presentation attribute: display. */
	WLF_SVG_ATTR_FILL, /**< SVG presentation attribute: fill. */
	WLF_SVG_ATTR_OPACITY, /**< SVG presentation attribute: opacity. */
	WLF_SVG_ATTR_FILL_OPACITY, /**< SVG presentation attribute: fill-opacity. */
	WLF_SVG_ATTR_STROKE, /**< SVG presentation attribute: stroke. */
	WLF_SVG_ATTR_STROKE_WIDTH, /**< SVG presentation attribute: stroke-width. */
	WLF_SVG_ATTR_STROKE_DASHARRAY, /**< SVG presentation attribute: stroke-dasharray. */
	WLF_SVG_ATTR_STROKE_DASHOFFSET, /**< SVG presentation attribute: stroke-dashoffset. */
	WLF_SVG_ATTR_STROKE_OPACITY, /**< SVG presentation attribute: stroke-opacity. */
	WLF_SVG_ATTR_STROKE_LINECAP, /**< SVG presentation attribute: stroke-linecap. */
	WLF_SVG_ATTR_STROKE_LINEJOIN, /**< SVG presentation attribute: stroke-linejoin. */
	WLF_SVG_ATTR_STROKE_MITERLIMIT, /**< SVG presentation attribute: stroke-miterlimit. */
	WLF_SVG_ATTR_FILL_RULE, /**< SVG presentation attribute: fill-rule. */
	WLF_SVG_ATTR_FONT_SIZE, /**< SVG presentation attribute: font-size. */
	WLF_SVG_ATTR_TRANSFORM, /**< SVG presentation attribute: transform. */
	WLF_SVG_ATTR_STOP_COLOR, /**< SVG presentation attribute: stop-color. */
	WLF_SVG_ATTR_STOP_OPACITY, /**< SVG presentation attribute: stop-opacity. */
	WLF_SVG_ATTR_OFFSET, /**< SVG presentation attribute: offset. */
	WLF_SVG_ATTR_PAINT_ORDER, /**< SVG presentation attribute: paint-order. */
	WLF_SVG_ATTR_FILTER, /**< SVG presentation attribute: filter. */
	WLF_SVG_ATTR_ID, /**< SVG presentation attribute: id. */
};

/**
 * @brief SVG element identifiers recognized by the parser.
 */
enum wlf_svg_element_name {
	WLF_SVG_EL_UNKNOWN = 0, /**< SVG element: unknown. */
	WLF_SVG_EL_G, /**< SVG group element (g). */
	WLF_SVG_EL_PATH, /**< SVG element: path. */
	WLF_SVG_EL_RECT, /**< SVG element: rect. */
	WLF_SVG_EL_CIRCLE, /**< SVG element: circle. */
	WLF_SVG_EL_ELLIPSE, /**< SVG element: ellipse. */
	WLF_SVG_EL_LINE, /**< SVG element: line. */
	WLF_SVG_EL_POLYLINE, /**< SVG element: polyline. */
	WLF_SVG_EL_POLYGON, /**< SVG element: polygon. */
	WLF_SVG_EL_LINEAR_GRADIENT, /**< SVG element: linearGradient. */
	WLF_SVG_EL_RADIAL_GRADIENT, /**< SVG element: radialGradient. */
	WLF_SVG_EL_STOP, /**< SVG element: stop. */
	WLF_SVG_EL_DEFS, /**< SVG element: defs. */
	WLF_SVG_EL_SVG, /**< SVG element: svg. */
	WLF_SVG_EL_SYMBOL, /**< SVG element: symbol. */
	WLF_SVG_EL_USE, /**< SVG element: use. */
	WLF_SVG_EL_TEXT, /**< SVG element: text. */
	WLF_SVG_EL_FILTER, /**< SVG element: filter. */
	WLF_SVG_EL_GAUSSIAN_BLUR, /**< SVG element: feGaussianBlur. */
	WLF_SVG_EL_DROP_SHADOW, /**< SVG element: feDropShadow. */
};

/**
 * @brief SVG gradient attribute identifiers recognized by the parser.
 */
enum wlf_svg_gradient_attr_name {
	WLF_SVG_GRADIENT_ATTR_UNKNOWN = 0, /**< SVG gradient attribute: unknown. */
	WLF_SVG_GRADIENT_ATTR_ID, /**< SVG gradient attribute: id. */
	WLF_SVG_GRADIENT_ATTR_GRADIENT_UNITS, /**< SVG gradient attribute: gradientUnits. */
	WLF_SVG_GRADIENT_ATTR_GRADIENT_TRANSFORM, /**< SVG gradient attribute: gradientTransform. */
	WLF_SVG_GRADIENT_ATTR_CX, /**< SVG gradient attribute: cx. */
	WLF_SVG_GRADIENT_ATTR_CY, /**< SVG gradient attribute: cy. */
	WLF_SVG_GRADIENT_ATTR_R, /**< SVG gradient attribute: r. */
	WLF_SVG_GRADIENT_ATTR_FX, /**< SVG gradient attribute: fx. */
	WLF_SVG_GRADIENT_ATTR_FY, /**< SVG gradient attribute: fy. */
	WLF_SVG_GRADIENT_ATTR_X1, /**< SVG gradient attribute: x1. */
	WLF_SVG_GRADIENT_ATTR_Y1, /**< SVG gradient attribute: y1. */
	WLF_SVG_GRADIENT_ATTR_X2, /**< SVG gradient attribute: x2. */
	WLF_SVG_GRADIENT_ATTR_Y2, /**< SVG gradient attribute: y2. */
	WLF_SVG_GRADIENT_ATTR_SPREAD_METHOD, /**< SVG gradient attribute: spreadMethod. */
	WLF_SVG_GRADIENT_ATTR_XLINK_HREF, /**< SVG gradient attribute: xlink:href. */
};

/**
 * @brief Root SVG attribute identifiers recognized by the parser.
 */
enum wlf_svg_root_attr_name {
	WLF_SVG_ROOT_ATTR_UNKNOWN = 0, /**< Root SVG attribute: unknown. */
	WLF_SVG_ROOT_ATTR_WIDTH, /**< Root SVG attribute: width. */
	WLF_SVG_ROOT_ATTR_HEIGHT, /**< Root SVG attribute: height. */
	WLF_SVG_ROOT_ATTR_VIEWBOX, /**< Root SVG attribute: viewBox. */
	WLF_SVG_ROOT_ATTR_PRESERVE_ASPECT_RATIO, /**< Root SVG attribute: preserveAspectRatio. */
};

/**
 * @brief Coordinate spaces used by SVG gradients.
 */
enum wlf_svg_gradient_units {
	WLF_SVG_USER_SPACE   = 0, /**< Gradient coords in user (document) space. */
	WLF_SVG_OBJECT_SPACE = 1, /**< Gradient coords relative to the object bounding box. */
};

/**
 * @brief Units associated with parsed SVG lengths.
 */
enum wlf_svg_units {
	WLF_SVG_UNITS_USER, /**< Unitless user coordinates. */
	WLF_SVG_UNITS_PX, /**< CSS pixels. */
	WLF_SVG_UNITS_PT, /**< Points (1/72 inch). */
	WLF_SVG_UNITS_PC, /**< Picas (12 points). */
	WLF_SVG_UNITS_MM, /**< Millimeters. */
	WLF_SVG_UNITS_CM, /**< Centimeters. */
	WLF_SVG_UNITS_IN, /**< Inches. */
	WLF_SVG_UNITS_PERCENT, /**< Percentage of the applicable reference dimension. */
	WLF_SVG_UNITS_EM, /**< Current font size. */
	WLF_SVG_UNITS_EX, /**< Current font x-height. */
};

/**
 * @brief Mapping from an SVG color name to its packed color.
 */
struct wlf_svg_named_color {
	const char* name; /**< Name used for lookup. */
	unsigned int color; /**< Packed SVG color value. */
};

/**
 * @brief Mapping from an SVG name to an integer identifier.
 */
struct wlf_svg_name_map {
	const char* name; /**< Name used for lookup. */
	int value; /**< Value associated with the lookup name. */
};

/**
 * @brief Mapping from an SVG name to a signed character value.
 */
struct wlf_svg_char_name_map {
	const char* name; /**< Name used for lookup. */
	char value; /**< Value associated with the lookup name. */
};

/**
 * @brief Mapping from an SVG name to an unsigned character value.
 */
struct wlf_svg_uchar_name_map {
	const char* name; /**< Name used for lookup. */
	unsigned char value; /**< Value associated with the lookup name. */
};

/**
 * @brief Color and offset of an SVG gradient stop.
 */
struct wlf_svg_gradient_stop {
	unsigned int color; /**< Packed SVG color value. */
	float offset; /**< Gradient stop position. */
};

/**
 * @brief Parsed SVG shape with geometry and paint attributes.
 */
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
	char filter_id[64];            /**< Optional 'id' of the referenced filter definition. */
	struct wlf_filter *filter;     /**< Resolved non-owning filter reference. */
	float xform[6];                /**< Root transform matrix for fill/stroke gradient. */
	struct wlf_svg_shape *next;    /**< Next shape in the linked list, or NULL. */
};

/**
 * @brief Unresolved SVG use reference and position.
 */
struct wlf_svg_use_data {
	char href[64]; /**< Identifier referenced by the SVG use element. */
	float x; /**< X coordinate in SVG user space. */
	float y; /**< Y coordinate in SVG user space. */
	struct wlf_svg_use_data *next; /**< Next definition in the linked list, or NULL. */
};

/**
 * @brief Parsed SVG document and its owned definitions.
 */
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
	struct wlf_svg_symbol_data *symbols; /**< Parsed &lt;symbol&gt; definitions. */
	struct wlf_svg_use_data *uses; /**< Parsed &lt;use&gt; elements. */
	struct wlf_filter *filters;    /**< Owned linked list of parsed filter definitions. */
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

/**
 * @brief A length / coordinate value together with its unit.
 */
struct wlf_svg_coordinate {
	float value; /**< Numeric magnitude. */
	int units; /**< Unit type (enum wlf_svg_units). */
};

/**
 * @brief Endpoint coordinates for a linear gradient.
 */
struct wlf_svg_linear_data {
	struct wlf_svg_coordinate x1, /**< First endpoint X coordinate and unit. */ y1, /**< First endpoint Y coordinate and unit. */ x2, /**< Second endpoint X coordinate and unit. */ y2; /**< Second endpoint Y coordinate and unit. */
};

/**
 * @brief Center / radius / focal-point coordinates for a radial gradient.
 */
struct wlf_svg_radial_data {
	struct wlf_svg_coordinate cx, /**< Gradient center X coordinate and unit. */ cy, /**< Gradient center Y coordinate and unit. */ r, /**< Gradient radius and unit. */ fx, /**< Gradient focal point X coordinate and unit. */ fy; /**< Gradient focal point Y coordinate and unit. */
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
		struct wlf_svg_linear_data linear; /**< Linear gradient endpoint coordinates. */
		struct wlf_svg_radial_data radial; /**< Radial gradient coordinates. */
	};
	char                           spread; /**< Spread method (wlf_svg_spread_type). */
	char                           units;  /**< Coordinate space (wlf_svg_gradient_units). */
	float                          xform[6]; /**< Affine transform coefficients [a, b, c, d, e, f]. */
	int                            nstops; /**< Number of gradient stops. */
	struct wlf_svg_gradient_stop  *stops; /**< Array of gradient stops. */
	struct wlf_svg_gradient_data  *next; /**< Next definition in the linked list, or NULL. */
};

/**
 * @brief Accumulated style / attribute state for the current element.
 *
 * The parser maintains a stack of these (depth WLF_SVG_MAX_ATTR) so that
 * nested groups can inherit and override presentation attributes.
 */
struct wlf_svg_attrib {
	char         id[64]; /**< SVG element identifier. */
	float        xform[6]; /**< Affine transform coefficients [a, b, c, d, e, f]. */
	unsigned int fillColor; /**< Packed fill color. */
	unsigned int strokeColor; /**< Packed stroke color. */
	float        opacity; /**< Overall opacity in the range [0, 1]. */
	float        fillOpacity; /**< Fill opacity in the range [0, 1]. */
	float        strokeOpacity; /**< Stroke opacity in the range [0, 1]. */
	char         fillGradient[64]; /**< Identifier of the referenced fill gradient. */
	char         strokeGradient[64]; /**< Identifier of the referenced stroke gradient. */
	char         filter[64]; /**< Identifier of the referenced SVG filter. */
	float        strokeWidth; /**< Stroke width in the current SVG coordinate space. */
	float        strokeDashOffset; /**< Offset into the stroke dash pattern. */
	float        strokeDashArray[WLF_SVG_MAX_DASHES]; /**< Stroke dash and gap lengths. */
	int          strokeDashCount; /**< Number of valid stroke dash entries. */
	char         strokeLineJoin; /**< Stroke join style from enum wlf_svg_line_join. */
	char         strokeLineCap; /**< Stroke cap style from enum wlf_svg_line_cap. */
	float        miterLimit; /**< Limit on the length of miter joins. */
	char         fillRule; /**< Interior fill rule from enum wlf_svg_fill_rule. */
	float        fontSize; /**< Inherited SVG font size. */
	unsigned int stopColor; /**< Packed gradient stop color. */
	float        stopOpacity; /**< Gradient stop opacity in the range [0, 1]. */
	float        stopOffset; /**< Gradient stop position. */
	char         hasFill; /**< Whether a fill paint has been specified. */
	char         hasStroke; /**< Whether a stroke paint has been specified. */
	char         visible; /**< Whether the current SVG element is visible. */
	unsigned char paintOrder; /**< Encoded ordering of fill, stroke and marker painting. */
};

/**
 * @brief Shapes collected while parsing a symbol element.
 */
struct wlf_svg_symbol_data {
	char                       id[64]; /**< SVG element identifier. */
	struct wlf_shape          *shapes;      /**< Head of the shape linked list. */
	struct wlf_shape          *shapes_tail; /**< Tail pointer for O(1) append. */
	struct wlf_svg_symbol_data *next;       /**< Next symbol definition. */
};

/**
 * @brief Full parser state. Allocated and freed by wlf_svg_parse_from_file() / wlf_svg_parse().
 */
struct wlf_svg_parser {
	struct wlf_svg_attrib              attr[WLF_SVG_MAX_ATTR]; /**< Stack of inherited SVG presentation attributes. */
	int                         attrHead; /**< Index of the active attribute stack entry. */
	float                      *pts; /**< Temporary path coordinate storage. */
	int                         npts; /**< Number of points in temporary path storage. */
	int                         cpts; /**< Allocated point capacity. */
	struct wlf_path *plist; /**< Paths collected for the current SVG shape. */
	struct wlf_svg_image       *image; /**< SVG image owned by this object. */
	struct wlf_svg_gradient_data *gradients; /**< Unresolved gradient definitions. */
	struct wlf_shape           *shapesTail; /**< Tail of the parsed shape list. */
	struct wlf_shape           *shape_geometry; /**< Geometry for the SVG shape currently being parsed. */
	struct wlf_svg_symbol_data *symbols;        /**< All parsed \<symbol\> definitions. */
	struct wlf_svg_symbol_data *current_symbol; /**< Non-NULL while inside \<symbol\>. */
	struct wlf_svg_use_data    *uses;           /**< All parsed \<use\> elements. */
	struct wlf_filter          *filters_tail;   /**< Tail of image->filters. */
	struct wlf_filter          *current_filter; /**< Filter currently collecting primitives. */
	float viewMinx, /**< Minimum viewBox X coordinate. */ viewMiny, /**< Minimum viewBox Y coordinate. */ viewWidth, /**< ViewBox width. */ viewHeight; /**< ViewBox height. */
	int   alignX, /**< Horizontal preserveAspectRatio alignment. */ alignY, /**< Vertical preserveAspectRatio alignment. */ alignType; /**< PreserveAspectRatio meet or slice policy. */
	float dpi; /**< Resolution used to convert physical SVG units. */
	char  pathFlag; /**< Whether path element parsing is active. */
	char  defsFlag; /**< Whether definition-only SVG content is being parsed. */
	char  textFlag; /**< Whether a text element is being parsed. */
	float textX; /**< Current text X coordinate. */
	float textY; /**< Current text Y coordinate. */
	float textFontSize; /**< Current text font size. */
	int   textAnchor; /**< Current text alignment anchor. */
	char  textFontFamily[64]; /**< Current text font family. */
	char  textContent[256]; /**< Accumulated text element content. */
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
