/* GNU Mailutils -- a suite of utilities for electronic mail
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA  */

#ifndef _ARGCV_H
#define _ARGCV_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __P
# if defined PROTOTYPES || (defined __STDC__ && __STDC__)
#  define __P(args) args
# else
#  define __P(args) ()
# endif
#endif /*__P */

extern int argcv_get    __P ((const char *command, const char *delim,
			      const char* cmnt,
			      int *argc, char ***argv));
extern int argcv_string __P ((int argc, char **argv, char **string));
extern int argcv_free   __P ((int argc, char **argv));
extern int argcv_unescape_char __P((int c));
extern int argcv_escape_char __P((int c));

#ifdef __cplusplus
}
#endif

#endif /* _ARGCV_H */
