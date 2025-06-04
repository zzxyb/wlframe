/**
 * @file        wlf_font_backend_macos.c
 * @brief       macOS font backend implementation using Core Text.
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 */

#ifdef __APPLE__

#include "wlf/font/wlf_font_backend.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>

// Helper function to convert CFString to C string
static char* cfstring_to_cstring(CFStringRef cfstr) {
	if (!cfstr) {
		return NULL;
	}

	CFIndex length = CFStringGetLength(cfstr);
	CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
	char *buffer = malloc(maxSize);
	if (!buffer) {
		return NULL;
	}

	if (!CFStringGetCString(cfstr, buffer, maxSize, kCFStringEncodingUTF8)) {
		free(buffer);
		return NULL;
	}

	return buffer;
}

// Helper function to convert font weight from Core Text to our enum
static enum wlf_font_weight ct_weight_to_wlf_weight(float ctWeight) {
	// Core Text weight values are typically -1.0 to 1.0
	if (ctWeight <= -0.7f) return WLF_FONT_WEIGHT_THIN;
	if (ctWeight <= -0.5f) return WLF_FONT_WEIGHT_EXTRA_LIGHT;
	if (ctWeight <= -0.3f) return WLF_FONT_WEIGHT_LIGHT;
	if (ctWeight <= -0.1f) return WLF_FONT_WEIGHT_NORMAL;
	if (ctWeight <= 0.1f) return WLF_FONT_WEIGHT_MEDIUM;
	if (ctWeight <= 0.3f) return WLF_FONT_WEIGHT_SEMI_BOLD;
	if (ctWeight <= 0.5f) return WLF_FONT_WEIGHT_BOLD;
	if (ctWeight <= 0.7f) return WLF_FONT_WEIGHT_EXTRA_BOLD;
	return WLF_FONT_WEIGHT_BLACK;
}

// Helper function to convert font style from Core Text to our enum
static enum wlf_font_style ct_style_to_wlf_style(float ctSlant) {
	if (ctSlant > 0.1f) return WLF_FONT_STYLE_ITALIC;
	if (ctSlant < -0.1f) return WLF_FONT_STYLE_OBLIQUE;
	return WLF_FONT_STYLE_NORMAL;
}

// Helper function to get font file path from CTFont
static char* get_font_file_path(CTFontRef font) {
	CFURLRef url = CTFontCopyAttribute(font, kCTFontURLAttribute);
	if (!url) {
		return NULL;
	}

	CFStringRef path = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	CFRelease(url);

	if (!path) {
		return NULL;
	}

	char *result = cfstring_to_cstring(path);
	CFRelease(path);
	return result;
}

// Helper function to populate font info from CTFont
static bool populate_font_info(CTFontRef font, struct wlf_font_info *info) {
	memset(info, 0, sizeof(*info));

	// Get font names
	CFStringRef familyName = CTFontCopyFamilyName(font);
	if (familyName) {
		info->family_name = cfstring_to_cstring(familyName);
		CFRelease(familyName);
	}

	CFStringRef styleName = CTFontCopyName(font, kCTFontStyleNameKey);
	if (styleName) {
		info->style_name = cfstring_to_cstring(styleName);
		CFRelease(styleName);
	}

	CFStringRef postscriptName = CTFontCopyPostScriptName(font);
	if (postscriptName) {
		info->postscript_name = cfstring_to_cstring(postscriptName);
		CFRelease(postscriptName);
	}

	// Get font file path
	info->file_path = get_font_file_path(font);

	// Get font traits
	CTFontSymbolicTraits traits = CTFontGetSymbolicTraits(font);
	info->is_monospace = (traits & kCTFontMonoSpaceTrait) != 0;

	// Get font weight and style
	CFDictionaryRef traitsDict = CTFontCopyTraits(font);
	if (traitsDict) {
		CFNumberRef weightNumber = CFDictionaryGetValue(traitsDict, kCTFontWeightTrait);
		if (weightNumber) {
			float weight;
			CFNumberGetValue(weightNumber, kCFNumberFloatType, &weight);
			info->weight = ct_weight_to_wlf_weight(weight);
		}

		CFNumberRef slantNumber = CFDictionaryGetValue(traitsDict, kCTFontSlantTrait);
		if (slantNumber) {
			float slant;
			CFNumberGetValue(slantNumber, kCFNumberFloatType, &slant);
			info->style = ct_style_to_wlf_style(slant);
		}

		CFRelease(traitsDict);
	}

	// Get supported languages
	CFArrayRef languages = CTFontCopySupportedLanguages(font);
	if (languages) {
		CFIndex count = CFArrayGetCount(languages);
		info->languages = calloc(count + 1, sizeof(char*));
		if (info->languages) {
			for (CFIndex i = 0; i < count; i++) {
				CFStringRef lang = CFArrayGetValueAtIndex(languages, i);
				info->languages[i] = cfstring_to_cstring(lang);
			}
		}
		CFRelease(languages);
	}

	info->is_scalable = true; // Most Core Text fonts are scalable
	info->width = WLF_FONT_WIDTH_NORMAL; // Default value

	return true;
}

