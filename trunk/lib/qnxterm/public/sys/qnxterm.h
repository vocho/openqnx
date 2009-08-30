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
 *  qnxterm.h   Terminal capability definitions
 *

 */
#ifndef __QNXTERM_H_INCLUDED

#ifdef NCURSES_VERSION
#error "using a term_ header with the ncurses functions is probably wrong"
#endif

#ifndef __QNXNTO__
#ifndef __CONSOLE_H_INCLUDED
 #include <sys/console.h>
#endif

#ifndef __MOUSE_H_INCLUDED
 #include <sys/mouse.h>
#endif
#endif

#define TERM_VERSION    2

#pragma pack(1)
struct term_state {
    char           name[18];
    short          version;

    char           ptr_size;  /* Size of ptr in this process's memory model */
    char           int_size;  /* Size of an int to this process */
    unsigned char  *scrbuf,
                   *attrbuf;
    void           *terminfo; /* Pointer to terminfo data                   */
    unsigned       init_mode;

    int            num_rows,
                   num_cols,
                   num_rows_save,
                   num_cols_save;
    int            row,
                   col,
                   exit_char,
                   cache_attr,
                   cache_pos,
                   region1,
                   region2;

    unsigned       qnx_term:1,           /* non-zero if QNX extentions are valid */
                   is_mono:1,
                   insert_on:1,
                   scan_mouse:1,
                   scan_resize:1,
                   mouse_cursor:1,       /* non-zero if cursor displayed */
                   save_am_mode:1;

    unsigned       color,
                   fill,
                   line_amount,
                   screen_amount;

    unsigned char  line_set_on;

    unsigned char  old_attr;

        int            spare[4];

    unsigned       win_version;
    long           win_flags;
    /*  All these mouse_* members are used internally. Only
        mouse_row, and mouse_col will always be accurate for
        external use, and only after a K_MOUSE_POS event        */
    unsigned       mouse_flags;
    char           mouse_oldchar[2];    /* this holds attr & char */
    unsigned       mouse_old_buttons;
    short          mouse_dy,
                   mouse_dx;
    int            mouse_row,
                   mouse_col,
                   mouse_old_row,
                   mouse_old_col;
    short          mouse_yskip,
                   mouse_xskip,
                   mouse_click;

    unsigned       save_keystart1;
    unsigned       save_keystart2;

    int            cost_move,
                   cost_left,
                   cost_right,
                   cost_up,
                   cost_down,
                   cost_left_p,
                   cost_right_p,
                   cost_up_p,
                   cost_down_p;

    /**** NOTE: The order of box_ members can not be changed ****/
    char           box_vertical,
                   box_horizontal,
                   box_top_left,
                   box_top_right,
                   box_bot_left,
                   box_bot_right,
                   box_top_tee,
                   box_bot_tee,
                   box_left_tee,
                   box_right_tee,
                   box_cross,
                   box_solid_block,
                   box_shade_block,
                   box_bot_solid_block,
                   box_top_solid_block,
                   box_diamond,
                   box_degree,
                   box_plus_minus,
                   box_bullet,
                   box_arrow_left,
                   box_arrow_right,
                   box_arrow_down,
                   box_arrow_up,
                   box_board,
                   box_lantern,
                   box_spare[33];
    } ;
#pragma pack()


