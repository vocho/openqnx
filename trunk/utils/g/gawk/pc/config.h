/* config.h.  Generated automatically by configure.  */
/* configh.in.  Generated automatically from configure.in by autoheader.  */
/*
 * acconfig.h -- configuration definitions for gawk.
 */

/* 
 * Copyright (C) 1995-2000 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
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


/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#define GETGROUPS_T gid_t

/* Define if the `getpgrp' function takes no argument.  */
#define GETPGRP_VOID 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define if you don't have vprintf but do have _doprnt.  */
/* #undef HAVE_DOPRNT */

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME 1

/* Define if you have the vprintf function.  */
#define HAVE_VPRINTF 1

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown
 */
/* #undef STACK_DIRECTION */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* #undef GETPGRP_IS_STANDARD */	/* getpgrp does/does not take an argument */
/* #undef HAVE_BCOPY */	/* we have the bcopy function */
#define HAVE_MEMCPY 1	/* we have the memcpy function */
#define HAVE_STRINGIZE 1	/* can use ANSI # operator in cpp */
#define HAVE_STRING_H 1	/* the <string.h> header file */
#define REGEX_MALLOC 1	/* use malloc instead of alloca in regex.c */
#define SPRINTF_RET int	/* return type of sprintf */

/* #undef BITOPS */  /* bitwise ops (undocumented feature) */
/* #undef NONDECDATA */ /* non-decimal input data (undocumented feature) */

/* Define if you have the fmod function.  */
#define HAVE_FMOD 1

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the random function.  */
#define HAVE_RANDOM 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strtod function.  */
#define HAVE_STRTOD 1

/* Define if you have the system function.  */
#define HAVE_SYSTEM 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <signum.h> header file.  */
/* #undef HAVE_SIGNUM_H */

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <strings.h> header file.  */
/* #undef HAVE_STRINGS_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1


/* Library search path */
#define DEFPATH  ".;c:/lib/awk;c:/gnu/lib/awk"

#if defined (_MSC_VER)
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#undef HAVE_UNISTD_H
#undef HAVE_SYS_PARAM_H
#undef HAVE_RANDOM
#define RANDOM_MISSING
/* msc strftime is incomplete, use supplied version */
#undef HAVE_STRFTIME
/* #define HAVE_TM_ZONE */
#define altzone timezone
#if defined(OS2)  /* declare alloca for bison */
void * alloca(unsigned);
#endif
#endif

# define HAVE_POPEN_H

#if (defined(_MSC_VER) && defined(MSDOS)) || defined(__MINGW32__)
# define system(s) os_system(s)
#endif

#if defined (_MSC_VER) || defined(__EMX__)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#if defined(DJGPP)
# define HAVE_LIMITS_H 1
# undef HAVE_POPEN_H
#endif

#if defined(__WIN32__) && defined(__CRTRSXNT__)
#include <crtrsxnt.h>
#endif

/* For vcWin32 */
#if defined(WIN32) && defined(_MSC_VER)
#define alloca _alloca
#define system(s) os_system(s)
#endif

#if defined(__MINGW32__)
#undef HAVE_SYS_PARAM_H
#endif
