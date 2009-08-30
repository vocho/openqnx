# Configure template for gzip.

# Copyright (C) 1999, 2000, 2001, 2002, 2006, 2007 Free Software
# Foundation, Inc.

# Copyright (C) 1992-1993 Jean-loup Gailly

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

AC_PREREQ([2.60])
AC_INIT([gzip], [1.3.12], [bug-gzip@gnu.org])
AC_CONFIG_SRCDIR(gzip.c)
AC_CONFIG_AUX_DIR(build-aux)
AC_CONFIG_HEADERS([lib/config.h:lib/config.hin])
AM_INIT_AUTOMAKE([gnits])

AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_PROG_CPP
AC_CHECK_TOOL([NM], [nm], [nm])
AC_CHECK_TOOL([AR], [ar], [ar])
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_SHELL

gl_EARLY
AC_SYS_LARGEFILE

gl_INIT

# cc -E produces incorrect asm files on SVR4, so postprocess it.
ASCPPPOST="sed '/^ *#/d; s,//.*,,; s/% /%/g; s/\\. /./g'"
AC_SUBST([ASCPPPOST])

AC_CACHE_CHECK([for underline in external names], [gzip_cv_underline],
  [gzip_cv_underline=yes
   AC_TRY_COMPILE([int foo() {return 0;}], [],
     [$NM conftest.$OBJEXT | grep _foo >/dev/null 2>&1 ||
	gzip_cv_underline=no])])
ASCPPFLAGS=
if test $gzip_cv_underline = no; then
  ASCPPFLAGS='-DNO_UNDERLINE'
fi
AC_SUBST([ASCPPFLAGS])
AC_OBJEXT

# Try to assemble match.S.
# "gcc -E match.s" ignores -E, so we must use _match.S.
AC_CACHE_CHECK([for an assembler syntax supported by this package],
  [gzip_cv_assembler],
  [gzip_cv_assembler=no
   case " $DEFS " in
   *' NO_ASM '*) ;;
   *)
     if cp $srcdir/lib/match.c _match.S &&
	eval "$CPP $CPPFLAGS $ASCPPFLAGS _match.S > _match.i" 2>/dev/null &&
	eval "$ASCPPPOST < _match.i > match_.s" 2>/dev/null; then
       if test ! -s match_.s || grep error < match_.s > /dev/null; then
	 :
       elif eval "$CC $CPPFLAGS $CFLAGS -c match_.s >/dev/null 2>&1" &&
	    test -f match_.$OBJEXT; then
	 rm -f match_.$OBJEXT
	 gzip_cv_assembler=yes
	 if echo 'void foo (void) {}' > conftest.c &&
	    eval "$CC $CPPFLAGS $CFLAGS -S conftest.c >/dev/null 2>&1" &&
	    grep '\.note\.GNU-stack' conftest.s >/dev/null 2>&1 &&
	    eval "$CC $CPPFLAGS $CFLAGS -c -Wa,--noexecstack match_.s >/dev/null 2>&1" &&
	    test -f match_.$OBJEXT; then
	   gzip_cv_assembler='yes, with -Wa,--noexecstack'
	 fi
       fi
     fi
     rm -f conftest* _match.i _match.S match_.s match_.$OBJEXT;;
   esac])
if test "$gzip_cv_assembler" != no; then
  AC_DEFINE(ASMV, ,
    [Define if an assembler version of longest_match is available.])
  AC_LIBOBJ(match)
fi
if test "$gzip_cv_assembler" = 'yes, with -Wa,--noexecstack'; then
  ASFLAGS_config='-Wa,--noexecstack'
else
  ASFLAGS_config=
fi
AC_SUBST([ASFLAGS_config])

AC_ISC_POSIX
AC_C_CONST
AC_HEADER_STDC
AC_CHECK_HEADERS_ONCE(fcntl.h limits.h memory.h time.h)
AC_CHECK_FUNCS_ONCE([chown fchmod fchown fdopendir lstat siginterrupt])
AC_HEADER_DIRENT
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_OFF_T

AC_PREFIX_PROGRAM(gzip)

AC_CONFIG_FILES([Makefile doc/Makefile lib/Makefile])
AC_OUTPUT