// Backend implementation functions
static bool macos_backend_init(void) {
	wlf_log(WLF_INFO, "Initializing macOS Core Text font backend");
	return true;
}

static void macos_backend_cleanup(void) {
	wlf_log(WLF_INFO, "Cleaning up macOS Core Text font backend");
}

static bool macos_enumerate_fonts(wlf_font_enum_callback callback, void *user_data) {
	CTFontCollectionRef collection = CTFontCollectionCreateFromAvailableFonts(NULL);
	if (!collection) {
		wlf_log(WLF_ERROR, "Failed to create font collection");
		return false;
	}

	CFArrayRef fonts = CTFontCollectionCreateMatchingFontDescriptors(collection);
	CFRelease(collection);

	if (!fonts) {
		wlf_log(WLF_ERROR, "Failed to get font descriptors");
		return false;
	}

	CFIndex count = CFArrayGetCount(fonts);
	bool should_continue = true;

	for (CFIndex i = 0; i < count && should_continue; i++) {
		CTFontDescriptorRef descriptor = CFArrayGetValueAtIndex(fonts, i);
		CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 12.0, NULL);

		if (font) {
			struct wlf_font_info info;
			if (populate_font_info(font, &info)) {
				should_continue = callback(&info, user_data);
				wlf_font_info_free(&info);
			}
			CFRelease(font);
		}
	}

	CFRelease(fonts);
	return true;
}

static bool macos_find_fonts(const char *pattern, wlf_font_enum_callback callback, void *user_data) {
	if (!pattern) {
		return macos_enumerate_fonts(callback, user_data);
	}

	// Create font descriptor with family name pattern
	CFStringRef familyName = CFStringCreateWithCString(NULL, pattern, kCFStringEncodingUTF8);
	if (!familyName) {
		return false;
	}

	CFDictionaryRef attributes = CFDictionaryCreate(
		NULL,
		(const void**)&kCTFontFamilyNameAttribute,
		(const void**)&familyName,
		1,
		&kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks
	);
	CFRelease(familyName);

	if (!attributes) {
		return false;
	}

	CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
	CFRelease(attributes);

	if (!descriptor) {
		return false;
	}

	CFArrayRef descriptors = CFArrayCreate(NULL, (const void**)&descriptor, 1, &kCFTypeArrayCallBacks);
	CFRelease(descriptor);

	if (!descriptors) {
		return false;
	}

	CTFontCollectionRef collection = CTFontCollectionCreateWithFontDescriptors(descriptors, NULL);
	CFRelease(descriptors);

	if (!collection) {
		return false;
	}

	CFArrayRef matchingFonts = CTFontCollectionCreateMatchingFontDescriptors(collection);
	CFRelease(collection);

	if (!matchingFonts) {
		return false;
	}

	CFIndex count = CFArrayGetCount(matchingFonts);
	bool should_continue = true;

	for (CFIndex i = 0; i < count && should_continue; i++) {
		CTFontDescriptorRef desc = CFArrayGetValueAtIndex(matchingFonts, i);
		CTFontRef font = CTFontCreateWithFontDescriptor(desc, 12.0, NULL);

		if (font) {
			struct wlf_font_info info;
			if (populate_font_info(font, &info)) {
				should_continue = callback(&info, user_data);
				wlf_font_info_free(&info);
			}
			CFRelease(font);
		}
	}

	CFRelease(matchingFonts);
	return true;
}

