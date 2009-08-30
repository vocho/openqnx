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
 *	Functions added strictly for the terminfo/tcap based version of 'ed'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#ifndef __QNXNTO__
#include <sys/dev.h>
#endif
#include <string.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

#include <sys/term.h>

#define NORM_ATTR				0x00
#define BLINK_ATTR				0x01
#define HIGHLIGHT_ATTR			0x02
#define INVERSE_ATTR			0x04
#define UNDERLINE_ATTR			0x08
#define HIGHLIGHT_BLINK_ATTR	0x03

char term_line_bfr[264];	/*	Enough to expand 132 characters		*/

char *es;

int allow_color;

void
clear_term_screen ()
	{
	register int	 i;
	register char	*lc;

	term_clear(3);

	i = screen_height * screen_width;
	lc = es;
	for( ; i; --i, lc += 2 ) {
		*lc = ' ';
		*( lc + 1 ) = NORM_ATTR;
	}
}

void
put_term_char( unsigned row, unsigned col, char c, unsigned attr )
{
	register char	lc, la;
	register char	*location;
	int n;

	if(!allow_color) 		attr &= 0x00ff;		/* No color	*/

	/*
	 *	Never update last screen position if terminal has automatic margins!
	 */
	if ( auto_right_margin  &&
		 row + 1 == screen_height  &&
		 col + 1 == screen_width)
		return;

	location = es + (2 * (row * screen_width + col));
	lc = *location;
	la = *( location + 1 );
	c = ( c == 0 ) ? lc : c;
	if( lc != ( char )c  ||  la != ( char )attr ) {
		n = map_term_line(term_line_bfr, &c, 1);
		if (n) term_type(row, col, term_line_bfr, n, attr);
		*location = ( char )c;
		*( location + 1 ) = ( char )attr;
	}
}

void
put_term_line( int row, int col, char *text, int len, unsigned attr )
{
	register char *location, *p = text;
	register int n = len;
	char use_clear_eol = 0;
	int num_nonblanks;

	/*
	 *	Ignore colour for now.
	 */
	if(!allow_color) 		attr &= 0x00FF;

	/*
	 *	Find first unmatched character or attribute.
	 *	(See if there is anything to update at all).
	 */
	location = es + (2 * ( row * screen_width + col ));
	while( n  &&  *location == *p  &&  *( location + 1 ) == (char) attr) {
		location += 2;
		--n;
		++p;
	}

	if( n == 0 )
		return;

	/*
	 *	Decide if the clear-to-end-of-line sequence can be used.
	 */
	if(attr == NORM_ATTR  &&  col + len == screen_width)	use_clear_eol = 1;

	/*
	 *	Reset the parameters
	 */
	col += len - n;
	len = n;

	num_nonblanks = use_clear_eol ? num_nonblank_chars( p, len ) : len;

	/*
	 *	Find max number of characters which need to be updated.
	 */
	len = num_unmatched_chars( row, col, p, len, attr );

	/*
	 *	If all the characters are blank,
	 *	and there are more than three that need to be updated
	 *	use clear-to-EOL if possible.
	 */
	if(use_clear_eol  &&  num_nonblanks == 0  &&  len > 2)
		{
		term_cur(row, col);
		term_clear( 1 );
		}
	else
		{
		term_cur( row, col );
		/*
		 *	If the last unmatched char is before the point where clear-to-EOL
		 *	can be used, display all the unmatched characters.
		 */
		if( len <= num_nonblanks ) {
			/*
			 *	If on last screen line and line extends to last column,
			 *		reduce line length by 1.
			 */
			if( row + 1 == screen_height  &&  col + len == screen_width )
				--len;
			n = map_term_line(term_line_bfr, p, len);
			if (n) term_type(row, col, term_line_bfr, n, attr);
			}
		else
			{
			/*
			 *	The only way to enter this section of code is if clear-to-EOL
			 *	can be used, else num_nonblanks would have been >= len.
			 *
			 *	See if breaking the update into 2 parts (character display
			 *	and clear-to-EOL) improves the performance.
			 */
			n = num_unmatched_chars( row, col, p, num_nonblanks, attr);

			/*
			 *	There must be more than 4 matching characters before the start
			 *	of the clear-to-EOL sequence to allow for cursor motion.
			 */
			if( num_nonblanks - n <= 4 )
				n = num_nonblanks;
			n = map_term_line(term_line_bfr, p, n);
			if (n) term_type(row, col, term_line_bfr, n, attr);

			/*
			 *	Now do the clear-to-EOL.
			 *	If there are less than 3 characters to clear, just put out
			 *	blanks.
			 */
			term_cur(row, col + num_nonblanks);
			if((n = len - num_nonblanks) < 3) {
				if (n) term_type( -1, -1, p + num_nonblanks, n, attr);
				}
			else
				term_clear( 1 );
			}
		}

	/* Undefine cursor position if a write to the last column occurred	*/
	if ( term_state.col == screen_width )
		term_state.col = 12345;

	/*
	 *	Update the screen map.
	 */
	n = len;
	while( n-- ) {
		*location = *p++;
		*( location + 1 ) = (char)attr;
		location += 2;
	}
}


