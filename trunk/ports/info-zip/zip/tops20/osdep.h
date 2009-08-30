/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
#ifndef TOPS20
#define TOPS20
#endif

#define NO_PROTO
#define NO_SYMLINK
#define NO_TERMIO
#define DIRENT
#define BIG_MEM
#define REALLY_SHORT_SYMS
#define window_size winsiz

extern int isatty();

#define FOPR "rb"
#define FOPM "r+b"
#define FOPW "w8"

#define CBSZ 524288
#define ZBSZ 524288

#include <sys/types.h>
#include <sys/stat.h>
