/*
 * $QNXtpLicenseC:
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
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).  
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#include "malloc-lib.h"
#include "mallocint.h"

/*
 * malloc errno error strings...
 * memory block: refers to the block requested by a user in a malloc
 * heap area: refers to the entire memory area of the allocator
 */

char *malloc_err_strings[] = 
{
	"no errors",
	"allocator inconsistency - Malloc chain is corrupted, pointers out of order",
	"allocator inconsistency - Malloc chain is corrupted, end before end pointer",
	//"Pointer is not within malloc area",
	"pointer does not point to heap area",
	//***"Malloc region does not have valid magic number in header",
	"possible overwrite - Malloc block header corrupted",
	//"Pointers between this segment and adjoining segments are invalid",
	"allocator inconsistency - Pointers between this segment and adjoining segments are invalid",
	//"Data has overrun beyond requested number of bytes",
	"data has been written outside allocated memory block",
	//"Data in free'd area has been modified",
	"data in free'd memory block has been modified",
	//"Data area is not in use (can't be freed or realloced)",
	"data area is not in use (can't be freed or realloced)",
	//"Unable to get additional memory from the system",
	"unable to get additional memory from the system",
	//"Pointer within malloc region, but outside of malloc data bounds",
	"pointer points to heap but not to a user writable area",
	//"Malloc segment in free list is in-use",
	"allocator inconsistency - Malloc segment in free list is in-use",
	//***"Malloc region doesn't have a valid CRC in header",
	"malloc region doesn't have a valid CRC in header",
	//"Free'd pointer isn't at start of malloc buffer",
	"free'd pointer isn't at start of allocated memory block",
    //we are about to dereference null pointer 
	"null pointer dereference",
	"signal received",
	(char *) 0
};

/*
 * Function:	malloc_perror()
 *
 * Purpose:	to print malloc_errno error message
 *
 * Arguments:	str	- string to print with error message
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 */
void
malloc_perror(char *str)
{
	register char 	* s;
	register char 	* t;

	if( str && *str)
	{
		for(s=str; *s; s++)
		{
			/* do nothing */;
		}

		(void) write(2,str,(unsigned)(s-str));
		(void) write(2,": ",(unsigned)2);
	}

	t = malloc_err_strings[malloc_errno];

	for(s=t; *s; s++)
	{
		/* do nothing */;
	}

	(void) write(2,t,(unsigned)(s-t));

	(void) write(2,"\n",(unsigned)1);
}

/*
 * $Log$
 * Revision 1.9  2007/05/04 17:15:12  elaskavaia
 * - signal catcher - produce a fatal error before exit
 * PR:43722
 * CI:alain
 * CI:dinglis
 *
 * Revision 1.8  2006/12/19 20:23:23  elaskavaia
 * - fixed problem with random errors message for npd problem
 * PR: 43567
 * CI: alain@qnx.com
 * CI: cburgess@qnx.com
 *
 * Revision 1.7  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.6  2005/02/13 23:15:40  shiv
 * some more cleanup.
 *
 * Revision 1.5  2005/02/11 19:00:28  shiv
 * Some more malloc_g changes.
 * Modified Files:
 * 	mallocint.h dbg/dump.c dbg/m_init.c dbg/m_perror.c
 *  	dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *
 * Revision 1.4  2004/08/13 13:34:06  thomasf
 * Initial adjustment of the debug malloc strings to make more of an
 * immediate impression on users.  PR 20987
 *
 * Revision 1.3  2004/03/26 19:17:48  thomasf
 * Fix to address PR 19481 where we had a typo.
 *
 * Revision 1.2  2004/02/12 15:43:16  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.1  2000/01/31 19:03:30  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.9  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.8  1991/12/04  09:23:38  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.7  91/11/25  14:41:55  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.6  91/11/24  00:49:27  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.5  90/08/29  21:25:08  cpcahil
 * added additional error message that was missing (and 
 * caused a core dump)
 * 
 * Revision 1.4  90/05/11  00:13:08  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:21  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:39:55  cpcahil
 * 1. added function header
 * 2. added rcs id and log strings.
 * 
 */
