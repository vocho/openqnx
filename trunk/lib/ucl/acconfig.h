/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* acconfig.h -- autoheader configuration file

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#ifndef __UCL_CONFIG_H
#define __UCL_CONFIG_H

/* $TOP$ */
@TOP@

/* acconfig.h

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* Define to your byte order.  */
#undef MFX_BYTE_ORDER

/* Define if your memcmp is broken.  */
#undef NO_MEMCMP

/* Define to `long' if <stddef.h> doesn't define.  */
#undef ptrdiff_t

/* The number of bytes in a ptrdiff_t.  */
#undef SIZEOF_PTRDIFF_T

/* The number of bytes in a size_t.  */
#undef SIZEOF_SIZE_T



/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */



@BOTTOM@

/* $BOTTOM$ */

#if defined(HAVE_SYS_RESOURCE_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_RESOURCE_H
#endif

#if defined(HAVE_SYS_TIMES_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_TIMES_H
#endif

#if defined(NO_MEMCMP)
#  undef /**/ HAVE_MEMCMP
#endif

#if (SIZEOF_CHAR_P <= 0)
#  undef /**/ SIZEOF_CHAR_P
#endif

#if (SIZEOF_PTRDIFF_T <= 0)
#  undef /**/ SIZEOF_PTRDIFF_T
#endif

#if (SIZEOF_UNSIGNED <= 0)
#  undef /**/ SIZEOF_UNSIGNED
#endif

#if (SIZEOF_UNSIGNED_LONG <= 0)
#  undef /**/ SIZEOF_UNSIGNED_LONG
#endif

#if (SIZEOF_UNSIGNED_SHORT <= 0)
#  undef /**/ SIZEOF_UNSIGNED_SHORT
#endif

#if (SIZEOF_SIZE_T <= 0)
#  undef /**/ SIZEOF_SIZE_T
#endif

#endif /* already included */

/*
vi:ts=4
*/
