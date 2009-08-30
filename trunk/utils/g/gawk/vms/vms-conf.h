/*
 * config.h -- configuration definitions for gawk.
 *
 * For VMS (assumes V4.6 or later; tested on V5.5-2)
 */

/* 
 * Copyright (C) 1991, 1992, 1995, 1996, 1999 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/* Define if using alloca.c.  */
#define C_ALLOCA
#define STACK_DIRECTION (-1)
#define REGEX_MALLOC	/* use malloc instead of alloca in regex.c */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

#define SPRINTF_RET int

/* Define if the `getpgrp' function takes no argument.  */
#define GETPGRP_VOID	1

#define HAVE_STRING_H	1	/* the <string.h> header file */

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP	1
/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY	1
/* Define if you have the memset function.  */
#define HAVE_MEMSET	1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR	1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR	1

/* Define if you have the strtod function.  */
#define HAVE_STRTOD	1

/* Define if you have the system function.  */
#define HAVE_SYSTEM	1

/* Define if you have the fmod function.  */
#define HAVE_FMOD	1

/* Define if you have the tzset function.  */
/* [Fake it in vms/vms_misc.c since missing/tzset.c won't compile.]  */
#define HAVE_TZSET	1
#define HAVE_TZNAME	1

#define STDC_HEADERS	1

#define HAVE_VPRINTF 1


/*******************************/
/* Gawk configuration options. */
/*******************************/

/*
 * DEFPATH
 *	VMS: "/AWK_LIBRARY" => "AWK_LIBRARY:"
 * The default search path for the -f option of gawk.  It is used
 * if the AWKPATH environment variable is undefined.
 *
 * Note: OK even if no AWK_LIBRARY logical name has been defined.
 */

#define DEFPATH	".,/AWK_LIBRARY"
#define ENVSEP	','

/*
 * Extended source file access.
 */
#define DEFAULT_FILETYPE ".awk"

/*
 * Pipe handling.
 */
#define PIPES_SIMULATED	1

/*
 * %g format in VAXCRTL is broken (chooses %e format when should use %f).
 */
#define GFMT_WORKAROUND	1

/*
 * VAX C
 *
 * As of V3.2, VAX C is not yet ANSI-compliant.  But it's close enough
 * for GAWK's purposes.  Comment this out for VAX C V2.4 and earlier.
 * Value of 0 should mean "not ANSI-C", but GAWK uses def/not-def tests.
 * YYDEBUG definition is needed for combination of VAX C V2.x and Bison.
 */
#if defined(VAXC) && !defined(__STDC__)
#define __STDC__	0
#define NO_TOKEN_PASTING
#ifndef __DECC	/* DEC C does not support #pragma builtins even in VAXC mode */
#define VAXC_BUILTINS
#endif
/* #define YYDEBUG 0 */
#endif

/*
 * DEC C
 *
 * Digital's ANSI complier.
 */
#ifdef __DECC
 /* DEC C implies DECC$SHR, which doesn't have the %g problem of VAXCRTL */
#undef GFMT_WORKAROUND
 /* DEC C V5.x introduces incompatibilities with prior porting efforts */
#define _DECC_V4_SOURCE
#define __SOCKET_TYPEDEFS
#if __VMS_VER >= 60200000
# undef __VMS_VER
# define __VMS_VER 60100000
#endif
#if __CRTL_VER >= 60200000
# undef __CRTL_VER
# define __CRTL_VER 60100000
#endif
#endif

/*
 * GNU C
 *
 * Versions of GCC (actually GAS) earlier than 1.38 don't produce the
 * right code for ``extern const'' constructs, and other usages of
 * const might not be right either.  The old set of include files from
 * the gcc-vms distribution did not contain prototypes, and this could
 * provoke some const-related compiler warnings.  If you've got an old
 * version of gcc for VMS, define 'const' out of existance, and by all
 * means obtain the most recent version!
 *
 * Note: old versions of GCC should also avoid defining STDC_HEADERS,
 *       because most of the ANSI-C required header files are missing.
 */
#ifdef __GNUC__
/* #define const */
/* #undef STDC_HEADERS */
#ifndef STDC_HEADERS
#define alloca __builtin_alloca
#define environ $$PsectAttributes_NOSHR$$environ	/* awful GAS kludge */
#endif
#undef  REGEX_MALLOC	/* use true alloca() in regex.c */
#endif

#define IN_CONFIG_H
#include "vms/redirect.h"
#undef  IN_CONFIG_H
