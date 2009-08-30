/*
 * $QNXtpLicenseC:
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
 *  term.h      Terminal information definitions
 */
#ifndef _TERM_H_INCLUDED

#ifdef NCURSES_VERSION
#error "using a term_ header with the ncurses functions is probably wrong"
#endif

#ifndef _TERMIOS_H_INCLUDED
 #include <termios.h>
#endif

#ifndef __QNXNTO__
#ifndef _CONSOLE_H_INCLUDED
 #include <sys/console.h>
#endif

#ifndef _MOUSE_H_INCLUDED
 #include <sys/mouse.h>
#endif

#else
typedef short int mpid_t;
#endif

#ifndef _STDIO_H_INCLUDED
 #include <stdio.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#if	!defined(__cplusplus) || (__WATCOMC__ < 1100)
typedef char bool;
#endif

typedef short int chtype;
typedef char *charptr;
typedef short int charoffset;

#define CUR     __cur_term->
#define CURS(s) ( (char *) (&__cur_term->_strtab[0] + __cur_term->s) )

#define auto_left_margin            CUR _bools._auto_left_margin
#define auto_right_margin           CUR _bools._auto_right_margin
#define no_esc_ctlc                 CUR _bools._no_esc_ctlc
#define ceol_standout_glitch        CUR _bools._ceol_standout_glitch
#define eat_newline_glitch          CUR _bools._eat_newline_glitch
#define erase_overstrike            CUR _bools._erase_overstrike
#define generic_type                CUR _bools._generic_type
#define hard_copy                   CUR _bools._hard_copy
#define has_meta_key                CUR _bools._has_meta_key
#define has_status_line             CUR _bools._has_status_line
#define insert_null_glitch          CUR _bools._insert_null_glitch
#define memory_above                CUR _bools._memory_above
#define memory_below                CUR _bools._memory_below
#define move_insert_mode            CUR _bools._move_insert_mode
#define move_standout_mode          CUR _bools._move_standout_mode
#define over_strike                 CUR _bools._over_strike
#define status_line_esc_ok          CUR _bools._status_line_esc_ok
#define dest_tabs_magic_smso        CUR _bools._dest_tabs_magic_smso
#define tilde_glitch                CUR _bools._tilde_glitch
#define transparent_underline       CUR _bools._transparent_underline
#define xon_xoff                    CUR _bools._xon_xoff
#define needs_xon_xoff              CUR _bools._needs_xon_xoff
#define prtr_silent                 CUR _bools._prtr_silent
#define hard_cursor                 CUR _bools._hard_cursor
#define non_rev_rmcup               CUR _bools._non_rev_rmcup
#define no_pad_char                 CUR _bools._no_pad_char
#define non_dest_scroll_region      CUR _bools._non_dest_scroll_region
#define can_change                  CUR _bools._can_change
#define back_color_erase            CUR _bools._back_color_erase
#define hue_lightness_saturation    CUR _bools._hue_lightness_saturation
#define col_addr_glitch             CUR _bools._col_addr_glitch
#define cr_cancels_micro_mode       CUR _bools._cr_cancels_micro_mode
#define has_print_wheel             CUR _bools._has_print_wheel
#define row_addr_glitch             CUR _bools._row_addr_glitch
#define semi_auto_right_margin      CUR _bools._semi_auto_right_margin
#define cpi_changes_res             CUR _bools._cpi_changes_res
#define lpi_changes_res             CUR _bools._lpi_changes_res

#define columns                     CUR _nums._columns
#define init_tabs                   CUR _nums._init_tabs
#define lines                       CUR _nums._lines
#define lines_of_memory             CUR _nums._lines_of_memory
#define magic_cookie_glitch         CUR _nums._magic_cookie_glitch
#define padding_baud_rate           CUR _nums._padding_baud_rate
#define virtual_terminal            CUR _nums._virtual_terminal
#define width_status_line           CUR _nums._width_status_line
#define num_labels                  CUR _nums._num_labels
#define label_height                CUR _nums._labl_height
#define label_width                 CUR _nums._labl_width
#define max_attributes              CUR _nums._max_attributes
#define maximum_windows             CUR _nums._maximum_windows
#define max_colors                  CUR _nums._max_colors
#define max_pairs                   CUR _nums._max_pairs
#define no_color_video              CUR _nums._no_color_video
#define buffer_capacity             CUR _nums._buffer_capacity
#define dot_vert_spacing            CUR _nums._dot_vert_spacing
#define dot_horz_spacing            CUR _nums._dot_horz_spacing
#define max_micro_address           CUR _nums._max_micro_address
#define max_micro_jump              CUR _nums._max_micro_jump
#define micro_char_size             CUR _nums._micro_char_size
#define micro_line_size             CUR _nums._micro_line_size
#define number_of_pins              CUR _nums._number_of_pins
#define output_res_char             CUR _nums._output_res_char
#define output_res_line             CUR _nums._output_res_line
#define output_res_horz_inch        CUR _nums._output_res_horz_inch
#define output_res_vert_inch        CUR _nums._output_res_vert_inch
#define print_rate                  CUR _nums._print_rate
#define wide_char_size              CUR _nums._wide_char_size

#define back_tab                    CURS( _strs._back_tab )
#define bell                        CURS( _strs._bell )
#define carriage_return             CURS(   _strs._carriage_return )
#define change_scroll_region        CURS(   _strs._change_scroll_region )
#define clear_all_tabs              CURS(   _strs._clear_all_tabs )
#define clear_screen                CURS(   _strs._clear_screen )
#define clr_eol                     CURS(   _strs._clr_eol )
#define clr_eos                     CURS(   _strs._clr_eos )
#define column_address              CURS(   _strs._column_address )
#define command_character           CURS(   _strs._command_character )
#define cursor_address              CURS(   _strs._crsr_address )
#define cursor_down                 CURS(   _strs._crsr_down )
#define cursor_home                 CURS(   _strs._crsr_home )
#define cursor_invisible            CURS(   _strs._crsr_invisible )
#define cursor_left                 CURS(   _strs._crsr_left )
#define cursor_mem_address          CURS(   _strs._crsr_mem_address )
#define cursor_normal               CURS(   _strs._crsr_normal )
#define cursor_right                CURS(   _strs._crsr_right )
#define cursor_to_ll                CURS(   _strs._crsr_to_ll )
#define cursor_up                   CURS(   _strs._crsr_up )
#define cursor_visible              CURS(   _strs._crsr_visible )
#define delete_character            CURS(   _strs._dlt_character )
#define delete_line                 CURS(   _strs._dlt_line )
#define dis_status_line             CURS(   _strs._dis_status_line )
#define down_half_line              CURS(   _strs._down_half_line )

