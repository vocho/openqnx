/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



/*
 *
 *  keycodes.h   QNX keycodes
 *

 *
 */

#ifndef __KEYCODES_H_INCLUDED
#define __KEYCODES_H_INCLUDED

#ifndef UNICODE_BASIC_LATIN
#define UNICODE_BASIC_LATIN
#endif

#ifndef UNICODE_LATIN_1_SUPPLEMENT
#define UNICODE_LATIN_1_SUPPLEMENT
#endif

#ifndef UNICODE_COMBINING_DIACRITICAL_MARKS
#define UNICODE_COMBINING_DIACRITICAL_MARKS
#endif

#ifndef UNICODE_PRIVATE_USE_AREA
#define UNICODE_PRIVATE_USE_AREA
#endif

#ifndef _UNICODE_H_INCLUDED
 #include <unicode.h>
#endif

/*
 * Keyboard modifiers
 */
#define KEYMODBIT_SHIFT                                         0
#define KEYMODBIT_CTRL                                          1
#define KEYMODBIT_ALT                                           2
#define KEYMODBIT_ALTGR                                         3
#define KEYMODBIT_SHL3                                          4
#define KEYMODBIT_MOD6                                          5
#define KEYMODBIT_MOD7                                          6
#define KEYMODBIT_MOD8                                          7

#define KEYMODBIT_SHIFT_LOCK                                    8
#define KEYMODBIT_CTRL_LOCK                                     9
#define KEYMODBIT_ALT_LOCK                                      10
#define KEYMODBIT_ALTGR_LOCK                                    11
#define KEYMODBIT_SHL3_LOCK                                     12
#define KEYMODBIT_MOD6_LOCK                                     13
#define KEYMODBIT_MOD7_LOCK                                     14
#define KEYMODBIT_MOD8_LOCK                                     15

#define KEYMODBIT_CAPS_LOCK                                     16
#define KEYMODBIT_NUM_LOCK                                      17
#define KEYMODBIT_SCROLL_LOCK                                   18

#define KEYMOD_SHIFT                                            (1 << KEYMODBIT_SHIFT)
#define KEYMOD_CTRL                                             (1 << KEYMODBIT_CTRL)
#define KEYMOD_ALT                                              (1 << KEYMODBIT_ALT)
#define KEYMOD_ALTGR                                            (1 << KEYMODBIT_ALTGR)
#define KEYMOD_SHL3                                             (1 << KEYMODBIT_SHL3)
#define KEYMOD_MOD6                                             (1 << KEYMODBIT_MOD6)
#define KEYMOD_MOD7                                             (1 << KEYMODBIT_MOD7)
#define KEYMOD_MOD8                                             (1 << KEYMODBIT_MOD8)

#define KEYMOD_SHIFT_LOCK                                       (1 << KEYMODBIT_SHIFT_LOCK)
#define KEYMOD_CTRL_LOCK                                        (1 << KEYMODBIT_CTRL_LOCK)
#define KEYMOD_ALT_LOCK                                         (1 << KEYMODBIT_ALT_LOCK)
#define KEYMOD_ALTGR_LOCK                                       (1 << KEYMODBIT_ALTGR_LOCK)
#define KEYMOD_SHL3_LOCK                                        (1 << KEYMODBIT_SHL3_LOCK)
#define KEYMOD_MOD6_LOCK                                        (1 << KEYMODBIT_MOD6_LOCK)
#define KEYMOD_MOD7_LOCK                                        (1 << KEYMODBIT_MOD7_LOCK)
#define KEYMOD_MOD8_LOCK                                        (1 << KEYMODBIT_MOD8_LOCK)

#define KEYMOD_CAPS_LOCK                                        (1 << KEYMODBIT_CAPS_LOCK)
#define KEYMOD_NUM_LOCK                                         (1 << KEYMODBIT_NUM_LOCK)
#define KEYMOD_SCROLL_LOCK                                      (1 << KEYMODBIT_SCROLL_LOCK)

/*
 * Keyboard flags
 */
#define KEY_DOWN                                                0x00000001      /* Key was pressed down */
#define KEY_REPEAT                                              0x00000002      /* Key was repeated */
#define KEY_SCAN_VALID                                          0x00000020      /* Scancode is valid */
#define KEY_SYM_VALID                                           0x00000040      /* Key symbol is valid */
#define KEY_CAP_VALID                                           0x00000080      /* Key cap is valid */
#define KEY_DEAD                                                0x40000000      /* Key symbol is a DEAD key */
#define KEY_OEM_CAP                                             0x80000000      /* Key cap is an OEM scan code from keyboard */

/*
 * Keyboard Indicators
 */
#define KEYIND_SCROLL_LOCK                                      0x01
#define KEYIND_NUM_LOCK                                         0x02
#define KEYIND_CAPS_LOCK                                        0x04


