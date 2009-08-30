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





#ifdef DEBUG
#include "freeze.h"

		  /*---------------------------*/
		  /*      DEBUG      MODULE    */
		  /*---------------------------*/

printcodes()
{
    /*
     * Just print out codes from input file.  For debugging.
     */
    register short k, c, col = 0;
#ifdef COMPAT
    if(new_flg)
#endif
	if(read_header() == EOF) {
		fprintf(stderr, "Bad header\n");
		return;
	}
    StartHuff();
    for (;;) {
#ifdef COMPAT
	    c = new_flg ? DecodeChar() : DecodeCOld();
#else
	    c = DecodeChar();
#endif
	    if (c == ENDOF)
		    break;
	    if (c < 256) {
		fprintf(stderr, "%5d%c", c,
			(col+=8) >= 74 ? (col = 0, '\n') : '\t' );
	    } else {
		c = c - 256 + THRESHOLD;
#ifdef COMPAT
		k = new_flg ? DecodePosition() : DecodePOld();
#else
		k = DecodePosition();
#endif
		fprintf(stderr, "%2d-%d%c", c, k,
			(col+=8) >= 74 ? (col = 0, '\n') : '\t' );
	    }
    }
    putc( '\n', stderr );
    exit( 0 );
}

/* for pretty char printing */

char *
pr_char(c)
	register uchar c;
{
	static char buf[5];
	register i = 4;
	buf[4] = '\0';
	if ( (isascii((int)c) && isprint((int)c) && c != '\\') || c == ' ' ) {
	    buf[--i] = c;
	} else {
	    switch( c ) {
	    case '\n': buf[--i] = 'n'; break;
	    case '\t': buf[--i] = 't'; break;
	    case '\b': buf[--i] = 'b'; break;
	    case '\f': buf[--i] = 'f'; break;
	    case '\r': buf[--i] = 'r'; break;
	    case '\\': buf[--i] = '\\'; break;
	    default:
		buf[--i] = '0' + c % 8;
		buf[--i] = '0' + (c / 8) % 8;
		buf[--i] = '0' + c / 64;
		break;
	    }
	    buf[--i] = '\\';
	}
	return &buf[i];
}
#endif
