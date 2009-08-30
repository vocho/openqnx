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




#ifndef __QNXNTO__

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/term.h>
#include <sys/qnxterm.h>
#include <sys/dev.h>
#include <sys/proxy.h>
#include <sys/console.h>
#include <signal.h>

#define t	term_state
#define ct	__cur_term

int
term_mouse_hide( void ) {
	if ( t.mouse_cursor ) {
		console_write( ct->_cc, 0,
			t.mouse_old_row * t.line_amount + t.mouse_old_col * 2,
			t.mouse_oldchar, 2,
			NULL, NULL, NULL );
		t.mouse_cursor = 0;
		}
	return( 0 );
	}

int
term_mouse_move( int row, int col ) {
	char oldchar[2];

	if ( !ct->_cc || t.scan_mouse ) 	return( 0 );
	if ( row != -1 ) 		t.mouse_row = row;
	if ( col != -1 ) 		t.mouse_col = col;

	if ( t.mouse_cursor ) {
		if ( ( t.mouse_old_row == t.mouse_row ) &&
			 ( t.mouse_old_col == t.mouse_col ) )
			return( 0 );
		term_mouse_hide();
		}

	t.mouse_old_row = t.mouse_row;
	t.mouse_old_col = t.mouse_col;

	console_read( ct->_cc, 0,
		t.mouse_row * t.line_amount + t.mouse_col * 2,
		t.mouse_oldchar, 2,
		NULL, NULL, NULL );

	oldchar[0] = t.mouse_oldchar[0];	
	oldchar[1] = 0x77 - (t.mouse_oldchar[1] & 0x77) | t.mouse_oldchar[1] & 0x88;

	console_write( ct->_cc, 0,
		t.mouse_old_row * t.line_amount + t.mouse_old_col * 2,
		oldchar, 2,
		NULL, NULL, NULL );

	t.mouse_cursor = 1;
	return( 0 );
	}
#endif
