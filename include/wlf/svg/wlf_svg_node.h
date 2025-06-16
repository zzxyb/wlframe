/**
 * @file        wlf_svg_node.h
 * @brief       SVG DOM node abstraction for wlframe.
 * @details     This file defines the SVG DOM node system, providing a complete
 *              abstraction of SVG elements and their properties. Each SVG element
 *              type has its own structure and implementation interface, allowing
 *              different backends to handle rendering, manipulation, and serialization.
 *
 *              The design follows the SVG specification and provides:
 *                  - Complete SVG element hierarchy
 *                  - Attribute management system
 *                  - Transform and styling support
 *                  - Backend-agnostic implementation interface
 *
 * @author      YaoBing Xiao
 * @date        2025-01-17
 * @version     v1.0
 * @par Copyright:
 * @par History:
 *      version: v1.0, YaoBing Xiao, 2025-01-17, initial version\n
 */

#ifndef SVG_WLF_SVG_NODE_H
#define SVG_WLF_SVG_NODE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief SVG node types based on SVG specification.
 */
enum wlf_svg_node_type {
	WLF_SVG_NODE_UNKNOWN = 0,

	/* Structure elements */
	WLF_SVG_NODE_SVG,          /**< Root <svg> element */
	WLF_SVG_NODE_G,            /**< Group <g> element */
	WLF_SVG_NODE_DEFS,         /**< Definitions <defs> element */
	WLF_SVG_NODE_USE,          /**< Use <use> element */
	WLF_SVG_NODE_SYMBOL,       /**< Symbol <symbol> element */
	WLF_SVG_NODE_MARKER,       /**< Marker <marker> element */

	/* Shape elements */
	WLF_SVG_NODE_RECT,         /**< Rectangle <rect> element */
	WLF_SVG_NODE_CIRCLE,       /**< Circle <circle> element */
	WLF_SVG_NODE_ELLIPSE,      /**< Ellipse <ellipse> element */
	WLF_SVG_NODE_LINE,         /**< Line <line> element */
	WLF_SVG_NODE_POLYLINE,     /**< Polyline <polyline> element */
	WLF_SVG_NODE_POLYGON,      /**< Polygon <polygon> element */
	WLF_SVG_NODE_PATH,         /**< Path <path> element */

	/* Text elements */
	WLF_SVG_NODE_TEXT,         /**< Text <text> element */
	WLF_SVG_NODE_TSPAN,        /**< Text span <tspan> element */
	WLF_SVG_NODE_TEXTPATH,     /**< Text path <textPath> element */

	/* Paint server elements */
	WLF_SVG_NODE_LINEAR_GRADIENT,  /**< Linear gradient <linearGradient> */
	WLF_SVG_NODE_RADIAL_GRADIENT,  /**< Radial gradient <radialGradient> */
	WLF_SVG_NODE_PATTERN,          /**< Pattern <pattern> element */
	WLF_SVG_NODE_STOP,             /**< Gradient stop <stop> element */

	/* Image and foreign elements */
	WLF_SVG_NODE_IMAGE,        /**< Image <image> element */
	WLF_SVG_NODE_FOREIGN_OBJECT, /**< Foreign object <foreignObject> */

	/* Animation elements */
	WLF_SVG_NODE_ANIMATE,      /**< Animate <animate> element */
	WLF_SVG_NODE_ANIMATE_TRANSFORM, /**< Animate transform <animateTransform> */
	WLF_SVG_NODE_ANIMATE_MOTION,    /**< Animate motion <animateMotion> */

	/* Filter elements */
	WLF_SVG_NODE_FILTER,       /**< Filter <filter> element */
	WLF_SVG_NODE_FE_GAUSSIAN_BLUR, /**< Gaussian blur filter */
	WLF_SVG_NODE_FE_OFFSET,    /**< Offset filter */
	WLF_SVG_NODE_FE_FLOOD,     /**< Flood filter */
	WLF_SVG_NODE_FE_COMPOSITE, /**< Composite filter */

	/* Clipping and masking */
	WLF_SVG_NODE_CLIP_PATH,    /**< Clip path <clipPath> element */
	WLF_SVG_NODE_MASK,         /**< Mask <mask> element */

	/* Metadata elements */
	WLF_SVG_NODE_TITLE,        /**< Title <title> element */
	WLF_SVG_NODE_DESC,         /**< Description <desc> element */
	WLF_SVG_NODE_METADATA,     /**< Metadata <metadata> element */

	/* Text content */
	WLF_SVG_NODE_TEXT_CONTENT, /**< Text content (CDATA) */
};

/**
 * @brief SVG units enumeration.
 */
enum wlf_svg_unit {
	WLF_SVG_UNIT_UNKNOWN = 0,
	WLF_SVG_UNIT_NONE,         /**< No unit (number) */
	WLF_SVG_UNIT_PX,           /**< Pixels */
	WLF_SVG_UNIT_EM,           /**< Em units */
	WLF_SVG_UNIT_EX,           /**< Ex units */
	WLF_SVG_UNIT_IN,           /**< Inches */
	WLF_SVG_UNIT_CM,           /**< Centimeters */
	WLF_SVG_UNIT_MM,           /**< Millimeters */
	WLF_SVG_UNIT_PT,           /**< Points */
	WLF_SVG_UNIT_PC,           /**< Picas */
	WLF_SVG_UNIT_PERCENT,      /**< Percentage */
};

