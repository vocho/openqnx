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
#include <stdlib.h>
#include <string.h>
#include <sys/qnxterm.h>
#include <sys/term.h>

#define t	term_state
#define ct	__cur_term

#ifndef __STDC__
#define const
#endif

int
term_type( row, col, text, len, a )
	int row, col, len;
	unsigned a;
	const char *text;
	{
	register int i, j;
	register unsigned char at1, attr;
	register unsigned char *p;
	register unsigned short *u;
	const unsigned char *q;

	if ( a & 0x8000 )	term_color( a );
	a |= t.color;


	if ( row == -1 ) {
		row = t.row;
		col = t.col;
		}
	else
		term_cur( row, col );

	if ( col >= t.num_cols ) return( 0 );
	if ( !len )	len = strlen( text );
	if ( col + len > t.num_cols ) len = t.num_cols - col;

	/*
	If on last line, don't type on the last char unless we can avoid
	scrolling, or if we specify the bottom corner or the screen
	*/
	if ( row >= lines-1 && col + len == t.num_cols && col != t.num_cols-1 &&
		 auto_right_margin && !eat_newline_glitch ) len--;

	if ( len <= 0 ) return( 0 );

	attr = (unsigned char) (a & 0x00ff);

	if ( t.scrbuf ) {				/* Update internal image if keeping one	*/
		p = t.scrbuf + (t.row * t.line_amount) + (t.col * 2);
		q = text;
		if ( t.is_mono )	at1 = 0x07;
		else	 			at1 = ( (t.color >> 8) & 0x77 );
		if ( attr & TERM_INVERSE )
			at1 = (at1 >> 4) | ((at1 & 0x07) << 4);
		/* Underline must be set after inverse for monochrome hardware	*/
		if ( t.is_mono  &&  (attr & TERM_ULINE) ) at1 = 0x01;
		if ( attr & TERM_BLINK )	at1 |= 0x80;
		if ( attr & TERM_HILIGHT )	at1 |= 0x08;

		/* Copy the data down into the video buffer	*/
		i = len;
		u = (unsigned short *) p;
		do {
			*u++ = (at1 << 8) | *q++;
			} while ( --i );

		if ( t.attrbuf ) 	/* Line attributes too if on terminal	*/
			memset( t.attrbuf+(t.row*t.line_amount/2)+t.col, t.line_set_on, len);

		if ( row < t.region1 ) t.region1 = row;
		if ( row > t.region2 ) t.region2 = row;
		}

	if ( !ct->_cc ) {				/* Not on console, so output to stdio	*/
		if (t.old_attr != attr) {
			/* Clear old attributes	as cheaply as possible	*/
			if ( t.old_attr == TERM_ULINE ) __putp( exit_underline_mode );
			else					 			__putp( exit_attribute_mode );

			if ( attr || t.line_set_on ) {		/* Set any attributes		*/
				if ( *set_attributes ) {
					__putp( __tparm( set_attributes,
						0,						/* Standout					*/
						attr & TERM_ULINE,
						attr & TERM_INVERSE,
						attr & TERM_BLINK,
						0,						/* Dim						*/
						attr & TERM_HILIGHT,	/* Bold						*/
						0,						/* Invisible				*/
						0,						/* Protect					*/
						t.line_set_on			/* Alternate character set	*/
					) );
					}
				else {
					if ( attr & TERM_BLINK )   __putp( enter_blink_mode );
					if ( attr & TERM_HILIGHT ) __putp( enter_bold_mode );
					if ( attr & TERM_INVERSE ) __putp( enter_reverse_mode );
					if ( attr & TERM_ULINE )   __putp( enter_underline_mode );
					if ( t.line_set_on ) __putp( enter_alt_charset_mode );
					}
				}
			if(!t.is_mono && !*set_color_pair && *set_foreground && *set_background) {
				i = t.color;
				t.color = 0;
				term_color(i);
				}
			}

		if ( *repeat_char  &&  (len > 4) ) { 	/* Can we rll the output ?	*/
			j = len;
			p = (unsigned char *) (q = text);
			while( j > 4 ) {
#if 0
				/* Early out for non-repeating string	*/
				if ( *(p+1) != *(p+2) ) break;
#endif
				while( j  &&  *p == *q ) {
					++p;
					--j;
					}
				i = (int) (p - q);
				if ( i <= 4 )		/* Run too small, don't bother with rll	*/
					fwrite( q, i, 1, ct->_outputfp );
				else
					__putp( __tparm( repeat_char, *q, i ) );
				q = p;
				}
			if ( j )	fwrite( q, j, 1, ct->_outputfp );			}
		else
			fwrite( text, len, 1, ct->_outputfp );

		if ( !t.cache_attr  &&  attr )			/* Clear any attributes		*/
			__putp( exit_attribute_mode );

		}

   	t.old_attr = attr;			/* Remember current set for next call		*/
	t.row = row;				/* Update global screen location variables	*/
	t.col = col + len;
	if ( t.col >= columns ) {
		/* Adjust for wrap around if the terminal wraps	on long lines	*/
		if ( auto_right_margin ) {
			/*
			A vt100 does not advance to next line when last position filled.
			it only advances on the character after.
			*/
			if ( eat_newline_glitch ) {
				putc( ct->_pad_char, ct->_outputfp );
				if ( t.row < lines-1 ) {
					putc( '\n', ct->_outputfp );
					++t.row;
					}
				t.col = 0;			/* Cursor stays there so move it to col 0 */
				putc( '\r', ct->_outputfp );
				/* We really should remember some state here @@@	*/
				}
			else {
				t.col -= columns;
				if ( t.row < lines-1 ) ++t.row;
				}
			}
			/* For non-wrapping terminals, cursor stays at rightmost column	*/
		else
			t.col = columns-1;
		}

	if ( a & TERM_FLUSH ) term_flush();
	return( len );
	}
