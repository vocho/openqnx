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


#ifdef __QNXNTO__
#define _STRINGS_H_INCLUDED
#endif
#include <stdio.h>
#include "malloc-lib.h"
#include "mallocint.h"

/*
 * memccpy - copy memory region up to specified byte or length
 */
void *
memccpy(void *ptr1, const void *ptr2, int ch, size_t len)
{
	int line = libmalloc_caller();
	return( DBmemccpy( (char *) NULL, line, ptr1, ptr2, ch, len) );
}

void *
DBmemccpy(const char *file, int line, void *ptr1, const void *ptr2, int ch, size_t len)
{
	register char		* myptr1;
	register const char	* myptr2;
	register size_t	  i;
	void		* rtn;


	if ((len == 0) && (!malloc_verify_access_level)) {
		return(NULL);
	}
	myptr1 = (char *) ptr1;
	myptr2 = (char *) ptr2;

	/*
	 * I know that the assignment could be done in the following, but
	 * I wanted to perform a check before any assignment, so first I 
	 * determine the length, check the pointers and then do the assignment.
	 */
	for( i=0; (i < len) && (myptr2[i] != ch); i++)
	{
	}

	/*
	 * if we found the character...
 	 */
	if( i < len )
	{
		rtn = myptr1+i+1;
		i++;
	}
	else
	{
		rtn = (char *) 0;
	}

	/*
	 * make sure we have enough room in both ptr1 and ptr2
	 */
        if( !in_malloc() && malloc_verify_access )
        {
	    ENTER();
	    malloc_check_data("memccpy", file, line, ptr1, i);
	    malloc_check_data("memccpy", file, line, ptr2, i);
	    LEAVE();
        }

 	while( i-- > 0 )
	{
		*(myptr1++) = *(myptr2++);
	}
	
	return( rtn );
}

/*
 * memchr - find a byte in a memory region
 */
void *
memchr(const void *ptr1,register int ch, size_t len)
{
	int line = libmalloc_caller();
	return( DBmemchr( (char *)NULL, line, ptr1, ch, len) );
}

void *
DBmemchr(const char *file, int line, const void *ptr1, register int ch, size_t len)
{
	register const char	* myptr1;
	size_t		  i;

	if ((len == 0) && (!malloc_verify_access_level)) {
		return(NULL);
	}
        if( !in_malloc() && malloc_verify_access )
        {
            ENTER();
	    malloc_check_data("memchr", file, line, ptr1, len);
	    LEAVE();
        }

	myptr1 = (char *) ptr1;

	for( i=0; (i < len) && (myptr1[i] != (char) ch); i++)
	{
	}

	if( i < len )
	{
		return( (void *) (myptr1+i) );
	}
	else
	{
		return( (void *) 0);	
	}
}

/*
 * memcpy  - copy one memory area to another
 * memmove - copy one memory area to another
 */
void * 
memmove(void *ptr1, const void *ptr2, register size_t len)
{
	int line = libmalloc_caller();
	return( DBmemmove( (char *) NULL, line,ptr1, ptr2, len) );
}

void * 
DBmemmove(const char *file, int line, void *ptr1, const void *ptr2, register size_t len)
{
	return( DBFmemcpy( "memmove", file, line, ptr1, ptr2, len) );
}


void *
memcpy(void *ptr1, const void *ptr2, register size_t len)
{
	int line = libmalloc_caller();
	return( DBmemcpy( (char *) NULL, line, ptr1, ptr2, len) );
}

void *
DBmemcpy(const char *file, int line, void *ptr1, const void *ptr2, register size_t len)
{
	return( DBFmemcpy( "memcpy", file, line ,ptr1, ptr2, len) );
}

