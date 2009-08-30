/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
#include <stdio.h>
#include <stdarg.h>

/* for Info and similar macroes. fprintf is already a macro and fprintf x
 * fools the preprocessor
 */

int _fprintf(FILE* fp, const char* fmt, ...)
{
    va_list ap;
    long n;

    va_start(ap, fmt);
    n = vfprintf(fp, fmt, (long*) ap);
    va_end(ap);
    return n;
}

