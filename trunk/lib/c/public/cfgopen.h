/*
 * $QNXLicenseC:
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
 *  cfgopen.h    Configuration file open routine
 *

 */

#ifndef _CFGOPEN_H_INCLUDE
#define _CFGOPEN_H_INCLUDE

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif

#ifndef _FCNTL_H_INCLUDED
#include <fcntl.h>
#endif

/* open() style options (same bits as open()) */
#define CFGFILE_RDONLY   0x00000000		/* Iterate through and open read only */
#define CFGFILE_WRONLY   0x00000001		/* Iterate through and open write only */
#define CFGFILE_RDWR     0x00000002		/* Open the file as read/write */
#define CFGFILE_CREAT    0x00000100		/* We want to create the file */
#define CFGFILE_TRUNC    0x00000200		/* We want to truncate the file */
#define CFGFILE_EXCL     0x00000400		/* Exclusive open */
#define CFGFILE_APPEND   0x00000008		/* Append open */

#define CFGFILE_NOFD	 0x00800000		/* Fill the buffer, but don't get an fd (returns 0) */

/* path location options */
#define CFGFILE_USER_NODE   0x01000000		/* $HOME + .cfg + confstr(CS_NODENAME) + path */
#define CFGFILE_USER        0x02000000		/* $HOME + .cfg + path */
#define CFGFILE_NODE        0x04000000		/* /nodecfg + confstr(CS_NODENAME) + path */
#define CFGFILE_GLOBAL      0x08000000		/* path */

#define CFGFILE_MASK        0xff000000


__BEGIN_DECLS

/*
 path       = Path to the file that we want to open  
 flags      = Open + Location flags
 historical = File to open as a last resort (optional)
 namebuf    = Buffer to save the pathname in (optional)
 nblen      = Length of saved pathname buffer (optional)
*/
int cfgopen(const char *path, unsigned flags, 
			const char *historical, char *namebuf, int nblen);

FILE *fcfgopen(const char *path, const char *mode, int location, 
			   const char *historical, char *namebuf, int nblen);

__END_DECLS

#endif

/* __SRCVERSION("cfgopen.h $Rev: 153052 $"); */
