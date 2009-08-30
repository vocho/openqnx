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

static unsigned *force_cc1_to_data[] __attribute__((unused)) = { 0 };

asm(".section .ctors,\"aw\"");
unsigned _ctors_end[] = { 0 };
asm(".previous");

extern unsigned _ctors_begin;

void gcov_init(void)
{
	void (*func)(void);
	unsigned *p;

	for( p = &_ctors_end[0]-1; p > &_ctors_begin; p-- ) {
		func = (void *)*p;
		func();
	}
}

#ifdef __X86__
void _init(void)
{
}
#endif
