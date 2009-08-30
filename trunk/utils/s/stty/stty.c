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





#ifdef __USAGE
%C	[-a|-g] [operands]
Options:
 -g                  Output in 'gettable' form
 -a                  Display all options
Operands:
 +<option>           Turn on option
 -<option>           Turn off option
 <cc>=<hvalue>       Set a control character
 <cc>=<char>         Set a control character
 +fix                Set terminal options to a sane mode
 +edit               Set terminal options to edit mode
 +raw                Set terminal options to raw mode
 +flush              Flush all pending input and output
 baud=<value>        Set baud rate (1 to 115200)
 ispeed=<value>      Set input baud rate
 ospeed=<value>      Set output baud rate
 par=<parity>        Set parity
 bits=<value>        Set data bits (5, 6, 7 or 8)
 stopb=<value>       Set stop bits (1 or 2)
 term=<string>       Load edit chars for this terminal type
 rows=rows[,cols]    Set console size
 font=<value>        Set console font
 protocol=<value>    Set console protocol (0=QNX4 1=ANSI)
Where:
 <value>  is a decimal number
 <hvalue> is a 2-digit hexadecimal code
 <char>   is a single ASCII character
 <string> is an ASCII string
 <parity> is one of 'none',   'odd',    'even',   'mark',   'space'.
 <option> is one of 'icanon', 'echo',   'isig',   'opost',  'echoe',
                    'echok',  'echonl', 'noflsh', 'ignbrk', 'brkint',
                    'ignpar', 'parmrk', 'inpck',  'istrip', 'inlcr',
                    'igncr',  'icrnl',  'ixon',   'ixoff',  'ihflow',
                    'ohflow', 'ispaged','ospaged','ihpaged','ohpaged',
                    'lkhflow','lksflow','wtsflow','cread',  'clocal',
                    'parenb', 'parodd', 'parstk', 'cstopb', 'hupcl',
                    'nopgrp', 'reset',  'lkiexten', 'load',
                    'cs5', 'cs6', 'cs7', 'cs8', 'evenp', 'parity', 'oddp',
                    'hup', 'isflow', 'osflow', 'iexten', 'nl', 'sane'
 <cc> is one of:    'erase',  'kill',   'eof',    'start',  'stop',
                    'min',    'time',   'eol',    'intr',   'quit',  'susp',
                    'pr1',    'pr2',    'pr3',    'pr4',
                    'up',     'down',   'left',   'right',  'ins',   'del',
                    'home',   'end',    'rub',    'can',
                    'sf1',    'sf2',    'sf3',    'sf4'.
Examples:
 stty 
 stty  </dev/ser1
 stty baud=1200 bits=8 stopb=1 par=none <//2/dev/ser1
#endif

#ifdef __USAGENTO
%C	[-a|-g] [operands]
Options:
 -g                  Output in 'gettable' form
 -a                  Display all options
Operands:
 +<option>           Turn on option
 -<option>           Turn off option
 <cc>=<hvalue>       Set a control character
 <cc>=<char>         Set a control character
 +fix                Set terminal options to a sane mode
 +edit               Set terminal options to edit mode
 +raw                Set terminal options to raw mode
 +flush              Flush all pending input and output
 baud=<value>        Set baud rate (1 to 115200)
 ispeed=<value>      Set input baud rate
 ospeed=<value>      Set output baud rate
 par=<parity>        Set parity
 bits=<value>        Set data bits (5, 6, 7 or 8)
 stopb=<value>       Set stop bits (1 or 2)
 term=<string>       Load edit chars for this terminal type
Where:
 <value>  is a decimal number
 <hvalue> is a 2-digit hexadecimal code
 <char>   is a single ASCII character
 <string> is an ASCII string
 <parity> is one of 'none',   'odd',    'even',   'mark',   'space'.
 <option> is one of 'icanon', 'echo',   'isig',   'opost',  'echoe',
                    'echok',  'echonl', 'noflsh', 'ignbrk', 'brkint',
                    'ignpar', 'parmrk', 'inpck',  'istrip', 'inlcr',
                    'igncr',  'icrnl',  'ixon',   'ixoff',  'isflow',
                    'osflow', 'ihflow', 'ohflow', 'ispaged','ospaged',
                    'ihpaged','ohpaged','cread',  'clocal', 'hup',
                    'parenb', 'parodd', 'parstk', 'cstopb', 'hupcl'
                    'tostop', 'onlcr',  'iexten', 'nl',     'sane',
                    'ek',     'load',   'cs5',    'cs6',    'cs7',
                    'cd8',    'evenp',  'parity', 'oddp'
 <cc> is one of:    'erase',  'kill',   'eof',    'start',  'stop',
                    'min',    'time',   'eol',    'intr',   'quit',  'susp',
                    'pr1',    'pr2',    'pr3',    'pr4',
                    'up',     'down',   'left',   'right',  'ins',   'del',
                    'home',   'end',    'rub',    'can',
                    'sf1',    'sf2',    'sf3',    'sf4'.
Examples:
 stty 
 stty  </dev/ser1
 stty baud=1200 bits=8 stopb=1 par=none <//2/dev/ser1
#endif

