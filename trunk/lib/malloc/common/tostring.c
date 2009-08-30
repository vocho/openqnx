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
#include "tostring.h"
#include <inttypes.h>

/*
 * Function:	tostring()
 *
 * Purpose:	to convert an integer to an ascii display string
 *
 * Arguments:	buf	- place to put the 
 *		val	- integer to convert
 *		len	- length of output field (0 if just enough to hold data)
 *		base	- base for number conversion (only works for base <= 16)
 *		fill	- fill char when len > # digits
 *
 * Returns:	length of string
 *
 * Narrative:	IF fill character is non-blank
 *		    Determine base
 *		        If base is HEX
 *		            add "0x" to begining of string
 *		        IF base is OCTAL
 *		            add "0" to begining of string
 *
 *		While value is greater than zero
 *		    use val % base as index into xlation str to get cur char
 *		    divide val by base
 *
 *		Determine fill-in length
 *
 *		Fill in fill chars
 *
 *		Copy in number
 *		
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 */


#define T_LEN 32

int tostring(char *buf, unsigned long val, int len, int base, char fill)
{
	char	* bufstart = buf;
	int	  i = T_LEN;
	char	* xbuf = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char	  tbuf[T_LEN];

	/*
	 * if we are filling with non-blanks, make sure the
	 * proper start string is added
	 */
	if( fill != ' ' )
	{
		switch(base)
		{
			case B_HEX:
				if( (len == 0) ||  (len > 2) )
				{
					*(buf++) = '0';
					*(buf++) = 'x';
					if( len )
					{
						len -= 2;
					}
				}
				break;
			case B_OCTAL:
				*(buf++) = fill;
				if( len )
				{
					len--;
				}
				break;
			default:
				break;
		}
	}

	if (val == 0)
	{
		tbuf[--i] = xbuf[0];
	}

	while( val > 0 )
	{
		uint64_t t;
		t = val % base;
		tbuf[--i] = xbuf[(int)t];
		val = val / base;
	}

	if( len )
	{
		len -= (T_LEN - i);

		if( len > 0 )
		{
			while(len-- > 0)
			{
				*(buf++) = fill;
			}
		}
		else
		{
			/* 
			 * string is too long so we must truncate
			 * off some characters.  We do this the easiest
			 * way by just incrementing i.  This means the
			 * most significant digits are lost.
			 */
			while( len++ < 0 )
			{
				i++;
			}
		}
	}

	while( i < T_LEN )
	{
		*(buf++) = tbuf[i++];
	}

	return( (int) (buf - bufstart) );
} /* tostring(... */

int tostring64(char *buf, uint64_t val, int len, int base, char fill)
{
	char	* bufstart = buf;
	int	  i = T_LEN;
	char	* xbuf = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char	  tbuf[T_LEN];

	/*
	 * if we are filling with non-blanks, make sure the
	 * proper start string is added
	 */
	if( fill != ' ' )
	{
		switch(base)
		{
			case B_HEX:
				if( (len == 0) ||  (len > 2) )
				{
					*(buf++) = '0';
					*(buf++) = 'x';
					if( len )
					{
						len -= 2;
					}
				}
				break;
			case B_OCTAL:
				*(buf++) = fill;
				if( len )
				{
					len--;
				}
				break;
			default:
				break;
		}
	}

	while( val > 0 )
	{
		uint64_t t;
		t = val % base;
		tbuf[--i] = xbuf[(int)t];
		val = val / base;
	}

	if( len )
	{
		len -= (T_LEN - i);

		if( len > 0 )
		{
			while(len-- > 0)
			{
				*(buf++) = fill;
			}
		}
		else
		{
			/* 
			 * string is too long so we must truncate
			 * off some characters.  We do this the easiest
			 * way by just incrementing i.  This means the
			 * most significant digits are lost.
			 */
			while( len++ < 0 )
			{
				i++;
			}
		}
	}

	while( i < T_LEN )
	{
		*(buf++) = tbuf[i++];
	}

	return( (int) (buf - bufstart) );

} /* tostring(... */
/*
 * $Log$
 * Revision 1.5  2006/09/28 19:02:49  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.4.2.1  2006/05/15 16:45:40  alain
 *
 * common/tostrin.c: Fix when the value was 0, it did not print anything.
 * mallocint.h:  rename malloc_trace_btdepth to malloc_error_btdepth
 * raise the VERSION to 1.2
 *
 * Revision 1.4  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.3  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
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
 * Revision 1.1  2001/02/09 22:28:12  furr
 * Added necessary rule changes and source code to support non-debugging
 * malloc library.
 *   - Keeps same info as libc/alloc with guard code and caller pc turned
 *     on
 *   - Supports malloc hooks for functionality such as mtrace, memusage
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk malloc-lib.h mallocint.h dbg/tostring.c
 *  	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	common/tostring.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h std/calloc.c std/context.h
 *  	std/free.c std/m_init.c std/malloc_wrapper.c std/mtrace.c
 *  	std/realloc.c
 *
 * Revision 1.2  2000/02/15 22:01:07  furr
 * Changed to a more reasonable heap marking algorithm for memory leak
 * detection.
 *   - won't blow the stack
 * Fixed up file and line printing in all dumps.
 *
 *  Modified Files:
 *  	mallocint.h dbg/dump.c dbg/malloc_g.c dbg/malloc_gc.c
 *  	dbg/process.c dbg/tostring.c test/memtest.C
 *
 * Revision 1.1  2000/01/31 19:03:31  bstecher
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
 * Revision 1.5  1991/11/25  14:42:06  cpcahil
 * Final changes in preparation for patch 4 release
 *
 * Revision 1.4  90/05/11  00:13:11  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:33  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:29:42  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.1  90/02/22  23:17:44  cpcahil
 * Initial revision
 * 
 */