/**
 * @brief SVG length value with unit.
 */
struct wlf_svg_length {
	float value;               /**< Numeric value */
	enum wlf_svg_unit unit;    /**< Unit type */
};

/**
 * @brief SVG color representation.
 */
struct wlf_svg_color {
	uint8_t r, g, b, a;        /**< RGBA components */
};

/**
 * @brief SVG point structure.
 */
struct wlf_svg_point {
	float x, y;
};

/**
 * @brief SVG rectangle structure.
 */
struct wlf_svg_rect {
	float x, y, width, height;
};

/**
 * @brief SVG transform matrix (3x3 for 2D transforms).
 */
struct wlf_svg_matrix {
	float m[6];  /**< [a, b, c, d, e, f] for matrix [a c e; b d f; 0 0 1] */
};

/**
 * @brief SVG paint type.
 */
enum wlf_svg_paint_type {
	WLF_SVG_PAINT_NONE = 0,
	WLF_SVG_PAINT_COLOR,       /**< Solid color */
	WLF_SVG_PAINT_GRADIENT,    /**< Gradient reference */
	WLF_SVG_PAINT_PATTERN,     /**< Pattern reference */
	WLF_SVG_PAINT_INHERIT,     /**< Inherit from parent */
	WLF_SVG_PAINT_CURRENT_COLOR, /**< Current color */
};

/**
 * @brief SVG paint specification.
 */
struct wlf_svg_paint {
	enum wlf_svg_paint_type type;
	union {
		struct wlf_svg_color color;
		char *url;             /**< Reference URL for gradients/patterns */
	};
	float opacity;             /**< Paint opacity [0-1] */
};

/**
 * @brief SVG stroke properties.
 */
struct wlf_svg_stroke {
	struct wlf_svg_paint paint;
	struct wlf_svg_length width;
	enum {
		WLF_SVG_LINECAP_BUTT,
		WLF_SVG_LINECAP_ROUND,
		WLF_SVG_LINECAP_SQUARE
	} linecap;
	enum {
		WLF_SVG_LINEJOIN_MITER,
		WLF_SVG_LINEJOIN_ROUND,
		WLF_SVG_LINEJOIN_BEVEL
	} linejoin;
	float miterlimit;
	float *dasharray;          /**< Dash pattern array */
	size_t dasharray_count;    /**< Number of dash values */
	struct wlf_svg_length dashoffset;
};

/**
 * @brief Forward declaration of SVG node.
 */
struct wlf_svg_node;

/**
 * @brief SVG node implementation interface.
 * Each backend implements these functions for specific node types.
 */
struct wlf_svg_node_impl {
	/**
	 * Parse attributes from string pairs.
	 * @param node Target node.
	 * @param name Attribute name.
	 * @param value Attribute value.
	 * @return true on success, false on failure.
	 */
	bool (*parse_attribute)(struct wlf_svg_node *node, const char *name, const char *value);

	/**
	 * Serialize node to string.
	 * @param node Source node.
	 * @param buffer Output buffer.
	 * @param buffer_size Buffer size.
	 * @return Number of bytes written, or -1 on error.
	 */
	int (*serialize)(const struct wlf_svg_node *node, char *buffer, size_t buffer_size);

	/**
	 * Calculate bounding box.
	 * @param node Source node.
	 * @param bbox Output bounding box.
	 * @return true on success, false on failure.
	 */
	bool (*get_bbox)(const struct wlf_svg_node *node, struct wlf_svg_rect *bbox);

	/**
	 * Clone/duplicate node.
	 * @param node Source node.
	 * @return Cloned node, or NULL on failure.
	 */
	struct wlf_svg_node *(*clone)(const struct wlf_svg_node *node);

	/**
	 * Validate node data.
	 * @param node Node to validate.
	 * @return true if valid, false otherwise.
	 */
	bool (*validate)(const struct wlf_svg_node *node);

	/**
	 * Cleanup node-specific resources.
	 * @param node Node to cleanup.
	 */
	void (*destroy)(struct wlf_svg_node *node);
};

/**
 * @brief Base SVG node structure.
 */
struct wlf_svg_node {
	const struct wlf_svg_node_impl *impl; /**< Implementation interface */
	enum wlf_svg_node_type type;          /**< Node type */

	/* Tree structure */
	struct wlf_svg_node *parent;          /**< Parent node */
	struct wlf_svg_node *first_child;     /**< First child node */
	struct wlf_svg_node *last_child;      /**< Last child node */
	struct wlf_svg_node *next_sibling;    /**< Next sibling node */
	struct wlf_svg_node *prev_sibling;    /**< Previous sibling node */

	/* Common attributes */
	char *id;                             /**< Element ID */
	char *class_name;                     /**< CSS class name */
	struct wlf_svg_matrix *transform;     /**< Transform matrix */

	/* Style properties */
	struct wlf_svg_paint fill;            /**< Fill paint */
	struct wlf_svg_stroke stroke;         /**< Stroke properties */
	float opacity;                        /**< Element opacity [0-1] */
	bool visible;                         /**< Visibility flag */

	/* Clipping and masking */
	char *clip_path;                      /**< Clip path reference */
	char *mask;                           /**< Mask reference */
	char *filter;                         /**< Filter reference */

	/* Node-specific data */
	void *data;                           /**< Type-specific data */
	size_t data_size;                     /**< Size of type-specific data */
};

#endif // SVG_WLF_SVG_NODE_H
