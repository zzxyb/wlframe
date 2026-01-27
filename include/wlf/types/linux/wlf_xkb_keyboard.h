#ifndef TYPES_WLF_XKB_KEYBOARD_H
#define TYPES_WLF_XKB_KEYBOARD_H

#include "wlf/types/wlf_keyboard.h"

#include <xkbcommon/xkbcommon.h>

struct wlf_keysym_map {
    xkb_keysym_t sym;
    enum wlf_key    key;
};

static const wlf_keysym_map keysym_map[] = {
    { XKB_KEY_Escape,     WLF_KEY_ESCAPE },
    { XKB_KEY_Tab,        WLF_KEY_TAB },
    { XKB_KEY_ISO_Left_Tab, WLF_KEY_TAB },
    { XKB_KEY_BackSpace,  WLF_KEY_BACKSPACE },
    { XKB_KEY_Return,     WLF_KEY_RETURN },
    { XKB_KEY_Insert,     WLF_KEY_INSERT },
    { XKB_KEY_Delete,     WLF_KEY_DELETE },

    { XKB_KEY_Left,       WLF_KEY_LEFT },
    { XKB_KEY_Right,      WLF_KEY_RIGHT },
    { XKB_KEY_Up,         WLF_KEY_UP },
    { XKB_KEY_Down,       WLF_KEY_DOWN },

    { XKB_KEY_Page_Up,    WLF_KEY_PAGE_UP },
    { XKB_KEY_Page_Down,  WLF_KEY_PAGE_DOWN },
    { XKB_KEY_Home,       WLF_KEY_HOME },
    { XKB_KEY_End,        WLF_KEY_END },

    { XKB_KEY_F1,  WLF_KEY_F1 },
    { XKB_KEY_F2,  WLF_KEY_F2 },
    { XKB_KEY_F3,  WLF_KEY_F3 },
    { XKB_KEY_F4,  WLF_KEY_F4 },
    { XKB_KEY_F5,  WLF_KEY_F5 },
    { XKB_KEY_F6,  WLF_KEY_F6 },
    { XKB_KEY_F7,  WLF_KEY_F7 },
    { XKB_KEY_F8,  WLF_KEY_F8 },
    { XKB_KEY_F9,  WLF_KEY_F9 },
    { XKB_KEY_F10, WLF_KEY_F10 },
    { XKB_KEY_F11, WLF_KEY_F11 },
    { XKB_KEY_F12, WLF_KEY_F12 },

    { XKB_KEY_Shift_L,    WLF_KEY_SHIFT },
    { XKB_KEY_Shift_R,    WLF_KEY_SHIFT },
    { XKB_KEY_Control_L,  WLF_KEY_CONTROL },
    { XKB_KEY_Control_R,  WLF_KEY_CONTROL },
    { XKB_KEY_Alt_L,      WLF_KEY_ALT },
    { XKB_KEY_Alt_R,      WLF_KEY_ALT },
    { XKB_KEY_Meta_L,     WLF_KEY_META },
    { XKB_KEY_Meta_R,     WLF_KEY_META },
    { XKB_KEY_Super_L,    WLF_KEY_META },
    { XKB_KEY_Super_R,    WLF_KEY_META },

    { XKB_KEY_Caps_Lock,  WLF_KEY_CAPS_LOCK },
    { XKB_KEY_Num_Lock,   WLF_KEY_NUM_LOCK },
    { XKB_KEY_Scroll_Lock,WLF_KEY_SCROLL_LOCK },
};

enum wlf_key xkb_keysym_to_wlf(xkb_keysym_t sym);

#endif // TYPES_WLF_XKB_KEYBOARD_H
