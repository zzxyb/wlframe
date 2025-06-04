/**
 * @file        wlf_font_backend_fontconfig.c
 * @brief       Linux FontConfig font backend implementation.
 * @author      YaoBing Xiao
 * @date        2025-06-17
 * @version     v1.0
 */

#ifdef __linux__

#include "wlf/font/wlf_font_backend.h"
#include "wlf/utils/wlf_log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <fontconfig/fontconfig.h>

// Helper function to convert FontConfig weight to our enum
static enum wlf_font_weight fc_weight_to_wlf_weight(int fcWeight) {
	if (fcWeight <= FC_WEIGHT_THIN) return WLF_FONT_WEIGHT_THIN;
	if (fcWeight <= FC_WEIGHT_EXTRALIGHT) return WLF_FONT_WEIGHT_EXTRA_LIGHT;
	if (fcWeight <= FC_WEIGHT_LIGHT) return WLF_FONT_WEIGHT_LIGHT;
	if (fcWeight <= FC_WEIGHT_NORMAL) return WLF_FONT_WEIGHT_NORMAL;
	if (fcWeight <= FC_WEIGHT_MEDIUM) return WLF_FONT_WEIGHT_MEDIUM;
	if (fcWeight <= FC_WEIGHT_DEMIBOLD) return WLF_FONT_WEIGHT_SEMI_BOLD;
	if (fcWeight <= FC_WEIGHT_BOLD) return WLF_FONT_WEIGHT_BOLD;
	if (fcWeight <= FC_WEIGHT_EXTRABOLD) return WLF_FONT_WEIGHT_EXTRA_BOLD;
	return WLF_FONT_WEIGHT_BLACK;
}

// Helper function to convert our weight enum to FontConfig weight
static int wlf_weight_to_fc_weight(enum wlf_font_weight weight) {
	switch (weight) {
		case WLF_FONT_WEIGHT_THIN: return FC_WEIGHT_THIN;
		case WLF_FONT_WEIGHT_EXTRA_LIGHT: return FC_WEIGHT_EXTRALIGHT;
		case WLF_FONT_WEIGHT_LIGHT: return FC_WEIGHT_LIGHT;
		case WLF_FONT_WEIGHT_NORMAL: return FC_WEIGHT_NORMAL;
		case WLF_FONT_WEIGHT_MEDIUM: return FC_WEIGHT_MEDIUM;
		case WLF_FONT_WEIGHT_SEMI_BOLD: return FC_WEIGHT_DEMIBOLD;
		case WLF_FONT_WEIGHT_BOLD: return FC_WEIGHT_BOLD;
		case WLF_FONT_WEIGHT_EXTRA_BOLD: return FC_WEIGHT_EXTRABOLD;
		case WLF_FONT_WEIGHT_BLACK: return FC_WEIGHT_BLACK;
	}
	return FC_WEIGHT_NORMAL;
}

// Helper function to convert FontConfig slant to our enum
static enum wlf_font_style fc_slant_to_wlf_style(int fcSlant) {
	switch (fcSlant) {
		case FC_SLANT_ROMAN: return WLF_FONT_STYLE_NORMAL;
		case FC_SLANT_ITALIC: return WLF_FONT_STYLE_ITALIC;
		case FC_SLANT_OBLIQUE: return WLF_FONT_STYLE_OBLIQUE;
	}
	return WLF_FONT_STYLE_NORMAL;
}

// Helper function to convert our style enum to FontConfig slant
static int wlf_style_to_fc_slant(enum wlf_font_style style) {
	switch (style) {
		case WLF_FONT_STYLE_NORMAL: return FC_SLANT_ROMAN;
		case WLF_FONT_STYLE_ITALIC: return FC_SLANT_ITALIC;
		case WLF_FONT_STYLE_OBLIQUE: return FC_SLANT_OBLIQUE;
	}
	return FC_SLANT_ROMAN;
}

// Helper function to convert FontConfig width to our enum
static enum wlf_font_width fc_width_to_wlf_width(int fcWidth) {
	if (fcWidth <= FC_WIDTH_ULTRACONDENSED) return WLF_FONT_WIDTH_ULTRA_CONDENSED;
	if (fcWidth <= FC_WIDTH_EXTRACONDENSED) return WLF_FONT_WIDTH_EXTRA_CONDENSED;
	if (fcWidth <= FC_WIDTH_CONDENSED) return WLF_FONT_WIDTH_CONDENSED;
	if (fcWidth <= FC_WIDTH_SEMICONDENSED) return WLF_FONT_WIDTH_SEMI_CONDENSED;
	if (fcWidth <= FC_WIDTH_NORMAL) return WLF_FONT_WIDTH_NORMAL;
	if (fcWidth <= FC_WIDTH_SEMIEXPANDED) return WLF_FONT_WIDTH_SEMI_EXPANDED;
	if (fcWidth <= FC_WIDTH_EXPANDED) return WLF_FONT_WIDTH_EXPANDED;
	if (fcWidth <= FC_WIDTH_EXTRAEXPANDED) return WLF_FONT_WIDTH_EXTRA_EXPANDED;
	return WLF_FONT_WIDTH_ULTRA_EXPANDED;
}