/*
 * QNX 4.0 STTY Utility
 *

 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <gulliver.h>

#ifdef __QNXNTO__
#include <sys/dcmd_chr.h>
#define _CONSOLE_H_INCLUDED
#define _MOUSE_H_INCLUDED
#ifdef __QNXNTO10__
#include <term.h>
#else
struct _strs {
    short int
    _back_tab,                  /*  Back tab                                */
    _bell,                      /*  Audible signal (bell)                   */
    _carriage_return,           /*  Carriage return (*)                     */
    _change_scroll_region,      /*  change to lines #1 thru #2 (vt100) (G)  */
    _clear_all_tabs,            /*  Clear all tab stops.                    */
    _clear_screen,              /*  Clear screen (*)                        */
    _clr_eol,                   /*  Clear to end of line                    */
    _clr_eos,                   /*  Clear to end of display (*)             */
    _column_address,            /*  Set cursor column (G)                   */
    _command_character,         /*  Term. settable cmd char in proto        */
    _crsr_address,              /*  Cursor motion to row #1 col #2 (G)      */
    _crsr_down,                 /*  Down one line                           */
    _crsr_home,                 /*  Home cursor (if no cup)                 */
    _crsr_invisible,            /*  Make cursor invisible                   */
    _crsr_left,                 /*  Move cursor left one space.             */
    _crsr_mem_address,          /*  Memory relative cursor addressing.      */
    _crsr_normal,               /*  Make cursor appear normal (undo vs/vi)  */
    _crsr_right,                /*  Non-destructive space (cursor right)    */
    _crsr_to_ll,                /*  Last line, first column (if no cup)     */
    _crsr_up,                   /*  Upline (cursor up)                      */
    _crsr_visible,              /*  Make cursor very visible                */
    _dlt_character,             /*  Delete character (*)                    */
    _dlt_line,                  /*  Delete line (*)                         */
    _dis_status_line,           /*  Disable status line                     */
    _down_half_line,            /*  Half-line down: forward 1/2 linefeed    */
    _entr_alt_charset_mode,     /*  Start alternate character set           */
    _entr_blink_mode,           /*  Turn on blinking                        */
    _entr_bold_mode,            /*  Turn on bold (extra bright) mode        */
    _entr_ca_mode,              /*  String to begin programs that use cup   */
    _entr_delete_mode,          /*  Delete mode (enter)                     */
    _entr_dim_mode,             /*  Turn on half-bright mode                */
    _entr_insert_mode,          /*  Insert mode (enter);                    */
    _entr_secure_mode,          /*  Turn on blank mode: chars invisible     */
    _entr_protected_mode,       /*  Turn on protected mode                  */
    _entr_reverse_mode,         /*  Turn on reverse video mode              */
    _entr_standout_mode,        /*  Begin stand out mode                    */
    _entr_underline_mode,       /*  Start underscore mode                   */
    _erase_chars,               /*  Erase #1 characters (G)                 */
    _exit_alt_charset_mode,     /*  End alternate character set             */
    _exit_attribute_mode,       /*  Turn off all attributes                 */
    _exit_ca_mode,              /*  String to end programs that use cup     */
    _exit_delete_mode,          /*  End delete mode                         */
    _exit_insert_mode,          /*  End insert mode;                        */
    _exit_standout_mode,        /*  End stand out mode                      */
    _exit_underline_mode,       /*  End underscore mode                     */
    _flash_screen,              /*  Visible bell (may not move cursor)      */
    _form_feed,                 /*  Hardcopy terminal page eject (*)        */
    _from_status_line,          /*  Return from status line                 */
    _init_1string,              /*  Terminal initialization string          */
    _init_2string,              /*  Terminal initialization string          */
    _init_3string,              /*  Terminal initialization string          */
    _init_file,                 /*  Name of file containing is              */
    _ins_character,             /*  Insert character                        */
    _ins_line,                  /*  Add new blank line (*)                  */
    _ins_padding,               /*  Insert pad after char inserted (*)      */
    _ky_backspace,              /*  KEY_BACKSPACE, 0407, Sent by backspace key  */
    _ky_catab,                  /*  KEY_CATAB, 0526, Sent by clear-all-tabs key */
    _ky_clear,                  /*  KEY_CLEAR, 0515, Sent by clear screen or erase key  */
    _ky_ctab,                   /*  KEY_CTAB, 0525, Sent by clear-tab key   */
    _ky_dc,                     /*  KEY_DC, 0512, Sent by delete character key  */
    _ky_dl,                     /*  KEY_DL, 0510, Sent by delete line key   */
    _ky_down,                   /*  KEY_DOWN, 0402, Sent by terminal down arrow key */
    _ky_eic,                    /*  KEY_EIC, 0514, Sent by rmir or smir in insert mode  */
    _ky_eol,                    /*  KEY_EOL, 0517, Sent by clear-to-end-of-line key */
    _ky_eos,                    /*  KEY_EOS, 0516, Sent by clear-to-end-of-screen key   */
    _ky_f0,                     /*  KEY_F(0), 0410, function key f0.        */
    _ky_f1,                     /*  KEY_F(1), 0411, function key f1.        */
    _ky_f10,                    /*  KEY_F(10), 0422, function key f10.      */
    _ky_f2,                     /*  KEY_F(2), 0412, function key f2.        */
    _ky_f3,                     /*  KEY_F(3), 0413, function key f3.        */
    _ky_f4,                     /*  KEY_F(4), 0414, function key f4.        */
    _ky_f5,                     /*  KEY_F(5), 0415, function key f5.        */
    _ky_f6,                     /*  KEY_F(6), 0416, function key f6.        */
    _ky_f7,                     /*  KEY_F(7), 0417, function key f7.        */
    _ky_f8,                     /*  KEY_F(8), 0420, function key f8.        */
    _ky_f9,                     /*  KEY_F(9), 0421, function key f9.        */
    _ky_home,                   /*  KEY_HOME, 0406, Sent by home key.       */
    _ky_ic,                     /*  KEY_IC, 0513, Sent by ins char/enter ins mode key   */
    _ky_il,                     /*  KEY_IL, 0511, Sent by insert line.      */
    _ky_left,                   /*  KEY_LEFT, 0404, Sent by left arrow key  */
    _ky_ll,                     /*  KEY_LL, 0533, Sent by home-down key     */
    _ky_npage,                  /*  KEY_NPAGE, 0522, Sent by next-page key  */
    _ky_ppage,                  /*  KEY_PPAGE, 0523, Sent by prev-page key  */
    _ky_right,                  /*  KEY_RIGHT, 0405, Sent by right arrow key*/
    _ky_sf,                     /*  KEY_SF, 0520, Snt by scroll-fwd/down key*/
    _ky_sr,                     /*  KEY_SR, 0521, Snt by scroll-bkwd/up key */
    _ky_stab,                   /*  KEY_STAB, 0524, Sent by set-tab key     */
    _ky_up,                     /*  KEY_UP, 0403, Sent by up arrow key      */
    _kpad_local,                /*  Out of "keypad transmit" mode           */
    _kpad_xmit,                 /*  Put terminal in "keypad transmit" mode  */
    _lab_f0,                    /*  Labels on function key f0 if not f0     */
    _lab_f1,                    /*  Labels on function key f1 if not f1     */
    _lab_f10,                   /*  Labels on function key f10 if not f10   */
    _lab_f2,                    /*  Labels on function key f2 if not f2     */
    _lab_f3,                    /*  Labels on function key f3 if not f3     */
    _lab_f4,                    /*  Labels on function key f4 if not f4     */
    _lab_f5,                    /*  Labels on function key f5 if not f5     */
    _lab_f6,                    /*  Labels on function key f6 if not f6     */
    _lab_f7,                    /*  Labels on function key f7 if not f7     */
    _lab_f8,                    /*  Labels on function key f8 if not f8     */
    _lab_f9,                    /*  Labels on function key f9 if not f9     */
    _meta_off,                  /*  Turn off "meta mode"                    */
    _meta_on,                   /*  Turn on "meta mode" (8th bit)           */
    _newline,                   /*  Newline: behaves like cr followed by lf */
    _pad_char,                  /*  Pad character (rather than null)        */
    _prm_dch,                   /*  Delete #1 chars (G*)                    */
    _prm_delete_line,           /*  Delete #1 lines (G*)                    */
    _prm_down_cursor,           /*  Move cursor down #1 lines. (G*)         */
    _prm_ich,                   /*  Insert #1 blank chars (G*)              */
    _prm_index,                 /*  Scroll forward #1 lines. (G)            */
    _prm_insert_line,           /*  Add #1 new blank lines (G*)             */
    _prm_left_cursor,           /*  Move cursor left #1 spaces (G)          */
    _prm_right_cursor,          /*  Move cursor right #1 spaces. (G*)       */
    _prm_rindex,                /*  Scroll backward #1 lines. (G)           */
    _prm_up_cursor,             /*  Move cursor up #1 lines. (G*)           */
    _pkey_key,                  /*  Prog funct key #1 to type string #2     */
    _pkey_local,                /*  Prog funct key #1 to execute string #2  */
    _pkey_xmit,                 /*  Prog funct key #1 to xmit string #2     */
    _print_screen,              /*  Print contents of the screen            */
    _prtr_off,                  /*  Turn off the printer                    */
    _prtr_on,                   /*  Turn on the printer                     */
    _repeat_char,               /*  Repeat char #1 #2 times.  (G*)          */
    _reset_1string,             /*  Reset terminal completely to sane modes */
    _reset_2string,             /*  Reset terminal completely to sane modes */
    _reset_3string,             /*  Reset terminal completely to sane modes */
    _reset_file,                /*  Name of file containing reset string.   */
    _restore_cursor,            /*  Restore cursor to position of last sc.  */
    _row_address,               /*  Like hpa but sets row. (G)              */
    _save_cursor,               /*  Save cursor position.                   */
    _scroll_forward,            /*  Scroll text up                          */
    _scroll_reverse,            /*  Scroll text down                        */
    _set_attributes,            /*  Define the video attributes (G9)        */
    _set_tab,                   /*  Set a tab in all rows, current column   */
    _set_window,                /*  Curr window is lines #1-#2 cols #3-#4   */
    _tab,                       /*  Tab to next 8 space hardware tab stop.  */
    _to_status_line,            /*  Go to status line, col #1               */
    _underline_char,            /*  Underscore one char and move past it    */
    _up_half_line,              /*  Half-line up (reverse 1/2 linefeed)     */
    _init_prog,                 /*  Path name of program for init.          */
    _ky_a1,                     /*  KEY_A1, 0534, Upper left of keypad      */
    _ky_a3,                     /*  KEY_A3, 0535, Upper right of keypad     */
    _ky_b2,                     /*  KEY_B2, 0536, Center of keypad          */
    _ky_c1,                     /*  KEY_C1, 0537, Lower left of keypad      */
    _ky_c3,                     /*  KEY_C3, 0540, Lower right of keypad     */
    _prtr_non,                  /*  Turn on the printer for #1 bytes.       */
    _char_padding,              /*  Like ip but when in replace mode        */
    _acs_chars,                 /*  Graphic charset pair aAbBcC - def=vt100 */
    _plab_norm,                 /*  Prog label #1 to show string #2         */
    _ky_btab,                   /*  KEY_BTAB, 0541, Back tab key            */
    _entr_xon_mode,             /*  Turn on xon/xoff handshaking            */
    _exit_xon_mode,             /*  Turn off xon/xoff handshaking           */
    _entr_am_mode,              /*  Turn on automatic margins               */
    _exit_am_mode,              /*  Turn off automatic margins              */
    _xon_character,             /*  X-on character                          */
    _xoff_character,            /*  X-off character                         */
    _ena_acs,                   /*  Enable alternate char set               */
    _labl_on,                   /*  Turn on soft labels                     */
    _labl_off,                  /*  Turn off soft labels                    */
    _ky_beg,                    /*  KEY_BEG, 0542, beg(inning) key          */
    _ky_cancel,                 /*  KEY_CANCEL, 0543, cancel key            */
    _ky_close,                  /*  KEY_CLOSE, 0544, close key              */
    _ky_command,                /*  KEY_COMMAND, 0545, cmd (command) key    */
    _ky_copy,                   /*  KEY_COPY, 0546, copy key                */
    _ky_create,                 /*  KEY_CREATE, 0547, create key            */
    _ky_end;                    /*  KEY_END, 0550, end key                  */
};
#endif
#else
#include <sys/dev.h>
#include <sys/ser_msg.h>
#include <sys/console.h>
#include <sys/qioctl.h>
#include <sys/kernel.h>
#include <sys/vc.h>
#include <sys/term.h>
#undef VEOL2
#undef VSWTCH
#undef VDSUSP
#undef VWERASE
#undef VREPRINT
#undef VDISCARD
#endif

extern long atol();