#ifdef __cplusplus
extern "C" {
#endif
extern struct   term_state term_state;

#if defined(__WATCOMC__) && !defined(__386__)
extern void __far *__memsetw( void far *__s, unsigned __c, size_t __n );
#pragma aux __memsetw = 0x89 0xfb /* mov bx,es */    \
                        0xf2 0xab /* rep stosw */    \
                        __parm [ ES DI ] [ AX ] [ CX ] \
                        __value [ ES BX ] __modify [ DI ];
#else
extern void *__memsetw( void *__s, unsigned __c, size_t __n );
#endif
extern void term_attr_type( int, int, const char *, int, unsigned,
                             unsigned, unsigned char );
extern void term_axis( int, int, int, int, int, int, int, unsigned );
extern void term_bar( int, int, int, unsigned, unsigned, unsigned );
extern void term_box( int, int, int, int, unsigned );
extern void term_box_fill( int, int, int, int, unsigned, unsigned, unsigned );
extern void term_box_on( void );
extern void term_box_off( void );
extern void term_clear( int );
extern void __term_clear_buffer( int );
extern void term_color( unsigned );
extern void term_cur( int, int );
extern int  term_delete_char( int );
extern int  term_delete_line( int, int );
extern void term_down( int );
extern unsigned  term_field( int, int, char *, int, const char *, unsigned );
extern void term_fill( unsigned );
extern void term_flush( void );
extern void term_home( void );
extern void term_init( void );
extern int  term_insert_char( int );
extern int  term_insert_line( int, int );
extern int  term_insert_on( void );
extern void term_insert_off( void );
extern unsigned term_key( void );
extern void term_left( int );
extern char * term_lmenu( int, int, const char * const *, const char *,
                          unsigned, unsigned *, unsigned );
extern int  term_load( void );
extern char * term_menu( int, int, const char * const *, const char *,
                          unsigned, unsigned *, unsigned );
extern int  term_mouse_on( void );
extern int  term_mouse_off( void );
#ifndef __QNXNTO__
extern int  term_mouse_process(unsigned *key, struct mouse_event *);
extern int  term_mouse_default(unsigned *key, struct mouse_event *);
extern int  term_mouse_handler(register int (*handler)(unsigned *key, struct mouse_event *));
extern unsigned term_mouse_flags(unsigned mask, unsigned bits);
extern int  term_mouse_hide(void);
extern int  term_mouse_move(int row, int col);
#endif
extern int  term_printf( int, int, unsigned, const char *, ... );
extern int  term_scroll_up( void );
extern int  term_scroll_down( void );
extern void term_get_line( int, int, char *, int );
extern unsigned term_receive( int );
extern int  term_relearn_size( void );
extern int  term_resize_on( void );
extern int  term_resize_off( void );
extern void term_restore( void );
extern int  term_restore_image( int, int, const char *, int );
extern void term_right( int );
extern int  term_save_image( int, int, char *, int );
extern int  term_window_scan(char *buf, int match, int *complete);
extern int  term_setup( void );
extern int  term_type( int, int, const char *, int, unsigned );
extern void term_unkey( unsigned );
extern void term_up( int );
extern void term_video_on( void );
extern void term_video_off( void );
#ifdef __cplusplus
};
#endif

/*
 *      Key definitions.
 */

#define K_TAB           0x0009    /* 0011*/
#define K_ESC           0x001b    /* 0033*/
#define K_ENTER         0x000d    /* 0015*/
#define K_BACKTAB       0x0161    /* 0541*/
#define K_CTL_TAB       0x0155    /* 0525*/

#define K_F1            0x0109    /* 0411*/
#define K_F2            0x010a    /* 0412*/
#define K_F3            0x010b    /* 0413*/
#define K_F4            0x010c    /* 0414*/
#define K_F5            0x010d    /* 0415*/
#define K_F6            0x010e    /* 0416*/
#define K_F7            0x010f    /* 0417*/
#define K_F8            0x0110    /* 0420*/
#define K_F9            0x0111    /* 0421*/
#define K_F10           0x0112    /* 0422*/
#define K_F11           0x0113    /* 0423*/
#define K_F12           0x0114    /* 0424*/

#define K_SHF_F1        0x0115    /* 0425*/
#define K_SHF_F2        0x0116    /* 0426*/
#define K_SHF_F3        0x0117    /* 0427*/
#define K_SHF_F4        0x0118    /* 0430*/
#define K_SHF_F5        0x0119    /* 0431*/
#define K_SHF_F6        0x011a    /* 0432*/
#define K_SHF_F7        0x011b    /* 0433*/
#define K_SHF_F8        0x011c    /* 0434*/
#define K_SHF_F9        0x011d    /* 0435*/
#define K_SHF_F10       0x011e    /* 0436*/
#define K_SHF_F11       0x011f    /* 0437*/
#define K_SHF_F12       0x0120    /* 0440*/
#define K_SHF_ENTER     0x0188    /* 0610*/
#define K_SHF_BACKSP    0x0193    /* 0623*/
#define K_SHF_KPDSTAR   K_PRTSC

