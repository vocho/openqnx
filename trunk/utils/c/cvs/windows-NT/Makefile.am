## Process this file with automake to produce Makefile.in

# *** Under Windows NT and Microsoft Visual C++, we use cvsnt.mak, not
# *** this makefile.  However, we need this file in order for 'make
# *** dist' to work properly on Unix machines.

# Makefile for GNU CVS windows-NT dist.
# Copyright (C) 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993, 1994,
#               1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003
#               Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

EXTRA_DIST = \
	config.h \
	config.h.in \
	ndir.h \
	pwd.h \
	rcmd.h \
	filesubr.c \
	mkdir.c \
	ndir.c \
	pwd.c \
	rcmd.c \
	run.c \
	sockerror.c \
	startserver.c \
	stripslash.c \
	waitpid.c \
	woe32.c \
	.cvsignore \
	stamp-ch

SUBDIRS = SCC

###
### Maintainer targets
###
all-local: config.h
config.h: stamp-ch
stamp-ch: config.h.in $(top_srcdir)/configure
	sed "s/%PACKAGE_VERSION%/$(VERSION)/" <$(srcdir)/config.h.in \
	  >config.tmp
	@cmp -s config.tmp $(srcdir)/config.h \
	  || (echo "Updating config.h"; \
	      cp config.tmp $(srcdir)/config.h)
	-@rm -f config.tmp
	@cp $(srcdir)/config.h $@

# for backwards compatibility with the old makefiles
realclean: maintainer-clean
.PHONY: realclean
