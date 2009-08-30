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
 *  unicode.h    Universal Multiple-Octet Coded Character Set 
 *               ISO/IEC 10646-1:1993(E)
 *

 *  
 */

#ifndef _UNICODE_H_INCLUDED
#define _UNICODE_H_INCLUDED

#ifdef UNICODE_BASIC_LATIN
#define UNICODE_BASIC_LATIN_FIRST                           0x0020
#define UNICODE_BASIC_LATIN_LAST                            0x007e
#define UCS_SPACE                                           0x0020
#define UCS_EXCLAMATION_MARK                                0x0021
#define UCS_QUOTATION_MARK                                  0x0022
#define UCS_NUMBER_SIGN                                     0x0023
#define UCS_DOLLAR_SIGN                                     0x0024
#define UCS_PERCENT_SIGN                                    0x0025
#define UCS_AMPERSAND                                       0x0026
#define UCS_APOSTROPHE                                      0x0027
#define UCS_LEFT_PARENTHESIS                                0x0028
#define UCS_RIGHT_PARENTHESIS                               0x0029
#define UCS_ASTERISK                                        0x002a
#define UCS_PLUS_SIGN                                       0x002b
#define UCS_COMMA                                           0x002c
#define UCS_HYPHEN_MINUS                                    0x002d
#define UCS_FULL_STOP                                       0x002e
#define UCS_SOLIDUS                                         0x002f
#define UCS_DIGIT_ZERO                                      0x0030
#define UCS_DIGIT_ONE                                       0x0031
#define UCS_DIGIT_TWO                                       0x0032
#define UCS_DIGIT_THREE                                     0x0033
#define UCS_DIGIT_FOUR                                      0x0034
#define UCS_DIGIT_FIVE                                      0x0035
#define UCS_DIGIT_SIX                                       0x0036
#define UCS_DIGIT_SEVEN                                     0x0037
#define UCS_DIGIT_EIGHT                                     0x0038
#define UCS_DIGIT_NINE                                      0x0039
#define UCS_COLON                                           0x003a
#define UCS_SEMICOLON                                       0x003b
#define UCS_LESS_THAN_SIGN                                  0x003c
#define UCS_EQUALS_SIGN                                     0x003d
#define UCS_GREATER_THAN_SIGN                               0x003e
#define UCS_QUESTION_MARK                                   0x003f
#define UCS_COMMERCIAL_AT                                   0x0040
#define UCS_LATIN_CAPITAL_LETTER_A                          0x0041
#define UCS_LATIN_CAPITAL_LETTER_B                          0x0042
#define UCS_LATIN_CAPITAL_LETTER_C                          0x0043
#define UCS_LATIN_CAPITAL_LETTER_D                          0x0044
#define UCS_LATIN_CAPITAL_LETTER_E                          0x0045
#define UCS_LATIN_CAPITAL_LETTER_F                          0x0046
#define UCS_LATIN_CAPITAL_LETTER_G                          0x0047
#define UCS_LATIN_CAPITAL_LETTER_H                          0x0048
#define UCS_LATIN_CAPITAL_LETTER_I                          0x0049
#define UCS_LATIN_CAPITAL_LETTER_J                          0x004a
#define UCS_LATIN_CAPITAL_LETTER_K                          0x004b
#define UCS_LATIN_CAPITAL_LETTER_L                          0x004c
#define UCS_LATIN_CAPITAL_LETTER_M                          0x004d
#define UCS_LATIN_CAPITAL_LETTER_N                          0x004e
#define UCS_LATIN_CAPITAL_LETTER_O                          0x004f
#define UCS_LATIN_CAPITAL_LETTER_P                          0x0050
#define UCS_LATIN_CAPITAL_LETTER_Q                          0x0051
#define UCS_LATIN_CAPITAL_LETTER_R                          0x0052
#define UCS_LATIN_CAPITAL_LETTER_S                          0x0053
#define UCS_LATIN_CAPITAL_LETTER_T                          0x0054
#define UCS_LATIN_CAPITAL_LETTER_U                          0x0055
#define UCS_LATIN_CAPITAL_LETTER_V                          0x0056
#define UCS_LATIN_CAPITAL_LETTER_W                          0x0057
#define UCS_LATIN_CAPITAL_LETTER_X                          0x0058
#define UCS_LATIN_CAPITAL_LETTER_Y                          0x0059
#define UCS_LATIN_CAPITAL_LETTER_Z                          0x005a
#define UCS_LEFT_SQUARE_BRACKET                             0x005b
#define UCS_REVERSE_SOLIDUS                                 0x005c
#define UCS_RIGHT_SQUARE_BRACKET                            0x005d
#define UCS_CIRCUMFLEX_ACCENT                               0x005e
#define UCS_LOW_LINE                                        0x005f
#define UCS_GRAVE_ACCENT                                    0x0060
#define UCS_LATIN_SMALL_LETTER_A                            0x0061
#define UCS_LATIN_SMALL_LETTER_B                            0x0062
#define UCS_LATIN_SMALL_LETTER_C                            0x0063
#define UCS_LATIN_SMALL_LETTER_D                            0x0064
#define UCS_LATIN_SMALL_LETTER_E                            0x0065
#define UCS_LATIN_SMALL_LETTER_F                            0x0066
#define UCS_LATIN_SMALL_LETTER_G                            0x0067
#define UCS_LATIN_SMALL_LETTER_H                            0x0068
#define UCS_LATIN_SMALL_LETTER_I                            0x0069
#define UCS_LATIN_SMALL_LETTER_J                            0x006a
#define UCS_LATIN_SMALL_LETTER_K                            0x006b
#define UCS_LATIN_SMALL_LETTER_L                            0x006c
#define UCS_LATIN_SMALL_LETTER_M                            0x006d
#define UCS_LATIN_SMALL_LETTER_N                            0x006e
#define UCS_LATIN_SMALL_LETTER_O                            0x006f
#define UCS_LATIN_SMALL_LETTER_P                            0x0070
#define UCS_LATIN_SMALL_LETTER_Q                            0x0071
#define UCS_LATIN_SMALL_LETTER_R                            0x0072
#define UCS_LATIN_SMALL_LETTER_S                            0x0073
#define UCS_LATIN_SMALL_LETTER_T                            0x0074
#define UCS_LATIN_SMALL_LETTER_U                            0x0075
#define UCS_LATIN_SMALL_LETTER_V                            0x0076
#define UCS_LATIN_SMALL_LETTER_W                            0x0077
#define UCS_LATIN_SMALL_LETTER_X                            0x0078
#define UCS_LATIN_SMALL_LETTER_Y                            0x0079
#define UCS_LATIN_SMALL_LETTER_Z                            0x007a
#define UCS_LEFT_CURLY_BRACKET                              0x007b
#define UCS_VERTICAL_LINE                                   0x007c
#define UCS_RIGHT_CURLY_BRACKET                             0x007d
#define UCS_TILDE                                           0x007e
#endif