#define K_CTL_F1        0x0121    /* 0441*/
#define K_CTL_F2        0x0122    /* 0442*/
#define K_CTL_F3        0x0123    /* 0443*/
#define K_CTL_F4        0x0124    /* 0444*/
#define K_CTL_F5        0x0125    /* 0445*/
#define K_CTL_F6        0x0126    /* 0446*/
#define K_CTL_F7        0x0127    /* 0447*/
#define K_CTL_F8        0x0128    /* 0450*/
#define K_CTL_F9        0x0129    /* 0451*/
#define K_CTL_F10       0x012a    /* 0452*/
#define K_CTL_F11       0x012b    /* 0453*/
#define K_CTL_F12       0x012c    /* 0454*/
#define K_CTL_ENTER     0x0157    /* 0527*/
#define K_CTL_BACKSP    0x0173    /* 0563*/
#define K_CTL_RUBOUT    K_CTL_BACKSP

#define K_ALT_F1        0x012d    /* 0455*/
#define K_ALT_F2        0x012e    /* 0456*/
#define K_ALT_F3        0x012f    /* 0457*/
#define K_ALT_F4        0x0130    /* 0460*/
#define K_ALT_F5        0x0131    /* 0461*/
#define K_ALT_F6        0x0132    /* 0462*/
#define K_ALT_F7        0x0133    /* 0463*/
#define K_ALT_F8        0x0134    /* 0464*/
#define K_ALT_F9        0x0135    /* 0465*/
#define K_ALT_F10       0x0136    /* 0466*/
#define K_ALT_F11       0x0137    /* 0467*/
#define K_ALT_F12       0x0138    /* 0470*/
#define K_ALT_ENTER     0x0191    /* 0621*/
#define K_ALT_BACKSP    0x0196    /* 0626*/

#define K_HOME          0x0106    /* 0406*/
#define K_UP            0x0103    /* 0403*/
#define K_PGUP          0x0153    /* 0523*/
#define K_KPDMINUS      0x0163    /* 0543*/
#define K_LEFT          0x0104    /* 0404*/
#define K_KPDFIVE       0x0165    /* 0545*/
#define K_RIGHT         0x0105    /* 0405*/
#define K_KPDPLUS       0x0181    /* 0601*/
#define K_END           0x0168    /* 0550*/
#define K_DOWN          0x0102    /* 0402*/
#define K_PGDN          0x0152    /* 0522*/
#define K_INSERT        0x014b    /* 0513*/
#define K_DELETE        0x014a    /* 0512*/
#define K_PRTSC         0x015a    /* 0532*/
#define K_SYSREQ        0x0197    /* 0627*/
#define K_BACKSP        0x0107    /* 0407*/
#define K_RUBOUT        0x007f    /* 0177*/
#define K_ERASE         0x0018    /* 0030*/

#define K_CTL_HOME      0x0187    /* 0607*/
#define K_CTL_UP        0x0150    /* 0520*/
#define K_CTL_PGUP      0x018e    /* 0616*/
#define K_CTL_KPDMINUS  0x017b    /* 0573*/
#define K_CTL_LEFT      0x0189    /* 0611*/
#define K_CTL_KPDFIVE   0x0166    /* 0546*/
#define K_CTL_RIGHT     0x0192    /* 0622*/
#define K_CTL_KPDPLUS   0x017c    /* 0574*/
#define K_CTL_END       0x0169    /* 0551*/
#define K_CTL_DOWN      0x0151    /* 0521*/
#define K_CTL_PGDN      0x018c    /* 0614*/
#define K_CTL_INSERT    0x0149    /* 0511*/
#define K_CTL_DELETE    0x0148    /* 0510*/
#define K_CTL_PRTSC     0x018f    /* 0617*/
#define K_CTL_KPDSTAR   K_CTRL_PRTSC

