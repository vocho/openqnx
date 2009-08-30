# Makefile for GNU tar documentation.

# Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2003, 2006 Free
# Software Foundation, Inc.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

info_TEXINFOS = tar.texi
tar_TEXINFOS = \
 dumpdir.texi\
 fdl.texi\
 freemanuals.texi\
 genfile.texi\
 getdate.texi\
 header.texi\
 intern.texi\
 rendition.texi\
 snapshot.texi\
 sparse.texi\
 value.texi
EXTRA_DIST = gendocs_template mastermenu.el texify.sed
DISTCLEANFILES=*.info* 

# The rendering level is anyone of PUBLISH, DISTRIB or PROOF.
# Just call `make RENDITION=PROOF [target]' if you want PROOF rendition.
RENDITION = DISTRIB

MAKEINFOFLAGS=-D$(RENDITION)

header.texi: $(top_srcdir)/src/tar.h
	sed -f $(srcdir)/texify.sed $(top_srcdir)/src/tar.h \
	  | expand >$@

master-menu: $(tar_TEXINFOS)
	emacs -batch -l mastermenu.el -f make-master-menu $(info_TEXINFOS)

check-options:
	@ARGP_HELP_FMT='usage-indent=0,short-opt-col=0,long-opt-col=0,\
doc-opt-col=0,opt-doc-col=0,header-col=0,rmargin=1' \
	$(top_builddir)/src/tar --usage | \
	sed -n 's/^\[--\([^]\=\[]*\).*/\1/p' | sort | uniq > opts.$$$$;\
	$(MAKEINFO) $(AM_MAKEINFOFLAGS) $(MAKEINFOFLAGS) -I $(srcdir) -E - \
            $(info_TEXINFOS) | \
	sed -n '/^@macro/,/^@end macro/d;s/@opindex *\([^@,]*\).*/\1/p' \
	     | sort | uniq > docs.$$$$;\
	(echo 'Not documented options:';\
	 join -v1 opts.$$$$ docs.$$$$;\
	 echo 'Non-existing options:';\
	 join -v2 opts.$$$$ docs.$$$$) > report.$$$$;\
	 rm opts.$$$$ docs.$$$$;\
	 if [ -n "`sed '1,2d' report.$$$$`" ]; then \
	   cat report.$$$$;\
	   rm report.$$$$;\
	   exit 1;\
	 fi;\
	 rm report.$$$$


clean-local:
	rm -rf manual

GENDOCS=gendocs.sh

TEXI2DVI=texi2dvi -t '@set $(RENDITION)' -E

# Make sure you set TEXINPUTS
# Usual value is:
# /usr/share/texmf/pdftex/plain/misc:/usr/share/texmf/pdftex/config
manual:
	TEXINPUTS=$(srcdir):$(top_srcdir)/build-tex:$(TEXINPUTS) \
	 MAKEINFO="$(MAKEINFO) $(MAKEINFOFLAGS)" \
	 TEXI2DVI="$(TEXI2DVI) -t @finalout" \
	$(GENDOCS) tar 'GNU tar manual'