static char* macos_get_font_path(const char *family_name, enum wlf_font_style style, enum wlf_font_weight weight) {
	if (!family_name) {
		return NULL;
	}

	CFStringRef familyName = CFStringCreateWithCString(NULL, family_name, kCFStringEncodingUTF8);
	if (!familyName) {
		return NULL;
	}

	// Create font descriptor
	CFMutableDictionaryRef attributes = CFDictionaryCreateMutable(
		NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue(attributes, kCTFontFamilyNameAttribute, familyName);
	CFRelease(familyName);

	// Add traits if specified
	CFMutableDictionaryRef traits = CFDictionaryCreateMutable(
		NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	// Convert weight to Core Text weight
	float ctWeight = 0.0f;
	switch (weight) {
		case WLF_FONT_WEIGHT_THIN: ctWeight = -0.8f; break;
		case WLF_FONT_WEIGHT_EXTRA_LIGHT: ctWeight = -0.6f; break;
		case WLF_FONT_WEIGHT_LIGHT: ctWeight = -0.4f; break;
		case WLF_FONT_WEIGHT_NORMAL: ctWeight = 0.0f; break;
		case WLF_FONT_WEIGHT_MEDIUM: ctWeight = 0.23f; break;
		case WLF_FONT_WEIGHT_SEMI_BOLD: ctWeight = 0.3f; break;
		case WLF_FONT_WEIGHT_BOLD: ctWeight = 0.4f; break;
		case WLF_FONT_WEIGHT_EXTRA_BOLD: ctWeight = 0.56f; break;
		case WLF_FONT_WEIGHT_BLACK: ctWeight = 0.8f; break;
	}

	CFNumberRef weightNumber = CFNumberCreate(NULL, kCFNumberFloatType, &ctWeight);
	CFDictionarySetValue(traits, kCTFontWeightTrait, weightNumber);
	CFRelease(weightNumber);

	// Convert style to Core Text slant
	float ctSlant = 0.0f;
	switch (style) {
		case WLF_FONT_STYLE_NORMAL: ctSlant = 0.0f; break;
		case WLF_FONT_STYLE_ITALIC: ctSlant = 0.2f; break;
		case WLF_FONT_STYLE_OBLIQUE: ctSlant = -0.2f; break;
	}

	CFNumberRef slantNumber = CFNumberCreate(NULL, kCFNumberFloatType, &ctSlant);
	CFDictionarySetValue(traits, kCTFontSlantTrait, slantNumber);
	CFRelease(slantNumber);

	CFDictionarySetValue(attributes, kCTFontTraitsAttribute, traits);
	CFRelease(traits);

	CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
	CFRelease(attributes);

	if (!descriptor) {
		return NULL;
	}

	CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 12.0, NULL);
	CFRelease(descriptor);

	if (!font) {
		return NULL;
	}

	char *path = get_font_file_path(font);
	CFRelease(font);

	return path;
}

static char* macos_get_default_font(const char *language) {
	// Get system font for the specified language
	CFStringRef langCode = NULL;
	if (language) {
		langCode = CFStringCreateWithCString(NULL, language, kCFStringEncodingUTF8);
	}

	CTFontRef systemFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 12.0, langCode);
	if (langCode) {
		CFRelease(langCode);
	}

	if (!systemFont) {
		systemFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 12.0, NULL);
	}

	if (!systemFont) {
		return NULL;
	}

	char *path = get_font_file_path(systemFont);
	CFRelease(systemFont);

	return path;
}

static char* macos_get_monospace_font(void) {
	CTFontRef monoFont = CTFontCreateUIFontForLanguage(kCTFontUIFontUserFixedPitch, 12.0, NULL);
	if (!monoFont) {
		// Fallback to system monospace font
		CFStringRef familyName = CFSTR("Monaco");
		CFDictionaryRef attributes = CFDictionaryCreate(
			NULL,
			(const void**)&kCTFontFamilyNameAttribute,
			(const void**)&familyName,
			1,
			&kCFTypeDictionaryKeyCallBacks,
			&kCFTypeDictionaryValueCallBacks
		);

		if (attributes) {
			CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);
			CFRelease(attributes);

			if (descriptor) {
				monoFont = CTFontCreateWithFontDescriptor(descriptor, 12.0, NULL);
				CFRelease(descriptor);
			}
		}
	}

	if (!monoFont) {
		return NULL;
	}

	char *path = get_font_file_path(monoFont);
	CFRelease(monoFont);

	return path;
}

static bool macos_is_available(void) {
	return true; // Always available on macOS
}

// Backend structure
const struct wlf_font_backend wlf_font_backend_macos = {
	.name = "CoreText",
	.description = "macOS Core Text font backend",
	.init = macos_backend_init,
	.cleanup = macos_backend_cleanup,
	.enumerate_fonts = macos_enumerate_fonts,
	.find_fonts = macos_find_fonts,
	.get_font_path = macos_get_font_path,
	.get_default_font = macos_get_default_font,
	.get_monospace_font = macos_get_monospace_font,
	.is_available = macos_is_available,
};

#endif // __APPLE__