#define K_ALT_HOME      0x0162    /* 0542*/
#define K_ALT_UP        0x0182    /* 0602*/
#define K_ALT_PGUP      0x0172    /* 0562*/
#define K_ALT_KPDMINUS  0x0195    /* 0625*/
#define K_ALT_LEFT      0x018a    /* 0612*/
#define K_ALT_KPDFIVE   0x0167    /* 0547*/
#define K_ALT_RIGHT     0x018b    /* 0613*/
#define K_ALT_KPDPLUS   0x0194    /* 0624*/
#define K_ALT_END       0x014f    /* 0517*/
#define K_ALT_DOWN      0x0183    /* 0603*/
#define K_ALT_PGDN      0x016f    /* 0557*/
#define K_ALT_INSERT    0x014c    /* 0514*/
#define K_ALT_DELETE    0x014e    /* 0516*/
#define K_ALT_KPDSTAR   0x0190    /* 0620*/
#define K_ALT_PRTSC     K_SYSREQ

#define K_ALT_A         0x014d    /* 0515*/
#define K_ALT_B         0x0154    /* 0524*/
#define K_ALT_C         0x0164    /* 0544*/
#define K_ALT_D         0x0156    /* 0526*/
#define K_ALT_E         0x016d    /* 0555*/
#define K_ALT_F         0x016a    /* 0552*/
#define K_ALT_G         0x0175    /* 0565*/
#define K_ALT_H         0x016b    /* 0553*/
#define K_ALT_I         0x016e    /* 0556*/
#define K_ALT_J         0x0177    /* 0567*/
#define K_ALT_K         0x0171    /* 0561*/
#define K_ALT_L         0x0174    /* 0564*/
#define K_ALT_M         0x016c    /* 0554*/
#define K_ALT_N         0x017a    /* 0572*/
#define K_ALT_O         0x0170    /* 0560*/
#define K_ALT_P         0x0178    /* 0570*/
#define K_ALT_Q         0x0179    /* 0571*/
#define K_ALT_R         0x0176    /* 0566*/
#define K_ALT_S         0x017d    /* 0575*/
#define K_ALT_T         0x017e    /* 0576*/
#define K_ALT_U         0x0198    /* 0630*/
#define K_ALT_V         0x0180    /* 0600*/
#define K_ALT_W         0x0184    /* 0604*/
#define K_ALT_X         0x0185    /* 0605*/
#define K_ALT_Y         0x0186    /* 0606*/
#define K_ALT_Z         0x018d    /* 0615*/

#define K_RESIZE        0x0300    /* 1400*/
#define K_ACTIVE        0x0301    /* 1402 window made active */
#define K_INACTIVE      0x0302    /* 1403 window make in-active */
#define K_CLASS         0xF000    /* Class mask   */

#define K_MOUSE_EVENT   0x1000    /* Mouse class  */
#define K_MOUSE_BLEFT   0x0400    /* button codes */
#define K_MOUSE_BMIDDLE 0x0200
#define K_MOUSE_BRIGHT  0x0100
#define K_MOUSE_XMASK   0x0070    /* X & Y offsets */
#define K_MOUSE_XDIR    0x0080    /* Direction bit, 1 for negative */
#define K_MOUSE_YMASK   0x0007
#define K_MOUSE_YDIR    0x0008

#define K_MOUSE_POS         0x2000    /* Cursor position class */
#define K_MOUSE_ETC         0x00f0    /* extra data. e.g. number of clicks */
#define K_MOUSE_ACTION      0x000f    /* Action mask  */
#define K_MOUSE_BSELECT     0x0400    /* Select button */
#define K_MOUSE_BADJUST     0x0200    /* Adjust button */
#define K_MOUSE_BMENU       0x0100    /* Menu button */
#define K_MOUSE_BUTTONS     (K_MOUSE_BSELECT|K_MOUSE_BADJUST|K_MOUSE_BMENU)
#define K_MOUSE_NULL        0x0000    /* Event that doesn't change windows */
#define K_MOUSE_CLICK       0x0001    /* Button pressed */
#define K_MOUSE_CURSOR      0x0002    /* Mouse moved */
#define K_MOUSE_RELEASE     0x0003    /* Button released */

