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
 *	diskman.h - Initial revision for PQNX 4.0
 *
 *	includes for the diskman set of utilities:
 *
 *		diskman
 *		chkfsys
 *		dcheck
 *		dinit
 *		fdformat
 *		fdisk
 *		zapf
 */

#ifndef __QNXNTO__
#include <sys/disk.h>
#include <sys/qioctl.h>
#include <sys/fd.h>
#include <sys/osinfo.h>
#else
#include <sys/stat.h>
#include <sys/dcmd_blk.h>
#endif

#ifndef PAUSEphrase
#define PAUSEphrase		"Please type the <Enter> key to continue..."
#endif
#define DISK_RAW		1
#define DISK_PARTN		0
#define DISK_EITHER		(-1)

#ifndef TRUE
#define TRUE			   1
#define FALSE			   0
#endif

#define MAXBAD			4000

#ifndef strequal
#define strequal(a,b)   (!strcmp(a,b))
#endif

#ifndef strnequal
#define strnequal(a,b,c) (!strncmp(a,b,c))
#endif

#ifdef __QNXNTO__
/* readblock/writeblock are 0-based, QNX4 block_* are 1-based */                
#define block_read(fd,bl,nbl,buf) readblock(fd,512,bl-1,nbl,buf)                
#define block_write(fd,bl,nbl,buf) writeblock(fd,512,bl-1,nbl,buf)
#endif

#ifdef __cplusplus
extern "C" {
#endif
void	 usageDM( int, char **, char **, char *, int, ... );

int		 check_disk( int, char *, char *, long );
char	*get_disk_name( char *, char * );
char	*get_rawdisk( char *, char * );
char	*get_partn( char *, char * );

#ifdef __cplusplus
};
#endif