void *
DBFmemcpy(const char *func, const char *file, int line, void *ptr1, const void *ptr2, register size_t len)
{
	register char		* myptr1;
	register const char	* myptr2;
	void		* rtn = ptr1;

	if ((len == 0) && (!malloc_verify_access_level)) {
		return(rtn);
	}
        if( !in_malloc() && malloc_verify_access )
        {
            ENTER();
	    malloc_check_data(func, file, line, ptr1, len);
	    malloc_check_data(func, file, line, ptr2, len);
            LEAVE();
        }

	myptr1 = ptr1;
	myptr2 = ptr2;

	/*
	 * while the normal memcpy does not guarrantee that it will 
	 * handle overlapping memory correctly, we will try...
	 */
	if( myptr1 > myptr2  && myptr1 < (myptr2+len))
	{
		myptr1 += (len-1);
		myptr2 += (len-1);
		while( len-- > 0 )
		{
			*(myptr1--) = *(myptr2--);
		}
	}
	else
	{
		while( len-- > 0 )
		{
			*(myptr1++) = *(myptr2++);
		}
	}
	
	return(rtn);
}

/*
 * memcmp - compare two memory regions
 */
int
memcmp(const void *ptr1, const void *ptr2, register size_t len)
{
	int line = libmalloc_caller();
	return( DBmemcmp((char *)NULL,line,ptr1,ptr2,len) );
}

int
DBmemcmp(const char *file, int line, const void *ptr1,  const void *ptr2, register size_t len)
{
	return( DBFmemcmp("memcmp",file,line,ptr1,ptr2,len) );
}

int
DBFmemcmp(const char *func, const char *file, int line,
	const void *ptr1, const void *ptr2, register size_t len)
{
	register const unsigned char	* myptr1;
	register const unsigned char	* myptr2;
	
	if ((len == 0) && (!malloc_verify_access_level)) {
		return(0);
	}
        if( !in_malloc() && malloc_verify_access )
        {
            ENTER();
	    malloc_check_data(func,file,line, ptr1, len);
	    malloc_check_data(func,file,line, ptr2, len);
            LEAVE();
        }

	myptr1 = ptr1;
	myptr2 = ptr2;

	while( len > 0  && (*myptr1 == *myptr2) )
	{
		len--;
		myptr1++;
		myptr2++;
	}

	/* 
	 * If stopped by len, return zero
	 */
	if( len == 0 )
	{
		return(0);
	}

	return( ((*myptr1 - *myptr2) < 0) ? -1 : 1);
}

/*
 * memset - set all bytes of a memory block to a specified value
 */
void * 
memset(void *ptr1, register int ch, register size_t len)
{
	int line = libmalloc_caller();
	return( DBmemset((char *)NULL,line,ptr1,ch,len) );
}

void * 
DBmemset(const char *file, int line, void *ptr1, register int ch, register size_t len)
{
	return( DBFmemset("memset",file,line,ptr1,ch,len) );
}

void * 
DBFmemset(const char *func, const char *file, int line,
	void *ptr1, register int ch, register size_t len)
{
	register char		* myptr1;
	char			* rtn = ptr1;

	if ((len == 0) && (!malloc_verify_access_level)) {
		return(rtn);
	}
        if( !in_malloc() && malloc_verify_access )
        {
            ENTER();
	    malloc_check_data(func, file, line, ptr1, len);
            LEAVE();
        }

	myptr1 = ptr1;

	while( len-- )
	{
		*(myptr1++) = ch;
	}

	return(rtn);
}

/*
 * bcopy - copy memory block to another area
 */
void 
bcopy(const void *ptr2, void *ptr1, size_t len)
{
	int line = libmalloc_caller();
	DBbcopy((char *)NULL,line,ptr2,ptr1,len);
}

void *
DBbcopy(const char *file, int line, const void *ptr2, void *ptr1, size_t len)
{
	return( DBFmemcpy("bcopy",file,line,ptr1,ptr2,len));
}

/*
 * bzero - clear block of memory to zeros
 */
void 
bzero(void *ptr1, size_t len)
{
	int line = libmalloc_caller();
	DBbzero((char *)NULL,line,ptr1,len);
}