#define enter_alt_charset_mode      CURS(   _strs._entr_alt_charset_mode )
#define enter_blink_mode            CURS(   _strs._entr_blink_mode )
#define enter_bold_mode             CURS(   _strs._entr_bold_mode )
#define enter_ca_mode               CURS(   _strs._entr_ca_mode )
#define enter_delete_mode           CURS(   _strs._entr_delete_mode )
#define enter_dim_mode              CURS(   _strs._entr_dim_mode )
#define enter_insert_mode           CURS(   _strs._entr_insert_mode )
#define enter_secure_mode           CURS(   _strs._entr_secure_mode )
#define enter_protected_mode        CURS(   _strs._entr_protected_mode )
#define enter_reverse_mode          CURS(   _strs._entr_reverse_mode )
#define enter_standout_mode         CURS(   _strs._entr_standout_mode )
#define enter_underline_mode        CURS(   _strs._entr_underline_mode )
#define exit_alt_charset_mode       CURS(   _strs._exit_alt_charset_mode )
#define exit_attribute_mode         CURS(   _strs._exit_attribute_mode )
#define exit_ca_mode                CURS(   _strs._exit_ca_mode )
#define exit_delete_mode            CURS(   _strs._exit_delete_mode )
#define exit_insert_mode            CURS(   _strs._exit_insert_mode )
#define exit_standout_mode          CURS(   _strs._exit_standout_mode )
#define exit_underline_mode         CURS(   _strs._exit_underline_mode )
#define flash_screen                CURS(   _strs._flash_screen )
#define form_feed                   CURS(   _strs._form_feed )
#define from_status_line            CURS(   _strs._from_status_line )
#define init_1string                CURS(   _strs._init_1string )
#define init_2string                CURS(   _strs._init_2string )
#define init_3string                CURS(   _strs._init_3string )
#define init_file                   CURS(   _strs._init_file )
#define insert_character            CURS(   _strs._ins_character )
#define insert_line                 CURS(   _strs._ins_line )
#define insert_padding              CURS(   _strs._insert_padding )
#define key_backspace               CURS(   _strs._ky_backspace )
#define key_catab                   CURS(   _strs._ky_catab )
#define key_clear                   CURS(   _strs._ky_clear )
#define key_ctab                    CURS(   _strs._ky_ctab )
#define key_dc                      CURS(   _strs._ky_dc )
#define key_dl                      CURS(   _strs._ky_dl )
#define key_down                    CURS(   _strs._ky_down )
#define key_eic                     CURS(   _strs._ky_eic )
#define key_eol                     CURS(   _strs._ky_eol )
#define key_eos                     CURS(   _strs._ky_eos )
#define key_f0                      CURS(   _strs._ky_f0 )
#define key_f1                      CURS(   _strs._ky_f1 )
#define key_f10                     CURS(   _strs._ky_f10 )
#define key_f2                      CURS(   _strs._ky_f2 )
#define key_f3                      CURS(   _strs._ky_f3 )
#define key_f4                      CURS(   _strs._ky_f4 )
#define key_f5                      CURS(   _strs._ky_f5 )
#define key_f6                      CURS(   _strs._ky_f6 )
#define key_f7                      CURS(   _strs._ky_f7 )
#define key_f8                      CURS(   _strs._ky_f8 )
#define key_f9                      CURS(   _strs._ky_f9 )
#define key_home                    CURS(   _strs._ky_home )
#define key_ic                      CURS(   _strs._ky_ic )
#define key_il                      CURS(   _strs._ky_il )
#define key_left                    CURS(   _strs._ky_left )
#define key_ll                      CURS(   _strs._ky_ll )
#define key_npage                   CURS(   _strs._ky_npage )
#define key_ppage                   CURS(   _strs._ky_ppage )
#define key_right                   CURS(   _strs._ky_right )
#define key_sf                      CURS(   _strs._ky_sf )
#define key_sr                      CURS(   _strs._ky_sr )
#define key_stab                    CURS(   _strs._ky_stab )
#define key_up                      CURS(   _strs._ky_up )
#define keypad_local                CURS(   _strs._kpad_local )
#define keypad_xmit                 CURS(   _strs._kpad_xmit )
#define lab_f0                      CURS(   _strs._lab_f0 )
#define lab_f1                      CURS(   _strs._lab_f1 )
#define lab_f10                     CURS(   _strs._lab_f10 )
#define lab_f2                      CURS(   _strs._lab_f2 )
#define lab_f3                      CURS(   _strs._lab_f3 )
#define lab_f4                      CURS(   _strs._lab_f4 )
#define lab_f5                      CURS(   _strs._lab_f5 )
#define lab_f6                      CURS(   _strs._lab_f6 )
#define lab_f7                      CURS(   _strs._lab_f7 )
#define lab_f8                      CURS(   _strs._lab_f8 )
#define lab_f9                      CURS(   _strs._lab_f9 )
#define meta_off                    CURS(   _strs._meta_off )
#define meta_on                     CURS(   _strs._meta_on )
#define newline                     CURS(   _strs._newline )
#define pad_char                    CURS(   _strs._pad_char )
#define parm_dch                    CURS(   _strs._prm_dch )
#define parm_delete_line            CURS(   _strs._prm_delete_line )
#define parm_down_cursor            CURS(   _strs._prm_down_cursor )
#define parm_ich                    CURS(   _strs._prm_ich )
#define parm_index                  CURS(   _strs._prm_index )
#define parm_insert_line            CURS(   _strs._prm_insert_line )
#define parm_left_cursor            CURS(   _strs._prm_left_cursor )
#define parm_right_cursor           CURS(   _strs._prm_right_cursor )
#define parm_rindex                 CURS(   _strs._prm_rindex )
#define parm_up_cursor              CURS(   _strs._prm_up_cursor )
#define pkey_key                    CURS(   _strs._pkey_key )
#define pkey_local                  CURS(   _strs._pkey_local )
#define pkey_xmit                   CURS(   _strs._pkey_xmit )
#define print_screen                CURS(   _strs._print_screen )
#define prtr_off                    CURS(   _strs._prtr_off )
#define prtr_on                     CURS(   _strs._prtr_on )
#define repeat_char                 CURS(   _strs._repeat_char )
#define reset_1string               CURS(   _strs._reset_1string )
#define reset_2string               CURS(   _strs._reset_2string )
#define reset_3string               CURS(   _strs._reset_3string )
#define reset_file                  CURS(   _strs._reset_file )
#define restore_cursor              CURS(   _strs._restore_cursor )
#define row_address                 CURS(   _strs._row_address )
#define save_cursor                 CURS(   _strs._savecursor )
#define scroll_forward              CURS(   _strs._scroll_forward )
#define scroll_reverse              CURS(   _strs._scroll_reverse )
#define set_attributes              CURS(   _strs._set_attributes )
#define set_tab                     CURS(   _strs._set_tab )
#define set_window                  CURS(   _strs._set_window )
#define tab                         CURS(   _strs._tab )
#define to_status_line              CURS(   _strs._to_status_line )
#define underline_char              CURS(   _strs._underline_char )
#define up_half_line                CURS(   _strs._up_half_line )
#define init_prog                   CURS(   _strs._init_prog )
#define key_a1                      CURS(   _strs._ky_a1 )
#define key_a3                      CURS(   _strs._ky_a3 )
#define key_b2                      CURS(   _strs._ky_b2 )
#define key_c1                      CURS(   _strs._ky_c1 )
#define key_c3                      CURS(   _strs._ky_c3 )
#define prtr_non                    CURS(   _strs._prtr_non )