#define _TNONE    0
#define _TCC      1
#define _TCTRL    2
#define _TINP     3
#define _TLOC     4
#define _TEK      5
#define _TISPD    6
#define _TOSPD    7
#define _TBAUD    8
#define _TPAR     9
#define _TSTOP   10
#define _TBITS   11
#define _TFIX    15
#define _TEDIT   16
#define _TOUT    17
#ifndef __QNXNTO__
#define _TCON    18
#endif
#define _TRAW    20
#define _TLOAD   21
#define _TTERM   22
#define _TROWS   23
#ifndef __QNXNTO__
#define _TIOCTL  24
#endif
#define _TFLUSH  25
#define _TFLOW   26
#ifndef __QNXNTO__
#define _TQF     27
#define _TFONT   28
#define _TPROTO  29
#define _TRESET  30
#endif

#define IS_CON		1
#define IS_SER		2
#define IS_PAR		3

struct keyword {
	char *name;
	unsigned has_arg;
	unsigned type;
	unsigned value;
	unsigned mask;
	} keywords[] = {
	/*
	 * POSIX 1003.2 required options
	 */
	{"eof",       1, _TCC,    VEOF,      0},
	{"eol",       1, _TCC,    VEOL,      0},
	{"erase",     1, _TCC,    VERASE,    0},
	{"intr",      1, _TCC,    VINTR,     0},
	{"kill",      1, _TCC,    VKILL,     0},
	{"quit",      1, _TCC,    VQUIT,     0},
	{"susp",      1, _TCC,    VSUSP,     0},
	{"start",     1, _TCC,    VSTART,    0},
	{"stop",      1, _TCC,    VSTOP,     0},
	{"min",       1, _TCC,    VMIN,      0},
	{"time",      1, _TCC,    VTIME,     0},
	{"parenb",    0, _TCTRL,  PARENB,    PARENB}, 
	{"parodd",    0, _TCTRL,  PARODD,    PARODD}, 
	{"cs5",       0, _TCTRL,  CS5,       (CS5|CS6|CS7|CS8)},
	{"cs6",       0, _TCTRL,  CS6,       (CS5|CS6|CS7|CS8)},    
	{"cs7",       0, _TCTRL,  CS7,       (CS5|CS6|CS7|CS8)},    
	{"cs8",       0, _TCTRL,  CS8,       (CS5|CS6|CS7|CS8)},    
	{"hupcl",     0, _TCTRL,  HUPCL,     HUPCL},  
	{"hup",       0, _TCTRL,  HUPCL,     HUPCL},  
	{"cstopb",    0, _TCTRL,  CSTOPB,    CSTOPB}, 
	{"cread",     0, _TCTRL,  CREAD,     CREAD},  
	{"clocal",    0, _TCTRL,  CLOCAL,    CLOCAL}, 
	{"evenp",     0, _TCTRL,  (PARENB|CS7),        (PARENB|PARODD|CS5|CS6|CS7|CS8)},
	{"parity",    0, _TCTRL,  (PARENB|CS7),        (PARENB|PARODD|CS5|CS6|CS7|CS8)},
	{"oddp",      0, _TCTRL,  (PARENB|CS7|PARODD), (PARENB|PARODD|CS5|CS6|CS7|CS8)},
	{"ignbrk",    0, _TINP,   IGNBRK,    IGNBRK}, 
	{"brkint",    0, _TINP,   BRKINT,    BRKINT}, 
	{"ignpar",    0, _TINP,   IGNPAR,    IGNPAR}, 
	{"parmrk",    0, _TINP,   PARMRK,    PARMRK}, 
	{"inpck",     0, _TINP,   INPCK,     INPCK},  
	{"istrip",    0, _TINP,   ISTRIP,    ISTRIP}, 
	{"inlcr",     0, _TINP,   INLCR,     INLCR},  
	{"igncr",     0, _TINP,   IGNCR,     IGNCR},  
	{"icrnl",     0, _TINP,   ICRNL,     ICRNL},  
	{"ixon",      0, _TINP,   IXON,      IXON},   
	{"ixoff",     0, _TINP,   IXOFF,     IXOFF},  
#ifdef IMAXBEL
	{"imaxbel",   0, _TINP,   IMAXBEL,   IMAXBEL},  
#endif
	{"nl",        0, _TINP,   ICRNL,     ICRNL},
	{"opost",     0, _TOUT,   OPOST,     OPOST},  
#ifdef ONLCR
	{"onlcr",     0, _TOUT,   ONLCR,     ONLCR},  
#endif
	{"isig",      0, _TLOC,   ISIG,      ISIG},   
	{"icanon",    0, _TLOC,   ICANON,    ICANON}, 
	{"iexten",    0, _TLOC,   IEXTEN,    IEXTEN}, 
	{"echo",      0, _TLOC,   ECHO,      ECHO},   
	{"echoe",     0, _TLOC,   ECHOE,     ECHOE},  
	{"echok",     0, _TLOC,   ECHOK,     ECHOK},  
#ifdef ECHOKE
	{"echoke",    0, _TLOC,   ECHOKE,    ECHOKE},  
#endif
	{"echonl",    0, _TLOC,   ECHONL,    ECHONL}, 
#ifdef ECHOCTL
	{"echoctl",   0, _TLOC,   ECHOCTL,   ECHOCTL},  
#endif
	{"noflsh",    0, _TLOC,   NOFLSH,    NOFLSH}, 
#ifdef TOSTOP
	{"tostop",    0, _TLOC,   TOSTOP,    TOSTOP},  
#endif
	{"ek",        0, _TEK,    0,         0},
	{"ispeed",    1, _TISPD,  0,         0},
	{"ospeed",    1, _TOSPD,  0,         0},      

	/*
	 * QNX extensions
	 */

	{"b",         1, _TBAUD,  0,         0},     
	{"baud",      1, _TBAUD,  0,         0},     
	{"par",       1, _TPAR,   0,         0},     
	{"stopb",     1, _TSTOP,  0,         0},     
	{"bits",      1, _TBITS,  0,         0},     
	{"parstk",    0, _TCTRL,  PARSTK,    PARSTK}, 
	{"ihflow",    0, _TCTRL,  IHFLOW,    IHFLOW},     
	{"ohflow",    0, _TCTRL,  OHFLOW,    OHFLOW},     
	{"osflow",    0, _TINP,   IXON,      IXON},   
	{"isflow",    0, _TINP,   IXOFF,     IXOFF},  
	{"pr1",       1, _TCC,    VPREFIX,   0},
	{"pr2",       1, _TCC,    VPREFIX+1, 0},
	{"pr3",       1, _TCC,    VPREFIX+2, 0},
	{"pr4",       1, _TCC,    VPREFIX+3, 0},
	{"sf1",       1, _TCC,    VSUFFIX,   0},
	{"sf2",       1, _TCC,    VSUFFIX+1, 0},
	{"sf3",       1, _TCC,    VSUFFIX+2, 0},
	{"sf4",       1, _TCC,    VSUFFIX+3, 0},
	{"up",        1, _TCC,    VUP,       0},
	{"down",      1, _TCC,    VDOWN,     0},
	{"left",      1, _TCC,    VLEFT,     0},
	{"right",     1, _TCC,    VRIGHT,    0},
	{"ins",       1, _TCC,    VINS,      0},
	{"del",       1, _TCC,    VDEL,      0},
	{"home",      1, _TCC,    VHOME,     0},
	{"end",       1, _TCC,    VEND,      0},
	{"rub",       1, _TCC,    VRUB,      0},
	{"can",       1, _TCC,    VCAN,      0},
	{"eol",       1, _TCC,    VEOL,      0},
#ifdef VEOL2
	{"eol2",      1, _TCC,    VEOL2,     0},
#endif
#ifdef VSWTCH
	{"swtch",     1, _TCC,    VSWTCH,    0},
#endif
#ifdef VDSUSP
	{"dsusp",     1, _TCC,    VDSUSP,    0},
#endif
#ifdef VWERASE
	{"werase",    1, _TCC,    VWERASE,   0},
#endif
#ifdef VLNEXT
	{"lnext",     1, _TCC,    VLNEXT,    0},
#endif
#ifdef VFWD
	{"fwd",       1, _TCC,    VFWD,      0},
#endif
#ifdef VLOGIN
	{"login",     1, _TCC,    VLOGIN,    0},
#endif
#ifdef VREPRINT
	{"reprint",   1, _TCC,    VREPRINT,  0},
#endif
#ifdef VDISCARD
	{"discard",   1, _TCC,    VDISCARD,  0},
#endif
	{"fix",       0, _TFIX,   0,         0},
	{"sane",      0, _TFIX,   0,         0},
	{"edit",      0, _TEDIT,  0,         0},
	{"raw",       0, _TRAW,   0,         0},
	{"load",      0, _TLOAD,  0,         0},
	{"term",      1, _TTERM,  0,         0},
	{"rows",      1, _TROWS,  0,         0},
	{"flush",     0, _TFLUSH, 0,         0},
#ifndef __QNXNTO__
	{"font",      1, _TFONT,  0,         0},
	{"protocol",  1, _TPROTO, 0,         0},
	{"reset",     0, _TRESET, 0,         0},
#endif
	{"ispaged",   0, _TFLOW,  TCIOFF,    TCION},
	{"ihpaged",   0, _TFLOW,  TCIOFFHW,  TCIONHW},
	{"ospaged",   0, _TFLOW,  TCOOFF,    TCOON},
	{"ohpaged",   0, _TFLOW,  TCOOFFHW,  TCOONHW},

#ifndef __QNXNTO__
	{"lkhflow",   0, _TQF,    TC_PROTECT_HFLOW, TC_PROTECT_HFLOW},
	{"lksflow",   0, _TQF,    TC_PROTECT_SFLOW, TC_PROTECT_SFLOW},
	{"lkiexten",  0, _TQF,    TC_PROTECT_IEXTEN, TC_PROTECT_IEXTEN},
	{"wtsflow",   0, _TQF,    TC_WAIT_SFLOW,    TC_WAIT_SFLOW},
	{"nopgrp",    0, _TQF,    TC_NOPGRP,        TC_NOPGRP},
	{"echoi",     0, _TQF,    TC_ECHOI,         TC_ECHOI},

	{"noboot",    0, _TCON,   CONSOLE_NOBOOT,    0},
	{"noswitch",  0, _TCON,   CONSOLE_NOSWITCH,  0},
	{"nodebug",   0, _TCON,   CONSOLE_NODEBUG,   0},
	{"noresize",  0, _TCON,   CONSOLE_NORESIZE,  0},
	{"nohotkey",  0, _TCON,   CONSOLE_NOHOTKEY,  0},
	{"nocolor",   0, _TCON,   CONSOLE_NOCOLOR,   0},
	{"invisible", 0, _TCON,   CONSOLE_INVISIBLE, 0},
	{"nohscroll", 0, _TCON,   CONSOLE_NOHSCROLL, 0},
	{"monocurs",  0, _TCON,   CONSOLE_MONOCURS,  0},
	{"scanmode",  0, _TCON,   CONSOLE_SCANMODE,  0},
	{"extmode",   0, _TCON,   CONSOLE_EXTMODE,   0},

	{"capslock",  0, _TIOCTL, 0x0004,    IS_CON},
	{"numlock",   0, _TIOCTL, 0x0002,    IS_CON},
	{"scrlock",   0, _TIOCTL, 0x0001,    IS_CON},
	{"DTR",       0, _TIOCTL, 0x0001,    IS_SER},
	{"RTS",       0, _TIOCTL, 0x0002,    IS_SER},
	{"BRK",       0, _TIOCTL, 0x4000,    IS_SER},
#endif

	{"",          0, _TNONE,  0,         0}

	};

