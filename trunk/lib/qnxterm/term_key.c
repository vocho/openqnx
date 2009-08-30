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
#include <sys/term.h>
#include <sys/qnxterm.h>
#ifndef __QNXNTO__
#include <sys/mouse.h>
#include <sys/kernel.h>
#include <sys/dev.h>
#endif

#define t	term_state
#define ct	__cur_term

unsigned
term_key() {
	unsigned event;

	term_flush();
	event = (unsigned) __getch();
	if ( event == K_RESIZE ) 	term_relearn_size();
	return( event );
	}

void
term_unkey(c)
	unsigned c;
	{
	__ungetch( c );
	}
