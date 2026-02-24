#ifndef SVG_WLF_SVG_H
#define SVG_WLF_SVG_H

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
	WLF_SVG_FLAGS_VISIBLE = 0x01
};

enum wlf_svg_paint_order {
	WLF_SVG_PAINT_FILL = 0x00,
	WLF_SVG_PAINT_MARKERS = 0x01,
	WLF_SVG_PAINT_STROKE = 0x02,
};

struct wlf_svg_gradient_stop {
	unsigned int color;
	float offset;
};

struct wlf_svg_gradient {
	float xform[6];
	char spread;
	float fx, fy;
	int nstops;
	struct wlf_svg_gradient_stop stops[1];
};

struct wlf_svg_paint {
	signed char type;
	union {
		unsigned int color;
		struct wlf_svg_gradient *gradient;
	};
};

struct wlf_svg_path
{
	float* pts;					// Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
	int npts;					// Total number of bezier points.
	char closed;				// Flag indicating if shapes should be treated as closed.
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	struct wlf_svg_path* next;		// Pointer to next path, or NULL if last element.
};

struct wlf_svg_shape
{
	char id[64];				// Optional 'id' attr of the shape or its group
	struct wlf_svg_paint fill;			// Fill paint
	struct wlf_svg_paint stroke;			// Stroke paint
	float opacity;				// Opacity of the shape.
	float strokeWidth;			// Stroke width (scaled).
	float strokeDashOffset;		// Stroke dash offset (scaled).
	float strokeDashArray[8];	// Stroke dash array (scaled).
	char strokeDashCount;		// Number of dash values in dash array.
	char strokeLineJoin;		// Stroke join type.
	char strokeLineCap;			// Stroke cap type.
	float miterLimit;			// Miter limit
	char fillRule;				// Fill rule, see NSVGfillRule.
    unsigned char paintOrder;	// Encoded paint order (3×2-bit fields) see NSVGpaintOrder
	unsigned char flags;		// Logical or of NSVG_FLAGS_* flags
	float bounds[4];			// Tight bounding box of the shape [minx,miny,maxx,maxy].
	char fillGradient[64];		// Optional 'id' of fill gradient
	char strokeGradient[64];	// Optional 'id' of stroke gradient
	float xform[6];				// Root transformation for fill/stroke gradient
	struct wlf_svg_path* paths;			// Linked list of paths in the image.
	struct wlf_svg_shape* next;		// Pointer to next shape, or NULL if last element.
};

struct wlf_svg_image
{
	float width;				// Width of the image.
	float height;				// Height of the image.
	struct wlf_svg_shape* shapes;			// Linked list of shapes in the image.
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
 * @brief Duplicate a wlframe SVG path.
 * @param path Source path.
 * @return Duplicated path, or NULL on failure.
 */
struct wlf_svg_path *wlf_svg_path_duplicate(struct wlf_svg_path *path);

/**
 * @brief Destroy a wlframe SVG image.
 * @param image Parsed image to destroy.
 */
void wlf_svg_destroy(struct wlf_svg_image *image);

#endif // SVG_WLF_SVG_H