char *ccc[] ={
	" intr",
	" quit",
	"erase",
	" kill",
	"  eof",
	"  eol",
#ifdef __QNXNTO__
	" eol2",
	"swtch",
#else
	"  ?06",
	"  ?07",
#endif
	"start",
	" stop",
	" susp",
#ifdef __QNXNTO__
	"dsusp",
	"reprint",
	"discard",
	"werase",
	"lnext",
#else
	"  ?11",
	"  ?12",
	"  ?13",
	"  ?14",
	"  ?15",
#endif
	"  min",
	" time",
#ifdef __QNXNTO__
	"  fwd",
	"login",
#else
	"  ?18",
	"  ?19",
#endif
	"  pr1",
	"  pr2",
	"  pr3",
	"  pr4",
	"  sf1",
	"  sf2",
	"  sf3",
	"  sf4",
	" left",
	"right",
	"   up",
	" down",
	"  ins",
	"  del",
	"  rub",
	"  can",
	" home",
	"  end",
	"  ?38",
	"  ?39"
	};

struct paritys {
	char *par_name;
	int par_bits;
	unsigned par_mask;
	unsigned par_value;
	} paritys[] = {

	{"odd",	0x01, (PARENB|PARODD|PARSTK), (PARENB|PARODD)},
	{"even",	0x03, (PARENB|PARODD|PARSTK), (PARENB)},
	{"mark",	0x05, (PARENB|PARODD|PARSTK), (PARENB|PARODD|PARSTK)},
	{"space",0x07, (PARENB|PARODD|PARSTK), (PARENB|PARSTK)},
	{"none", 0x00, (PARENB|PARODD|PARSTK), 0},
	{NULL,0,0,0}
	};

struct controls {
	char *ctrl_name;
	unsigned ctrl_bits;
	} controls[] = {

	{"DTR",		0x01},
	{"RTS",		0x02},
	{"BREAK",	0x04},
	{"cts",		0x10},
	{"dsr",		0x20},
	{"ri",		0x40},
	{"cd",		0x80},
	{NULL,0}
	};

struct controls pcontrols[] ={

	{"ERROR",		0x0800},
	{"Online",		0x1000},
	{"PaperOut",	0x2000},
	{NULL,0}
	};
char tbuf[40 + 1];

char term_char = '\n';

struct termios tios;
struct termios tios2;

#ifdef ONLCR
#define TIOS_ONLCR	ONLCR
#else
#define TIOS_ONLCR	0
#endif

#ifdef ECHOKE
#define TIOS_ECHOKE	ECHOKE
#else
#define TIOS_ECHOKE	0
#endif

#ifdef ECHOCTL
#define TIOS_ECHOCTL	ECHOCTL
#else
#define TIOS_ECHOCTL	0
#endif

#ifdef IMAXBEL
#define TIOS_IMAXBEL	IMAXBEL
#else
#define TIOS_IMAXBEL	0
#endif

struct termios raw_tios ={
	/* c_iflag */ 		0,
	/* c_oflag */		0,
#ifdef __QNXNTO__
	/* c_cflag */		(CREAD|HUPCL),
#else
	/* c_cflag */		(CREAD),
#endif
#ifdef __QNXNTO__
	/* c_lflag */		0,
#else
	/* c_lflag */		(IEXTEN),
#endif
	};

struct termios edit_tios ={
	/* c_iflag */ 		(ICRNL|BRKINT|TIOS_IMAXBEL),
	/* c_oflag */		(OPOST|TIOS_ONLCR),
#ifdef __QNXNTO__
	/* c_cflag */		(CREAD|HUPCL),
#else
	/* c_cflag */		(CREAD),
#endif
#ifdef ECHOKE
	/* c_lflag */		(ICANON|ISIG|ECHO|ECHOE|TIOS_ECHOKE|IEXTEN|TIOS_ECHOCTL),
#else
	/* c_lflag */		(ICANON|ISIG|ECHO|ECHOK|ECHOE|IEXTEN|TIOS_ECHOCTL),
#endif
	};

struct termios tios_mask ={
	/* c_iflag */ 		(ICRNL|BRKINT|PARMRK|IGNBRK|IGNPAR|ISTRIP|INLCR|IGNCR|TIOS_IMAXBEL),
	/* c_oflag */		(OPOST|TIOS_ONLCR),
	/* c_cflag */		(CREAD|CLOCAL|HUPCL),
	/* c_lflag */		(ICANON|ISIG|ECHO|ECHOK|ECHOE|ECHONL|IEXTEN|NOFLSH|TOSTOP|TIOS_ECHOCTL|TIOS_ECHOKE),
	};

struct termios null_tios;
int out_cols = 80;

int
pr_opt(name, value, bit, ref, mask, count)
char *name;
unsigned value, bit, ref, mask;
int *count;
{
	int len;

	len = strlen(name) + 1;

	if((value & bit) != (ref & bit)  ||  ((mask|ref) & bit) == 0) {
		if(*count != 0)
			++len;
		if((*count + len) >= out_cols) {
			printf("\n");
			*count = 0;
		}
		if(*count != 0)
			printf(" ");
		*count += len;
		printf("%c%s", (value & bit) ? '+' : '-', name);
		return(1);
	}
	return(0);
}

int fd;
long obase, sbase;

#define KUP       0
#define KDOWN     1
#define KLEFT     2
#define KRIGHT    3
#define KINS      4
#define KDEL      5
#define KHOME     6
#define KEND      7

#define NKEYS 8
int key_idx[NKEYS] ={
	VUP, VDOWN, VLEFT, VRIGHT, VINS, VDEL, VHOME, VEND,
	};
char keys[NKEYS][10];
char tfile[40 + 1];

struct termiohdr {
	short int magic;
	short int name_size;
	short int bool_count;
	short int num_count;
	short int offset_count;
	short int str_size;
	} thdr;

#ifndef __QNXNTO__
struct _ser_reset ser_reset_msg;
#endif

int get_str( index, offset )
int index;
int offset;
{
	long pos;
	int n;
	short num;
	char *buf;
	int len = 0;

	buf = &keys[index][0];

	if(offset >= (thdr.offset_count * 2)) {
		return(-1);
	}

	pos = lseek( fd, obase + offset, SEEK_SET);
	if(pos != obase + offset) {
		fprintf(stderr, "STTY: Seek error\n");
		return(-1);
	}
	read( fd, &num, 2);
	/* QNX terminfo is little endian for now */
	num = ENDIAN_LE16(num);
	if( num < 0 ) {
		return(-1);
	}
	if(num >= (thdr.str_size)) {
		fprintf(stderr, "STTY: Data error\n");
		return(-1);
	}
	pos = lseek( fd, sbase + num, SEEK_SET);
	if(pos != sbase + num) {
		fprintf(stderr, "STTY: Seek error\n");
		return(-1);
	}
	while( len < 10 ) {
		n = read( fd, buf + len, 1);
		if( n != 1 ) {
			fprintf(stderr, "STTY: Read error\n");
			return(-1);
		}
		if(*(buf+len) == 0)
			break;
		++len;
		}
	return(0);
}

