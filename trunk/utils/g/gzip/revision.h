/* revision.h -- define the version number

   Copyright (C) 1998, 1999, 2001, 2002, 2006 Free Software Foundation, Inc.
   Copyright (C) 1992-1993 Jean-loup Gailly.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#define PATCHLEVEL 0
#define REVDATE "2002-09-30"

/* This version does not support compression into old compress format: */
#ifdef LZW
#  undef LZW
#endif

/* $Id: revision.h 171047 2008-06-20 13:42:23Z rmansfield $ */
