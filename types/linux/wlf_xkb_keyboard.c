#include "wlf/types/linux/wlf_xkb_keyboard.h"

enum wlf_key xkb_keysym_to_wlf(xkb_keysym_t sym) {
	for (size_t i = 0; i < sizeof(keysym_map)/sizeof(keysym_map[0]); ++i) {
		if (keysym_map[i].sym == sym) {
			return keysym_map[i].key;
		}
	}

	if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) {
		return WLF_KEY_A + (sym - XKB_KEY_A);
	}

	if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) {
		return WLF_KEY_A + (sym - XKB_KEY_a);
	}

	if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) {
		return WLF_KEY_0 + (sym - XKB_KEY_0);
	}

	if (sym >= 0x20 && sym <= 0x7E) {
		switch (sym) {
		case ' ': return WLF_KEY_SPACE;
		case '!': return WLF_KEY_EXCLAM;
		case '"': return WLF_KEY_QUOTEDBL;
		case '#': return WLF_KEY_NUMBER;
		case '$': return WLF_KEY_DOLLAR;
		case '%': return WLF_KEY_PERCENT;
		case '&': return WLF_KEY_AMPERSAND;
		case '\'':return WLF_KEY_APOSTROPHE;
		case '(': return WLF_KEY_PAREN_LEFT;
		case ')': return WLF_KEY_PAREN_RIGHT;
		case '*': return WLF_KEY_ASTERISK;
		case '+': return WLF_KEY_PLUS;
		case ',': return WLF_KEY_COMMA;
		case '-': return WLF_KEY_MINUS;
		case '.': return WLF_KEY_PERIOD;
		case '/': return WLF_KEY_SLASH;
		case ':': return WLF_KEY_COLON;
		case ';': return WLF_KEY_SEMICOLON;
		case '<': return WLF_KEY_LESS;
		case '=': return WLF_KEY_EQUAL;
		case '>': return WLF_KEY_GREATER;
		case '?': return WLF_KEY_QUESTION;
		case '@': return WLF_KEY_AT;
		}
	}

	return WLF_KEY_UNKNOWN;
}
