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
#include <string.h>
#include <sys/term.h>
#include <sys/qnxterm.h>
#include <malloc.h>
#pragma aux __STKOVERFLOW aborts;
extern void __STKOVERFLOW( void );

/* Frame Type 1 */

#define TL1     0xc9 
#define TR1     0xbb
#define BL1     0xc8
#define BR1     0xbc
#define V1      0xba
#define H1      0xcd

/* Frame Type 2 */

#define TL2     0xd5
#define TR2     0xb8
#define BL2     0xd4
#define BR2     0xbe
#define V2      0xb3
#define H2      0xcd

/* Frame Type 3 */

#define TL3     0xd6
#define TR3     0xb7
#define BL3     0xd3
#define BR3     0xbd
#define V3      0xba
#define H3      0xc4

#define t	term_state
#define ct	__cur_term

void term_box_fill( row, col, width, height, attr, frame, fill_char )
	int row, col, width, height;
	unsigned attr, frame, fill_char;
	{
	int		full;
	char	*line;
	unsigned char tl ='+', tr = '+', bl = '+', br = '+', v = '|', h = '-', i;

	if ( ( line = alloca( width + 1 ) ) == NULL ) {
		return;
		}
/* @@@ col_addr_glitch == IBM graphics characters are available */
	if ( frame > TERM_BOX_FRAME && t.qnx_term && col_addr_glitch ) {
		switch( frame ) {
			case TERM_BOX_DOUBLE:
				tl = TL1;
				tr = TR1;
				bl = BL1;
				br = BR1;
				v  = V1;
				h  = H1;
				break;
  
			case TERM_BOX_DBL_TOP:
				tl = TL2;
				tr = TR2;
				bl = BL2;
				br = BR2;
				v  = V2;
				h  = H2;
				break;

			case TERM_BOX_DBL_SIDE:
				tl = TL3;
				tr = TR3;
				bl = BL3;
				br = BR3;
				v  = V3;
				h  = H3;
				break;
			}
		}
	else {
		switch( frame ) {
			case TERM_BOX_NO_FRAME:
				tl = tr = bl = br = v = h = fill_char;
				break;

			default:	/* A non-QNX terminal always uses a single line	*/
				tl = t.box_top_left;
				tr = t.box_top_right;
				bl = t.box_bot_left;
				br = t.box_bot_right;
				v  = t.box_vertical;
				h  = t.box_horizontal;
				break;
			}
		}
	memset( line, h, width );
	line[width] = '\0';

	line[ 0       ] = tl;
	line[ width-1 ] = tr;
	term_box_on();
	term_type( row, col, line, width, attr );

	line[ 0       ] = bl;
	line[ width-1 ] = br;
	term_type( row + height - 1, col, line, width, attr );

	memset( line, fill_char, width );

	/* a fill char of 0xffff means don't fill */
	full = (fill_char != 0xffff) || (col + width >= t.num_cols-1);
	/*
	Draw each vertical line seperately to minimize output data.
	If drawing to last column, then draw together to avoid 
	eat_newline_glitch
	*/
	for( i = 1 ; i < height - 1 ; ++i ) {
		term_type( row + i, col, &v, 1, attr );
		if ( fill_char != 0xffff && width >= 2) {
			term_box_off();
			term_type( row + i, col + 1, line, width - 2, attr);
			term_box_on();
			}
		if ( full )
			term_type( row + i, col + width - 1, &v, 1, attr );
		}
	if ( !full )
		for( i = 1 ; i < height - 1 ; ++i )
			term_type( row + i, col + width - 1, &v, 1, attr );

	term_box_off();
	}

void
term_box_on() {
	if ( !ct->_cc && !t.line_set_on )
		__putp( enter_alt_charset_mode );
	t.line_set_on = 1;
	}

void
term_box_off() {
	if ( !ct->_cc && t.line_set_on )
		__putp( exit_alt_charset_mode );
	t.line_set_on = 0;
	}