/*
 * Keyboard codes for Key caps and Key symbols
 */
#define KEYCODE_PC_KEYS                                         UNICODE_PRIVATE_USE_AREA_FIRST + 0x1000

#define KEYCODE_PAUSE                                           (KEYCODE_PC_KEYS + 0x13)
#define KEYCODE_SCROLL_LOCK                                     (KEYCODE_PC_KEYS + 0x14)
#define KEYCODE_PRINT                                           (KEYCODE_PC_KEYS + 0x61)
#define KEYCODE_SYSREQ                                          (KEYCODE_PC_KEYS + 0x6a)
#define KEYCODE_BREAK                                           (KEYCODE_PC_KEYS + 0x6b)

#define KEYCODE_ESCAPE                                          (KEYCODE_PC_KEYS + 0x1b)
#define KEYCODE_BACKSPACE                                       (KEYCODE_PC_KEYS + 0x08)
#define KEYCODE_TAB                                             (KEYCODE_PC_KEYS + 0x09)
#define KEYCODE_BACK_TAB                                        (KEYCODE_PC_KEYS + 0x89)
#define KEYCODE_RETURN                                          (KEYCODE_PC_KEYS + 0x0d)
#define KEYCODE_CAPS_LOCK                                       (KEYCODE_PC_KEYS + 0xe5)
#define KEYCODE_LEFT_SHIFT                                      (KEYCODE_PC_KEYS + 0xe1)
#define KEYCODE_RIGHT_SHIFT                                     (KEYCODE_PC_KEYS + 0xe2)
#define KEYCODE_LEFT_CTRL                                       (KEYCODE_PC_KEYS + 0xe3)
#define KEYCODE_RIGHT_CTRL                                      (KEYCODE_PC_KEYS + 0xe4)
#define KEYCODE_LEFT_ALT                                        (KEYCODE_PC_KEYS + 0xe9)
#define KEYCODE_RIGHT_ALT                                       (KEYCODE_PC_KEYS + 0xea)
#define KEYCODE_MENU                                            (KEYCODE_PC_KEYS + 0x67)
#define KEYCODE_LEFT_HYPER                                      (KEYCODE_PC_KEYS + 0xed)
#define KEYCODE_RIGHT_HYPER                                     (KEYCODE_PC_KEYS + 0xee)

#define KEYCODE_INSERT                                          (KEYCODE_PC_KEYS + 0x63)
#define KEYCODE_HOME                                            (KEYCODE_PC_KEYS + 0x50)
#define KEYCODE_PG_UP                                           (KEYCODE_PC_KEYS + 0x55)
#define KEYCODE_DELETE                                          (KEYCODE_PC_KEYS + 0xff)
#define KEYCODE_END                                             (KEYCODE_PC_KEYS + 0x57)
#define KEYCODE_PG_DOWN                                         (KEYCODE_PC_KEYS + 0x56)
#define KEYCODE_LEFT                                            (KEYCODE_PC_KEYS + 0x51)
#define KEYCODE_RIGHT                                           (KEYCODE_PC_KEYS + 0x53)
#define KEYCODE_UP                                              (KEYCODE_PC_KEYS + 0x52)
#define KEYCODE_DOWN                                            (KEYCODE_PC_KEYS + 0x54)

#define KEYCODE_NUM_LOCK                                        (KEYCODE_PC_KEYS + 0x7f)
#define KEYCODE_KP_PLUS                                         (KEYCODE_PC_KEYS + 0xab)
#define KEYCODE_KP_MINUS                                        (KEYCODE_PC_KEYS + 0xad)
#define KEYCODE_KP_MULTIPLY                                     (KEYCODE_PC_KEYS + 0xaa)
#define KEYCODE_KP_DIVIDE                                       (KEYCODE_PC_KEYS + 0xaf)
#define KEYCODE_KP_ENTER                                        (KEYCODE_PC_KEYS + 0x8d)
#define KEYCODE_KP_HOME                                         (KEYCODE_PC_KEYS + 0xb7)
#define KEYCODE_KP_UP                                           (KEYCODE_PC_KEYS + 0xb8)
#define KEYCODE_KP_PG_UP                                        (KEYCODE_PC_KEYS + 0xb9)
#define KEYCODE_KP_LEFT                                         (KEYCODE_PC_KEYS + 0xb4)
#define KEYCODE_KP_FIVE                                         (KEYCODE_PC_KEYS + 0xb5)
#define KEYCODE_KP_RIGHT                                        (KEYCODE_PC_KEYS + 0xb6)
#define KEYCODE_KP_END                                          (KEYCODE_PC_KEYS + 0xb1)
#define KEYCODE_KP_DOWN                                         (KEYCODE_PC_KEYS + 0xb2)
#define KEYCODE_KP_PG_DOWN                                      (KEYCODE_PC_KEYS + 0xb3)
#define KEYCODE_KP_INSERT                                       (KEYCODE_PC_KEYS + 0xb0)
#define KEYCODE_KP_DELETE                                       (KEYCODE_PC_KEYS + 0xae)

