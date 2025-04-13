#pragma once

#ifndef u_short
typedef unsigned short u_short;
#endif
#ifndef byte
typedef unsigned char byte;
#endif

typedef enum : u_short {
    KEY_NULL,
    KEY_A = 4,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_ENTER,
    KEY_ESC,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_LEFT_BRACE,
    KEY_RIGHT_BRACE,
    KEY_BACKSLASH,
    KEY_NON_US_HASH,
    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_TILDE,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_PRINTSCREEN,
    KEY_SCROLLLOCK,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGE_UP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGE_DOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_NUM_LOCK,
    KEY_NUMPAD_DIVIDE,
    KEY_NUMPAD_MULTIPLY,
    KEY_NUMPAD_MINUS,
    KEY_NUMPAD_PLUS,
    KEY_NUMPAD_ENTER,
    KEY_NUMPAD_1,
    KEY_NUMPAD_2,
    KEY_NUMPAD_3,
    KEY_NUMPAD_4,
    KEY_NUMPAD_5,
    KEY_NUMPAD_6,
    KEY_NUMPAD_7,
    KEY_NUMPAD_8,
    KEY_NUMPAD_9,
    KEY_NUMPAD_0,
    KEY_NUMPAD_DOT,
    KEY_NON_US_BACKSLASH,
    KEY_APPLICATION,
    KEY_POWER,
    KEY_KEYPAD_EQUAL,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_EXECUTE,
    KEY_HELP,
    KEY_MENU,
    KEY_SELECT,
    KEY_STOP,
    KEY_AGAIN,
    KEY_UNDO,
    KEY_CUT,
    KEY_COPY,
    KEY_PASTE,
    KEY_FIND,
    KEY_MUTE,
    KEY_VOLUME_UP,
    KEY_VOLUME_DOWN,
    KEY_LOCKING_CAPS_LOCK,
    KEY_LOCKING_NUM_LOCK,
    KEY_LOCKING_SCROLL_LOCK,
    KEY_KEYPAD_COMMA,
    KEY_KEYPAD_EQUAL_SIGN_AS400,
    KEY_INTERNATIONAL1,
    KEY_INTERNATIONAL2,
    KEY_INTERNATIONAL3,
    KEY_INTERNATIONAL4,
    KEY_INTERNATIONAL5,
    KEY_INTERNATIONAL6,
    KEY_INTERNATIONAL7,
    KEY_INTERNATIONAL8,
    KEY_INTERNATIONAL9,
    KEY_LANG1,
    KEY_LANG2,
    KEY_LANG3,
    KEY_LANG4,
    KEY_LANG5,
    KEY_LANG6,
    KEY_LANG7,
    KEY_LANG8,
    KEY_LANG9,
    KEY_ALTERNATE_ERASE,
    KEY_SYSREQ,
    KEY_CANCEL,
    KEY_CLEAR,
    KEY_PRIOR,
    KEY_RETURN,
    KEY_SEPARATOR,
    KEY_OUT,
    KEY_OPER,
    KEY_CLEAR_AGAIN,
    KEY_CRSEL,
    KEY_EXSEL,
    KEY_NUMPAD_00 = 176,
    KEY_NUMPAD_000,
    KEY_THOUSANDS_SEPARATOR,
    KEY_DECIMAL_SEPARATOR,
    KEY_CURRENCY_UNIT,
    KEY_CURRENCY_SUB_UNIT,
    KEY_NUMPAD_LEFT_PARENTHESIS,
    KEY_NUMPAD_RIGHT_PARENTHESIS,
    KEY_NUMPAD_LEFT_CURLY_BRACE,
    KEY_NUMPAD_RIGHT_CURLY_BRACE,
    KEY_NUMPAD_TAB,
    KEY_NUMPAD_BACKSPACE,
    KEY_NUMPAD_A,
    KEY_NUMPAD_B,
    KEY_NUMPAD_C,
    KEY_NUMPAD_D,
    KEY_NUMPAD_E,
    KEY_NUMPAD_F,
    KEY_NUMPAD_XOR,
    KEY_NUMPAD_POWER,
    KEY_NUMPAD_PERCENT,
    KEY_NUMPAD_LESS_THAN,
    KEY_NUMPAD_GREATER_THAN,
    KEY_NUMPAD_AMPERSAND,
    KEY_NUMPAD_DOUBLE_AMPERSAND,
    KEY_NUMPAD_PIPE,
    KEY_NUMPAD_DOUBLE_PIPE,
    KEY_NUMPAD_COLON,
    KEY_NUMPAD_HASH_SIGN,
    KEY_NUMPAD_SPACE,
    KEY_NUMPAD_AT,
    KEY_NUMPAD_EXCLAMATION_MARK,
    KEY_NUMPAD_MEMORY_STORE,
    KEY_NUMPAD_MEMORY_RECALL,
    KEY_NUMPAD_MEMORY_CLEAR,
    KEY_NUMPAD_MEMORY_ADD,
    KEY_NUMPAD_MEMORY_SUBTRACT,
    KEY_NUMPAD_MEMORY_MULTIPLY,
    KEY_NUMPAD_MEMORY_DIVIDE,
    KEY_NUMPAD_POS_NEG,
    KEY_NUMPAD_CLEAR,
    KEY_NUMPAD_CLEAR_ENTRY,
    KEY_NUMPAD_BINARY,
    KEY_NUMPAD_OCTAL,
    KEY_NUMPAD_DECIMAL,
    KEY_NUMPAD_HEXADECIMAL,
    KEY_LEFT_CTRL = 224,
    KEY_LEFT_SHIFT,
    KEY_LEFT_ALT,
    KEY_LEFT_GUI,
    KEY_RIGHT_CTRL,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_ALT,
    KEY_RIGHT_GUI,

    // extended
    KEY_E_BEGIN = 1024,
    KEY_E_POWER,
    KEY_E_RESET,
    KEY_E_SLEEP,
    KEY_E_PLAY_PAUSE,
    KEY_E_STOP,
    KEY_E_NEXT_TRACK,
    KEY_E_PREV_TRACK,
    KEY_E_MUTE,
    KEY_E_VOL_INC,
    KEY_E_VOL_DEC,
    KEY_E_AL_WORD,
    KEY_E_AL_TEXT,
    KEY_E_AL_SPREADSHEET,
    KEY_E_AL_GRAPHIC,
    KEY_E_AL_PRESENTATION,
    KEY_E_AL_DATABASE,
    KEY_E_AL_EMAIL,
    KEY_E_AL_NEWS,
    KEY_E_AL_VOICE_MAIL,
    KEY_E_AL_ADDRESS_BOOK,
    KEY_E_AL_CALENDAR,
    KEY_E_AL_PROJECT,
    KEY_E_AL_JOURNAL,
    KEY_E_AL_FINANCE,
    KEY_E_AL_CALCULATOR,
    KEY_E_AL_AV_CAPTURE,
    KEY_E_AL_LOCAL_BROWSER,
    KEY_E_AL_LAN_WAN_BROWSER,
    KEY_E_AL_INTERNET_BROWSER,
    KEY_E_AL_ISP_CONNECT,
    KEY_E_AL_NETWORK_CONFERENCE,
    KEY_E_AL_NETWORK_CHAT,
    KEY_E_AL_TELEPHONY,
    KEY_E_AL_LOGON,
    KEY_E_AL_LOGOFF,
    KEY_E_AL_LOGON_LOGOFF,
    KEY_E_AL_TERMINAL_LOCK,
    KEY_E_AL_CONTROL_PANEL,
    KEY_E_AL_RUN,
    KEY_E_AL_TASK_MANAGER,
    KEY_E_AL_SELECT_TASK,
    KEY_E_AL_NEXT_TASK,
    KEY_E_AL_PREV_TASK,
    KEY_E_AL_PREEMT_TASK,
    KEY_E_AL_HELP_CENTER,
    KEY_E_AL_POWER_STATUS,
    KEY_E_AL_DOCUMENTS,
    KEY_E_AL_THESAURUS,
    KEY_E_AL_DICTIONARY,
    KEY_E_AL_DESKTOP,
    KEY_E_AL_SPELL_CHECK,
    KEY_E_AL_GRAMMAR_CHECK,
    KEY_E_AL_WIRELESS_STATUS,
    KEY_E_AL_KEYBOARD_LAYOUT,
    KEY_E_AL_VIRUS_PROTECTION,
    KEY_E_AL_ENCRYPTION,
    KEY_E_AL_SCREEN_SAVER,
    KEY_E_AL_ALARMS,
    KEY_E_AL_CLOCK,
    KEY_E_AL_FILE_BROWSER,
    KEY_E_AL_IMAGE_BROWSER,
    KEY_E_AL_AUDIO_BROWSER,
    KEY_E_AL_MOVIE_BROWSER,
    KEY_E_AL_DRM,
    KEY_E_AL_DIGITAL_WALLET,
    KEY_E_AL_INSTANT_MESSAGING,
    KEY_E_AL_OEM_FEATURES,
    KEY_E_AL_OEM_HELP,
    KEY_E_AL_ONLINE_COMMUNITY,
    KEY_E_AL_ENTERTAINMENT,
    KEY_E_AL_ONLINE_SHOPPING,
    KEY_E_AL_SMARTCARD,
    KEY_E_AL_MARKET_MONITOR,
    KEY_E_AL_CUSTOM_CORP_NEWS,
    KEY_E_AL_ONLINE_ACTIVITIES,
    KEY_E_AL_RESEARCH,
    KEY_E_AL_AUDIO_PLAYER,

    // private controls
    KEY_P_BEGIN = 2048,
    KEY_P_REBOOT,
    KEY_P_USB_BURN,
    KEY_P_NKRO_ON_OFF,
    KEY_P_NKRO,
    KEY_P_6KRO,
    KEY_P_RGB_ON_OFF,
    KEY_P_RGB_ON,
    KEY_P_RGB_OFF,
    KEY_P_RGB_BRIGHTNESS_UP,
    KEY_P_RGB_BRIGHTNESS_DOWN,
    KEY_P_RGB_MODE_SWITCH,
    KEY_P_RGB_MODE_NEXT,
    KEY_P_RGB_MODE_PREV,

} KeyCodes;

static_assert(KEY_EXSEL == 164, "KEY defination section 1 error");
static_assert(KEY_NUMPAD_HEXADECIMAL == 221, "KEY defination section 2 error");
static_assert(KEY_RIGHT_GUI == 231, "KEY defination section 3 error");

