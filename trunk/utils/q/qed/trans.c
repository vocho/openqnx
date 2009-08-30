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
 * April 1980 by  D. T. Dodge
 */


#include <stdio.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int translate(int recursive)
	{
	extern char hexstr[];
	char *p, *p1;
	struct macro_entry *mp;
	char c;

	p = lp;

	if(*p++ != ' ')
		return(ERROR9);

	c = *p;
	esc_line(p);

	if(*p == '\0'  ||  (*(p+1) != ' '  &&  *(p+1) != '\n'))
		return(ERROR9);

	lp = "\n";	/* Forces main to fetch a new line */
	if(c == '?') {
		c = *(p + 2);
		p = firstp->textp;
		p[0] = 't';
		p[1] = ' ';
		p[2] = '\\';
		p[3] = hexstr[(c >> 4) & 0xf];
		p[4] = hexstr[c & 0xf];
		p[5] = ' ';
		if(mp = lookup(c)) {
			p1 = &mp->mstr[0];
			p += 6;
			while(*p1)
				if(*p1 < ' ') {
					p[0] = '\\';
					p[1] = hexstr[(*p1 >> 4) & 0xf];
					p[2] = hexstr[*p1++ & 0xf];
					p += 3;
					}
				else
					if((*p++ = *p1++) == '\\')
						*p++ = '\\';
			}
		*p = '\0';
		change_state(CMD);
		return( 0 );
		}
	else
		return(install(*p, p+2, recursive));
	}
