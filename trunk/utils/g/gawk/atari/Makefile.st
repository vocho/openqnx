# Makefile for GNU Awk - ST version.
#
# This makefile hand edited from Makefile automatically generated
# by configure - gcc 2.x.x compiler with TOS version of libraries
# and modified system().
# Check comments in this Makefile and adjust to your needs!!
#
# This Makefile assumes that you are using Bourne compatible shell
# (like bash). If this is not the case you will have to edit various
# targets or perform some actions by hand.
#
# Copyright (C) 1986, 1988-1996 the Free Software Foundation, Inc.
# 
# This file is part of GAWK, the GNU implementation of the
# AWK Programming Language.
# 
# GAWK is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# GAWK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA


MAKEINFO = makeinfo --no-split

srcdir = .

# native compiler with freshly compiled gawk.ttp to fix awklib
GAWK = gawk.ttp
CC = gcc

# cross-compiler and gawk already installed on the system (any awk will do)
# GAWK = gawk
# CC = cgcc

# WIDTH and EXT have to be both defined or both undefined
# WIDTH = -mshort -DINT_IS_16BIT
# EXT = 16

OFLAGS = -O2 -Wall -fomit-frame-pointer $(WIDTH)
LDFLAGS = $(WIDTH)
YACC = bison -y

# xstrip -k in target gawk.ttp removes all symbols but _stksize
# allowing for stack size manipulations without recompiling (with fixstk)
INSTALL = xstrip -k ; cp -p
INSTALL_PROGRAM = ${INSTALL}
#INSTALL_DATA = ${INSTALL} -m 644
INSTALL_DATA = ${INSTALL}

LIBS = -lpml$(EXT)

ALLOCA = 

#all these definitions likely require changes
exec_prefix = ${prefix}
prefix = /usr/local
binprefix =
manprefix =

bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
manexta = l
mandir = ${prefix}/man/man$(manexta)
manext = .$(manexta)
infodir = ${prefix}/info
#datadir = ${prefix}/share/awk
datadir = ${prefix}/lib/awk
libexecdir = ${exec_prefix}/lib/awk

#DEFPATH = ".:$(datadir)"
# datadir is passed to the next Makefile level and through sed
# you may need many more backslashes than that if you have to use
# them at all - sigh...
DEFPATH = ".,c:\\lib\\awk,c:\\gnu\\lib\\awk"

SHELL = /bin/sh

SHELL = /bin/sh
#CFLAGS = -g -O
CFLAGS = $(OFLAGS)
COMPFLAGS = $(CFLAGS) -DGAWK -I. -I$(srcdir) -DHAVE_CONFIG_H

MFLAGS = "CC=$(CC)" \
  "CFLAGS=$(CFLAGS)" \
   GAWK=../$(GAWK) \
   AWK=awk \
   PWCAT=pwcat.ttp GRCAT=grcat.ttp \
  "GCOM='{sub(/\":\"/, \"\\\",\\\"\"); print}'" \
  "COMPFLAGS=$(COMPFLAGS)" \
  "LDFLAGS=$(LDFLAGS)" \
  "INSTALL=$(INSTALL)" \
  "INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
  "INSTALL_DATA=$(INSTALL_DATA)" \
  "bindir=$(bindir)" \
  "libdir=$(libdir)" \
  "mandir=$(mandir)" \
  "manext=$(manext)" \
  "infodir=$(infodir)" \
  "datadir=$(datadir)" \
  "libexecdir=$(libexecdir)"

MMAKE = $(MAKE) $(MFLAGS)

# object files
AWKOBJS = array.o builtin.o eval.o field.o gawkmisc.o io.o main.o \
	missing.o msg.o node.o re.o version.o

ALLOBJS = $(AWKOBJS) awktab.o

# LIBOBJS
#	GNU and other stuff that gawk uses as library routines.
LIBOBJS= getopt.o getopt1.o regex.o dfa.o random.o $(ALLOCA)

# source and documentation files
SRC = array.c builtin.c eval.c field.c gawkmisc.c io.c main.c \
	missing.c msg.c node.c re.c version.c

ALLSRC= $(SRC) awktab.c

AWKSRC= awk.h awk.y custom.h $(ALLSRC) patchlevel.h protos.h random.h

LIBSRC = alloca.c dfa.c dfa.h regex.c regex.h getopt.h getopt.c getopt1.c random.c

COPIES = missing/system.c missing/tzset.c \
	missing/memcmp.c missing/memcpy.c missing/memset.c \
	missing/strncasecmp.c missing/strchr.c \
	missing/strerror.c missing/strtod.c \
	missing/strftime.c missing/strftime.3 

DOCS= doc/gawk.1 doc/gawk.texi doc/texinfo.tex

TEXFILES= doc/gawk.aux doc/gawk.cp doc/gawk.cps doc/gawk.fn doc/gawk.fns \
	  doc/gawk.ky doc/gawk.kys doc/gawk.pg doc/gawk.pgs doc/gawk.toc \
	  doc/gawk.tp doc/gawk.tps doc/gawk.vr doc/gawk.vrs