// Helper function to get string from FontConfig pattern
static char* fc_get_string(FcPattern *pattern, const char *property) {
	FcChar8 *value = NULL;
	if (FcPatternGetString(pattern, property, 0, &value) == FcResultMatch && value) {
		return strdup((char*)value);
	}
	return NULL;
}

// Helper function to get languages from FontConfig pattern
static char** fc_get_languages(FcPattern *pattern) {
	FcLangSet *langset = NULL;
	if (FcPatternGetLangSet(pattern, FC_LANG, 0, &langset) != FcResultMatch || !langset) {
		return NULL;
	}

	FcStrSet *langs = FcLangSetGetLangs(langset);
	if (!langs) {
		return NULL;
	}

	FcStrList *list = FcStrListCreate(langs);
	if (!list) {
		FcStrSetDestroy(langs);
		return NULL;
	}

	// Count languages
	size_t count = 0;
	FcChar8 *lang;
	while ((lang = FcStrListNext(list)) != NULL) {
		count++;
	}
	FcStrListDone(list);

	if (count == 0) {
		FcStrSetDestroy(langs);
		return NULL;
	}

	// Allocate array
	char **result = calloc(count + 1, sizeof(char*));
	if (!result) {
		FcStrSetDestroy(langs);
		return NULL;
	}

	// Fill array
	list = FcStrListCreate(langs);
	size_t i = 0;
	while ((lang = FcStrListNext(list)) != NULL && i < count) {
		result[i++] = strdup((char*)lang);
	}
	FcStrListDone(list);
	FcStrSetDestroy(langs);

	return result;
}

// Helper function to populate font info from FontConfig pattern
static bool populate_font_info(FcPattern *pattern, struct wlf_font_info *info) {
	memset(info, 0, sizeof(*info));

	// Get font names
	info->family_name = fc_get_string(pattern, FC_FAMILY);
	info->style_name = fc_get_string(pattern, FC_STYLE);
	info->postscript_name = fc_get_string(pattern, FC_POSTSCRIPT_NAME);
	info->file_path = fc_get_string(pattern, FC_FILE);

	// Get font properties
	int weight = FC_WEIGHT_NORMAL;
	FcPatternGetInteger(pattern, FC_WEIGHT, 0, &weight);
	info->weight = fc_weight_to_wlf_weight(weight);

	int slant = FC_SLANT_ROMAN;
	FcPatternGetInteger(pattern, FC_SLANT, 0, &slant);
	info->style = fc_slant_to_wlf_style(slant);

	int width = FC_WIDTH_NORMAL;
	FcPatternGetInteger(pattern, FC_WIDTH, 0, &width);
	info->width = fc_width_to_wlf_width(width);

	// Check if monospace
	int spacing = FC_PROPORTIONAL;
	FcPatternGetInteger(pattern, FC_SPACING, 0, &spacing);
	info->is_monospace = (spacing == FC_MONO);

	// Check if scalable
	FcBool scalable = FcFalse;
	FcPatternGetBool(pattern, FC_SCALABLE, 0, &scalable);
	info->is_scalable = (scalable == FcTrue);

	// Get supported languages
	info->languages = fc_get_languages(pattern);

	// Get character sets (simplified)
	FcCharSet *charset = NULL;
	if (FcPatternGetCharSet(pattern, FC_CHARSET, 0, &charset) == FcResultMatch && charset) {
		// For simplicity, we'll just indicate basic charset support
		info->character_sets = calloc(4, sizeof(char*));
		if (info->character_sets) {
			if (FcCharSetHasChar(charset, 0x0041)) { // 'A'
				info->character_sets[0] = strdup("Latin");
			}
			if (FcCharSetHasChar(charset, 0x4E00)) { // CJK
				info->character_sets[1] = strdup("CJK");
			}
			if (FcCharSetHasChar(charset, 0x0400)) { // Cyrillic
				info->character_sets[2] = strdup("Cyrillic");
			}
		}
	}

	return true;
}

// Backend implementation functions
static bool fontconfig_backend_init(void) {
	if (!FcInit()) {
		wlf_log(WLF_ERROR, "Failed to initialize FontConfig");
		return false;
	}
	wlf_log(WLF_INFO, "Initialized FontConfig font backend");
	return true;
}

static void fontconfig_backend_cleanup(void) {
	FcFini();
	wlf_log(WLF_INFO, "Cleaned up FontConfig font backend");
}

