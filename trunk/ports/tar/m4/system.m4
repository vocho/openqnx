# Configuration for paxutils/lib/system.h.

# Copyright (C) 2005, 2006 Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

AC_DEFUN([PU_SYSTEM],[
  AC_REQUIRE([AC_HEADER_STDC])

  AC_CHECK_HEADERS_ONCE([string.h memory.h fcntl.h sys/wait.h \
 sys/gentape.h sys/tape.h sys/device.h sys/param.h sys/tprintf.h sys/mtio.h \
 sgtty.h sys/io/trioctl.h locale.h])

  AC_CHECK_HEADERS([sys/buf.h], [], [],
   [#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif])

  AC_REQUIRE([AC_HEADER_TIME])

  AC_CHECK_MEMBERS([struct stat.st_blksize]) dnl instead of AC_STRUCT_ST_BLKSIZE
  AC_REQUIRE([AC_STRUCT_ST_BLOCKS])
  AC_REQUIRE([AC_STRUCT_ST_BLKSIZE])

  AC_CHECK_FUNCS_ONCE(lstat mkfifo setlocale)
  AC_REQUIRE([gl_INTTYPES_H])
  AC_REQUIRE([gl_STDINT_H])
])
