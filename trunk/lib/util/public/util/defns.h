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



#ifndef _DEFNS_H_INCLUDED
#define _DEFNS_H_INCLUDED
/*
	for portability (ejohnson)
*/

#if 0 /*use inttypes.h instead*/

#define	int32	long
#define	int16	short
#define	int8	char
#define	uint32	unsigned long
#define	uint16	unsigned short
#define	uint8	unsigned char
#define uint	unsigned short
#define uchar	unsigned char
#define byte	unsigned char

#define	Rint32	register long
#define	Rint16	register short
#define	Rint8	register char
#define	Ruint32	register unsigned long
#define	Ruint16	register unsigned short
#define	Ruint8	register unsigned char
#define Rbyte	register unsigned char

#endif

#define bool    int

#ifndef TRUE
	#define	TRUE	(1==1)
#endif

#ifndef true
	#define	true	(1==1)
#endif

#ifndef FALSE
	#define	FALSE	(1==0)
#endif

#ifndef false
	#define	false	(1==0)
#endif

/* 
CASESTR() macro definition -
Use:
	replace:

		switch(c[0]<<8 | c[1]) 
		{
        	case 'ab':
			etc..
		}

	with:		

		switch(CASESTR(c))
		{
			case 'ab':
			etc..
		}
*/

#define CASESTR(str) ( ((str)[0]<<8) | (str)[1])

/* added from Grant's header file */

#ifdef MAIND
#	define EXTRN     /**/
#	define INITIAL(s) = {s}
#else
#	define EXTRN     extern
#	define INITIAL(s) /**/
#endif


#ifndef ERR
#define ERR		-1
#endif
#ifndef NOERR
#define NOERR	0
#endif

#ifdef OK
#	undef OK
#endif

#define OK		1

#ifndef NDEBUG
#ifndef DBG
#	define DBG(stmt) stmt
#endif
#ifndef ASSERT
#	define ASSERT(cond) if (!(cond)) fprintf(stderr,"ASSERT Error : '%s'\n","cond")
#endif
#ifndef ERROR
#	define ERROR(code,string) fprintf(stderr,"ERROR %3d: %s",code,string)
#endif
#ifndef DBUG
#	define DBUG(string) fprintf(stderr,"%s",(string))
#endif
#else
#ifndef DBG
#	define DBG(stmt) /**/
#endif
#ifndef ASSERT
#	define ASSERT(cond) /**/
#endif
#ifndef ERROR
#	define ERROR(code,string) /**/
#endif
#ifndef DBUG
#	define DBUG(string) /**/
#endif
#endif                  
#endif