#define KEYCODE_F1                                              (KEYCODE_PC_KEYS + 0xbe)
#define KEYCODE_F2                                              (KEYCODE_PC_KEYS + 0xbf)
#define KEYCODE_F3                                              (KEYCODE_PC_KEYS + 0xc0)
#define KEYCODE_F4                                              (KEYCODE_PC_KEYS + 0xc1)
#define KEYCODE_F5                                              (KEYCODE_PC_KEYS + 0xc2)
#define KEYCODE_F6                                              (KEYCODE_PC_KEYS + 0xc3)
#define KEYCODE_F7                                              (KEYCODE_PC_KEYS + 0xc4)
#define KEYCODE_F8                                              (KEYCODE_PC_KEYS + 0xc5)
#define KEYCODE_F9                                              (KEYCODE_PC_KEYS + 0xc6)
#define KEYCODE_F10                                             (KEYCODE_PC_KEYS + 0xc7)
#define KEYCODE_F11                                             (KEYCODE_PC_KEYS + 0xc8)
#define KEYCODE_F12                                             (KEYCODE_PC_KEYS + 0xc9)

#define KEYCODE_SPACE                                           UCS_SPACE
#define KEYCODE_EXCLAM                                          UCS_EXCLAMATION_MARK
#define KEYCODE_QUOTE                                           UCS_QUOTATION_MARK
#define KEYCODE_NUMBER                                          UCS_NUMBER_SIGN
#define KEYCODE_DOLLAR                                          UCS_DOLLAR_SIGN
#define KEYCODE_PERCENT                                         UCS_PERCENT_SIGN
#define KEYCODE_AMPERSAND                                       UCS_AMPERSAND
#define KEYCODE_APOSTROPHE                                      UCS_APOSTROPHE
#define KEYCODE_LEFT_PAREN                                      UCS_LEFT_PARENTHESIS
#define KEYCODE_RIGHT_PAREN                                     UCS_RIGHT_PARENTHESIS
#define KEYCODE_ASTERISK                                        UCS_ASTERISK
#define KEYCODE_PLUS                                            UCS_PLUS_SIGN
#define KEYCODE_COMMA                                           UCS_COMMA
#define KEYCODE_MINUS                                           UCS_HYPHEN_MINUS
#define KEYCODE_PERIOD                                          UCS_FULL_STOP
#define KEYCODE_SLASH                                           UCS_SOLIDUS
#define KEYCODE_ZERO                                            UCS_DIGIT_ZERO
#define KEYCODE_ONE                                             UCS_DIGIT_ONE
#define KEYCODE_TWO                                             UCS_DIGIT_TWO
#define KEYCODE_THREE                                           UCS_DIGIT_THREE
#define KEYCODE_FOUR                                            UCS_DIGIT_FOUR
#define KEYCODE_FIVE                                            UCS_DIGIT_FIVE
#define KEYCODE_SIX                                             UCS_DIGIT_SIX
#define KEYCODE_SEVEN                                           UCS_DIGIT_SEVEN
#define KEYCODE_EIGHT                                           UCS_DIGIT_EIGHT
#define KEYCODE_NINE                                            UCS_DIGIT_NINE
#define KEYCODE_COLON                                           UCS_COLON
#define KEYCODE_SEMICOLON                                       UCS_SEMICOLON
#define KEYCODE_LESS_THAN                                       UCS_LESS_THAN_SIGN
#define KEYCODE_EQUAL                                           UCS_EQUALS_SIGN
#define KEYCODE_GREATER_THAN                                    UCS_GREATER_THAN_SIGN
#define KEYCODE_QUESTION                                        UCS_QUESTION_MARK
#define KEYCODE_AT                                              UCS_COMMERCIAL_AT
#define KEYCODE_CAPITAL_A                                       UCS_LATIN_CAPITAL_LETTER_A
#define KEYCODE_CAPITAL_B                                       UCS_LATIN_CAPITAL_LETTER_B
#define KEYCODE_CAPITAL_C                                       UCS_LATIN_CAPITAL_LETTER_C
#define KEYCODE_CAPITAL_D                                       UCS_LATIN_CAPITAL_LETTER_D
#define KEYCODE_CAPITAL_E                                       UCS_LATIN_CAPITAL_LETTER_E
#define KEYCODE_CAPITAL_F                                       UCS_LATIN_CAPITAL_LETTER_F
#define KEYCODE_CAPITAL_G                                       UCS_LATIN_CAPITAL_LETTER_G
#define KEYCODE_CAPITAL_H                                       UCS_LATIN_CAPITAL_LETTER_H
#define KEYCODE_CAPITAL_I                                       UCS_LATIN_CAPITAL_LETTER_I
#define KEYCODE_CAPITAL_J                                       UCS_LATIN_CAPITAL_LETTER_J
#define KEYCODE_CAPITAL_K                                       UCS_LATIN_CAPITAL_LETTER_K
#define KEYCODE_CAPITAL_L                                       UCS_LATIN_CAPITAL_LETTER_L
#define KEYCODE_CAPITAL_M                                       UCS_LATIN_CAPITAL_LETTER_M
#define KEYCODE_CAPITAL_N                                       UCS_LATIN_CAPITAL_LETTER_N
#define KEYCODE_CAPITAL_O                                       UCS_LATIN_CAPITAL_LETTER_O
#define KEYCODE_CAPITAL_P                                       UCS_LATIN_CAPITAL_LETTER_P
#define KEYCODE_CAPITAL_Q                                       UCS_LATIN_CAPITAL_LETTER_Q
#define KEYCODE_CAPITAL_R                                       UCS_LATIN_CAPITAL_LETTER_R
#define KEYCODE_CAPITAL_S                                       UCS_LATIN_CAPITAL_LETTER_S
#define KEYCODE_CAPITAL_T                                       UCS_LATIN_CAPITAL_LETTER_T
#define KEYCODE_CAPITAL_U                                       UCS_LATIN_CAPITAL_LETTER_U
#define KEYCODE_CAPITAL_V                                       UCS_LATIN_CAPITAL_LETTER_V
#define KEYCODE_CAPITAL_W                                       UCS_LATIN_CAPITAL_LETTER_W
#define KEYCODE_CAPITAL_X                                       UCS_LATIN_CAPITAL_LETTER_X
#define KEYCODE_CAPITAL_Y                                       UCS_LATIN_CAPITAL_LETTER_Y
#define KEYCODE_CAPITAL_Z                                       UCS_LATIN_CAPITAL_LETTER_Z
#define KEYCODE_LEFT_BRACKET                                    UCS_LEFT_SQUARE_BRACKET
#define KEYCODE_BACK_SLASH                                      UCS_REVERSE_SOLIDUS
#define KEYCODE_RIGHT_BRACKET                                   UCS_RIGHT_SQUARE_BRACKET
#define KEYCODE_CIRCUMFLEX                                      UCS_CIRCUMFLEX_ACCENT
#define KEYCODE_UNDERSCORE                                      UCS_LOW_LINE
#define KEYCODE_GRAVE                                           UCS_GRAVE_ACCENT
#define KEYCODE_A                                               UCS_LATIN_SMALL_LETTER_A
#define KEYCODE_B                                               UCS_LATIN_SMALL_LETTER_B
#define KEYCODE_C                                               UCS_LATIN_SMALL_LETTER_C
#define KEYCODE_D                                               UCS_LATIN_SMALL_LETTER_D
#define KEYCODE_E                                               UCS_LATIN_SMALL_LETTER_E
#define KEYCODE_F                                               UCS_LATIN_SMALL_LETTER_F
#define KEYCODE_G                                               UCS_LATIN_SMALL_LETTER_G
#define KEYCODE_H                                               UCS_LATIN_SMALL_LETTER_H
#define KEYCODE_I                                               UCS_LATIN_SMALL_LETTER_I
#define KEYCODE_J                                               UCS_LATIN_SMALL_LETTER_J
#define KEYCODE_K                                               UCS_LATIN_SMALL_LETTER_K
#define KEYCODE_L                                               UCS_LATIN_SMALL_LETTER_L
#define KEYCODE_M                                               UCS_LATIN_SMALL_LETTER_M
#define KEYCODE_N                                               UCS_LATIN_SMALL_LETTER_N
#define KEYCODE_O                                               UCS_LATIN_SMALL_LETTER_O
#define KEYCODE_P                                               UCS_LATIN_SMALL_LETTER_P
#define KEYCODE_Q                                               UCS_LATIN_SMALL_LETTER_Q
#define KEYCODE_R                                               UCS_LATIN_SMALL_LETTER_R
#define KEYCODE_S                                               UCS_LATIN_SMALL_LETTER_S
#define KEYCODE_T                                               UCS_LATIN_SMALL_LETTER_T
#define KEYCODE_U                                               UCS_LATIN_SMALL_LETTER_U
#define KEYCODE_V                                               UCS_LATIN_SMALL_LETTER_V
#define KEYCODE_W                                               UCS_LATIN_SMALL_LETTER_W
#define KEYCODE_X                                               UCS_LATIN_SMALL_LETTER_X
#define KEYCODE_Y                                               UCS_LATIN_SMALL_LETTER_Y
#define KEYCODE_Z                                               UCS_LATIN_SMALL_LETTER_Z
#define KEYCODE_LEFT_BRACE                                      UCS_LEFT_CURLY_BRACKET
#define KEYCODE_BAR                                             UCS_VERTICAL_LINE
#define KEYCODE_RIGHT_BRACE                                     UCS_RIGHT_CURLY_BRACKET
#define KEYCODE_TILDE                                           UCS_TILDE

