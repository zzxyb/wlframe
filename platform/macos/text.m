#include "wlf/platform/macos/text.h"

#include "wlf/utils/wlf_log.h"

#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static CTFontRef create_font(const struct wlf_text_options *options) {
	CGFloat size = options->font_size * options->raster_scale;
	CTFontRef font = NULL;
	if (options->font_family == NULL ||
			strcmp(options->font_family, "sans-serif") == 0) {
		font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
	} else {
		CFStringRef family = CFStringCreateWithCString(NULL,
			options->font_family, kCFStringEncodingUTF8);
		if (family != NULL) {
			font = CTFontCreateWithName(family, size, NULL);
			CFRelease(family);
		}
	}
	if (font == NULL) {
		font = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, size, NULL);
	}
	if (font == NULL) {
		return NULL;
	}

	CTFontSymbolicTraits traits = 0;
	CTFontSymbolicTraits mask = 0;
	if (options->weight == WLF_TEXT_FONT_WEIGHT_BOLD) {
		traits |= kCTFontBoldTrait;
		mask |= kCTFontBoldTrait;
	}
	if (options->slant == WLF_TEXT_FONT_SLANT_ITALIC ||
			options->slant == WLF_TEXT_FONT_SLANT_OBLIQUE) {
		traits |= kCTFontItalicTrait;
		mask |= kCTFontItalicTrait;
	}
	if (mask != 0) {
		CTFontRef styled = CTFontCreateCopyWithSymbolicTraits(font, size,
			NULL, traits, mask);
		if (styled != NULL) {
			CFRelease(font);
			font = styled;
		}
	}
	return font;
}

static CFAttributedStringRef create_attributed_string(
		const struct wlf_text_options *options, CTFontRef font) {
	CFStringRef string = CFStringCreateWithCString(NULL, options->text,
		kCFStringEncodingUTF8);
	if (string == NULL) {
		return NULL;
	}
	struct wlf_color color = wlf_color_clamp(&options->color);
	CGFloat components[4] = {color.r, color.g, color.b, color.a};
	CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
	CGColorRef cg_color = CGColorCreate(color_space, components);
	CGColorSpaceRelease(color_space);
	if (cg_color == NULL) {
		CFRelease(string);
		return NULL;
	}
	const void *keys[] = {kCTFontAttributeName, kCTForegroundColorAttributeName};
	const void *values[] = {font, cg_color};
	CFDictionaryRef attributes = CFDictionaryCreate(NULL, keys, values, 2,
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	CGColorRelease(cg_color);
	if (attributes == NULL) {
		CFRelease(string);
		return NULL;
	}
	CFAttributedStringRef attributed = CFAttributedStringCreate(NULL,
		string, attributes);
	CFRelease(attributes);
	CFRelease(string);
	return attributed;
}

static void flip_rows(uint8_t *data, size_t stride, uint32_t height) {
	uint8_t *row = malloc(stride);
	if (row == NULL) {
		return;
	}
	for (uint32_t y = 0; y < height / 2; ++y) {
		uint8_t *top = data + (size_t)y * stride;
		uint8_t *bottom = data + (size_t)(height - y - 1) * stride;
		memcpy(row, top, stride);
		memcpy(top, bottom, stride);
		memcpy(bottom, row, stride);
	}
	free(row);
}

static void macos_text_raster_destroy(struct wlf_text *text,
		struct wlf_text_raster *raster) {
	(void)text;
	free(raster->private_data);
	*raster = (struct wlf_text_raster){0};
}

static bool macos_text_rasterize(struct wlf_text *text,
		const struct wlf_text_options *options,
		struct wlf_text_raster *raster) {
	(void)text;
	@autoreleasepool {
		CTFontRef font = create_font(options);
		if (font == NULL) return false;
		CFAttributedStringRef attributed =
			create_attributed_string(options, font);
		if (attributed == NULL) {
			CFRelease(font);
			return false;
		}
		CTFramesetterRef framesetter =
			CTFramesetterCreateWithAttributedString(attributed);
		if (framesetter == NULL) {
			CFRelease(attributed);
			CFRelease(font);
			return false;
		}

		CFRange fit_range = {0};
		CGSize measured = CTFramesetterSuggestFrameSizeWithConstraints(
			framesetter, CFRangeMake(0, 0), NULL,
			CGSizeMake(1000000, 1000000), &fit_range);
		double natural_width = ceil(measured.width);
		double natural_height = ceil(measured.height);
		raster->metrics = (struct wlf_text_metrics){
			.width = natural_width,
			.height = natural_height,
			.baseline = CTFontGetAscent(font),
		};
		if (options->text[0] == '\0' || natural_width <= 0 ||
				natural_height <= 0) {
			CFRelease(framesetter);
			CFRelease(attributed);
			CFRelease(font);
			return true;
		}

		double clipped_width = natural_width;
		if (options->max_width > 0) {
			double max_width = ceil(options->max_width * options->raster_scale);
			if (max_width < clipped_width) clipped_width = max_width;
		}
		if (clipped_width <= 0 || clipped_width > UINT32_MAX ||
				natural_height > UINT32_MAX) {
			CFRelease(framesetter);
			CFRelease(attributed);
			CFRelease(font);
			return clipped_width <= 0;
		}
		uint32_t width = (uint32_t)clipped_width;
		uint32_t height = (uint32_t)natural_height;
		if (width > UINT32_MAX / 4) {
			CFRelease(framesetter);
			CFRelease(attributed);
			CFRelease(font);
			return false;
		}
		size_t stride = (size_t)width * 4;
		if (height > SIZE_MAX / stride) {
			CFRelease(framesetter);
			CFRelease(attributed);
			CFRelease(font);
			return false;
		}
		uint8_t *pixels = calloc(height, stride);
		CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(pixels, width, height,
			8, stride, color_space, kCGBitmapByteOrder32Little |
			kCGImageAlphaPremultipliedFirst);
		CGColorSpaceRelease(color_space);
		if (pixels == NULL || context == NULL) {
			free(pixels);
			if (context != NULL) CGContextRelease(context);
			CFRelease(framesetter);
			CFRelease(attributed);
			CFRelease(font);
			return false;
		}

		CGPathRef path = CGPathCreateWithRect(
			CGRectMake(0, 0, natural_width, natural_height), NULL);
		CTFrameRef frame = CTFramesetterCreateFrame(framesetter,
			CFRangeMake(0, 0), path, NULL);
		CGPathRelease(path);
		if (frame != NULL) {
			CGContextSetTextMatrix(context, CGAffineTransformIdentity);
			CTFrameDraw(frame, context);
			CFRelease(frame);
		}
		CGContextRelease(context);
		flip_rows(pixels, stride, height);

		raster->width = width;
		raster->height = height;
		raster->stride = (uint32_t)stride;
		raster->data = pixels;
		raster->private_data = pixels;
		CFRelease(framesetter);
		CFRelease(attributed);
		CFRelease(font);
		return frame != NULL;
	}
}

static void macos_text_destroy(struct wlf_text *text) {
	free(text);
}

static const struct wlf_text_impl macos_text_impl = {
	.name = "macos-core-text",
	.rasterize = macos_text_rasterize,
	.destroy_raster = macos_text_raster_destroy,
	.destroy = macos_text_destroy,
};

struct wlf_macos_text *wlf_macos_text_create(void) {
	struct wlf_macos_text *text = calloc(1, sizeof(*text));
	if (text == NULL) {
		wlf_log_errno(WLF_ERROR, "failed to allocate Core Text implementation");
		return NULL;
	}
	wlf_text_init(&text->base, &macos_text_impl);
	return text;
}
