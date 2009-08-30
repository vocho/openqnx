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
#include <sys/qnxterm.h>
#include <sys/term.h>

#define t	term_state
#define ct	__cur_term

void
term_color( unsigned color ) {
	unsigned f, b, c;

	if ( (color & 0xff00) == t.color ) return;
	if ( !(color & 0x8000) ) return;
	if ( t.is_mono ) {
		t.color = 0x0700;
		return;
		}
	else
		t.color = color & 0xff00;
	if ( !ct->_cc ) {
		color >>= 8;	/* Only interested in upper 8 bits	*/
		c = color & 0x0077;
		f = c & 0x07;
		b = c >> 4;
		c = f * 10 + b;
		if( *set_color_pair ) 
			__putp( __tparm( set_color_pair, c ) );
		else {
			__putp( __tparm( set_foreground, f ) );
			__putp( __tparm( set_background, b ) );
			}
		}
	}

void
term_fill( unsigned fill ) {
	unsigned f, b, c;

	if ( t.fill == fill ) return;
	if ( t.is_mono ) {
		t.fill = 0x0700;
		return;
		}
	if ( !(fill & 0x8000) ) return;
	t.fill = fill;
	if ( !ct->_cc && t.qnx_term ) {
		if ( *micro_row_address == NULL ) return;
#if 0	/* remove the hard-coded string */
		putchar( '\033' );
		putchar( '!' );
		putchar( ((fill & 0x0300) >> 8) + '0' );
		putchar( ((fill & 0x7000) >>12) + '0' );
		fflush( stdout );
#else
		c = ( fill & 0x7300 ) >> 8;
		f = c & 0x07;
		b = c >> 4;
		c = f * 10 + b;
/* @@@ enter_micro_mode == set fill color, QNX terminals use this */
		__putp( __tparm( micro_row_address, c ) );
#endif
		}
	}


#ifdef NEVER
	/* QNX to ANSI color mapping table	*/
	static char map[] = { 0, 4, 2, 6, 1, 5, 3, 7, 0, 0 };
	unsigned f, b, f1, b1, c;

	if ( !strncmp( t.term_name, "qnx", 3 ) ) {

		f = color & 0x7;
		b = (color >> 4) & 0x7;
/*
		if ( t.ansi_color_mapping == 'y' ) {
			f = map[ f ];
			b = map[ b ];
			}
*/	
		if ( t.color_for_first == 'n' ) {
			f1 = b + t.color_boff;
			b1 = f + t.color_foff;
			}
		else {
			f1 = f + t.color_foff;
			b1 = b + t.color_boff;
			}

	if ( t.color_type )
		if ( (
		/*
			Since ANSI terminals clear colors when they clear attributes, we
			must always force the colors back on.
		*/
		 ( (color != tcap_color) || (t.ansi_color_mapping == 'y' ) ) ) ) {
			term_esc( t.color_beg );
	
			if ( t.color_type == 'c' ) {
				fputc( f1, ct->_outputfp );
				term_esc( t.color_sep );
				fputc( f1, ct->_outputfp );
				}
			else
				tprintf( "%d%s%d", f1, &t.color_sep, b1 );
	
			/* On an ANSI terminal, attributes off also clears colors	*/
			if ( t.color_end[0] ) term_esc( t.color_end );
			fflush( stdout );
			}

	/* Rebuild tcap_color from extracted colors, in case of ANSI mapping	*/
	tcap_color = 0x80 | f | (b << 4);
	}
#endif