#define KEYCODE_NO_BREAK_SPACE                                  UCS_NO_BREAK_SPACE
#define KEYCODE_INVERTED_EXCLAMATION_MARK                       UCS_INVERTED_EXCLAMATION_MARK
#define KEYCODE_CENT_SIGN                                       UCS_CENT_SIGN
#define KEYCODE_POUND_SIGN                                      UCS_POUND_SIGN
#define KEYCODE_CURRENCY_SIGN                                   UCS_CURRENCY_SIGN
#define KEYCODE_YEN_SIGN                                        UCS_YEN_SIGN
#define KEYCODE_BROKEN_BAR                                      UCS_BROKEN_BAR
#define KEYCODE_SECTION_SIGN                                    UCS_SECTION_SIGN
#define KEYCODE_DIAERESIS                                       UCS_DIAERESIS
#define KEYCODE_COPYRIGHT_SIGN                                  UCS_COPYRIGHT_SIGN
#define KEYCODE_FEMININE_ORDINAL_INDICATOR                      UCS_FEMININE_ORDINAL_INDICATOR
#define KEYCODE_LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK       UCS_LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK
#define KEYCODE_NOT_SIGN                                        UCS_NOT_SIGN
#define KEYCODE_SOFT_HYPHEN                                     UCS_SOFT_HYPHEN
#define KEYCODE_REGISTERED_SIGN                                 UCS_REGISTERED_SIGN
#define KEYCODE_MACRON                                          UCS_MACRON
#define KEYCODE_DEGREE_SIGN                                     UCS_DEGREE_SIGN
#define KEYCODE_PLUS_MINUS_SIGN                                 UCS_PLUS_MINUS_SIGN
#define KEYCODE_SUPERSCRIPT_TWO                                 UCS_SUPERSCRIPT_TWO
#define KEYCODE_SUPERSCRIPT_THREE                               UCS_SUPERSCRIPT_THREE
#define KEYCODE_ACUTE_ACCENT                                    UCS_ACUTE_ACCENT
#define KEYCODE_MICRO_SIGN                                      UCS_MICRO_SIGN
#define KEYCODE_PILCROW_SIGN                                    UCS_PILCROW_SIGN
#define KEYCODE_MIDDLE_DOT                                      UCS_MIDDLE_DOT
#define KEYCODE_CEDILLA                                         UCS_CEDILLA
#define KEYCODE_SUPERSCRIPT_ONE                                 UCS_SUPERSCRIPT_ONE
#define KEYCODE_MASCULINE_ORDINAL_INDICATOR                     UCS_MASCULINE_ORDINAL_INDICATOR
#define KEYCODE_RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK      UCS_RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK
#define KEYCODE_VULGAR_FRACTION_ONE_QUARTER                     UCS_VULGAR_FRACTION_ONE_QUARTER
#define KEYCODE_VULGAR_FRACTION_ONE_HALF                        UCS_VULGAR_FRACTION_ONE_HALF
#define KEYCODE_VULGAR_FRACTION_THREE_QUARTERS                  UCS_VULGAR_FRACTION_THREE_QUARTERS
#define KEYCODE_INVERTED_QUESTION_MARK                          UCS_INVERTED_QUESTION_MARK
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_GRAVE               UCS_LATIN_CAPITAL_LETTER_A_WITH_GRAVE
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_A_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX          UCS_LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_TILDE               UCS_LATIN_CAPITAL_LETTER_A_WITH_TILDE
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS           UCS_LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS
#define KEYCODE_LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE          UCS_LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE
#define KEYCODE_LATIN_CAPITAL_LIGATURE_AE                       UCS_LATIN_CAPITAL_LIGATURE_AE
#define KEYCODE_LATIN_CAPITAL_LETTER_C_WITH_CEDILLA             UCS_LATIN_CAPITAL_LETTER_C_WITH_CEDILLA
#define KEYCODE_LATIN_CAPITAL_LETTER_E_WITH_GRAVE               UCS_LATIN_CAPITAL_LETTER_E_WITH_GRAVE
#define KEYCODE_LATIN_CAPITAL_LETTER_E_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_E_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX          UCS_LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS           UCS_LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS
#define KEYCODE_LATIN_CAPITAL_LETTER_I_WITH_GRAVE               UCS_LATIN_CAPITAL_LETTER_I_WITH_GRAVE
#define KEYCODE_LATIN_CAPITAL_LETTER_I_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_I_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX          UCS_LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS           UCS_LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS
#define KEYCODE_LATIN_CAPITAL_LETTER_ETH                        UCS_LATIN_CAPITAL_LETTER_ETH
#define KEYCODE_LATIN_CAPITAL_LETTER_N_WITH_TILDE               UCS_LATIN_CAPITAL_LETTER_N_WITH_TILDE
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_GRAVE               UCS_LATIN_CAPITAL_LETTER_O_WITH_GRAVE
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_O_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX          UCS_LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_TILDE               UCS_LATIN_CAPITAL_LETTER_O_WITH_TILDE
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS           UCS_LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS
#define KEYCODE_MULTIPLICATION_SIGN                             UCS_MULTIPLICATION_SIGN
#define KEYCODE_LATIN_CAPITAL_LETTER_O_WITH_STROKE              UCS_LATIN_CAPITAL_LETTER_O_WITH_STROKE
#define KEYCODE_LATIN_CAPITAL_LETTER_U_WITH_GRAVE               UCS_LATIN_CAPITAL_LETTER_U_WITH_GRAVE
#define KEYCODE_LATIN_CAPITAL_LETTER_U_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_U_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX          UCS_LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS           UCS_LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS
#define KEYCODE_LATIN_CAPITAL_LETTER_Y_WITH_ACUTE               UCS_LATIN_CAPITAL_LETTER_Y_WITH_ACUTE
#define KEYCODE_LATIN_CAPITAL_LETTER_THORN                      UCS_LATIN_CAPITAL_LETTER_THORN
#define KEYCODE_LATIN_SMALL_LETTER_SHARP_S                      UCS_LATIN_SMALL_LETTER_SHARP_S
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_GRAVE                 UCS_LATIN_SMALL_LETTER_A_WITH_GRAVE
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_A_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX            UCS_LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_TILDE                 UCS_LATIN_SMALL_LETTER_A_WITH_TILDE
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_A_WITH_DIAERESIS
#define KEYCODE_LATIN_SMALL_LETTER_A_WITH_RING_ABOVE            UCS_LATIN_SMALL_LETTER_A_WITH_RING_ABOVE
#define KEYCODE_LATIN_SMALL_LIGATURE_AE                         UCS_LATIN_SMALL_LIGATURE_AE
#define KEYCODE_LATIN_SMALL_LETTER_C_WITH_CEDILLA               UCS_LATIN_SMALL_LETTER_C_WITH_CEDILLA
#define KEYCODE_LATIN_SMALL_LETTER_E_WITH_GRAVE                 UCS_LATIN_SMALL_LETTER_E_WITH_GRAVE
#define KEYCODE_LATIN_SMALL_LETTER_E_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_E_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX            UCS_LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_SMALL_LETTER_E_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_E_WITH_DIAERESIS
#define KEYCODE_LATIN_SMALL_LETTER_I_WITH_GRAVE                 UCS_LATIN_SMALL_LETTER_I_WITH_GRAVE
#define KEYCODE_LATIN_SMALL_LETTER_I_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_I_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX            UCS_LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_SMALL_LETTER_I_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_I_WITH_DIAERESIS
#define KEYCODE_LATIN_SMALL_LETTER_ETH                          UCS_LATIN_SMALL_LETTER_ETH
#define KEYCODE_LATIN_SMALL_LETTER_N_WITH_TILDE                 UCS_LATIN_SMALL_LETTER_N_WITH_TILDE
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_GRAVE                 UCS_LATIN_SMALL_LETTER_O_WITH_GRAVE
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_O_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX            UCS_LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_TILDE                 UCS_LATIN_SMALL_LETTER_O_WITH_TILDE
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_O_WITH_DIAERESIS
#define KEYCODE_DIVISION_SIGN                                   UCS_DIVISION_SIGN
#define KEYCODE_LATIN_SMALL_LETTER_O_WITH_STROKE                UCS_LATIN_SMALL_LETTER_O_WITH_STROKE
#define KEYCODE_LATIN_SMALL_LETTER_U_WITH_GRAVE                 UCS_LATIN_SMALL_LETTER_U_WITH_GRAVE
#define KEYCODE_LATIN_SMALL_LETTER_U_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_U_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX            UCS_LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX
#define KEYCODE_LATIN_SMALL_LETTER_U_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_U_WITH_DIAERESIS
#define KEYCODE_LATIN_SMALL_LETTER_Y_WITH_ACUTE                 UCS_LATIN_SMALL_LETTER_Y_WITH_ACUTE
#define KEYCODE_LATIN_SMALL_LETTER_THORN                        UCS_LATIN_SMALL_LETTER_THORN
#define KEYCODE_LATIN_SMALL_LETTER_Y_WITH_DIAERESIS             UCS_LATIN_SMALL_LETTER_Y_WITH_DIAERESIS

