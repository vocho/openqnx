/* reading patches */

/* $Id: pch.h,v 1.11 2003/05/20 13:56:03 eggert Exp $ */

/* Copyright (C) 1986, 1987, 1988 Larry Wall

   Copyright (C) 1990, 1991, 1992, 1993, 1997, 1998, 1999, 2000, 2001,
   2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

LINENUM pch_end (void);
LINENUM pch_first (void);
LINENUM pch_hunk_beg (void);
LINENUM pch_newfirst (void);
LINENUM pch_prefix_context (void);
LINENUM pch_ptrn_lines (void);
LINENUM pch_repl_lines (void);
LINENUM pch_suffix_context (void);
bool pch_swap (void);
bool pch_write_line (LINENUM, FILE *);
bool there_is_another_patch (void);
char *pfetch (LINENUM);
char pch_char (LINENUM);
int another_hunk (enum diff, bool);
int pch_says_nonexistent (bool);
size_t pch_line_len (LINENUM);
time_t pch_timestamp (bool);
void do_ed_script (FILE *);
void open_patch_file (char const *);
void re_patch (void);
void set_hunkmax (void);
