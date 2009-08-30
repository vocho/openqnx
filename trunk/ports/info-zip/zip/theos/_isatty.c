/*
  Copyright (c) 1990-1999 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 1999-Oct-05 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.cdrom.com/pub/infozip/license.html
*/
/* replace standard library function who needs a FILE* */

#pragma library

#include <stdio.h>
#include <fcntl.h>
#include <sc.h>
#include <lub.h>

short _isatty(int fd)
{
    register short lub;

    lub = (int) _fcntl(&stdin[fd], 5, (size_t) 0);
    return (lub >= CONIN && lub <= CONOUT)
        || (lub >= COM1 && lub <= COM4)
        || (lub >= COM5 && lub <= COM16);
}