#define K_WIN_EVENT         0x3000    /* QNX windows event */
#define K_WIN_RESTORE       0x0001    /* window restored from icon */
#define K_WIN_ICON          0x0002    /* window made an icon */
#define K_WIN_STRING        0x0003    /* string to follow, terminated by '"' */

/*  Manifests for the term functions:   */

/*
 * ATTRIBUTE properties:
 */
#define TERM_NORMAL         0x0000
#define TERM_BLINK          0x0001
#define TERM_HILIGHT        0x0002
#define TERM_INVERSE        0x0004
#define TERM_ULINE          0x0008
#define TERM_FLUSH          0x0040

#define TERM_BLACK          0x8000
#define TERM_BLUE           0x8100
#define TERM_GREEN          0x8200
#define TERM_CYAN           0x8300
#define TERM_RED            0x8400
#define TERM_MAGENTA        0x8500
#define TERM_YELLOW         0x8600
#define TERM_WHITE          0x8700

#define TERM_BLACK_BG       0x8000
#define TERM_BLUE_BG        0x9000
#define TERM_GREEN_BG       0xA000
#define TERM_CYAN_BG        0xB000
#define TERM_RED_BG         0xC000
#define TERM_MAGENTA_BG     0xD000
#define TERM_YELLOW_BG      0xE000
#define TERM_WHITE_BG       0xF000

/*
 * term_axis()
 */
#define TERM_HOR_UP     0
#define TERM_HOR_DN     1
#define TERM_VERT_L     2
#define TERM_VERT_R     3

#define TERM_NO_CAP     0
#define TERM_CAP_LOW    1
#define TERM_CAP_HI     2
#define TERM_CAP_BOTH   3

/*
 * term_bar()
 */
#define TERM_BAR_HORIZ  0
#define TERM_BAR_VERT   1
#define TERM_BAR_CV_1   2
#define TERM_BAR_CV_2   3

/*
 * term_box_fill()
 */
#define TERM_BOX_NO_FRAME   0
#define TERM_BOX_FRAME      1
#define TERM_BOX_DOUBLE     2
#define TERM_BOX_DBL_TOP    3
#define TERM_BOX_DBL_SIDE   4

/*
 * term_clear()
 */
#define TERM_CLS_SCR        0
#define TERM_CLS_EOL        1
#define TERM_CLS_EOS        2
#define TERM_CLS_SCRH       3

/*
 * term_menu()
 */
#define TERM_MENU_DISPLAY   0x01
#define TERM_MENU_NO_CANCEL 0x02
#define TERM_MENU_UNKNOWN   0x04
#define TERM_MENU_NO_CEOL   0x08
#define TERM_MENU_NO_MOUSE  0x10

/*
 * term_printf()
 */
#define TERM_PRINTF_MAX     160

/*
 * term_mouse_flags()
 */
#define TERM_MOUSE_FOLLOW    0x0001    /* term_state.mouse_row/col follow mouse */
#define TERM_MOUSE_ADJUST    0x0002    /* like 'a' pane option in Qwindows */
#define TERM_MOUSE_HELD      0x0004    /* line 'B' pane option in Qwindows */
#define TERM_MOUSE_MOVED     0x0008    /* line 'c' pane option in Qwindows */
#define TERM_MOUSE_MENU      0x0010    /* line 'M' pane option in Qwindows */
#define TERM_MOUSE_SELECT    0x0020    /* line 'S' pane option in Qwindows */
#define TERM_MOUSE_RELEASE   0x0040    /* line 'u' pane option in Qwindows */
#define TERM_MOUSE_REVERSE   0x8000    /* for left-handed person */


#define __QNXTERM_H_INCLUDED
#endif