#define char_padding                CURS( _strs._char_padding )
#define acs_chars                   CURS( _strs._acs_chars )
#define plab_norm                   CURS( _strs._plab_norm )
#define key_btab                    CURS( _strs._ky_btab )
#define enter_xon_mode              CURS( _strs._entr_xon_mode )
#define exit_xon_mode               CURS( _strs._exit_xon_mode )
#define enter_am_mode               CURS( _strs._entr_am_mode )
#define exit_am_mode                CURS( _strs._exit_am_mode )
#define xon_character               CURS( _strs._xon_character )
#define xoff_character              CURS( _strs._xoff_character )
#define ena_acs                     CURS( _strs._ena_acs )
#define label_on                    CURS( _strs._labl_on )
#define label_off                   CURS( _strs._labl_off )
#define key_beg                     CURS( _strs._ky_beg )
#define key_cancel                  CURS( _strs._ky_cancel )
#define key_close                   CURS( _strs._ky_close )
#define key_command                 CURS( _strs._ky_command )
#define key_copy                    CURS( _strs._ky_copy )
#define key_create                  CURS( _strs._ky_create )
#define key_end                     CURS( _strs._ky_end )
#define key_enter                   CURS( _strs._ky_enter )
#define key_exit                    CURS( _strs._ky_exit )
#define key_find                    CURS( _strs._ky_find )
#define key_help                    CURS( _strs._ky_help )
#define key_mark                    CURS( _strs._ky_mark )
#define key_message                 CURS( _strs._ky_message )
#define key_move                    CURS( _strs._ky_move )
#define key_next                    CURS( _strs._ky_next )
#define key_open                    CURS( _strs._ky_open )
#define key_options                 CURS( _strs._ky_options )
#define key_previous                CURS( _strs._ky_previous )
#define key_print                   CURS( _strs._ky_print )
#define key_redo                    CURS( _strs._ky_redo )
#define key_reference               CURS( _strs._ky_reference )
#define key_refresh                 CURS( _strs._ky_refresh )
#define key_replace                 CURS( _strs._ky_replace )
#define key_restart                 CURS( _strs._ky_restart )
#define key_resume                  CURS( _strs._ky_resume )
#define key_save                    CURS( _strs._ky_save )
#define key_suspend                 CURS( _strs._ky_suspend )
#define key_undo                    CURS( _strs._ky_undo )
#define key_sbeg                    CURS( _strs._ky_sbeg )
#define key_scancel                 CURS( _strs._ky_scancel )
#define key_scommand                CURS( _strs._ky_scommand )
#define key_scopy                   CURS( _strs._ky_scopy )
#define key_screate                 CURS( _strs._ky_screate )
#define key_sdc                     CURS( _strs._ky_sdc )
#define key_sdl                     CURS( _strs._ky_sdl )
#define key_select                  CURS( _strs._ky_select )
#define key_send                    CURS( _strs._ky_send )
#define key_seol                    CURS( _strs._ky_seol )
#define key_sexit                   CURS( _strs._ky_sexit )
#define key_sfind                   CURS( _strs._ky_sfind )
#define key_shelp                   CURS( _strs._ky_shelp )
#define key_shome                   CURS( _strs._ky_shome )
#define key_sic                     CURS( _strs._ky_sic )
#define key_sleft                   CURS( _strs._ky_sleft )
#define key_smessage                CURS( _strs._ky_smessage )
#define key_smove                   CURS( _strs._ky_smove )
#define key_snext                   CURS( _strs._ky_snext )
#define key_soptions                CURS( _strs._ky_soptions )
#define key_sprevious               CURS( _strs._ky_sprevious )
#define key_sprint                  CURS( _strs._ky_sprint )
#define key_sredo                   CURS( _strs._ky_sredo )
#define key_sreplace                CURS( _strs._ky_sreplace )
#define key_sright                  CURS( _strs._ky_sright )
#define key_srsume                  CURS( _strs._ky_srsume )
#define key_ssave                   CURS( _strs._ky_ssave )
#define key_ssuspend                CURS( _strs._ky_ssuspend )
#define key_sundo                   CURS( _strs._ky_sundo )
#define req_for_input               CURS( _strs._req_for_input )
#define key_f11                     CURS( _strs._ky_f11 )
#define key_f12                     CURS( _strs._ky_f12 )
#define key_f13                     CURS( _strs._ky_f13 )
#define key_f14                     CURS( _strs._ky_f14 )
#define key_f15                     CURS( _strs._ky_f15 )
#define key_f16                     CURS( _strs._ky_f16 )
#define key_f17                     CURS( _strs._ky_f17 )
#define key_f18                     CURS( _strs._ky_f18 )
#define key_f19                     CURS( _strs._ky_f19 )
#define key_f20                     CURS( _strs._ky_f20 )
#define key_f21                     CURS( _strs._ky_f21 )
#define key_f22                     CURS( _strs._ky_f22 )
#define key_f23                     CURS( _strs._ky_f23 )
#define key_f24                     CURS( _strs._ky_f24 )
#define key_f25                     CURS( _strs._ky_f25 )
#define key_f26                     CURS( _strs._ky_f26 )
#define key_f27                     CURS( _strs._ky_f27 )
#define key_f28                     CURS( _strs._ky_f28 )
#define key_f29                     CURS( _strs._ky_f29 )
#define key_f30                     CURS( _strs._ky_f30 )
#define key_f31                     CURS( _strs._ky_f31 )
#define key_f32                     CURS( _strs._ky_f32 )
#define key_f33                     CURS( _strs._ky_f33 )
#define key_f34                     CURS( _strs._ky_f34 )
#define key_f35                     CURS( _strs._ky_f35 )
#define key_f36                     CURS( _strs._ky_f36 )
#define key_f37                     CURS( _strs._ky_f37 )
#define key_f38                     CURS( _strs._ky_f38 )
#define key_f39                     CURS( _strs._ky_f39 )
#define key_f40                     CURS( _strs._ky_f40 )
#define key_f41                     CURS( _strs._ky_f41 )
#define key_f42                     CURS( _strs._ky_f42 )
#define key_f43                     CURS( _strs._ky_f43 )
#define key_f44                     CURS( _strs._ky_f44 )
#define key_f45                     CURS( _strs._ky_f45 )
#define key_f46                     CURS( _strs._ky_f46 )
#define key_f47                     CURS( _strs._ky_f47 )
#define key_f48                     CURS( _strs._ky_f48 )
#define key_f49                     CURS( _strs._ky_f49 )
#define key_f50                     CURS( _strs._ky_f50 )
#define key_f51                     CURS( _strs._ky_f51 )
#define key_f52                     CURS( _strs._ky_f52 )
#define key_f53                     CURS( _strs._ky_f53 )
#define key_f54                     CURS( _strs._ky_f54 )
#define key_f55                     CURS( _strs._ky_f55 )
#define key_f56                     CURS( _strs._ky_f56 )
#define key_f57                     CURS( _strs._ky_f57 )
#define key_f58                     CURS( _strs._ky_f58 )
#define key_f59                     CURS( _strs._ky_f59 )
#define key_f60                     CURS( _strs._ky_f60 )
#define key_f61                     CURS( _strs._ky_f61 )
#define key_f62                     CURS( _strs._ky_f62 )
#define key_f63                     CURS( _strs._ky_f63 )
#define clr_bol                     CURS( _strs._clr_bol )
#define clear_margins               CURS( _strs._clear_margins )
#define set_left_margin             CURS( _strs._set_left_margin )
#define set_right_margin            CURS( _strs._set_right_margin )
#define label_format                CURS( _strs._labl_format )
#define set_clock                   CURS( _strs._set_clock )
#define display_clock               CURS( _strs._display_clock )
#define remove_clock                CURS( _strs._remove_clock )
#define create_window               CURS( _strs._create_window )
#define goto_window                 CURS( _strs._goto_window )
#define hangup                      CURS( _strs._hangup )
#define dial_phone                  CURS( _strs._dial_phone )
#define quick_dial                  CURS( _strs._quick_dial )
#define tone                        CURS( _strs._tone )
#define pulse                       CURS( _strs._pulse )
#define flash_hook                  CURS( _strs._flash_hook )
#define fixed_pause                 CURS( _strs._fixed_pause )
#define wait_tone                   CURS( _strs._wait_tone )
#define user0                       CURS( _strs._user0 )
#define user1                       CURS( _strs._user1 )
#define user2                       CURS( _strs._user2 )
#define user3                       CURS( _strs._user3 )
#define user4                       CURS( _strs._user4 )
#define user5                       CURS( _strs._user5 )
#define user6                       CURS( _strs._user6 )
#define user7                       CURS( _strs._user7 )
#define user8                       CURS( _strs._user8 )
#define user9                       CURS( _strs._user9 )
#define orig_pair                   CURS( _strs._orig_pair )
#define orig_colors                 CURS( _strs._orig_colors )
#define initialize_color            CURS( _strs._initialize_color )
#define initialize_pair             CURS( _strs._initialize_pair )
#define set_color_pair              CURS( _strs._set_color_pair )
#define set_foreground              CURS( _strs._set_foreground )
#define set_background              CURS( _strs._set_background )
#define change_char_pitch           CURS( _strs._change_char_pitch )
#define change_line_pitch           CURS( _strs._change_line_pitch )
#define change_res_horz             CURS( _strs._change_res_horz )
#define change_res_vert             CURS( _strs._change_res_vert )
#define define_char                 CURS( _strs._define_char )
#define enter_doublewide_mode       CURS( _strs._entr_doublewide_mode )
#define enter_draft_quality         CURS( _strs._entr_draft_quality )
#define enter_italics_mode          CURS( _strs._entr_italics_mode )
#define enter_leftward_mode         CURS( _strs._entr_leftward_mode )
#define enter_micro_mode            CURS( _strs._entr_micro_mode )
#define enter_near_letter_quality   CURS( _strs._entr_near_letter_quality )
#define enter_normal_quality        CURS( _strs._entr_normal_quality )
#define enter_shadow_mode           CURS( _strs._entr_shadow_mode )
#define enter_subscript_mode        CURS( _strs._entr_subscript_mode )
#define enter_superscript_mode      CURS( _strs._entr_superscript_mode )
#define enter_upward_mode           CURS( _strs._entr_upward_mode )
#define exit_doublewide_mode        CURS( _strs._exit_doublewide_mode )
#define exit_italics_mode           CURS( _strs._exit_italics_mode )
#define exit_leftward_mode          CURS( _strs._exit_leftward_mode )
#define exit_micro_mode             CURS( _strs._exit_micro_mode )
#define exit_shadow_mode            CURS( _strs._exit_shadow_mode )
#define exit_subscript_mode         CURS( _strs._exit_subscript_mode )
#define exit_superscript_mode       CURS( _strs._exit_superscript_mode )
#define exit_upward_mode            CURS( _strs._exit_upward_mode )
#define micro_column_address        CURS( _strs._micro_column_address )
#define micro_down                  CURS( _strs._micro_down )
#define micro_left                  CURS( _strs._micro_left )
#define micro_right                 CURS( _strs._micro_right )
#define micro_row_address           CURS( _strs._micro_row_address )
#define micro_up                    CURS( _strs._micro_up )
#define order_of_pins               CURS( _strs._order_of_pins )
#define parm_down_micro             CURS( _strs._prm_down_micro )
#define parm_left_micro             CURS( _strs._prm_left_micro )
#define parm_right_micro            CURS( _strs._prm_right_micro )
#define parm_up_micro               CURS( _strs._prm_up_micro )
#define select_char_set             CURS( _strs._select_char_set )
#define set_bottom_margin           CURS( _strs._set_bottom_margin )
#define set_bottom_margin_parm      CURS( _strs._set_bottom_margin_parm )
#define set_left_margin_parm        CURS( _strs._set_left_margin_parm )
#define set_right_margin_parm       CURS( _strs._set_right_margin_parm )
#define set_top_margin              CURS( _strs._set_top_margin )
#define set_top_margin_parm         CURS( _strs._set_top_margin_parm )
#define start_bit_image             CURS( _strs._start_bit_image )
#define start_char_set_def          CURS( _strs._start_char_set_def )
#define stop_bit_image              CURS( _strs._stop_bit_image )
#define stop_char_set_def           CURS( _strs._stop_char_set_def )
#define subscript_characters        CURS( _strs._subscript_characters )
#define superscript_characters      CURS( _strs._superscript_characters )
#define these_cause_cr              CURS( _strs._these_cause_cr )
#define zero_motion                 CURS( _strs._zero_motion )
#define char_set_names              CURS( _strs._char_set_names )

