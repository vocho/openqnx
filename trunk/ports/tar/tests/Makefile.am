# Makefile for GNU tar regression tests.

# Copyright (C) 1996, 1997, 1999, 2000, 2001, 2003, 2004, 2005, 
# 2006, 2007 Free Software Foundation, Inc.

# François Pinard <pinard@iro.umontreal.ca>, 1988.
# Sergey Poznyakoff <gray@mirddin.farlep.net>, 2004.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

EXTRA_DIST = $(TESTSUITE_AT) testsuite package.m4 star/README star/quicktest.sh
DISTCLEANFILES       = atconfig $(check_SCRIPTS)
MAINTAINERCLEANFILES = Makefile.in $(TESTSUITE)

## ------------ ##
## package.m4.  ##
## ------------ ##

$(srcdir)/package.m4: $(top_srcdir)/configure.ac
	{                                      \
	  echo '# Signature of the current package.'; \
	  echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	  echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	  echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	  echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	  echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

#

## ------------ ##
## Test suite.  ##
## ------------ ##

TESTSUITE_AT = \
 T-empty.at\
 T-null.at\
 testsuite.at\
 append.at\
 append01.at\
 append02.at\
 chtype.at\
 comprec.at\
 delete01.at\
 delete02.at\
 delete03.at\
 delete04.at\
 delete05.at\
 exclude.at\
 extrac01.at\
 extrac02.at\
 extrac03.at\
 extrac04.at\
 extrac05.at\
 extrac06.at\
 extrac07.at\
 gzip.at\
 grow.at\
 incremental.at\
 incr01.at\
 incr02.at\
 incr03.at\
 incr04.at\
 indexfile.at\
 ignfail.at\
 link01.at\
 listed01.at\
 listed02.at\
 long01.at\
 longv7.at\
 lustar01.at\
 lustar02.at\
 lustar03.at\
 multiv01.at\
 multiv02.at\
 multiv03.at\
 multiv04.at\
 multiv05.at\
 old.at\
 options.at\
 options02.at\
 pipe.at\
 recurse.at\
 rename01.at\
 rename02.at\
 rename03.at\
 same-order01.at\
 same-order02.at\
 shortrec.at\
 sparse01.at\
 sparse02.at\
 sparse03.at\
 sparsemv.at\
 sparsemvp.at\
 spmvp00.at\
 spmvp01.at\
 spmvp10.at\
 truncate.at\
 update.at\
 volsize.at\
 volume.at\
 verbose.at\
 version.at\
 star/gtarfail.at\
 star/gtarfail2.at\
 star/multi-fail.at\
 star/ustar-big-2g.at\
 star/ustar-big-8g.at\
 star/pax-big-10g.at

TESTSUITE = $(srcdir)/testsuite

AUTOTEST = $(AUTOM4TE) --language=autotest
$(TESTSUITE): package.m4 $(TESTSUITE_AT)
	$(AUTOTEST) -I $(srcdir) testsuite.at -o $@.tmp
	mv $@.tmp $@

atconfig: $(top_builddir)/config.status
	cd $(top_builddir) && ./config.status tests/$@

clean-local:
	test ! -f $(TESTSUITE) || $(SHELL) $(TESTSUITE) --clean

check-local: atconfig atlocal $(TESTSUITE)
	$(SHELL) $(TESTSUITE)

check-full:
	FULL_TEST=1 $(MAKE) check

#check_SCRIPTS = tar

# Run the test suite on the *installed* tree.
installcheck-local:
	$(SHELL) $(TESTSUITE) AUTOTEST_PATH=$(exec_prefix)/bin


## ------------ ##
## genfile      ##
## ------------ ##

check_PROGRAMS = genfile

genfile_SOURCES = genfile.c argcv.c argcv.h

localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir)/lib -I../lib -I$(top_srcdir)/src
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"
LDADD = ../lib/libtar.a $(LIBINTL) $(LIB_CLOCK_GETTIME)
