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

#define t	term_state

/* Write text to screen with toggled attributes		*/
/* @@@ The len argument is not used !				*/
void term_attr_type( y, x, string, len, attr, new_attr, toggle )
	int y, x, len;
	const char *string;
	unsigned attr, new_attr;
	unsigned char toggle;
	{
	register char *s, *p;
	register int  amount;
	unsigned old_attr = attr;

	if( len == 0 )
		len = 32500;

	if ( y == -1 ) {
		y = t.row;
		x = t.col;
		}
	else term_cur( y, x );

	s = p = (char *)string;
	for(;;) {
		while ( *p && *p != toggle && len-- > 0 )	/* Scan for toggle char		*/
			++p;
		if ( amount = (int) (p - s) )				/* Output previous text		*/
			term_type( -1, -1, s, amount, attr );
		if ( len <= 0 || !*p ) return;
		++p;										/* Skip over toggle			*/
		--len;
		if ( len <= 0 || !*p ) return;
		s = p;
		if (attr == old_attr)	attr = new_attr;
		else					attr = old_attr;
		}
	}
