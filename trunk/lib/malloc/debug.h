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
/************************************************************************/
/*									*/
/* this include sets up some macro functions which can be used while	*/
/* debugging the program, and then left in the code, but turned of by	*/
/* just not defining "DEBUG".  This way your production version of 	*/
/* the program will not be filled with bunches of debugging code	*/
/*									*/
/************************************************************************/
/*
 * $Id: debug.h 153052 2008-08-13 01:17:50Z coreos $
 */

#ifdef DEBUG

#if DEBUG == 1			/* if default level			*/
#undef DEBUG
#define DEBUG	100		/*   use level 100			*/
#endif

#include <stdio.h>

#define DEBUG0(val,str)\
				{\
				  if( DEBUG > val ) \
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,str);\
				}
#define DEBUG1(val,str,a1)\
			        {\
				  char _debugbuf[100];\
				  if( DEBUG > val )\
				   {\
				    sprintf(_debugbuf,str,a1);\
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,_debugbuf);\
				   }\
		       		}

#define DEBUG2(val,str,a1,a2)\
			        {\
				 char _debugbuf[100];\
				  if( DEBUG > val )\
				   {\
				    sprintf(_debugbuf,str,a1,a2);\
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,_debugbuf);\
				   }\
		       		}

#define DEBUG3(val,str,a1,a2,a3)\
			        {\
				  char _debugbuf[100];\
				  if( DEBUG > val )\
				   {\
				    sprintf(_debugbuf,str,a1,a2,a3);\
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,_debugbuf);\
				   }\
		       		}

#define DEBUG4(val,str,a1,a2,a3,a4)\
			         {\
				  char _debugbuf[100];\
				  if( DEBUG > val )\
				   {\
				    sprintf(_debugbuf,str,a1,a2,a3,a4);\
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,_debugbuf);\
				   }\
		       		}

#define DEBUG5(val,str,a1,a2,a3,a4,a5)\
			         {\
				  char _debugbuf[100];\
				  if( DEBUG > val )\
				   {\
				    sprintf(_debugbuf,str,a1,a2,a3,a4,a5);\
				    fprintf(stderr,"%s(%d): %s\n",\
						__FILE__,__LINE__,_debugbuf);\
				   }\
		       		}

#else

#define DEBUG0(val,s)
#define DEBUG1(val,s,a1)
#define DEBUG2(val,s,a1,a2)
#define DEBUG3(val,s,a1,a2,a3)
#define DEBUG4(val,s,a1,a2,a3,a4)
#define DEBUG5(val,s,a1,a2,a3,a4,a5)

#endif /* DEBUG */


/*
 * $Log$
 * Revision 1.3  2005/06/03 01:22:47  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
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
 * Revision 1.3  1991/11/25  14:41:51  cpcahil
 * Final changes in preparation for patch 4 release
 *
 * Revision 1.2  90/05/11  00:13:08  cpcahil
 * added copyright statment
 * 
 * Revision 1.1  90/02/23  07:09:01  cpcahil
 * Initial revision
 * 
 */
