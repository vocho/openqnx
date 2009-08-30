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
#include <string.h>
#include <unistd.h>
#include "externs.h"

extern void miniproc_start(void);

/*
 * ktest.c
 *	Minimal stub to see Neutrino boot
 */
main(int argc, char *argv[])
{
	unsigned	len;

	kprintf( "starting her up\n" );
	miniproc_start();
	kprintf( "miniproc running\n" );
	sleep(1);
	len = (argc == 1) ? 10 : strtoul(argv[1], NULL, 0);
	kprintf("sleep(%u)\n", len);
	sleep( len );
	kprintf("back\n");
	write( 1, "Quick test\n", 11 );
	printf("Hello, %s%s\n", "world", "." );
	kprintf("said 'hi'\n");
	for( ;; ) {
		putchar( '.' );
		fflush( stdout );
		sleep( 1 );
	}
}

__SRCVERSION("ktest.c $Rev: 201493 $");