#pragma pack(1)

struct _strs {
    charoffset
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
    _ky_end,                    /*  KEY_END, 0550, end key                  */
    _ky_enter,                  /*  KEY_ENTER, 0527, Enter/send: unreliable */
    _ky_exit,                   /*  KEY_EXIT, 0551, exit key                */
    _ky_find,                   /*  KEY_FIND, 0552, find key                */
    _ky_help,                   /*  KEY_HELP, 0553, help key                */
    _ky_mark,                   /*  KEY_MARK, 0554, mark key                */
    _ky_message,                /*  KEY_MESSAGE, 0555, message key          */
    _ky_move,                   /*  KEY_MOVE, 0556, move key                */
    _ky_next,                   /*  KEY_NEXT, 0557, next object key         */
    _ky_open,                   /*  KEY_OPEN, 0560, open key                */
    _ky_options,                /*  KEY_OPTIONS, 0561, options key          */
    _ky_previous,               /*  KEY_PREVIOUS, 0562, previous object key */
    _ky_print,                  /*  KEY_PRINT, 0532, print or copy          */
    _ky_redo,                   /*  KEY_REDO, 0563, redo key                */
    _ky_reference,              /*  KEY_REFERENCE, 0564, ref(erence) key    */
    _ky_refresh,                /*  KEY_REFRESH, 0565, refresh key          */
    _ky_replace,                /*  KEY_REPLACE, 0566, replace key          */
    _ky_restart,                /*  KEY_RESTART, 0567, restart key          */
    _ky_resume,                 /*  KEY_RESUME, 0570, resume key            */
    _ky_save,                   /*  KEY_SAVE, 0571, save key                */
    _ky_suspend,                /*  KEY_SUSPEND, 0627, suspend key          */
    _ky_undo,                   /*  KEY_UNDO, 0630, undo key                */
    _ky_sbeg,                   /*  KEY_SBEG, 0572, shifted beginning key   */
    _ky_scancel,                /*  KEY_SCANCEL, 0573, shifted cancel key   */
    _ky_scommand,               /*  KEY_SCOMMAND, 0574, shifted command key */
    _ky_scopy,                  /*  KEY_SCOPY, 0575, shifted copy key       */
    _ky_screate,                /*  KEY_SCREATE, 0576, shifted create key   */
    _ky_sdc,                    /*  KEY_SDC, 0577, shifted delete char key  */
    _ky_sdl,                    /*  KEY_SDL, 0600, shifted delete line key  */
    _ky_select,                 /*  KEY_SELECT, 0601, select key            */
    _ky_send,                   /*  KEY_SEND, 0602, shifted end key         */
    _ky_seol,                   /*  KEY_SEOL, 0603, shifted clear line key  */
    _ky_sexit,                  /*  KEY_SEXIT, 0604, shifted exit key       */
    _ky_sfind,                  /*  KEY_SFIND, 0605, shifted find key       */
    _ky_shelp,                  /*  KEY_SHELP, 0606, shifted help key       */
    _ky_shome,                  /*  KEY_SHOME, 0607, shifted home key       */
    _ky_sic,                    /*  KEY_SIC, 0610, shifted input key        */
    _ky_sleft,                  /*  KEY_SLEFT, 0611, shifted left arrow key */
    _ky_smessage,               /*  KEY_SMESSAGE, 0612, shifted message key */
    _ky_smove,                  /*  KEY_SMOVE, 0613, shifted move key       */
    _ky_snext,                  /*  KEY_SNEXT, 0614, shifted next key       */
    _ky_soptions,               /*  KEY_SOPTIONS, 0615, shifted options key */
    _ky_sprevious,              /*  KEY_SPREVIOUS, 0616, shifted prev key   */
    _ky_sprint,                 /*  KEY_SPRINT, 0617, shifted print key     */
    _ky_sredo,                  /*  KEY_SREDO, 0620, shifted redo key       */
    _ky_sreplace,               /*  KEY_SREPLACE, 0621, shifted replace key */
    _ky_sright,                 /*  KEY_SRIGHT, 0622, shifted right arrow   */
    _ky_srsume,                 /*  KEY_SRSUME, 0623, shifted resume key    */
    _ky_ssave,                  /*  KEY_SSAVE, 0624, shifted save key       */
    _ky_ssuspend,               /*  KEY_SSUSPEND, 0625, shifted suspend key */
    _ky_sundo,                  /*  KEY_SUNDO, 0626, shifted undo key       */
    _req_for_input,             /*  send next input char (for ptys)         */
    _ky_f11,                    /*  KEY_F(11), 0423, function key f11.      */
    _ky_f12,                    /*  KEY_F(12), 0424, function key f12.      */
    _ky_f13,                    /*  KEY_F(13), 0425, function key f13.      */
    _ky_f14,                    /*  KEY_F(14), 0426, function key f14.      */
    _ky_f15,                    /*  KEY_F(15), 0427, function key f15.      */
    _ky_f16,                    /*  KEY_F(16), 0430, function key f16.      */
    _ky_f17,                    /*  KEY_F(17), 0431, function key f17.      */
    _ky_f18,                    /*  KEY_F(18), 0432, function key f18.      */
    _ky_f19,                    /*  KEY_F(19), 0433, function key f19.      */
    _ky_f20,                    /*  KEY_F(20), 0434, function key f20.      */
    _ky_f21,                    /*  KEY_F(21), 0435, function key f21.      */
    _ky_f22,                    /*  KEY_F(22), 0436, function key f22.      */
    _ky_f23,                    /*  KEY_F(23), 0437, function key f23.      */
    _ky_f24,                    /*  KEY_F(24), 0440, function key f24.      */
    _ky_f25,                    /*  KEY_F(25), 0441, function key f25.      */
    _ky_f26,                    /*  KEY_F(26), 0442, function key f26.      */
    _ky_f27,                    /*  KEY_F(27), 0443, function key f27.      */
    _ky_f28,                    /*  KEY_F(28), 0444, function key f28.      */
    _ky_f29,                    /*  KEY_F(29), 0445, function key f29.      */
    _ky_f30,                    /*  KEY_F(30), 0446, function key f30.      */
    _ky_f31,                    /*  KEY_F(31), 0447, function key f31.      */
    _ky_f32,                    /*  KEY_F(32), 0450, function key f32.      */
    _ky_f33,                    /*  KEY_F(33), 0451, function key f33.      */
    _ky_f34,                    /*  KEY_F(34), 0452, function key f34.      */
    _ky_f35,                    /*  KEY_F(35), 0453, function key f35.      */
    _ky_f36,                    /*  KEY_F(36), 0454, function key f36.      */
    _ky_f37,                    /*  KEY_F(37), 0455, function key f37.      */
    _ky_f38,                    /*  KEY_F(38), 0456, function key f38.      */
    _ky_f39,                    /*  KEY_F(39), 0457, function key f39.      */
    _ky_f40,                    /*  KEY_F(40), 0460, function key f40.      */
    _ky_f41,                    /*  KEY_F(41), 0461, function key f41.      */
    _ky_f42,                    /*  KEY_F(42), 0462, function key f42.      */
    _ky_f43,                    /*  KEY_F(43), 0463, function key f43.      */
    _ky_f44,                    /*  KEY_F(44), 0464, function key f44.      */
    _ky_f45,                    /*  KEY_F(45), 0465, function key f45.      */
    _ky_f46,                    /*  KEY_F(46), 0466, function key f46.      */
    _ky_f47,                    /*  KEY_F(47), 0467, function key f47.      */
    _ky_f48,                    /*  KEY_F(48), 0470, function key f48.      */
    _ky_f49,                    /*  KEY_F(49), 0471, function key f49.      */
    _ky_f50,                    /*  KEY_F(50), 0472, function key f50.      */
    _ky_f51,                    /*  KEY_F(51), 0473, function key f51.      */
    _ky_f52,                    /*  KEY_F(52), 0474, function key f52.      */
    _ky_f53,                    /*  KEY_F(53), 0475, function key f53.      */
    _ky_f54,                    /*  KEY_F(54), 0476, function key f54.      */
    _ky_f55,                    /*  KEY_F(55), 0477, function key f55.      */
    _ky_f56,                    /*  KEY_F(56), 0500, function key f56.      */
    _ky_f57,                    /*  KEY_F(57), 0501, function key f57.      */
    _ky_f58,                    /*  KEY_F(58), 0502, function key f58.      */
    _ky_f59,                    /*  KEY_F(59), 0503, function key f59.      */
    _ky_f60,                    /*  KEY_F(60), 0504, function key f60.      */
    _ky_f61,                    /*  KEY_F(61), 0505, function key f61.      */
    _ky_f62,                    /*  KEY_F(62), 0506, function key f62.      */
    _ky_f63,                    /*  KEY_F(63), 0507, function key f63.      */
    _clr_bol,                   /*  Clear to beginning of line, inclusive   */
    _clear_margins,             /*  Clear left and right soft margins       */
    _set_left_margin,           /*  Set soft left margin                    */
    _set_right_margin,          /*  Set soft right margin                   */
    _labl_format,               /*  Label format                            */
    _set_clock,                 /*  Set time-of-day clock                   */
    _display_clock,             /*  Display time-of-day clock               */
    _remove_clock,              /*  Remove time-of-day clock                */
    _create_window,             /*  Define win #1 to go from #2,#3 to #4,#5 */
    _goto_window,               /*  Got to window #1                        */
    _hangup,                    /*  Hang-up phone                           */
    _dial_phone,                /*  Dial phone #1                           */
    _quick_dial,                /*  Dial phone #1, no progress detection    */
    _tone,                      /*  Select touch tone dialing               */
    _pulse,                     /*  Select pulse dialing                    */
    _flash_hook,                /*  Flash the switch hook                   */
    _fixed_pause,               /*  Pause for 2-3 seconds                   */
    _wait_tone,                 /*  Wait for dial tone                      */
    _user0,                     /*  User string 0                           */
    _user1,                     /*  User string 1                           */
    _user2,                     /*  User string 2                           */
    _user3,                     /*  User string 3                           */
    _user4,                     /*  User string 4                           */
    _user5,                     /*  User string 5                           */
    _user6,                     /*  User string 6                           */
    _user7,                     /*  User string 7                           */
    _user8,                     /*  User string 8                           */
    _user9,                     /*  User string 9                           */
    _orig_pair,                 /*  Original color-pair                     */
    _orig_colors,               /*  Original colors                         */
    _initialize_color,          /*  Initialize the definition of color      */
    _initialize_pair,           /*  Initialize color pair                   */
    _set_color_pair,            /*  Set color pair                          */
    _set_foreground,            /*  Set foregrounf color                    */
    _set_background,            /*  Set background color                    */
    _change_char_pitch,         /*  Change no. characters per inch          */
    _change_line_pitch,         /*  Change no. lines per inch               */
    _change_res_horz,           /*  Change horizontal resolution            */
    _change_res_vert,           /*  Change vertical resolution              */
    _define_char,               /*  Define a character in a character set   */
    _entr_doublewide_mode,      /*  Enable double wide printing             */
    _entr_draft_quality,        /*  Set draft quality print                 */
    _entr_italics_mode,         /*  Enable italics                          */
    _entr_leftward_mode,        /*  Enable leftward carriage motion         */
    _entr_micro_mode,           /*  Enable micro motion capabilities        */
    _entr_near_letter_quality,  /*  Set near-letter quality print           */
    _entr_normal_quality,       /*  Set normal quality print                */
    _entr_shadow_mode,          /*  Enable shadow printing                  */
    _entr_subscript_mode,       /*  Enable subscript printing               */
    _entr_superscript_mode,     /*  Enable superscript printing             */
    _entr_upward_mode,          /*  Enable upward carriage motion           */
    _exit_doublewide_mode,      /*  Disable double wide printing            */
    _exit_italics_mode,         /*  Disable italics                         */
    _exit_leftward_mode,        /*  Enable right (normal) carriage motion   */
    _exit_micro_mode,           /*  Disable micro motion capabilities       */
    _exit_shadow_mode,          /*  Disable shadow printing                 */
    _exit_subscript_mode,       /*  Disable subscript printing              */
    _exit_superscript_mode,     /*  Disable superscript printing            */
    _exit_upward_mode,          /*  Enable down (normal) carriage motion    */
    _micro_column_address,      /*  Like column_address for micro adjustmnt */
    _micro_down,                /*  Like cursor_down for micro adjustment   */
    _micro_left,                /*  Like cursor_left for micro adjustment   */
    _micro_right,               /*  Like cursor_right for micro adjustment  */
    _micro_row_address,         /*  Like row_address for micro adjustment   */
    _micro_up,                  /*  Like cursor_up for micro adjustment     */
    _order_of_pins,             /*  Matches software bits to print-head pin */
    _prm_down_micro,            /*  Like parm_down_cursor for micro adjust. */
    _prm_left_micro,            /*  Like parm_left_cursor for micro adjust. */
    _prm_right_micro,           /*  Like parm_right_cursor for micro adjust */
    _prm_up_micro,              /*  Like parm_up_cursor for micro adjust.   */
    _select_char_set,           /*  Select character set                    */
    _set_bottom_margin,         /*  Set soft bottom margin at current line  */
    _set_bottom_margin_parm,    /*  Set soft bottom margin                  */
    _set_left_margin_parm,      /*  Set soft left margin                    */
    _set_right_margin_parm,     /*  Set soft right margin                   */
    _set_top_margin,            /*  Set soft top margin at current line     */
    _set_top_margin_parm,       /*  Set soft top margin                     */
    _start_bit_image,           /*  Start printing bit image graphics       */
    _start_char_set_def,        /*  Start definition of a character set     */
    _stop_bit_image,            /*  End printing bit image graphics         */
    _stop_char_set_def,         /*  End definition of a character set       */
    _subscript_characters,      /*  List of ``subscript-able'' characters   */
    _superscript_characters,    /*  List of ``superscript-able'' characters */
    _these_cause_cr,            /*  Printing any of these chars causes cr   */
    _zero_motion,               /*  No motion for the subsequent character  */
    _char_set_names;            /*  List of character set names             */
};

