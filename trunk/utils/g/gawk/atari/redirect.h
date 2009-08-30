/*
 * redirect.h --- definitions for functions that are OS specific.
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991-1993 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/* This file is already conditioned on atarist in awk.h */

#define read _text_read /* we do not want all these CR's to mess our input */
extern int _text_read(int, char *, int);
#ifndef __MINT__
#undef NGROUPS_MAX
#endif /* __MINT__ */
