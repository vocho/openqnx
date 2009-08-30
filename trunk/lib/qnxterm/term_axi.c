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
#include <malloc.h>
#pragma aux __STKOVERFLOW aborts;
extern void __STKOVERFLOW( void );

#define t	term_state

void term_axis(row, col, type, cap, start, repeat, length, attr)
int row, col, type, cap;
int start, repeat, length;
unsigned attr;
	{
	register char *p;
	int i;
	char tee, *buf;
	static int map1[] ={ 0, 2, 2, 3 };
	static int map2[] ={ 1, 3, 0, 1 };

	if ( ( buf = alloca( length + 2 ) ) == NULL ) {
		__STKOVERFLOW();
		}
	term_box_on();
	tee = (p = &t.box_top_tee)[type];
	if(tee == t.box_cross)
		tee = (type < TERM_VERT_R) ? t.box_horizontal : t.box_vertical;

	for(i = 1 ; i <= length ; ++i)
/*		if(i >= start  &&  (i - start) % repeat == 0)*/
		buf[i] = (repeat && i >= start  &&  (i - start) % repeat == 0) ?
					t.box_cross : tee;

	p -= 4; 
	if ( cap & TERM_CAP_LOW)	buf[1]		= p[map1[type]];
	if ( cap & TERM_CAP_HI ) 	buf[length] = p[map2[type]];

	switch(type) {
		case TERM_HOR_DN:
		case TERM_HOR_UP:
			term_type(row, col, buf + 1, length, attr);
			break;

		case TERM_VERT_L:
		case TERM_VERT_R:
			for(i = 1 ; i <= length ; ++i)
				term_type(row - i + 1 , col, buf + i, 1, attr);
			break;
		}
	term_box_off();
	}