MISC =	NEWS COPYING FUTURES Makefile.in PROBLEMS README PORTS POSIX.STD \
	configure configure.in acconfig.h configh.in ACKNOWLEDGMENT \
	ChangeLog INSTALL LIMITATIONS install-sh mkinstalldirs aclocal.m4 \
	stamp-h.in

OTHERS= amiga doc pc atari vms README_d posix awklib

ALLDOC= doc/gawk.dvi $(TEXFILES) doc/gawk.info*

MAKEFILEIN = Makefile.in awklib/Makefile.in doc/Makefile.in test/Makefile.in

# Release of gawk.  There can be no leading or trailing white space here!
REL=3.0

# clear out suffixes list
.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(COMPFLAGS) $<

# rules to build gawk
all:	$(GAWK) awklib/all

alldoc: all doc/all

$(GAWK):	$(ALLOBJS) $(LIBOBJS) $(REOBJS)
	$(CC) -o $(GAWK) $(COMPFLAGS) $(LDFLAGS) $(ALLOBJS) $(LIBOBJS) \
	$(REOBJS) $(LIBS)

$(ALLOBJS):	awk.h dfa.h regex.h config.h custom.h

$(LIBOBJS):	config.h custom.h

gawkmisc.o:	$(srcdir)/atari/gawkmisc.atr
	$(CC) -c $(COMPFLAGS) -DDEFPATH='$(DEFPATH)' $(srcdir)/gawkmisc.c

# this rule needed or not - depending on your library
missing.o io.o:	
	$(CC) -c $(COMPFLAGS) -DPIPES_SIMULATED $(srcdir)/$<

# cheat with defines to force an inclusion of a proper code
getopt.o:	getopt.h
	$(CC) $(CFLAGS) -D_LIBC -D__alloca=__builtin_alloca -c getopt.c
  
getopt.o:	getopt.h

getopt1.o:	getopt.h

random.o:	random.h

main.o:	patchlevel.h

awktab.c:	awk.y
	$(YACC) -v $(srcdir)/awk.y && \
	if test -f y.tab.c ; then mv y.tab.c ytab.c ; else true ; fi && \
	sed '/^extern char .malloc(), .realloc();$$/d' ytab.c >awktab.c && \
	rm ytab.c

# VMS POSIX make won't apply the default .c.o rule to awktab.o for some reason
awktab.o:	awktab.c awk.h
	$(CC) -c $(COMPFLAGS) $(srcdir)/awktab.c

alloca.o:	alloca.c

install:	$(GAWK) info installdirs
	$(INSTALL_PROGRAM) $(GAWK) $(bindir) && chmod 755 $(bindir)/$(GAWK)
	cd awklib && $(MMAKE) install

installdirs: mkinstalldirs
	$(srcdir)/mkinstalldirs $(bindir) $(datadir) \
		$(libdir) $(infodir) $(mandir) $(libexecdir)

installdoc:	info
	cd doc && $(MMAKE) install

#	
#installtotal:	installdirs install installdoc

uninstall:
	rm -f $(bindir)/$(GAWK)
	cd awklib && $(MMAKE) uninstall
	-rmdir $(datadir) $(libexecdir)
#	cd doc && $(MMAKE) uninstall

clean:
	rm -rf $(GAWK) *.o core y.output
	cd awklib && $(MMAKE) clean
# the following does not always make sense (when crosscompiling)
#	cd test && $(MAKE) $(MFLAGS) clean
#	cd doc && $(MAKE) $(MFLAGS) clean

local-distclean:
	rm -f Makefile *.orig *.rej */*.orig */*.rej awk.output \
		gmon.out make.out config.h config.status config.cache \
		config.log stamp-h *~

distclean: clean local-distclean
	cd doc && $(MMAKE) distclean
	cd awklib && $(MMAKE) distclean
	cd test && $(MMAKE) distclean

maintainer-clean: clean local-distclean
	@echo "This command is intended for maintainers to use; it"
	@echo "deletes files that may require special tools to rebuild."
	rm -f awktab.c TAGS tags
	cd doc && $(MMAKE) maintainer-clean
#	cd test && $(MMAKE) maintainer-clean
#	cd awklib && $(MMAKE) maintainer-clean

clobber:	maintainer-clean

TAGS:
	etags $(AWKSRC)

tags:
	ctags $(AWKSRC)

dvi:	$(srcdir)/doc/gawk.texi
	cd doc && $(MMAKE) dvi

info:	$(srcdir)/doc/gawk.texi
	cd doc && $(MMAKE) info

doc/all:
	cd doc && $(MMAKE) all

awklib/all:
	cd awklib && $(MMAKE) all

# to run this target you have to adjust test/Makefile quite a bit
# in order to make it palatable to your shell
#
check:	$(GAWK)
	cd test; $(MMAKE) -k

test:	check