#define KEYCODE_COMBINING_GRAVE_ACCENT                          UCS_COMBINING_GRAVE_ACCENT
#define KEYCODE_COMBINING_ACUTE_ACCENT                          UCS_COMBINING_ACUTE_ACCENT
#define KEYCODE_COMBINING_CIRCUMFLEX_ACCENT                     UCS_COMBINING_CIRCUMFLEX_ACCENT
#define KEYCODE_COMBINING_TILDE                                 UCS_COMBINING_TILDE
#define KEYCODE_COMBINING_MACRON                                UCS_COMBINING_MACRON
#define KEYCODE_COMBINING_OVERLINE                              UCS_COMBINING_OVERLINE
#define KEYCODE_COMBINING_BREVE                                 UCS_COMBINING_BREVE
#define KEYCODE_COMBINING_DOT_ABOVE                             UCS_COMBINING_DOT_ABOVE
#define KEYCODE_COMBINING_DIAERESIS                             UCS_COMBINING_DIAERESIS
#define KEYCODE_COMBINING_HOOK_ABOVE                            UCS_COMBINING_HOOK_ABOVE
#define KEYCODE_COMBINING_RING_ABOVE                            UCS_COMBINING_RING_ABOVE
#define KEYCODE_COMBINING_DOUBLE_ACUTE_ACCENT                   UCS_COMBINING_DOUBLE_ACUTE_ACCENT
#define KEYCODE_COMBINING_CARON                                 UCS_COMBINING_CARON
#define KEYCODE_COMBINING_VERTICAL_LINE_ABOVE                   UCS_COMBINING_VERTICAL_LINE_ABOVE
#define KEYCODE_COMBINING_DOUBLE_VERTICAL_LINE_ABOVE            UCS_COMBINING_DOUBLE_VERTICAL_LINE_ABOVE
#define KEYCODE_COMBINING_DOUBLE_GRAVE_ACCENT                   UCS_COMBINING_DOUBLE_GRAVE_ACCENT
#define KEYCODE_COMBINING_CANDRABINDU                           UCS_COMBINING_CANDRABINDU
#define KEYCODE_COMBINING_INVERTED_BREVE                        UCS_COMBINING_INVERTED_BREVE
#define KEYCODE_COMBINING_TURNED_COMMA_ABOVE                    UCS_COMBINING_TURNED_COMMA_ABOVE
#define KEYCODE_COMBINING_COMMA_ABOVE                           UCS_COMBINING_COMMA_ABOVE
#define KEYCODE_COMBINING_REVERSED_COMMA_ABOVE                  UCS_COMBINING_REVERSED_COMMA_ABOVE
#define KEYCODE_COMBINING_COMMA_ABOVE_RIGHT                     UCS_COMBINING_COMMA_ABOVE_RIGHT
#define KEYCODE_COMBINING_GRAVE_ACCENT_BELOW                    UCS_COMBINING_GRAVE_ACCENT_BELOW
#define KEYCODE_COMBINING_ACUTE_ACCENT_BELOW                    UCS_COMBINING_ACUTE_ACCENT_BELOW
#define KEYCODE_COMBINING_LEFT_TACK_BELOW                       UCS_COMBINING_LEFT_TACK_BELOW
#define KEYCODE_COMBINING_RIGHT_TACK_BELOW                      UCS_COMBINING_RIGHT_TACK_BELOW
#define KEYCODE_COMBINING_LEFT_ANGLE_BELOW                      UCS_COMBINING_LEFT_ANGLE_BELOW
#define KEYCODE_COMBINING_HORN                                  UCS_COMBINING_HORN
#define KEYCODE_COMBINING_LEFT_HALF_RING_BELOW                  UCS_COMBINING_LEFT_HALF_RING_BELOW
#define KEYCODE_COMBINING_UP_TACK_BELOW                         UCS_COMBINING_UP_TACK_BELOW
#define KEYCODE_COMBINING_DOWN_TACK_BELOW                       UCS_COMBINING_DOWN_TACK_BELOW
#define KEYCODE_COMBINING_PLUS_SIGN_BELOW                       UCS_COMBINING_PLUS_SIGN_BELOW
#define KEYCODE_COMBINING_MINUS_SIGN_BELOW                      UCS_COMBINING_MINUS_SIGN_BELOW
#define KEYCODE_COMBINING_PALATALIZED_HOOK_BELOW                UCS_COMBINING_PALATALIZED_HOOK_BELOW
#define KEYCODE_COMBINING_RETROFLEX_HOOK_BELOW                  UCS_COMBINING_RETROFLEX_HOOK_BELOW
#define KEYCODE_COMBINING_DOT_BELOW                             UCS_COMBINING_DOT_BELOW
#define KEYCODE_COMBINING_DIAERESIS_BELOW                       UCS_COMBINING_DIAERESIS_BELOW
#define KEYCODE_COMBINING_RING_BELOW                            UCS_COMBINING_RING_BELOW
#define KEYCODE_COMBINING_COMMA_BELOW                           UCS_COMBINING_COMMA_BELOW
#define KEYCODE_COMBINING_CEDILLA                               UCS_COMBINING_CEDILLA
#define KEYCODE_COMBINING_OGONEK                                UCS_COMBINING_OGONEK
#define KEYCODE_COMBINING_VERTICAL_LINE_BELOW                   UCS_COMBINING_VERTICAL_LINE_BELOW
#define KEYCODE_COMBINING_BRIDGE_BELOW                          UCS_COMBINING_BRIDGE_BELOW
#define KEYCODE_COMBINING_INVERTED_DOUBLE_ARCH_BELOW            UCS_COMBINING_INVERTED_DOUBLE_ARCH_BELOW
#define KEYCODE_COMBINING_CARON_BELOW                           UCS_COMBINING_CARON_BELOW
#define KEYCODE_COMBINING_CIRCUMFLEX_ACCENT_BELOW               UCS_COMBINING_CIRCUMFLEX_ACCENT_BELOW
#define KEYCODE_COMBINING_BREVE_BELOW                           UCS_COMBINING_BREVE_BELOW
#define KEYCODE_COMBINING_INVERTED_BREVE_BELOW                  UCS_COMBINING_INVERTED_BREVE_BELOW
#define KEYCODE_COMBINING_TILDE_BELOW                           UCS_COMBINING_TILDE_BELOW
#define KEYCODE_COMBINING_MACRON_BELOW                          UCS_COMBINING_MACRON_BELOW
#define KEYCODE_COMBINING_LOW_LINE                              UCS_COMBINING_LOW_LINE
#define KEYCODE_COMBINING_DOUBLE_LOW_LINE                       UCS_COMBINING_DOUBLE_LOW_LINE
#define KEYCODE_COMBINING_TILDE_OVERLAY                         UCS_COMBINING_TILDE_OVERLAY
#define KEYCODE_COMBINING_SHORT_STROKE_OVERLAY                  UCS_COMBINING_SHORT_STROKE_OVERLAY
#define KEYCODE_COMBINING_LONG_STROKE_OVERLAY                   UCS_COMBINING_LONG_STROKE_OVERLAY
#define KEYCODE_COMBINING_SHORT_SOLIDUS_OVERLAY                 UCS_COMBINING_SHORT_SOLIDUS_OVERLAY
#define KEYCODE_COMBINING_LONG_SOLIDUS_OVERLAY                  UCS_COMBINING_LONG_SOLIDUS_OVERLAY
#define KEYCODE_COMBINING_RIGHT_HALF_RING_BELOW                 UCS_COMBINING_RIGHT_HALF_RING_BELOW
#define KEYCODE_COMBINING_INVERTED_BRIDGE_BELOW                 UCS_COMBINING_INVERTED_BRIDGE_BELOW
#define KEYCODE_COMBINING_SQUARE_BELOW                          UCS_COMBINING_SQUARE_BELOW
#define KEYCODE_COMBINING_SEAGULL_BELOW                         UCS_COMBINING_SEAGULL_BELOW
#define KEYCODE_COMBINING_X_ABOVE                               UCS_COMBINING_X_ABOVE
#define KEYCODE_COMBINING_VERTICAL_TILDE                        UCS_COMBINING_VERTICAL_TILDE
#define KEYCODE_COMBINING_DOUBLE_OVERLINE                       UCS_COMBINING_DOUBLE_OVERLINE
#define KEYCODE_COMBINING_GRAVE_TONE_MARK                       UCS_COMBINING_GRAVE_TONE_MARK
#define KEYCODE_COMBINING_ACUTE_TONE_MARK                       UCS_COMBINING_ACUTE_TONE_MARK
#define KEYCODE_COMBINING_GREEK_PEROSPOMENI                     UCS_COMBINING_GREEK_PEROSPOMENI
#define KEYCODE_COMBINING_GREEK_KORONIS                         UCS_COMBINING_GREEK_KORONIS
#define KEYCODE_COMBINING_GREEK_DIALYTIKA                       UCS_COMBINING_GREEK_DIALYTIKA
#define KEYCODE_COMBINING_GREEK_YPOGEGRAMMENI                   UCS_COMBINING_GREEK_YPOGEGRAMMENI
#define KEYCODE_COMBINING_DOUBLE_TILDE                          UCS_COMBINING_DOUBLE_TILDE
#define KEYCODE_COMBINING_DOUBLE_INVERTED_BREVE                 UCS_COMBINING_DOUBLE_INVERTED_BREVE

#endif
