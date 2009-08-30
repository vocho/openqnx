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




#undef __INLINE_FUNCTIONS__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/term.h>
#include <sys/qnxterm.h>

#define t	term_state
#define ct	__cur_term

/*	returns 0 for no match, or K_EVENT_POS event to match keys
	match is # of chars in buf to match. complete is changed
	to 1 if buf can match a valid sequence, and is set to 2 if
	it does match a valid sequence.
*/
int
term_window_scan(char *buf, int match, int *complete) {
	int	flag = 0;
	int i, num[10];
	long flagmask;
	char *p1, *p2;
	int l1, l2;
	int type;

/* @@@ max_micro_address		madde#	Yd#	Click Messages 1=QNX, 2=Xterm */
	type = max_micro_address;
	if ( type != 1 && type != 2 ) {
		return(0);
		}
/* @@@ change_res_horz	==	QNX/Xterm Mouse Prefix 1 */
	l1 = strlen( p1 = change_res_horz );
/* @@@ change_res_vert	==	QNX/Xterm Mouse Prefix 2 */
	l2 = strlen( p2 = change_res_vert );
	i = 0;

	if ( *p1 && !strncmp( p1, buf, min( match, l1 ) ) ) {
		if ( match >= l1 ) {
			i = 1;
			}
		else {
			*complete = 1;
			}
		}
	if ( *p2 && !strncmp( p2, buf, min( match, l2 ) ) ) {
		if ( match >= l2 ) {
			i = 2;
			}
		else {
			*complete = 1;
			}
		}
	if ( i == 0 ) {
		return(0);
		}

	match -= l1;
	buf += l1; 

	if ( type == 2 ) {
		if ( match < 3 ) {
			*complete = 1;
			}
		else {
			*complete = 2;
			ct->_shift_held = ( buf[0] & 4 ) ? 1 : 0;
			ct->_alt_held = ( buf[0] & 8 ) ? 1 : 0;
			ct->_ctrl_held = ( buf[0] & 16 ) ? 1 : 0;
			term_state.mouse_col = buf[1] - '!';
			term_state.mouse_row = buf[2] - '!';
			switch( buf[0] & 0x03 ) {
			case 0:
				if ( t.mouse_flags & TERM_MOUSE_SELECT )
					return( K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BSELECT|1<<4 );
				break;
			case 1:
				if ( t.mouse_flags & TERM_MOUSE_ADJUST )
					return( K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BADJUST|1<<4 );
				break;
			case 2:
				if ( t.mouse_flags & TERM_MOUSE_MENU )
					return( K_MOUSE_POS|K_MOUSE_CLICK|K_MOUSE_BMENU|1<<4 );
				break;
			case 3:
				if ( t.mouse_flags & TERM_MOUSE_RELEASE )
					return( K_MOUSE_POS|K_MOUSE_RELEASE|K_MOUSE_BUTTONS );
				break;
			default:
				*complete = 0;
				break;
				}
			}
		return(0);
		}

	if ( i == 2 ) {
		*complete = 2;
		return( K_WIN_EVENT | K_WIN_STRING );
		}

	if ( match && buf[0] == '>' ) {
		flag = 1;
		buf++;
		match--;
		}

	num[i = 0] = 0;
	while ( match ) {
		if ( *buf >= '0' && *buf <= '9' ) {
			if ( num[i] > 3276 ) break;
			num[i] = num[i] * 10 + *buf - '0';
			}
		else if ( *buf == ';' ) {
			num[++i] = 0;
			if ( i >= 10 ) break;
			}
		else if ( !flag && *buf == 't' ) {
			*complete = 2;
			switch(num[0]) {
			case 1:
				if(i == 0) return( K_WIN_EVENT|K_WIN_RESTORE );
				break;
			case 2:
				if(i == 0) return( K_WIN_EVENT|K_WIN_ICON );
				break;
			case 5:
				if(i == 0) return( K_ACTIVE );
				break;
			case 6:
				if(i == 0) return( K_INACTIVE );
				break;
			case 8:
				if(i == 2) {
					term_state.num_rows = num[1];
					term_state.num_cols = num[2];
					return( K_RESIZE );
					}
				break;
			case 3:	/* pos in tips */
			case 4: /* size in tips */
			case 9: /* logical size */
				break;
			case 10:
				if(i == 1) term_state.win_version = num[1];
				break;
			case 31:
				if(i == 4) {
					term_state.mouse_row = num[1];
					term_state.mouse_col = num[2];					
					flag = (num[3]<<8) & K_MOUSE_BUTTONS;
					if(	((term_state.mouse_flags & TERM_MOUSE_SELECT) &&
						 (flag & K_MOUSE_BSELECT)) ||
						((term_state.mouse_flags & TERM_MOUSE_ADJUST) &&
						 (flag & K_MOUSE_BADJUST)) ||
						((term_state.mouse_flags & TERM_MOUSE_MENU) &&
						 (flag & K_MOUSE_BMENU))	) {
						return( K_MOUSE_POS|K_MOUSE_CLICK + flag +
								((num[4]<<4) & K_MOUSE_ETC) );
						}
					}
				break;
			case 32:
				if(i == 3) {
					term_state.mouse_row = num[1];
					term_state.mouse_col = num[2];					
					if(term_state.mouse_flags & TERM_MOUSE_RELEASE) {
						return( K_MOUSE_POS|K_MOUSE_RELEASE +
								((num[3]<<8) & K_MOUSE_BUTTONS) );
						}
					}
				break;
			case 33:
				if(i == 3) {
					term_state.mouse_row = num[1];
					term_state.mouse_col = num[2];					
					if(	(term_state.mouse_flags & TERM_MOUSE_MOVED) ||
						(	(term_state.mouse_flags & TERM_MOUSE_HELD) &&
							num[3])	) {
						return( K_MOUSE_POS|K_MOUSE_CURSOR +
								((num[3]<<8) & K_MOUSE_BUTTONS) );
						}
					}
				break;
				}
			return 0;
			}
		else if ( flag && (*buf == 'h' || *buf == 'l' )) {
			for(flag = 0; flag <= i; flag++) {
				flagmask = 1 << (num[flag] - 1);
				term_state.win_flags &= ~flagmask;
				if(*buf == 'h') term_state.win_flags |= flagmask;
				}
			*complete = 2;
			return 0;
			}
		else break;
		match--;
		buf++;
		}
	if ( match == 0 ) *complete = 1;
	return 0;
	}
