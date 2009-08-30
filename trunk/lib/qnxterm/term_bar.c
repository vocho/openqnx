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

void term_bar(row, col, type, level1, level2, attr)
int row, col, type;
unsigned level1, level2;
unsigned attr;
	{
	unsigned n1, n2, i;
	char block, *buf;

	term_box_on();
	if ( ( buf = alloca( level2 + 2 ) ) == NULL ) {
		__STKOVERFLOW();
		}
top:
	n2 = level2;
	n1 = (level1 > n2) ? n2 : level1;

	block = (type == TERM_BAR_HORIZ) ? t.box_bot_solid_block
								 : t.box_solid_block;

	if ( type >= TERM_BAR_CV_1 ) {
		n1 /= 2;
		n2 = (n2 + 1)/2;
		}

	buf[1] = '\0';
	for(i = 1; i <= n1; ++i) buf[i] = block;
	for(     ; i <= n2; ++i) buf[i] = '.';

	switch(type) {
	case TERM_BAR_HORIZ:
		if(n2) term_type( row, col, buf + 1, n2, attr );
		break;

	case TERM_BAR_CV_1:
		if ( level1 & 0x01 ) buf[n1 + 1] = t.box_bot_solid_block;
	case TERM_BAR_VERT:
		for ( i = 1 ; i <= n2 ; ++i)
			term_type(row - i + 1 , col, buf + i, 1, attr);
		break;

	case TERM_BAR_CV_2:
		term_type(row, col, level1 ? &t.box_top_solid_block : " ", 1, attr);
		type = TERM_BAR_CV_1;
		if(level1 > 0  &&  level2 > 0) {
			--level1;
			--level2;
			--row;
			goto top;	/* decided against recursive call because buf alloca */
			}
		}
	term_box_off();
	}
