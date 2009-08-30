# Makefile for GNU tar sources.

# Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2003, 2006
# Free Software Foundation, Inc.

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

bin_PROGRAMS = tar

noinst_HEADERS = arith.h common.h tar.h
tar_SOURCES = \
 buffer.c\
 compare.c\
 create.c\
 delete.c\
 extract.c\
 xheader.c\
 incremen.c\
 list.c\
 misc.c\
 names.c\
 sparse.c\
 system.c\
 tar.c\
 transform.c\
 update.c\
 utf8.c

INCLUDES = -I$(top_srcdir)/lib -I../ -I../lib

LDADD = ../lib/libtar.a $(LIBINTL) $(LIBICONV)

tar_LDADD = $(LDADD) $(LIB_CLOCK_GETTIME)