#ifdef UNICODE_LATIN_1_SUPPLEMENT
#define UNICODE_LATIN_1_SUPPLEMENT_FIRST                    0x00a0
#define UNICODE_LATIN_1_SUPPLEMENT_LAST                     0x00ff
#define UCS_NO_BREAK_SPACE                                  0x00a0
#define UCS_INVERTED_EXCLAMATION_MARK                       0x00a1
#define UCS_CENT_SIGN                                       0x00a2
#define UCS_POUND_SIGN                                      0x00a3
#define UCS_CURRENCY_SIGN                                   0x00a4
#define UCS_YEN_SIGN                                        0x00a5
#define UCS_BROKEN_BAR                                      0x00a6
#define UCS_SECTION_SIGN                                    0x00a7
#define UCS_DIAERESIS                                       0x00a8
#define UCS_COPYRIGHT_SIGN                                  0x00a9
#define UCS_FEMININE_ORDINAL_INDICATOR                      0x00aa
#define UCS_LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK       0x00ab
#define UCS_NOT_SIGN                                        0x00ac
#define UCS_SOFT_HYPHEN                                     0x00ad
#define UCS_REGISTERED_SIGN                                 0x00ae
#define UCS_MACRON                                          0x00af
#define UCS_DEGREE_SIGN                                     0x00b0
#define UCS_PLUS_MINUS_SIGN                                 0x00b1
#define UCS_SUPERSCRIPT_TWO                                 0x00b2
#define UCS_SUPERSCRIPT_THREE                               0x00b3
#define UCS_ACUTE_ACCENT                                    0x00b4
#define UCS_MICRO_SIGN                                      0x00b5
#define UCS_PILCROW_SIGN                                    0x00b6
#define UCS_MIDDLE_DOT                                      0x00b7
#define UCS_CEDILLA                                         0x00b8
#define UCS_SUPERSCRIPT_ONE                                 0x00b9
#define UCS_MASCULINE_ORDINAL_INDICATOR                     0x00ba
#define UCS_RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK      0x00bb
#define UCS_VULGAR_FRACTION_ONE_QUARTER                     0x00bc
#define UCS_VULGAR_FRACTION_ONE_HALF                        0x00bd
#define UCS_VULGAR_FRACTION_THREE_QUARTERS                  0x00be
#define UCS_INVERTED_QUESTION_MARK                          0x00bf
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_GRAVE               0x00c0
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_ACUTE               0x00c1
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_CIRCUMFLEX          0x00c2
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_TILDE               0x00c3
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_DIAERESIS           0x00c4
#define UCS_LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE          0x00c5
#define UCS_LATIN_CAPITAL_LIGATURE_AE                       0x00c6
#define UCS_LATIN_CAPITAL_LETTER_C_WITH_CEDILLA             0x00c7
#define UCS_LATIN_CAPITAL_LETTER_E_WITH_GRAVE               0x00c8
#define UCS_LATIN_CAPITAL_LETTER_E_WITH_ACUTE               0x00c9
#define UCS_LATIN_CAPITAL_LETTER_E_WITH_CIRCUMFLEX          0x00ca
#define UCS_LATIN_CAPITAL_LETTER_E_WITH_DIAERESIS           0x00cb
#define UCS_LATIN_CAPITAL_LETTER_I_WITH_GRAVE               0x00cc
#define UCS_LATIN_CAPITAL_LETTER_I_WITH_ACUTE               0x00cd
#define UCS_LATIN_CAPITAL_LETTER_I_WITH_CIRCUMFLEX          0x00ce
#define UCS_LATIN_CAPITAL_LETTER_I_WITH_DIAERESIS           0x00cf
#define UCS_LATIN_CAPITAL_LETTER_ETH                        0x00d0       /* Icelandic */
#define UCS_LATIN_CAPITAL_LETTER_N_WITH_TILDE               0x00d1
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_GRAVE               0x00d2
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_ACUTE               0x00d3
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_CIRCUMFLEX          0x00d4
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_TILDE               0x00d5
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_DIAERESIS           0x00d6
#define UCS_MULTIPLICATION_SIGN                             0x00d7
#define UCS_LATIN_CAPITAL_LETTER_O_WITH_STROKE              0x00d8
#define UCS_LATIN_CAPITAL_LETTER_U_WITH_GRAVE               0x00d9
#define UCS_LATIN_CAPITAL_LETTER_U_WITH_ACUTE               0x00da
#define UCS_LATIN_CAPITAL_LETTER_U_WITH_CIRCUMFLEX          0x00db
#define UCS_LATIN_CAPITAL_LETTER_U_WITH_DIAERESIS           0x00dc
#define UCS_LATIN_CAPITAL_LETTER_Y_WITH_ACUTE               0x00dd
#define UCS_LATIN_CAPITAL_LETTER_THORN                      0x00de       /* Icelandic */
#define UCS_LATIN_SMALL_LETTER_SHARP_S                      0x00df       /* German */
#define UCS_LATIN_SMALL_LETTER_A_WITH_GRAVE                 0x00e0
#define UCS_LATIN_SMALL_LETTER_A_WITH_ACUTE                 0x00e1
#define UCS_LATIN_SMALL_LETTER_A_WITH_CIRCUMFLEX            0x00e2
#define UCS_LATIN_SMALL_LETTER_A_WITH_TILDE                 0x00e3
#define UCS_LATIN_SMALL_LETTER_A_WITH_DIAERESIS             0x00e4
#define UCS_LATIN_SMALL_LETTER_A_WITH_RING_ABOVE            0x00e5
#define UCS_LATIN_SMALL_LIGATURE_AE                         0x00e6
#define UCS_LATIN_SMALL_LETTER_C_WITH_CEDILLA               0x00e7
#define UCS_LATIN_SMALL_LETTER_E_WITH_GRAVE                 0x00e8
#define UCS_LATIN_SMALL_LETTER_E_WITH_ACUTE                 0x00e9
#define UCS_LATIN_SMALL_LETTER_E_WITH_CIRCUMFLEX            0x00ea
#define UCS_LATIN_SMALL_LETTER_E_WITH_DIAERESIS             0x00eb
#define UCS_LATIN_SMALL_LETTER_I_WITH_GRAVE                 0x00ec
#define UCS_LATIN_SMALL_LETTER_I_WITH_ACUTE                 0x00ed
#define UCS_LATIN_SMALL_LETTER_I_WITH_CIRCUMFLEX            0x00ee
#define UCS_LATIN_SMALL_LETTER_I_WITH_DIAERESIS             0x00ef
#define UCS_LATIN_SMALL_LETTER_ETH                          0x00f0       /* Icelandic */
#define UCS_LATIN_SMALL_LETTER_N_WITH_TILDE                 0x00f1
#define UCS_LATIN_SMALL_LETTER_O_WITH_GRAVE                 0x00f2
#define UCS_LATIN_SMALL_LETTER_O_WITH_ACUTE                 0x00f3
#define UCS_LATIN_SMALL_LETTER_O_WITH_CIRCUMFLEX            0x00f4
#define UCS_LATIN_SMALL_LETTER_O_WITH_TILDE                 0x00f5
#define UCS_LATIN_SMALL_LETTER_O_WITH_DIAERESIS             0x00f6
#define UCS_DIVISION_SIGN                                   0x00f7
#define UCS_LATIN_SMALL_LETTER_O_WITH_STROKE                0x00f8
#define UCS_LATIN_SMALL_LETTER_U_WITH_GRAVE                 0x00f9
#define UCS_LATIN_SMALL_LETTER_U_WITH_ACUTE                 0x00fa
#define UCS_LATIN_SMALL_LETTER_U_WITH_CIRCUMFLEX            0x00fb
#define UCS_LATIN_SMALL_LETTER_U_WITH_DIAERESIS             0x00fc
#define UCS_LATIN_SMALL_LETTER_Y_WITH_ACUTE                 0x00fd
#define UCS_LATIN_SMALL_LETTER_THORN                        0x00fe       /* Icelandic */
#define UCS_LATIN_SMALL_LETTER_Y_WITH_DIAERESIS             0x00ff
#endif

