# Make gzip/lib				-*-Makefile-*-

# Copyright (C) 2006 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


include gnulib.mk

libgzip_a_LIBADD += $(LIBOBJS)
libgzip_a_DEPENDENCIES += $(LIBOBJS)

match.$(OBJEXT): match.c
	cp $(srcdir)/match.c _match.S
	$(CPP) $(CPPFLAGS) $(ASCPPFLAGS) _match.S > _match.i
	@ASCPPPOST@ < _match.i > match_.s
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ASFLAGS_config) -c match_.s
	mv match_.$(OBJEXT) $@
	rm -f _match.S _match.i match_.s

MOSTLYCLEANFILES += _match.S _match.i match_.s