struct _bool_struct {
    char
    _auto_left_margin,          /*  cub1 wraps from column 0 to last column */
    _auto_right_margin,         /*  Terminal has automatic margins          */
    _no_esc_ctlc,               /*  Beehive (f1=escape, f2=ctrl C)          */
    _ceol_standout_glitch,      /*  Standout not erased by overwriting (hp) */
    _eat_newline_glitch,        /*  newline ignored after 80 cols (Concept) */
    _erase_overstrike,          /*  Can erase overstrikes with a blank      */
    _generic_type,              /*  Generic line type (e.g. dialup, switch) */
    _hard_copy,                 /*  Hardcopy terminal                       */
    _has_meta_key,              /*  Has a meta key (shift, sets parity bit) */
    _has_status_line,           /*  Has extra "status line"                 */
    _ins_null_glitch,           /*  Insert mode distinguishes nulls         */
    _mem_above,                 /*  Display may be retained above the scrn  */
    _mem_below,                 /*  Display may be retained below the scrn  */
    _move_insert_mode,          /*  Safe to move while in insert mode       */
    _move_standout_mode,        /*  Safe to move in standout modes          */
    _over_strike,               /*  Terminal overstrikes                    */
    _status_line_esc_ok,        /*  Escape can be used on the status line   */
    _dest_tabs_magic_smso,      /*  Tabs destructive, magic so char (t1061) */
    _tilde_glitch,              /*  Hazeltine; can't print ~'s              */
    _transparent_underline,     /*  underline character overstrikes         */
    _xon_xoff,                  /*  Terminal uses xon/xoff handshaking      */
    _needs_xon_xoff,            /*  Padding won't work, xon/xoff required   */
    _prtr_silent,               /*  Printer won't echo on screen.           */
    _hard_cursor,               /*  Cursor is hard to see.                  */
    _non_rev_rmcup,             /*  Smcup does not reverse rmcup.           */
    _no_pad_char,               /*  Pad character doesn't exist.            */
    _non_dest_scroll_region,    /*  Scrolling region is non-destructive.    */
    _can_change,                /*  Can re-define existing color            */
    _back_color_erase,          /*  Erases screen with current background   */
    _hue_lightness_saturation,  /*  HLS color notation is used (Tek)        */
    _col_addr_glitch,           /*  Only positive motion for hpa/mhpa caps  */
    _cr_cancels_micro_mode,     /*  Using cr turns off micro mode           */
    _has_print_wheel,           /*  Printer needs operator to chng char set */
    _row_addr_glitch,           /*  Only positive motion for vpa/mvpa caps  */
    _semi_auto_right_margin,    /*  Printing in last column causes cr       */
    _cpi_changes_res,           /*  Changing char. pitch changes resolution */
    _lpi_changes_res,           /*  Changing line pitch changes resolution  */
    reserved[3];
};