int num_nonblank_chars( char *text, int len )
{
	register char *p = text,
					*endp = text + len;

	while( --endp >= p )
		if( ' ' != *endp )
			break;

	return( 1 + endp - p );
}


int num_unmatched_chars( int row, int col, char *text, int len, unsigned attr )
{
	register char *location, *p;
	register int n = len;

	if(!allow_color) 		attr &= 0x00ff;
	location = es + (2 * ( row * screen_width + col + n - 1 ));
	p = text + n - 1;
	while( n  &&  *location == *p  &&  *( location + 1 ) == attr ) {
		location -= 2;
		--n;
		--p;
	}
	return( n );
}


int map_term_line (char *dest, char *src, int len)
	{
	register char *d = dest, *s = src;
	register int n = len;
	unsigned c;

	/* Video writes can display every character	*/
	if ( memory_mapped ) {
		memcpy( dest, src, len );
		return( len );
		}

	/*
	 *	Map non-displaying chars to '?'.
	 */
	while( n ) {
		c = (unsigned)*s;
		if ( !term_state.qnx_term )
			*d = (c < ' '  ||  c > '~') ? '?' : *s;
		else if(c >= ' '  &&  c < 0xA0  ||  c > 0xA9)			/*
			 *	Normal ASCII characters, 0x80-0x9F and 0xAA-0xFF
			 *	can be passed through without an ESCAPE.
			 */
			*d = *s;

		else if(c < ' ') {
			/*
			 *	Control characters need an escape.
			 */
			*d = 0x1b;
			*++d = *s;
			/*
			Turn position caching off, since the escape chars will throw off
			the computed cursor position.
			*/
			term_state.cache_pos = 0;


		} else
			/*
			 *	This is a character in the range of 0xA0-0xA9.
			 */
			switch(c) {

			case 0xA0:	/*	HOME	*/
			case 0xA1:	/*	UP		*/
			case 0xA4:	/*	LEFT	*/
			case 0xA6:	/*	RIGHT	*/
			case 0xA9:	/*	DOWN	*/
				*d = 0x1B;
				*++d = *s;
				term_state.cache_pos = 0;
				break;

			default:
				*d = *s;
			}
		++d;
		++s;
		--n;
		}

	return(d - dest);
	}

void
init_terminal()
{
	register int screen_bytes;
#ifndef __QNXNTO__
	struct _dev_info_entry info;
#endif

	screen_bytes = screen_height * screen_width * 2;

	if( NULL == ( es = malloc( screen_bytes ))) {
		fprintf( stderr, "INSUFFICIENT MEMORY FOR SCREEN MAP.\n" );
		exit( -1 );
		}
	if( allow_color = __has_colors()) {
		if( __cur_term->Otty.c_ospeed < B9600 )
				allow_color = 0;
#ifndef __QNXNTO__
		else {
			dev_info(__cur_term->Filedes, &info);
			if(!strcmp(info.driver_type, "serial"))
				allow_color = 0;
			}
#endif
		}
	}

void
del_term_line( int row, int nrows )
{
	register char	*lc;
	register int	 n;

	/*
	 *	Move the screen image up nrows
	 */

	memmove(es + (2 * row * screen_width),
			es + (2 * ( row + nrows ) * screen_width),
			2 * ( screen_height - row - nrows ) * screen_width );

	/*
	 *	Then blank the last nrows
	 */

	lc = es + (2 * (screen_height - nrows) * screen_width);
	for(n = nrows * screen_width; n; --n, lc += 2) {
		*lc = ' ';
		*(lc + 1) = NORM_ATTR;
	}

	/*
	 *	Now really delete the screen_height on the terminal
	 */
	term_delete_line( row, nrows );
	}

void
ins_term_line( int row, int nrows)
{
	register char	*lc;
	register int	 n;

	/*
	 *	Move the screen image down nrows
	 */

	memmove(es + (2 * ( row + nrows ) * screen_width),
			es + (2 * row * screen_width),
			2 * ( screen_height - row - nrows ) * screen_width );

	/*
	 *	Then blank the first nrows beginning at row
	 */

	lc = es + (2 * row * screen_width);

	for(n = nrows * screen_width; n; --n, lc += 2) {
		*lc = ' ';
		*(lc + 1) = NORM_ATTR;
	}

	/*
	 *	Now really insert the lines on the terminal
	 */
	term_insert_line( row, nrows );
	}

void
set_memory_mapped() {
	memory_mapped = __cur_term->_cc != NULL;
	}