int
main(argc, argv)
int argc;
char *argv[];
{
	char *p;
	char *s;
	struct keyword *k;
	char *p1;
	unsigned arg;
	int is_plus, is_minus, is_equal;
	int changed = 0;
	int g_flag = 0;
	int a_flag = 0;
	int no_arg = 1;
	int i, n;
	int rows, cols;
	int prefix;
	int suffix;
	char *devtype = "";
    long lval[2];
	struct termios *ref, *mref;
	int count;
	int hflow_changed = 0;
	int sflow_changed = 0;
	int iexten_changed = 0;
#ifdef __QNXNTO__
	struct _ttyinfo info;
#else 
	struct _console_ctrl *cc;
	struct _dev_info_entry dinfo;
#endif

	if(tcgetattr(0, &tios) == -1) {
		fprintf(stderr, "%s: Not a terminal device\n", argv[0]);
		exit(EXIT_FAILURE);
	}

#ifdef __QNXNTO__
	if(tcgetsize(1, 0, &out_cols) == -1)
#else
	if(dev_size(1, -1, -1, 0, &out_cols) == -1)
#endif
		out_cols = 80;

#define SWITCH_CHAR( c )	(('-' << 8) + (c))

	for(arg = 1; arg < argc; ++arg) {
		strncpy(&tbuf[0], argv[arg], sizeof(tbuf));
		p = &tbuf[0];
		switch((*p << 8) | *(p+1)) {

		case SWITCH_CHAR('g'):
			g_flag = 1;
			a_flag = 1;
			term_char = ' ';
			out_cols = INT_MAX;
			break;

		case SWITCH_CHAR('a'):
			a_flag = 1;
			break;

		default:
			no_arg = 0;
			is_plus = 0;
			is_minus = 0;
			is_equal = 0;
			if(*p == '+') {
				++p;
				is_plus = 1;
			} else if(*p == '-') {
				++p;
				is_minus = 1;
			}
			p1 = p;
			while (*p1  &&  *p1 != '=') ++p1;
			if(*p1 == '=') {
				*p1++ = 0;
				is_equal = 1;
				if(*p1 == 0) {
					fprintf(stderr,"%s: Missing argument '%s'\n", argv[0], argv[arg]);
					continue;
				}
			}
			if(*p >= '0'  &&  *p <= '9') {
				/*
				 * Special case for numeric arguments alone,
				 * assume it means baud=nnn
				 */
				p1 = p;
				p = "baud";
				is_equal = 1;
			}
			/*
			 * Now lookup the keyword
			 */
			k = &keywords[0];
			while(k->type != _TNONE) {
				if(strcmp(k->name, p) == 0)
					break;
				++k;
			}
 			if(k->type == _TNONE) {
				fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
				continue;
			}
			/*
			 * Get the argument for this parameter
			 */
			if(k->has_arg) {
				if(is_plus != 0  ||  is_minus != 0) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					continue;
				}
				if(is_equal == 0) {
					if(arg >= (argc-1)) {
						fprintf(stderr,"%s: Missing argument '%s'\n", argv[0], argv[arg]);
						continue;
					}
					p1 = argv[++arg];
				}
			} else {
				if(is_equal != 0) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					continue;
				}
			}

			/*
			 * We have found a match with a valid parameter
			 */
			switch(k->type) {
			case _TCC:
				if(strcmp(p1, "^-") == 0
						||  strcmp(p1, "undef") == 0
						||  strcmp(p1, "<undef>") == 0)
					tios.c_cc[k->value] = _POSIX_VDISABLE;
				else if(strlen(p1) == 1)
					tios.c_cc[k->value] = (short unsigned) *p1;
				else if(strcmp(p1, "^?") == 0)
					tios.c_cc[k->value] = 0x7F;
				else if(strlen(p1) == 2  &&  *p1 == '^'
						&&  (*(p1+1) >= '@'  &&  *(p1+1) <= '_'
							 || *(p1+1) >= 'a'  &&  *(p1+1) <= 'z'))
					tios.c_cc[k->value] = toupper(*(p1+1)) - '@';
				else if(strlen(p1) == 2  &&  isxdigit(*p1)  &&  isxdigit(*(p1+1)))
					tios.c_cc[k->value] = atoh(p1);
				else {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
				changed=1;
				break;
			case _TCTRL:
				if(is_minus) {
					if(strcmp(p, "evenp") == 0
							||  strcmp(p, "parity") == 0
							||  strcmp(p, "oddp") == 0) {
						tios.c_cflag = (tios.c_cflag & ~(PARENB|CS5|CS6|CS7))
									 | (CS8);
					} else
						tios.c_cflag = (tios.c_cflag & ~k->mask);
				} else
					tios.c_cflag = (tios.c_cflag & ~k->mask) | (k->value);

				if(k->mask & PARENB) {
					/*
					 * Make sure that INPCK "tracks" PARENB
					 */
					if(tios.c_cflag & PARENB)
						tios.c_iflag |= INPCK;
					else
						tios.c_iflag &= ~INPCK;
				}
				changed = 1;
				if(strcmp(p+1, "hflow") == 0)
					hflow_changed = 1;
				break;
			case _TINP:
				if(is_minus) {
					if(strcmp(p, "nl") == 0)
						tios.c_iflag = (tios.c_iflag & ~(ICRNL|INLCR|IGNCR));
					else
						tios.c_iflag = (tios.c_iflag & ~k->mask);
				} else
					tios.c_iflag = (tios.c_iflag & ~k->mask) | (k->value);
				if(strcmp(p+1, "sflow") == 0  ||  strncmp(p, "ixo", 3) == 0)
					sflow_changed = 1;
				if(strcmp(p+1, "iexten") == 0)
					iexten_changed = 1;
				changed = 1;
				break;
			case _TLOC:
				if(is_minus)
					tios.c_lflag = (tios.c_lflag & ~k->mask);
				else
					tios.c_lflag = (tios.c_lflag & ~k->mask) | (k->value);
				changed = 1;
				break;
			case _TOUT:
				if(is_minus)
					tios.c_oflag = (tios.c_oflag & ~k->mask);
				else
					tios.c_oflag = (tios.c_oflag & ~k->mask) | (k->value);
				changed = 1;
				break;
#ifndef __QNXNTO__
			case _TQF:
				if(is_minus)
					tios.c_qflag = (tios.c_qflag & ~k->mask);
				else
					tios.c_qflag = (tios.c_qflag & ~k->mask) | (k->value);
				changed = 1;
				break;
#endif
			case _TFLOW:
				if(is_minus)
					tcflow( 0, k->mask );
				else
					tcflow( 0, k->value );
				break;
			case _TEK:
				if(is_minus) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
				tios.c_cc[VERASE] = 0x7F;
				tios.c_cc[VKILL] = 0x18;
				changed = 1;
				break;
			case _TISPD:
				tios.c_ispeed = atol(p1);
				changed = 1;
				break;
			case _TOSPD:
				tios.c_ospeed = atol(p1);
				changed = 1;
				break;
			case _TBAUD:
				tios.c_ispeed = atol(p1);
				tios.c_ospeed = atol(p1);
				changed = 1;
				break;
			case _TFIX:
				if(is_minus) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
				if(tios.c_lflag & ICANON)
					goto tedit;
				else
					goto traw;
			case _TEDIT:
				if(is_minus) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
tedit:
				tios.c_iflag		= (tios.c_iflag & ~tios_mask.c_iflag)
									| edit_tios.c_iflag;
				tios.c_oflag		= (tios.c_oflag & ~tios_mask.c_oflag)
									| edit_tios.c_oflag;
				tios.c_lflag		= (tios.c_lflag & ~tios_mask.c_lflag)
									| edit_tios.c_lflag;
				tios.c_cflag		= (tios.c_cflag & ~tios_mask.c_cflag)
									| edit_tios.c_cflag;
				tios.c_cc[VEOF]		= 0x04;
				tios.c_cc[VEOL]		= _POSIX_VDISABLE;
				tios.c_cc[VERASE]	= 0x7F;
				tios.c_cc[VINTR]	= 0x03;
				tios.c_cc[VKILL]	= 0x15;
				tios.c_cc[VMIN]		= 0x01;
				tios.c_cc[VTIME]	= 0x00;
#ifdef __QNXNTO__
				tios.c_cc[VQUIT]	= 0x1c;
				tios.c_cc[VSUSP]	= 0x1a;
#else
				tios.c_cc[VQUIT]	= _POSIX_VDISABLE;
				tios.c_cc[VSUSP]	= _POSIX_VDISABLE;
#endif
				tios.c_cc[VSTART]	= 0x11;
				tios.c_cc[VSTOP]	= 0x13;
				changed = 1;
				break;
			case _TRAW:
				if(is_minus) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
traw:
				tios.c_iflag		= (tios.c_iflag & ~tios_mask.c_iflag)
									| raw_tios.c_iflag;
				tios.c_oflag		= (tios.c_oflag & ~tios_mask.c_oflag)
									| raw_tios.c_oflag;
				tios.c_lflag		= (tios.c_lflag & ~tios_mask.c_lflag)
									| raw_tios.c_lflag;
				tios.c_cflag		= (tios.c_cflag & ~tios_mask.c_cflag)
									| raw_tios.c_cflag;
				tios.c_cc[VEOF]		= 0x04;
				tios.c_cc[VEOL]		= _POSIX_VDISABLE;
				tios.c_cc[VERASE]	= 0x7F;
				tios.c_cc[VINTR]	= 0x03;
				tios.c_cc[VKILL]	= 0x15;
				tios.c_cc[VMIN]		= 0x01;
				tios.c_cc[VTIME]	= 0x00;
#ifdef __QNXNTO__
				tios.c_cc[VQUIT]	= 0x1c;
				tios.c_cc[VSUSP]	= 0x1a;
#else
				tios.c_cc[VQUIT]	= _POSIX_VDISABLE;
				tios.c_cc[VSUSP]	= _POSIX_VDISABLE;
#endif
				tios.c_cc[VSTART]	= 0x11;
				tios.c_cc[VSTOP]	= 0x13;
				changed = 1;
				break;
			case _TLOAD:
				if(is_minus) {
					fprintf(stderr,"%s: Invalid argument '%s'\n", argv[0], argv[arg]);
					break;
				}
				p = getenv( "TERM" );
				goto tterm;
			case _TTERM:
				p = p1;
tterm:
				if(p == 0  ||  strlen(p) == 0) {
					fprintf(stderr, "%s: Terminal type not defined\n", argv[0]);
					continue;
				}
				sprintf(&tfile[0], "/usr/lib/terminfo/%c/%s", p[0], p);
				fd = open( &tfile[0], O_RDONLY );
				if( fd == -1 ) {
					fprintf(stderr, "%s: Unable to open %s\n", argv[0], &tfile[0]);
					continue;
				}
				if( (n=read( fd, &thdr, sizeof(thdr))) != sizeof(thdr) ) {
					fprintf(stderr, "%s: Error reading file: %s\n", argv[0], &tfile[0]);
					continue;
				}


				/* QNX terminfo is little endian for now */
				thdr.magic = ENDIAN_LE16(thdr.magic);
				thdr.name_size = ENDIAN_LE16(thdr.name_size);
				thdr.bool_count = ENDIAN_LE16(thdr.bool_count);
				thdr.num_count = ENDIAN_LE16(thdr.num_count);
				thdr.offset_count = ENDIAN_LE16(thdr.offset_count);
				thdr.str_size = ENDIAN_LE16(thdr.str_size);

				/*
				 * Extract keyboard edit codes
				 */
			    obase = sizeof(thdr);
			    obase += thdr.name_size;
			    obase += thdr.bool_count;
			    obase = (obase + 1) & 0xFFFE; /* Word align indices */
			    obase += thdr.num_count * 2;
			    sbase = obase + (thdr.offset_count * 2);

				get_str( KUP,    offsetof(struct _strs, _ky_up));
				get_str( KDOWN,  offsetof(struct _strs, _ky_down));
				get_str( KLEFT,  offsetof(struct _strs, _ky_left));
				get_str( KRIGHT, offsetof(struct _strs, _ky_right));
				get_str( KINS,   offsetof(struct _strs, _ky_ic));
				get_str( KDEL,   offsetof(struct _strs, _ky_dc));
				get_str( KHOME,  offsetof(struct _strs, _ky_home));
				get_str( KEND,   offsetof(struct _strs, _ky_end));

				for(prefix = 0; prefix < 4; ++prefix) {
					if(keys[KUP][prefix] == 0)
						break;
					for(i = 0; i < NKEYS; ++i) {
						if(keys[i][0] == 0)
							continue;
						if(keys[i][prefix] != keys[KUP][prefix])
							break;
					}				
					if(i < NKEYS)
						break;
				}

				for(suffix = 0; suffix < 4; ++suffix) {
					if(keys[KUP][prefix+1+suffix] == 0)
						break;
					for(i = 0; i < NKEYS; ++i) {
						if(keys[i][0] == 0)
							continue;
						if(keys[i][prefix+1+suffix] != keys[KUP][prefix+1+suffix])
							break;
					}				
					if(i < NKEYS)
						break;
				}

				for(i = 0; i < 4; ++i) {
					tios.c_cc[VPREFIX+i] = 0;
					tios.c_cc[VSUFFIX+i] = 0;
				}
				for(i = 0; i < prefix; ++i) {
					tios.c_cc[VPREFIX+i] = keys[KUP][i];					
				}
				for(i = 0; i < NKEYS; ++i) {
					tios.c_cc[key_idx[i]] = keys[i][prefix];
				}
				for(i = 0; i < suffix; ++i) {
					tios.c_cc[VSUFFIX+i] = keys[KUP][prefix+1+i];					
				}
				close( fd );
				changed = 1;
				break;
#ifndef __QNXNTO__
			case _TCON:
				cc = console_open(0, 0);
				if(cc == NULL) {
					fprintf(stderr,"%s: Not a console device\n", argv[0]);
					continue;
				}
				if(is_minus)
					console_ctrl(cc, cc->console, 0, k->value);
				else
					console_ctrl(cc, cc->console, k->value, k->value);
				console_close(cc);
				break;

			case _TIOCTL:
				/*
				 * Get state of hardware lines
				 */
				lval[0] = is_minus ? 0 : 0xFFFF;
				lval[1] = (long) k->value;
				if(qnx_ioctl( 0, QCTL_DEV_CTL, &lval[0], 8, &lval[0], 4 ) != 0 ) {
					fprintf(stderr, "STTY:  Unable to IOCTL\n");
				}
				break;
#endif

			case _TROWS:
				s = p1;
				while(*s && *s != ',') ++s;
				if(*s == ',')
					*s++ = 0;
				rows = atoi(p1);
#ifdef __QNXNTO__
				if(*s)
					cols = atoi(s);
				else
					tcgetsize(0, 0, &cols);
				i = tcsetsize(0, rows, cols);
#else
				cols = (*s) ? atoi(s) : -1;
				i = dev_size(0, rows, cols, &rows, &cols);
#endif
				if(i == -1) {
					fprintf(stderr,"%s: Unable to set device size\n", argv[0]);
				}
				break;

#ifndef __QNXNTO__
			case _TFONT:
				cc = console_open(0, 0);
				if(cc == NULL) {
					fprintf(stderr,"%s: Not a console device\n", argv[0]);
					continue;
				}
				i = console_font(cc, 0, atoi(p1) );
				if(i == -1) {
					fprintf(stderr,"%s: Unable to set console font\n", argv[0]);
				}
				console_close(cc);
				break;

			case _TPROTO:
				cc = console_open(0, 0);
				if(cc == NULL) {
					fprintf(stderr,"%s: Not a console device\n", argv[0]);
					continue;
				}
				i = console_protocol(cc, 0, atoi(p1) );
				if(i == -1) {
					fprintf(stderr,"%s: Unable to set console protocol\n", argv[0]);
				}
				console_close(cc);
				break;
#endif

			case _TPAR:
				for(i = 0; i < 5; ++i) {
					if(strcmp(p1, paritys[i].par_name) == 0) {
						tios.c_cflag = (tios.c_cflag & ~(PARENB|PARODD|PARSTK))
									 | paritys[i].par_value;
						/*
						 * Make sure that INPCK "tracks" PARENB
						 */
						if(tios.c_cflag & PARENB)
							tios.c_iflag |= INPCK;
						else
							tios.c_iflag &= ~INPCK;
						changed = 1;
						break;
					}
				}
				if(i < 5)
					break;
				fprintf(stderr, "%s: Invalid parity parameter (%s)\n", argv[0], p1);
				continue;

			case _TSTOP:
				i = atoi(p1);
				if(i < 1  ||  i > 2) {
					fprintf(stderr,"%s: Invalid stop bits parameter (%s)\n", argv[0], p1);
					continue;
				}
				if(i == 2)
					tios.c_cflag |= CSTOPB;
				else
					tios.c_cflag &= ~CSTOPB;
				changed = 1;
				break;

			case _TBITS:
				i = atoi(p1);
				if(i < 5  ||  i > 8) {
					fprintf(stderr, "%s: Invalid data bits parameter (%s)\n", argv[0], p1);
					continue;
					}
				tios.c_cflag = (tios.c_cflag & ~CSIZE) | ((i - 5) << 4);
				changed = 1;
				break;

			case _TFLUSH:
				tcflush( 0, TCIOFLUSH );
				break;

#ifndef __QNXNTO__
			case _TRESET:
					{
					struct _ser_reset ser_reset_msg;
					pid_t vid;

					dev_info(0, &dinfo);
					vid = qnx_vc_attach( dinfo.nid, dinfo.driver_pid,
										 sizeof(ser_reset_msg), 0 );

					memset( &ser_reset_msg, 0, sizeof(ser_reset_msg) );
					ser_reset_msg.type = _SER_RESET;
					ser_reset_msg.unit = dinfo.unit - 1;
					ser_reset_msg.major = dinfo.major;
					Send( vid, &ser_reset_msg, &ser_reset_msg,
						  sizeof(ser_reset_msg), sizeof(ser_reset_msg) );

					qnx_vc_detach( vid );
					break;
					}
#endif
			}
		}
	}

	if(changed) {
#ifndef __QNXNTO__
		if(   (hflow_changed   && (tios.c_qflag & TC_PROTECT_HFLOW) != 0)
		   || (iexten_changed  && (tios.c_qflag & TC_PROTECT_IEXTEN) != 0)
		   || (sflow_changed   && (tios.c_qflag & TC_PROTECT_SFLOW) != 0) ) {

			/*
			 * If hflow is locked, and hflow is being changed
			 * stty must temporarily "unlock" the hflow
			 */
			memcpy( &tios2, &tios, sizeof(tios) );
			if(hflow_changed)
				tios2.c_qflag &= ~(TC_PROTECT_HFLOW);
			if(sflow_changed)
				tios2.c_qflag &= ~(TC_PROTECT_SFLOW);
			if(iexten_changed)
				tios2.c_qflag &= ~(TC_PROTECT_IEXTEN);
			tcsetattr(0, TCSANOW, &tios2);
		}
#endif
		tcsetattr(0, TCSANOW, &tios);
	}