struct _num_struct {
    short int
        _columns,               /*  Number of columns in a line             */
        _init_tabs,             /*  Tabs initially every # spaces.          */
        _lines,                 /*  Number of lines on screen or page       */
        _lines_of_memory,       /*  Lines of memory if > lines  0 => varies */
        _magic_cookie_glitch,   /*  Number blank chars left by smso or rmso */
        _padding_baud_rate,     /*  Lowest baud rate where padding needed   */
        _virtual_terminal,      /*  Virtual terminal number (CB/Unix)       */
        _width_status_line,     /*  # columns in status line                */
        _num_labels,            /*  # of labels on screen (start at 1)      */
        _labl_height,           /*  # rows in each label                    */
        _labl_width,            /*  # cols in each label                    */
        _max_attributes,        /*  max combined video attrs term displays  */
        _maximum_windows,       /*  Maximum number of defineable windows    */
        _max_colors,            /*  max # of color on the screen            */
        _max_pairs,             /*  max # of color pairs on the screen      */
        _no_color_video,        /*  Video attrs that can't be used w/ color */
        _buffer_capacity,       /*  Number of bytes buffered before print   */
        _dot_vert_spacing,      /*  Spacing of pins vert pins per inch      */
        _dot_horz_spacing,      /*  Spacing of dots hor dots per inch       */
        _max_micro_address,     /*  Maximum value in micro_..._address      */
        _max_micro_jump,        /*  Maximum value in parm_..._micro         */
        _micro_char_size,       /*  Character step size when in micro mode  */
        _micro_line_size,       /*  Line step size when in micro mode       */
        _number_of_pins,        /*  Number of pins in print-head            */
        _output_res_char,       /*  Horizontal resolution in units per char */
        _output_res_line,       /*  Vertical resolution in units per line   */
        _output_res_horz_inch,  /*  Horizontal resolution in units per inch */
        _output_res_vert_inch,  /*  Vertical resolution in units per inch   */
        _print_rate,            /*  Print rate in characters per second     */
        _wide_char_size;        /*  Char step size when in double wide mode */
};

