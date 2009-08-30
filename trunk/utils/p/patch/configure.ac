# Configure `patch'.

# Copyright (C) 1993, 1997, 1998, 1999, 2002, 2003 Free Software
# Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

AC_PREREQ(2.57)
AC_INIT(patch, 2.5.9, bug-patch@gnu.org)
AC_CONFIG_SRCDIR(patch.c)
AC_CONFIG_HEADER(config.h:config.hin)
AC_ARG_PROGRAM

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
# Use ed_PROGRAM, not ED_PROGRAM,
# because <errno.h> reserves symbols starting with `E'.
AC_PATH_PROG(ed_PROGRAM, ed, ed)

AC_GNU_SOURCE
AC_AIX
AC_MINIX
AC_PROG_CC_STDC
AC_ISC_POSIX
AC_SYS_LARGEFILE
AC_EXEEXT

AC_C_PROTOTYPES
AC_C_CONST

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h string.h unistd.h utime.h varargs.h)

AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AM_STDBOOL_H
jm_CHECK_TYPE_STRUCT_UTIMBUF

gl_BACKUPFILE
gl_DIRNAME
gl_ERROR
gl_FUNC_MEMCHR
gl_FUNC_RMDIR
gl_GETOPT
gl_PREREQ_XMALLOC
gl_QUOTE
gl_QUOTEARG

dnl This should be in gnulib, but isn't for some reason.
AC_DEFUN([jm_PREREQ_ADDEXT],
[
  dnl For addext.c.
  AC_REQUIRE([AC_SYS_LONG_FILE_NAMES])
  AC_CHECK_FUNCS(pathconf)
  AC_CHECK_HEADERS(limits.h string.h unistd.h)
])
jm_PREREQ_ADDEXT

AC_CHECK_DECLS([free, getenv, malloc, mktemp])
AC_CHECK_FUNCS(_doprintf geteuid getuid isascii memcmp mktemp \
  pathconf raise sigaction sigprocmask sigsetmask strerror)
AC_REPLACE_FUNCS(mkdir strncasecmp)
AC_FUNC_FSEEKO
jm_FUNC_GLIBC_UNLOCKED_IO
jm_FUNC_MALLOC
jm_FUNC_REALLOC
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_SETMODE_DOS
AC_FUNC_VPRINTF
PATCH_FUNC_MKDIR_TAKES_ONE_ARG

jm_AC_DOS
AC_SYS_LONG_FILE_NAMES

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
