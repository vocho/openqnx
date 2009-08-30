/*
 * $QNXtpLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




/*
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *%    Copyright (C) 1992, by WATCOM Systems Inc. All rights     %
 *%    reserved. No part of this software may be reproduced     %
 *%    in any form or by any means - graphic, electronic or     %
 *%    mechanical, including photocopying, recording, taping     %
 *%    or information storage and retrieval systems - except     %
 *%    with the written permission of WATCOM Systems Inc.       %
 *%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  Note:
     This file is used by the C++ iostream library.
     It supports the distinction between the various operating systems
     for the open() mode bits.
     This allows the iostream library to remain platform independent.

  Mapping between C++ openmode and C openmode:    

  C++ openmode           C openmode
  ------------           ----------
  in                O_RDONLY
  in | bin               O_RDONLY | O_BINARY
  out | trunc            O_WRONLY | O_CREAT | O_TRUNC
  out | app              O_WRONLY | O_CREAT | O_APPEND
  out | trunc | bin      O_WRONLY | O_CREAT | O_BINARY
  out | app | bin        O_WRONLY | O_CREAT | O_APPEND | O_BINARY
  in | out               O_RDWR | O_CREAT
  in | out | trunc       O_RDWR | O_CREAT | O_TRUNC
  in | out | app         O_RDWR | O_CREAT | O_APPEND
  in | out | bin         O_RDWR | O_CREAT | O_BINARY
  in | out | trunc       O_RDWR | O_CREAT | O_TRUNC
  in | out | app | bin        O_RDWR | O_CREAT | O_APPEND | O_BINARY

  nocreate               -> remove O_CREAT

  Modified:    By:       Reason:
  ---------    ---       -------
  02-sep-92    Greg Bentz     Initial implementation
  22-mar-93    Greg Bentz     Make update ios_mode parm to reflect absence
                    of O_TEXT significance on some systems.
  15-oct-93    Raymond Tang   Modify the condition to check if "noreplace"
                    is applicable.
*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// these must be the same as is defined in iostream.h

#define __in          0x0001    // - open for input
#define __out         0x0002    // - open for output
#define __atend       0x0004    // - seek to end after opening
#define __append      0x0008    // - open for output, append to the end
#define __truncate    0x0010    // - discard contents after opening
#define __nocreate    0x0020    // - open only an existing file
#define __noreplace   0x0040    // - open only a new file
#define __text        0x0080    // - open as text file
#define __binary      0x0100    // - open as binary file

int __plusplus_open( const char *name, int *pios_mode, int prot )
{
    int ios_mode = *pios_mode;
    int mode;

    // Figure out the POSIX "open" function parameters based on ios_mode:
    if( (ios_mode & (__in | __out)) == (__in | __out) ) {
     mode = O_RDWR|O_CREAT;
    } else if( ios_mode & __in ) {
     mode = O_RDONLY;
    } else if( ios_mode & __out ) {
     mode = O_WRONLY|O_CREAT;
    } else {
     return( -1 );
    }
    if( ios_mode & __append ) {
     mode |= O_APPEND;
    }
    if( ios_mode & __truncate ) {
     mode |= O_TRUNC;
    }
    if( ios_mode & __nocreate ) {
     mode &= ~O_CREAT;
    }
    #if defined(__QNX__) || defined(__PENPOINT__)
     *pios_mode &= ~(__binary|__text);
    #else
     if( ios_mode & __binary ) {
         mode |= O_BINARY;
     } else {
         mode |= O_TEXT;
         *pios_mode |= __text;
     }
    #endif

    // If "noreplace" is specified and O_CREAT is set,
    // then an error will occur if the file already exists:
    if( (ios_mode&__noreplace) && (mode|O_CREAT) ) {
     struct stat buf;
     
     if( stat( name, &buf) != -1) {
         return( -1 );
     }
    }
    return( open( name, mode, prot ) );
}

__SRCVERSION("__plusplus_open.c $Rev: 153052 $");