void *
DBbzero(const char *file, int line, void *ptr1, size_t len)
{
	return( DBFmemset("bzero",file,line,ptr1,'\0',len) );
}

/*
 * bcmp - compary memory blocks
 */
int
bcmp(const void *ptr2, const void *ptr1, size_t len)
{
	int line = libmalloc_caller();
	return( DBbcmp((char *)NULL,line,ptr2, ptr1, len) );
}

int
DBbcmp(const char *file, int line, const void *ptr2, const void *ptr1, size_t len)
{
	return( DBFmemcmp("bcmp",file,line,ptr1,ptr2,len) );
}

/*
 * $Log$
 * Revision 1.8  2005/06/03 01:22:48  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.7  2005/03/29 18:22:44  shiv
 * PR24134
 * PR24010
 * PR24008
 * PR24184
 * The malloc lib used to report errors when mem* and str* functions
 * were called (for those that take a length parameter) with
 * a length of zero, if the other arguments are not valid..
 * In general these would not cause errors, since
 * no actual data is moved. But since the errors being reported could
 * be useful, the option to increase the level of verbosity for this
 * has been provided. the environment variable
 * MALLOC_CKACCESS_LEVEL can be used or the mallopt call
 * with the option mallopt(MALLOC_CKACCESS_LEVEL, arg)
 * can be used. By default the level is zero, a non-zero
 * level will turn on strict checking and reporting of errors
 * if the arguments are not valid.
 * Also fixed PR24184 which had an incorrect function name
 * being passed in (strchr instead of strrchr... thanx kirk)
 * Modified Files:
 * 	mallocint.h dbg/m_init.c dbg/malloc_chk.c dbg/malloc_g.c
 * 	dbg/mallopt.c dbg/memory.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h
 *
 * Revision 1.6  2005/01/16 20:38:45  shiv
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
 * Revision 1.5  2004/02/12 15:43:17  shiv
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
 * Revision 1.4  2003/10/22 20:51:04  shiv
 * PR17401: memcmp in debug malloc lib needs to do comparisons
 * using unsigned rather than signed.
 *
 * Revision 1.3  2000/02/14 16:34:27  furr
 * Fixed bzero, bcopy
 *
 *  Modified Files:
 *  	mallocint.h dbg/memory.c public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h
 *
 * Revision 1.2  2000/02/08 19:16:23  furr
 * Fix up guard code problems re. underlying implementation
 * Fixed up problems for Java such as locking on mem functions
 *
 *  Modified Files:
 *  	dbg/free.c dbg/malloc_chk.c dbg/memory.c dbg/realloc.c
 *  	dbg/string.c
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
 * Revision 1.2  1996/08/18 21:01:12  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:06:40  furr
 * Initial revision
 *
 * Revision 1.13  1992/01/24  04:49:05  cpcahil
 * changed memccpy to only check number of chars it will copy.
 *
 * Revision 1.12  1991/12/31  21:31:26  cpcahil
 * changes for patch 6.  See CHANGES file for more info
 *
 * Revision 1.11  1991/12/02  19:10:13  cpcahil
 * changes for patch release 5
 *
 * Revision 1.10  91/11/25  14:42:03  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.9  91/11/24  00:49:31  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.8  91/05/21  18:33:47  cpcahil
 * fixed bug in memccpy() which checked an extra byte if the first character
 * after the specified length matched the search character.
 * 
 * Revision 1.7  90/08/29  21:27:58  cpcahil
 * fixed value of check in memccpy when character was not found.
 * 
 * Revision 1.6  90/07/16  20:06:26  cpcahil
 * fixed several minor bugs found with Henry Spencer's string/mem tester 
 * program.
 * 
 * 
 * Revision 1.5  90/05/11  15:39:36  cpcahil
 * fixed bug in memccpy().
 * 
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:29  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:29:41  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 * 
 */