#ifdef UNICODE_COMBINING_DIACRITICAL_MARKS
#define UNICODE_COMBINING_DIACRITICAL_MARKS_FIRST           0x0300
#define UNICODE_COMBINING_DIACRITICAL_MARKS_LAST            0x036f
#define UCS_COMBINING_GRAVE_ACCENT                          0x0300       /* Varia */
#define UCS_COMBINING_ACUTE_ACCENT                          0x0301       /* Oxia */
#define UCS_COMBINING_CIRCUMFLEX_ACCENT                     0x0302
#define UCS_COMBINING_TILDE                                 0x0303
#define UCS_COMBINING_MACRON                                0x0304
#define UCS_COMBINING_OVERLINE                              0x0305
#define UCS_COMBINING_BREVE                                 0x0306       /* Vrachy */
#define UCS_COMBINING_DOT_ABOVE                             0x0307
#define UCS_COMBINING_DIAERESIS                             0x0308       /* Dialytika */
#define UCS_COMBINING_HOOK_ABOVE                            0x0309
#define UCS_COMBINING_RING_ABOVE                            0x030a
#define UCS_COMBINING_DOUBLE_ACUTE_ACCENT                   0x030b
#define UCS_COMBINING_CARON                                 0x030c
#define UCS_COMBINING_VERTICAL_LINE_ABOVE                   0x030d       /* Tonos */
#define UCS_COMBINING_DOUBLE_VERTICAL_LINE_ABOVE            0x030e
#define UCS_COMBINING_DOUBLE_GRAVE_ACCENT                   0x030f
#define UCS_COMBINING_CANDRABINDU                           0x0310
#define UCS_COMBINING_INVERTED_BREVE                        0x0311
#define UCS_COMBINING_TURNED_COMMA_ABOVE                    0x0312
#define UCS_COMBINING_COMMA_ABOVE                           0x0313       /* Psili */
#define UCS_COMBINING_REVERSED_COMMA_ABOVE                  0x0314       /* Dasia */
#define UCS_COMBINING_COMMA_ABOVE_RIGHT                     0x0315
#define UCS_COMBINING_GRAVE_ACCENT_BELOW                    0x0316
#define UCS_COMBINING_ACUTE_ACCENT_BELOW                    0x0317
#define UCS_COMBINING_LEFT_TACK_BELOW                       0x0318
#define UCS_COMBINING_RIGHT_TACK_BELOW                      0x0319
#define UCS_COMBINING_LEFT_ANGLE_BELOW                      0x031a
#define UCS_COMBINING_HORN                                  0x031b
#define UCS_COMBINING_LEFT_HALF_RING_BELOW                  0x031c
#define UCS_COMBINING_UP_TACK_BELOW                         0x031d
#define UCS_COMBINING_DOWN_TACK_BELOW                       0x031e
#define UCS_COMBINING_PLUS_SIGN_BELOW                       0x031f
#define UCS_COMBINING_MINUS_SIGN_BELOW                      0x0320
#define UCS_COMBINING_PALATALIZED_HOOK_BELOW                0x0321
#define UCS_COMBINING_RETROFLEX_HOOK_BELOW                  0x0322
#define UCS_COMBINING_DOT_BELOW                             0x0323
#define UCS_COMBINING_DIAERESIS_BELOW                       0x0324
#define UCS_COMBINING_RING_BELOW                            0x0325
#define UCS_COMBINING_COMMA_BELOW                           0x0326
#define UCS_COMBINING_CEDILLA                               0x0327
#define UCS_COMBINING_OGONEK                                0x0328
#define UCS_COMBINING_VERTICAL_LINE_BELOW                   0x0329
#define UCS_COMBINING_BRIDGE_BELOW                          0x032a
#define UCS_COMBINING_INVERTED_DOUBLE_ARCH_BELOW            0x032b
#define UCS_COMBINING_CARON_BELOW                           0x032c
#define UCS_COMBINING_CIRCUMFLEX_ACCENT_BELOW               0x032d
#define UCS_COMBINING_BREVE_BELOW                           0x032e
#define UCS_COMBINING_INVERTED_BREVE_BELOW                  0x032f
#define UCS_COMBINING_TILDE_BELOW                           0x0330
#define UCS_COMBINING_MACRON_BELOW                          0x0331
#define UCS_COMBINING_LOW_LINE                              0x0332
#define UCS_COMBINING_DOUBLE_LOW_LINE                       0x0333
#define UCS_COMBINING_TILDE_OVERLAY                         0x0334
#define UCS_COMBINING_SHORT_STROKE_OVERLAY                  0x0335
#define UCS_COMBINING_LONG_STROKE_OVERLAY                   0x0336
#define UCS_COMBINING_SHORT_SOLIDUS_OVERLAY                 0x0337
#define UCS_COMBINING_LONG_SOLIDUS_OVERLAY                  0x0338
#define UCS_COMBINING_RIGHT_HALF_RING_BELOW                 0x0339
#define UCS_COMBINING_INVERTED_BRIDGE_BELOW                 0x033a
#define UCS_COMBINING_SQUARE_BELOW                          0x033b
#define UCS_COMBINING_SEAGULL_BELOW                         0x033c
#define UCS_COMBINING_X_ABOVE                               0x033d
#define UCS_COMBINING_VERTICAL_TILDE                        0x033e
#define UCS_COMBINING_DOUBLE_OVERLINE                       0x033f
#define UCS_COMBINING_GRAVE_TONE_MARK                       0x0340       /* Vietnamese */
#define UCS_COMBINING_ACUTE_TONE_MARK                       0x0341       /* Vietnamese */
#define UCS_COMBINING_GREEK_PEROSPOMENI                     0x0342
#define UCS_COMBINING_GREEK_KORONIS                         0x0343
#define UCS_COMBINING_GREEK_DIALYTIKA                       0x0344
#define UCS_COMBINING_GREEK_YPOGEGRAMMENI                   0x0345
#define UCS_COMBINING_DOUBLE_TILDE                          0x0360
#define UCS_COMBINING_DOUBLE_INVERTED_BREVE                 0x0361
#endif

#ifdef UNICODE_PRIVATE_USE_AREA
#define UNICODE_PRIVATE_USE_AREA_FIRST                      0xe000
#define UNICODE_PRIVATE_USE_AREA_LAST                       0xf8ff
#endif

#endif

/* __SRCVERSION("unicode.h $Rev: 153052 $"); */