/*
 *  This definition for term struct allows the boolean, number and string
 *  information to grow in the future and still allow object file
 *  compatibility.
 */

#define INP_QSIZE   32
#define _DELAY      0
#define _NODELAY    1
#define _HALFDELAY  2
#define _KPD_OFF    0
#define _KPD_ON     1
#define _KPD_DELAY  2
#define _VT52_CUP   1
#define _VT100_CUP  2
#define _TERMINFO_VERSION   1

struct term {
    int    Filedes;                 /* Output file descriptor               */
    int    _version;                /* Version of this struct               */
    struct termios Otty;            /* Old state of the tty link            */
    struct _bool_struct _bools;     /* The essential terminal information   */
    struct _num_struct  _nums;
    struct _strs        _strs;      /*  Offsets into the _strtab strings    */
    chtype  sgr_mode;               /* current phys. graphic rendition      */
    chtype  sgr_faked;              /* attributes faked by vidputs          */
    char    _inpmode;               /* input mode                           */
    char    _kpdmode;               /* Function key processing, 1=ON        */
    int     _delay;                 /* timeout for inputs                   */
    int     _inputfd;               /* input file descriptor                */
    FILE    *_outputfp;             /* stream of the Filedes                */
    unsigned keystart[16];          /* Bit array of start bytes for fn keys */
    bool    _fl_rawmode,            /* in cbreak(=1) or raw(=2) mode        */
            fl_typeahdok,           /* ok to use typeahead                  */
            _cursorstate,           /* cursor: 0=invis, 1=norm, 2=vvis      */
            _iwait;                 /* true if input-pending                */
    short   _regs[26];              /* tparm static registers               */
    short   _ungot_queue[INP_QSIZE],    /* Ungetch'ed integers              */
            _ungotten;              /* # chars ungotten by ungetch()        */
    char    _input_queue[INP_QSIZE];    /* Input bytes to process           */
    char    _chars_on_queue;        /* # chars on queue                     */
    char    _termname[15];          /* Terminal name from TERM envar        */
    char    _tty;                   /* 1=tty, 0=not a tty ( file ).         */
    char    _pad_char;              /* Padding char for delays              */
    bool    _key_null:1,            /* 1=report K_MOUSE_POS always          */
            _shift_held:1,          /* 1=Shift down valid only for mouse    */
            _ctrl_held:1,           /* 1=Ctrl down valid only for mouse     */
            _alt_held:1,            /* 1=Alt down valid only for mouse      */
            _extend_key:1,          /* 1=right Ctrl/Alt/Shift               */
            _console_input:1,       /* 1=input from "console" type device   */
            _input_winch:1,         /* 1=input will SIGWINCH                */
            _window_changed:1;      /* 1=got a SIGWINCH                     */
    char    _cup_type;              /* Cursor position type vt52=1, vt100=2 */
#ifndef __QNXNTO__
    nid_t   _inputfd_nid;           /* Node for input device                */
    nid_t   _outputfd_nid;          /* Node for output device               */
#endif
    mpid_t   _dproxy, _cproxy, _mproxy;  /* Dev, console and mouse proxy     */
    mpid_t   _dproxyr, _cproxyr, _mproxyr; /* Remote proxies */
    int     _dproxy_armed,
            _cproxy_armed,
            _mproxy_armed;
    short   _min, _time, _timeout, _nonblock, _was_timeout;
    int     _timerid;
    struct  _mouse_ctrl *_mm;
    struct  _console_ctrl *_cc;
#ifndef __QNXNTO__
    int     (*_mouse_handler)(unsigned *, struct mouse_event *);
#endif
    int     (*_win_key_scan)();
    char    *terminal_name;         /* Full name from terminfo file         */
    char    _strtab[1];             /* Extended array for string table      */
};
#pragma pack()

typedef struct term TERMINAL;

extern TERMINAL *__cur_term;

#ifdef __cplusplus
extern "C" {
#endif
extern int      __baudrate( void );
extern void     __del_curterm( struct term * );
extern int      __getch( void );
extern void     __halfdelay( int );
extern int      __has_colors( void );
extern void     __keypad( int, int );
extern void     __nodelay( int, int );
extern void     __notimeout( int, int );
extern char     __putchar( char );
extern void     __putp( char * );
extern void     __resetty( void );
extern void     __restartterm( char *, int, int * );
extern void     __savetty( void );
extern void     __set_curterm( struct term * );
extern void     __setupterm( char *, int, int * );
extern void     __tgetent( char *, char * );
extern int      __tgetflag( int );
extern int      __tgetnum( int );
extern char    *__tgetstr( char *, char * );
extern char    *__tgoto( char *, int, int );
extern char    *__tparm( char *, ... );
extern void     __tputs( char *, int, int (*)() );
extern void     __ungetch( int );
extern void     __vidattr( int );
extern void     __vidputs( int, int (*)() );
#ifdef __cplusplus
};
#endif

#define _TERM_H_INCLUDED
#endif
