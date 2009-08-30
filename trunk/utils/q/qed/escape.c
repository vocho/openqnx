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
 * April 1980 by D. T. Dodge
 */

#include <stdio.h>
#include "manif.h"

char hexstr[] = { "0123456789abcdef" };

char *esc_line(char *tp) {
	register char *p2, *p1;
	char c;
	char *esc_char();

	for(p1 = p2 = tp ; *p2 != '\0' ;) {
		if((c = *p2++) == '\\')
			p2 = esc_char(p2, &c);
		*p1++ = c;
		}
	*p1 = '\0';
	return( tp );
	}


char *esc_char( char *str, char *cp) {
	if((*cp = hex(*str)) < 16) {
			*cp = (*cp << 4) + hex(*++str);
		return(str + 1);
		}

	*cp = *str;
	return(str + 1);
	}


void
expand(char c) 	{

	if(c < 040  &&  c != '\t'  &&  c != 0x1b) {
		putchar('\\');
		putchar(hexstr[(c >> 4) & 0xf]);
		putchar(hexstr[c & 0xf]);
		return;
		}
	putchar(c);
	}



int hex(char c) {

	if(c >= '0'  &&  c <= '9')
		return(c - '0');

	c |= ' ';
	if(c >= 'a'  &&  c <= 'f')
		return(c - 'a' + 10);

	return(16);
	}
