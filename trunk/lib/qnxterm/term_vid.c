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




/*				16-Nov-87 10:16:48am										*/

/*--------------------------------------------------------------------------*/
/*  History: term_video.c, V0.0, 16-Nov-87 10:16:48am, Dan Hildebrand		*/
/*----------------------------------------------------------------------------

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/qnxterm.h>
#include <sys/term.h>

#define t	term_state
#define ct	__cur_term

#ifndef __STDC__
#define const
char *malloc();
#endif

static void
term_error( msg )
	char *msg;
	{
	perror( msg );
	exit( -1 );
	}

/*-- Allocate buffer to contain terminal attrbutes and screen image

	It is assumed that the terminal cannot resize like the console can.
	Terminals which do not need to enter an alternate charset mode for line
	drawing characters do not need to have an attrbuf allocated. This buf
	is currently only used to contain a bit indicating if the character at
	that relative position in the screen buffer is a line character or not.
*/
void
term_video_on() {
	if ( !t.scrbuf ) {
		if ( (t.scrbuf = malloc( t.screen_amount )) == NULL ) {
			errno = ENOMEM;
			term_error( "term_video_on1" );
			}
		__memsetw( t.scrbuf, (t.fill & 0x7700)|' ', t.screen_amount/2);
		}
	if ( *enter_alt_charset_mode ) {
		if ( !t.attrbuf ) {
			if ( (t.attrbuf = malloc( t.screen_amount/2 )) == NULL ) {
				errno = ENOMEM;
				term_error( "term_video_on2" );
				}
			}
		}
	if ( term_relearn_size() == -1 )
		term_error( "term_video_on3" );
	}

/*-- Release buffer that contains terminal attribute image	---*/
void
term_video_off() {	
	/*
	The screen buffer is always automatically allocated on the console.
	Only allow it to be free'd for a serial line that explicitly
	allocated it.
	*/
	if ( !ct->_cc  &&  t.scrbuf ) {
		free( t.scrbuf );
		t.scrbuf = NULL;
		}
	if ( t.attrbuf ) {
		free( t.attrbuf );
		t.attrbuf = NULL;
		}
	}

void
term_get_line( row, col, buffer, length )
	int		row, col, length;
	char	*buffer;
	{
	register char *v, *p = buffer;
	register int i;

	if ( !t.scrbuf ) term_error( "term_get_line" );
	v = t.scrbuf + ( (row * t.line_amount) + (col * 2) );
	for ( i = 0; i < length; ++i ) {
		*p = *v;
		++p;
		v += 2;
		}
	}

/*-- Read a piece of the screen back into a specified buffer ---*/
int
term_save_image( row, col, buffer, length )
	int		row, col, length;
	char *buffer;
	{
	register int i;
	int bytes = t.attrbuf ? 3 : 2;		/* Some terminals need 3 bytes	*/

	if ( !t.scrbuf ) term_error( "term_save_image" );
	if ( !length ) 	return( bytes );

	i = row * t.line_amount + col * 2;
	memcpy( buffer, t.scrbuf + i, length*2 );
	if ( t.attrbuf ) memcpy( buffer + length*2, t.attrbuf + i/2, length );
	return( bytes );
	}

/*-- Write into the screen from a specified buffer ---*/
int
term_restore_image( row, col, buffer, length )
	int			row, col, length;
	const char *buffer;
	{
	unsigned char *p, *a = NULL;
	const char *b;
	unsigned char attr, line_attr;
	int i;
	unsigned att;
	unsigned char no_attr = 0;
	int bytes = t.attrbuf ? 3 : 2;
	unsigned char line[133];

	if ( !t.scrbuf ) term_error( "term_restore_image" );
	if ( !length ) return( bytes );
	/* Copy the line to restore into the video and attribute buffer		*/
	i = row * t.line_amount + col*2;
	memcpy( t.scrbuf + i, buffer, length*2 );
	if ( t.attrbuf ) memcpy( a = t.attrbuf + i/2, buffer + length*2, length );

	if ( ct->_cc ) {
		if ( row < t.region1 ) t.region1 = row;
		i = ( i + length*2 ) / t.line_amount;
		if ( i > t.region2 ) t.region2 = i;
		return( bytes );	/* All done, if using video buffer	*/
		}

	/* Must be a terminal, so serially output the data	*/
	term_cur( row, col );
	b = buffer;
	/* This has to point somewhere if an attr buf is not allocated	*/
	if ( !t.attrbuf ) a = &no_attr;
	/* @@@ This could be made smarter to not refresh already present text	*/
while (length > 0) {
	i = min(length, sizeof line - 1); /* wojtek says i goofed previously -glen */
	length -= i;
	while( i > 0 ) {
		attr = *(b+1);				/* Video text/attrbute buffer			*/
		line_attr = *a;				/* Line attribute buffer				*/
		p = line;					/* Output text buffer					*/
		while(	( attr == *(b+1))	&&
				( line_attr == *a)	&&
				  i > 0 ) {			/* Extract bytes with same attributes	*/
			*p++ = *b;				/* Save char in output buffer			*/
			if ( t.attrbuf ) ++a;	/* Advance to next line attribute byte	*/
			b += 2;					/* Advance to next video char/attr pair	*/
			--i;					/* One less character to restore		*/
			}

		/* Color attribute byte		*/
		if ( !t.is_mono ) {
		att	 =	0x8000					|			/* Color enabled	*/
				((attr & 0x0077)<<8)	|			/* Color bits		*/
				((attr & 0x0008)>>2)	|			/* Highlight		*/
				((attr & 0x0080)>>7);				/* Blink			*/
/*
@@@ How do we determine inverse on a mono terminal ?
For mono terminal, use absence of color#8 in terminfo to change behavior
*/
			}
		else {
		/* Monochrome attribute byte	*/
		att	 =	((attr & 0x0008)>>2)	|			/* Highlight		*/
				((attr & 0x0080)>>7)	|			/* Blink			*/
				(((attr & 0x0007)==0x01)?0x0008:0)|	/* Underline		*/
				((attr & 0x0070) ? 0x0004 : 0);		/* Inverse			*/
			}

		if ( line_attr ) term_box_on();
		term_type( -1, -1, line, p - &line[0], att );
		if ( line_attr ) term_box_off();
		}
}
	return( bytes );
	}
