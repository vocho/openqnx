/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
#ifndef __tanzip_h   /* prevent multiple inclusions */
#define __tanzip_h

  int       zopen  (const char *, int);
  int       zclose (int);
  unsigned  zread (int, char *, unsigned);
  void      nskformatopt(char **);

#endif /* !__tanzip_h */
