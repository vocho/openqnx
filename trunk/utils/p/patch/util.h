/* utility functions for `patch' */

/* $Id: util.h,v 1.20 2003/05/20 13:56:48 eggert Exp $ */

/* Copyright (C) 1986 Larry Wall

   Copyright (C) 1992, 1993, 1997, 1998, 1999, 2001, 2002, 2003 Free
   Software Foundation, Inc.

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

/* An upper bound on the print length of a signed decimal line number.
   Add one for the sign.  */
#define LINENUM_LENGTH_BOUND (sizeof (LINENUM) * CHAR_BIT / 3 + 1)

XTERN enum backup_type backup_type;

bool ok_to_reverse (char const *, ...) __attribute__ ((format (printf, 1, 2)));
void ask (char const *, ...) __attribute__ ((format (printf, 1, 2)));
void say (char const *, ...) __attribute__ ((format (printf, 1, 2)));

void fatal (char const *, ...)
	__attribute__ ((noreturn, format (printf, 1, 2)));
void pfatal (char const *, ...)
	__attribute__ ((noreturn, format (printf, 1, 2)));

char *fetchname (char *, int, time_t *);
char *savebuf (char const *, size_t);
char *savestr (char const *);
char const *version_controller (char const *, bool, struct stat const *, char **, char **);
bool version_get (char const *, char const *, bool, bool, char const *, struct stat *);
int create_file (char const *, int, mode_t);
int systemic (char const *);
char *format_linenum (char[LINENUM_LENGTH_BOUND + 1], LINENUM);
void Fseek (FILE *, file_offset, int);
void copy_file (char const *, char const *, int, mode_t);
void exit_with_signal (int) __attribute__ ((noreturn));
void ignore_signals (void);
void init_time (void);
void memory_fatal (void) __attribute__ ((noreturn));
void move_file (char const *, int volatile *, char *, mode_t, bool);
void read_fatal (void) __attribute__ ((noreturn));
void remove_prefix (char *, size_t);
void removedirs (char *);
void set_signals (bool);
void write_fatal (void) __attribute__ ((noreturn));