static bool fontconfig_enumerate_fonts(wlf_font_enum_callback callback, void *user_data) {
	FcPattern *pattern = FcPatternCreate();
	if (!pattern) {
		wlf_log(WLF_ERROR, "Failed to create FontConfig pattern");
		return false;
	}

	FcObjectSet *objectSet = FcObjectSetBuild(
		FC_FAMILY, FC_STYLE, FC_POSTSCRIPT_NAME, FC_FILE,
		FC_WEIGHT, FC_SLANT, FC_WIDTH, FC_SPACING, FC_SCALABLE,
		FC_LANG, FC_CHARSET, NULL);

	if (!objectSet) {
		FcPatternDestroy(pattern);
		wlf_log(WLF_ERROR, "Failed to create FontConfig object set");
		return false;
	}

	FcFontSet *fontSet = FcFontList(NULL, pattern, objectSet);
	FcPatternDestroy(pattern);
	FcObjectSetDestroy(objectSet);

	if (!fontSet) {
		wlf_log(WLF_ERROR, "Failed to get font list from FontConfig");
		return false;
	}

	bool should_continue = true;
	for (int i = 0; i < fontSet->nfont && should_continue; i++) {
		struct wlf_font_info info;
		if (populate_font_info(fontSet->fonts[i], &info)) {
			should_continue = callback(&info, user_data);
			wlf_font_info_free(&info);
		}
	}

	FcFontSetDestroy(fontSet);
	return true;
}

static bool fontconfig_find_fonts(const char *pattern_str, wlf_font_enum_callback callback, void *user_data) {
	if (!pattern_str) {
		return fontconfig_enumerate_fonts(callback, user_data);
	}

	FcPattern *pattern = FcNameParse((const FcChar8*)pattern_str);
	if (!pattern) {
		wlf_log(WLF_ERROR, "Failed to parse FontConfig pattern: %s", pattern_str);
		return false;
	}

	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcObjectSet *objectSet = FcObjectSetBuild(
		FC_FAMILY, FC_STYLE, FC_POSTSCRIPT_NAME, FC_FILE,
		FC_WEIGHT, FC_SLANT, FC_WIDTH, FC_SPACING, FC_SCALABLE,
		FC_LANG, FC_CHARSET, NULL);

	if (!objectSet) {
		FcPatternDestroy(pattern);
		wlf_log(WLF_ERROR, "Failed to create FontConfig object set");
		return false;
	}

	FcFontSet *fontSet = FcFontList(NULL, pattern, objectSet);
	FcPatternDestroy(pattern);
	FcObjectSetDestroy(objectSet);

	if (!fontSet) {
		wlf_log(WLF_ERROR, "Failed to find fonts matching pattern: %s", pattern_str);
		return false;
	}

	bool should_continue = true;
	for (int i = 0; i < fontSet->nfont && should_continue; i++) {
		struct wlf_font_info info;
		if (populate_font_info(fontSet->fonts[i], &info)) {
			should_continue = callback(&info, user_data);
			wlf_font_info_free(&info);
		}
	}

	FcFontSetDestroy(fontSet);
	return true;
}

static char* fontconfig_get_font_path(const char *family_name, enum wlf_font_style style, enum wlf_font_weight weight) {
	if (!family_name) {
		return NULL;
	}

	FcPattern *pattern = FcPatternCreate();
	if (!pattern) {
		return NULL;
	}

	FcPatternAddString(pattern, FC_FAMILY, (const FcChar8*)family_name);
	FcPatternAddInteger(pattern, FC_SLANT, wlf_style_to_fc_slant(style));
	FcPatternAddInteger(pattern, FC_WEIGHT, wlf_weight_to_fc_weight(weight));

	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult result;
	FcPattern *matched = FcFontMatch(NULL, pattern, &result);
	FcPatternDestroy(pattern);

	if (!matched) {
		return NULL;
	}

	char *file_path = fc_get_string(matched, FC_FILE);
	FcPatternDestroy(matched);

	return file_path;
}

static char* fontconfig_get_default_font(const char *language) {
	FcPattern *pattern = FcPatternCreate();
	if (!pattern) {
		return NULL;
	}

	if (language) {
		FcPatternAddString(pattern, FC_LANG, (const FcChar8*)language);
	}

	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult result;
	FcPattern *matched = FcFontMatch(NULL, pattern, &result);
	FcPatternDestroy(pattern);

	if (!matched) {
		return NULL;
	}

	char *file_path = fc_get_string(matched, FC_FILE);
	FcPatternDestroy(matched);

	return file_path;
}

static char* fontconfig_get_monospace_font(void) {
	FcPattern *pattern = FcPatternCreate();
	if (!pattern) {
		return NULL;
	}

	FcPatternAddInteger(pattern, FC_SPACING, FC_MONO);

	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult result;
	FcPattern *matched = FcFontMatch(NULL, pattern, &result);
	FcPatternDestroy(pattern);

	if (!matched) {
		return NULL;
	}

	char *file_path = fc_get_string(matched, FC_FILE);
	FcPatternDestroy(matched);

	return file_path;
}

static bool fontconfig_is_available(void) {
	// Check if FontConfig is available
	return FcConfigGetCurrent() != NULL;
}

// Backend structure
const struct wlf_font_backend wlf_font_backend_fontconfig = {
	.name = "FontConfig",
	.description = "Linux FontConfig font backend",
	.init = fontconfig_backend_init,
	.cleanup = fontconfig_backend_cleanup,
	.enumerate_fonts = fontconfig_enumerate_fonts,
	.find_fonts = fontconfig_find_fonts,
	.get_font_path = fontconfig_get_font_path,
	.get_default_font = fontconfig_get_default_font,
	.get_monospace_font = fontconfig_get_monospace_font,
	.is_available = fontconfig_is_available,
};

#endif // __linux__