#ifndef __QNXNTO__
	dev_info(0, &dinfo);
#endif

	/*
	 * Display settings if no changes
	 */
	if(no_arg  ||  a_flag  ||  g_flag) {
		count = 0;
		if(g_flag == 0) {
#ifdef __QNXNTO__                                                             
			char *p;

			devctl(0, DCMD_CHR_TTYINFO, &info, sizeof info, 0);

			p=strrchr(info.ttyname,'/');
            if (p==NULL) p=info.ttyname;
			else p++;

			if (strncmp(p,"ser",3)==0) {
				devtype="serial";
			} else if (strncmp(p,"con",3)==0) {
				devtype="console";
			} else if (strncmp(p,"tty",3)==0) {
				devtype="pseudo";
			} else if (strncmp(p, "par", 3)==0) {
				devtype="parallel";
			} else {
				devtype="(Unknown)";
			}

			printf("Name:  %s\n", info.ttyname);
			printf("Type:  %s\n", devtype);
			printf("Opens: %d\n", info.opencount);

#else
			printf("Name:  %s\n", &dinfo.tty_name[0]);
			printf("Type:  %s\n", &dinfo.driver_type[0]);
			printf("Opens: %d (%c%c)\n", dinfo.open_count,
				(dinfo.flags & _DEV_IS_READERS) ? 'R' : '-',
				(dinfo.flags & _DEV_IS_WRITERS) ? 'W' : '-'
				);
			printf("Sigint Grp: %d, Sighup pid: %d\n", dinfo.pgrp, dinfo.session );

			devtype=dinfo.driver_type;
#endif
		}

		if(a_flag) {
			ref = &null_tios;
			mref = &null_tios;
		} else if(tios.c_lflag & ICANON) {
			ref = &edit_tios;
			mref = &tios_mask;
			printf("+edit");
			count += 5;
		} else {
			ref = &raw_tios;
			mref = &tios_mask;
			printf("+raw");
			count += 4;
		}
		if(a_flag && strcmp(devtype, "serial") != 0) {
			pr_opt("parenb", tios.c_cflag, PARENB, ref->c_cflag, mref->c_cflag, &count);
			pr_opt("parodd", tios.c_cflag, PARODD, ref->c_cflag, mref->c_cflag, &count);
			pr_opt("parstk", tios.c_cflag, PARSTK, ref->c_cflag, mref->c_cflag, &count);
			pr_opt("cstopb", tios.c_cflag, CSTOPB, ref->c_cflag, mref->c_cflag, &count);
			pr_opt("inpck",  tios.c_iflag, INPCK , 0, 0, &count);
		}
		pr_opt("hupcl",  tios.c_cflag, HUPCL , ref->c_cflag, mref->c_cflag, &count);
		pr_opt("cread",  tios.c_cflag, CREAD , ref->c_cflag, mref->c_cflag, &count);
		pr_opt("clocal", tios.c_cflag, CLOCAL, ref->c_cflag, mref->c_cflag, &count);

		pr_opt("isig",   tios.c_lflag, ISIG  , ref->c_lflag, mref->c_lflag, &count);
		pr_opt("icanon", tios.c_lflag, ICANON, ref->c_lflag, mref->c_lflag, &count);
		pr_opt("iexten", tios.c_lflag, IEXTEN, ref->c_lflag, mref->c_lflag, &count);
		pr_opt("echo",   tios.c_lflag, ECHO  , ref->c_lflag, mref->c_lflag, &count);
		pr_opt("echoe",  tios.c_lflag, ECHOE , ref->c_lflag, mref->c_lflag, &count);
		pr_opt("echok",  tios.c_lflag, ECHOK , ref->c_lflag, mref->c_lflag, &count);
#ifdef ECHOKE
		pr_opt("echoke", tios.c_lflag, ECHOKE, ref->c_lflag, mref->c_lflag, &count);
#endif
		pr_opt("echonl", tios.c_lflag, ECHONL, ref->c_lflag, mref->c_lflag, &count);
#ifdef ECHOCTL
		pr_opt("echoctl", tios.c_lflag, ECHOCTL, ref->c_lflag, mref->c_lflag, &count);
#endif
		pr_opt("noflsh", tios.c_lflag, NOFLSH, ref->c_lflag, mref->c_lflag, &count);
#ifdef TOPSTOP
		pr_opt("tostop", tios.c_lflag, TOSTOP, ref->c_lflag, mref->c_lflag, &count);
#endif

		pr_opt("ignbrk", tios.c_iflag, IGNBRK, ref->c_iflag, mref->c_iflag, &count);
		pr_opt("brkint", tios.c_iflag, BRKINT, ref->c_iflag, mref->c_iflag, &count);
		pr_opt("ignpar", tios.c_iflag, IGNPAR, ref->c_iflag, mref->c_iflag, &count);
		pr_opt("parmrk", tios.c_iflag, PARMRK, ref->c_iflag, mref->c_iflag, &count);
		pr_opt("istrip", tios.c_iflag, ISTRIP, ref->c_iflag, mref->c_iflag, &count);
		pr_opt("inlcr",  tios.c_iflag, INLCR , ref->c_iflag, mref->c_iflag, &count);
		pr_opt("igncr",  tios.c_iflag, IGNCR , ref->c_iflag, mref->c_iflag, &count);
		pr_opt("icrnl",  tios.c_iflag, ICRNL , ref->c_iflag, mref->c_iflag, &count);
#ifdef IMAXBEL
		pr_opt("imaxbel",  tios.c_iflag, IMAXBEL , ref->c_iflag, mref->c_iflag, &count);
#endif

		pr_opt("opost",  tios.c_oflag, OPOST , ref->c_oflag, mref->c_oflag, &count);
#ifdef ONLCR
		pr_opt("onlcr",  tios.c_oflag, ONLCR , ref->c_oflag, mref->c_oflag, &count);
#endif

		if(count > 0) {
			fprintf(stdout,"%c", term_char);
			count = 0;
		}

		pr_opt("isflow",  tios.c_iflag,  IXOFF,     0, a_flag ? 0 : -1, &count);
		pr_opt("osflow",  tios.c_iflag,  IXON,      0, a_flag ? 0 : -1, &count);
		pr_opt("ihflow",  tios.c_cflag,  IHFLOW,    0, a_flag ? 0 : -1, &count);
		pr_opt("ohflow",  tios.c_cflag,  OHFLOW,    0, a_flag ? 0 : -1, &count);
#ifndef __QNXNTO__
		pr_opt("lkhflow", tios.c_qflag,  TC_PROTECT_HFLOW, 0, a_flag ? 0 : -1, &count);
		pr_opt("lksflow", tios.c_qflag,  TC_PROTECT_SFLOW, 0, a_flag ? 0 : -1, &count);
		pr_opt("lkiexten", tios.c_qflag, TC_PROTECT_IEXTEN, a_flag ? 0 : TC_PROTECT_IEXTEN, a_flag ? 0 : -1, &count);
		pr_opt("wtsflow", tios.c_qflag,  TC_WAIT_SFLOW,    0, a_flag ? 0 : -1, &count);
		pr_opt("nopgrp",  tios.c_qflag,  TC_NOPGRP,        0, a_flag ? 0 : -1, &count);
		pr_opt("echoi",   tios.c_qflag,  TC_ECHOI,         0, a_flag ? 0 : -1, &count);
		pr_opt("ispaged", tios.c_status, TC_ISFLOW, 0, a_flag ? 0 : -1, &count);
		pr_opt("ospaged", tios.c_status, TC_OSFLOW, 0, a_flag ? 0 : -1, &count);
		pr_opt("ihpaged", tios.c_status, TC_IHFLOW, 0, a_flag ? 0 : -1, &count);
		pr_opt("ohpaged", tios.c_status, TC_OHFLOW, 0, a_flag ? 0 : -1, &count);
#endif

		if(count > 0)
			fprintf(stdout,"%c", term_char);

		for(n = 0, i = 0; i < NCCS; ++i) {
			if(a_flag  ||  tios.c_cc[i] != _POSIX_VDISABLE  ||  i==VMIN  ||  i==VTIME) {
				if(ccc[i][0] == '?'  ||  ccc[i][1] == '?'  ||  ccc[i][2] == '?')
					continue;
				if(i != VMIN && i != VTIME && tios.c_cc[i] == _POSIX_VDISABLE)
					printf( "%s=^- ", ccc[i] );
				else if(i != VMIN && i != VTIME && tios.c_cc[i] == 0x7F)
					printf( "%s=^? ", ccc[i] );
				else if(i != VMIN  &&  i != VTIME
						&&  tios.c_cc[i] <= 0x1F)
					printf( "%s=^%c ", ccc[i], tios.c_cc[i] + '@');
				else
					printf( "%s=%2.2X ", ccc[i], tios.c_cc[i]);
				if((++n % 8) == 0)
					printf( "%c", term_char);
			}
		}
		if((n % 8) != 0)
			printf( "%c", term_char);

		if(a_flag || strcmp(devtype, "serial") == 0) {
			if(tios.c_cflag & PARENB) {
				if(tios.c_cflag & PARSTK) {
					if(tios.c_cflag & PARODD)
						printf("par=mark");
					else
						printf("par=space");
				} else {
					if(tios.c_cflag & PARODD)
						printf("par=odd");
					else
						printf("par=even");
				}
				/*
				 * Show lack of INPCK when parity is enabled
				 * as a special (unusual) case
				 */
	            if((tios.c_iflag & INPCK) == 0)
					printf(" -inpck");
			} else {
				printf("par=none");
			}
			if(a_flag) {
	            if((tios.c_iflag & INPCK) != 0)
					printf(" +inpck");
			}
			printf(" bits=%d", (unsigned)(((tios.c_cflag & CSIZE) >> 4) + 5));
			if(tios.c_cflag & CSTOPB)
				printf(" stopb=2");
			else
				printf(" stopb=1");
			if(tios.c_ospeed == tios.c_ispeed) {
				printf(" baud=%ld", tios.c_ospeed);
			} else {
				printf(" ispeed=%ld", tios.c_ispeed);
				printf(" ospeed=%ld", tios.c_ospeed);
			}
#ifdef __QNXNTO__
			if(tcgetsize(0, &rows, &cols) != -1)
#else
			if(dev_size(0, -1, -1, &rows, &cols) != -1)
#endif
				printf( " rows=%d,%d", rows, cols );

			printf("%c", term_char);
		}

#ifndef __QNXNTO__
		if(strcmp(devtype, "console") == 0) {
			cc = console_open(0, 0);
			if(cc) {
				/*
				 * Get state of hardware lines
				 */
				lval[0] = 0;
				lval[1] = 0;
				if(qnx_ioctl( 0, QCTL_DEV_CTL, &lval[0], 8, &lval[0], 4 ) == 0 ) {
					printf("%cnumlock",  (lval[0] & 0x000002L) ? '+' : '-');
					printf(" %ccapslock", (lval[0] & 0x000004L) ? '+' : '-');
					printf(" %cscrlock", (lval[0] & 0x000001L) ? '+' : '-');
					}
				i = console_font(cc, 0, -1);
				if(i != -1)
					printf( " font=%d", i );

				i = console_protocol(cc, 0, -1);
				if(i != -1)
					printf( " protocol=%d", i );

				printf("%c", term_char);

				i = console_ctrl(cc, cc->console, 0, 0);

				if(i & CONSOLE_NOBOOT  ||  a_flag)
					printf( "%cnoboot",    (i & CONSOLE_NOBOOT)   ? '+' : '-');
				if(i & CONSOLE_NOSWITCH  ||  a_flag)
					printf( " %cnoswitch", (i & CONSOLE_NOSWITCH) ? '+' : '-');
				if(i & CONSOLE_NODEBUG  ||  a_flag)
					printf( " %cnodebug",  (i & CONSOLE_NODEBUG)  ? '+' : '-');
				if(i & CONSOLE_NORESIZE  ||  a_flag)
					printf( " %cnoresize", (i & CONSOLE_NORESIZE) ? '+' : '-');
				if(i & CONSOLE_NOHOTKEY  ||  a_flag)
					printf( " %cnohotkey", (i & CONSOLE_NOHOTKEY) ? '+' : '-');
				if(a_flag || (i & (CONSOLE_NOBOOT|CONSOLE_NOSWITCH|CONSOLE_NODEBUG|CONSOLE_NORESIZE|CONSOLE_NOHOTKEY)))
					printf( "%c", term_char);
				if(i & CONSOLE_INVISIBLE  ||  a_flag)
					printf( "%cinvisible", (i & CONSOLE_INVISIBLE) ? '+' : '-');
				if(i & CONSOLE_NOHSCROLL  ||  a_flag)
					printf( " %cnohscroll", (i & CONSOLE_NOHSCROLL) ? '+' : '-');
				if(i & CONSOLE_NOCOLOR  ||  a_flag)
					printf( " %cnocolor",   (i & CONSOLE_NOCOLOR)  ? '+' : '-');
				if(i & CONSOLE_MONOCURS ||  a_flag)
					printf( " %cmonocurs", (i & CONSOLE_MONOCURS) ? '+' : '-');
				if(i & CONSOLE_SCANMODE  ||  a_flag)
					printf( " %cscanmode", (i & CONSOLE_SCANMODE) ? '+' : '-');
				if(i & CONSOLE_EXTMODE  ||  a_flag)
					printf( " %cextmode",  (i & CONSOLE_EXTMODE)     ? '+' : '-');
				printf( "%c", term_char);
				console_close(cc);
			}
		}

		if(strcmp(devtype, "serial") == 0) {
			struct _ser_query		smsg;
			struct _ser_query_reply	rmsg;
			pid_t vid;

			memset( &smsg, 0, sizeof(smsg) );
			memset( &rmsg, 0, sizeof(rmsg) );
			smsg.type = _SER_QUERY;
			smsg.unit = dinfo.unit - 1;
			smsg.major = dinfo.major;

			/*
			 * Get state of hardware lines
			 */
			lval[0] = 0;
			lval[1] = 0;
			if(qnx_ioctl( 0, QCTL_DEV_CTL, &lval[0], 8, &lval[0], 4 ) == 0 ) {
				printf("%cDTR",  (lval[0] & 0x000001L) ? '+' : '-');
				printf(" %cRTS", (lval[0] & 0x000002L) ? '+' : '-');
				printf(" %cBRK", (lval[0] & 0x004000L) ? '+' : '-');
				if(g_flag == 0) {
					printf(" %ccts", (lval[0] & 0x100000L) ? '+' : '-');
					printf(" %cdsr", (lval[0] & 0x200000L) ? '+' : '-');
					printf(" %cri",  (lval[0] & 0x400000L) ? '+' : '-');
					printf(" %ccd",  (lval[0] & 0x800000L) ? '+' : '-');
					/*
					 * Query the driver process directly
					 */
					vid = qnx_vc_attach( dinfo.nid, dinfo.driver_pid,
							 sizeof(rmsg), 0 );
					if( Send( vid, &smsg, &rmsg,
							  sizeof(smsg), sizeof(rmsg)) == 0
							&&  rmsg.status == 0
							&&  rmsg.iobase != 0 ) {
						printf( " ioport=%03X irq=%d", rmsg.iobase, rmsg.irq );
						if(rmsg.slots != 0  &&  rmsg.slot != 0) {
							printf( " slot=%d", rmsg.slot );
						}
					}
					qnx_vc_detach( vid );
				}

				printf("%c", term_char);
			}
		}
		if(g_flag == 0  &&  strcmp(devtype, "parallel") == 0) {
			/*
			 * Get state of hardware lines
			 */
			lval[0] = 0;
			lval[1] = 0;
			if(qnx_ioctl( 0, QCTL_DEV_CTL, &lval[0], 8, &lval[0], 4 ) == 0 ) {
				printf("Printer State: ");
				printf("%cERR",  (lval[0] & 0x0008L) ? '-' : '+');
				printf(" %cONL", (lval[0] & 0x0010L) ? '+' : '-');
				printf(" %cPE",  (lval[0] & 0x0020L) ? '+' : '-');
				printf(" %cBSY", (lval[0] & 0x0080L) ? '-' : '+');
				printf("%c", term_char);
			}
		}
#else  /* NTO only section */
		if(strcmp(devtype, "parallel") ==0) {
		  lval[0]=0;
		  lval[1]=0;
		  if (devctl(0,DCMD_CHR_LINESTATUS, &lval[0], 4, NULL)==0) {
			printf("Printer State: ");
			printf("%cERR",  (lval[0] & 0x0008L) ? '-' : '+');
			printf(" %cONL", (lval[0] & 0x0010L) ? '+' : '-');
			printf(" %cPE",  (lval[0] & 0x0020L) ? '+' : '-');
			printf(" %cBSY", (lval[0] & 0x0080L) ? '-' : '+');
			printf("%c", term_char);
		  }
		}
#endif
	}

	return(EXIT_SUCCESS);
}
