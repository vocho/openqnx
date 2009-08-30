/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/*
 *  Header declaration(s) which are forced to go after earlier includes
 */

#ifndef __tannsk_h   /* prevent multiple inclusions */
#define __tannsk_h

/* ztimbuf is declared in zip\tailor.h after include of tandem.h */
int utime (const char *, const ztimbuf *);

#endif /* !__tannsk_h */
