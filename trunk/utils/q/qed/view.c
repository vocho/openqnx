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





#include <stdio.h>
#include "manif.h"
#include "struct.h"
#include <sys/qnxterm.h>

#define EXT extern
#include "externs.h"

int view(int laddr) {
	int n, c;
	char c1;

	if(laddr < 0)
		return(ERROR5);

	if(laddr)
		curln = laddr;

	switch(c1 = *lp++) {

	case 'a':
		if((c  = getint() - 1) < STATUS_AREA   ||  c  > TEXT_AREA)
			return(ERROR5);
		if(*lp++ != ' ')
			return(ERROR5);

/*
		n = ((n = getint()) > 7) ? (n << 8) | 0x02 : n << 8;
		attributes[c] = (attributes[c] & 0x00fd) | n | 0x8000;
*/
/*	n = ((n = getint()) > 7 && dev_type < 3) ? (n << 8) | 0x02 : n << 8;	@@@	*/
		n = ((n = getint()) > 7) ? (n << 8) | 0x02 : n << 8;
		if(*lp++ != ' ')
			return(ERROR5);
		n = (n & 0x0fff) | ((getint() & 0x7) << 12);
		attributes[c] = (attributes[c] & 0x00fd) | n | 0x8000;
		if(c == 2) {
			term_fill(attributes[2]);
			term_color(attributes[2]);
			term_clear(TERM_CLS_SCR);
			}
		mark_line(0);
		clr_flag = 1;
		return(OK);

	case 'c':
		if((n = getint()) < 0  ||  n > screen_height-SCREEN_OFFSET)
			return(ERROR5);

		if(n)
			center_line = n - 1;
		else {
			screen_row = imax(1, imin(lastln - center_line, curln + 1));
			clear_screen1(1);
			clr_flag = 1;
			mark_line(0);
			update_screen();
			}

		return(OK);

	case 'f':
	case 'q':
		view_quick = !view_quick;
		return(OK);

	case 'l':
		n = left_margin;
		goto marg;
	case 'r':
		n = right_margin;
marg:
		if((c = *lp++) == '+')
			n += getint();
		else if(c == '-')
			n -= getint();
		else if(c == '.')
			n = curcol;
		else {
			--lp;
			n = getint();
			}

		if(c1 == 'l') {
			left_margin = imin(right_margin-1, imax(1, n));
			cc_reg = (n == left_margin);
			}
		else {
			right_margin = imax(left_margin+1, imin(LINE_LENGTH, n));
			cc_reg = (n == right_margin);
			}

		firstp->lflags |= DIRTY_FLAG;
		if(nmarks)
			mark_line(marker1);

		return(OK);

/*
	case 'm':
		menu = !menu;
		return(OK);
*/

	case 's':
		if(*lp == '-') {
			++lp;
			n = -getint();
			}
		else
			n = getint();

		screen_row = imax(1, imin(lastln - center_line, screen_row + n));
		curln = imax(1, imin(lastln, curln + n));

		clear_screen1(1);
		clr_flag = 1;
		mark_line(0);
		update_screen();
		return(OK);

	case 't':
		if((n = getint()) == 2  ||  n == 4  ||  n == 8) {
			tab_len = n - 1;
			mark_line(0);
			update_screen();
			}
		return(OK);

	case 'z':
/*
		if(zoom > NSCREEN_SIZES  ||  screen_sizes[zoom] == 0)
			zoom = 1;
		set_video_size(zoom);
*/
		clear_screen1(1);
		clr_flag = 1;
		mark_line(0);
		update_screen();
		return(OK);
		}

	return(ERROR2);
	}
