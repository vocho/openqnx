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
 *  sys/dir.h
 *

 */
#ifndef _DIR_H_INCLUDED
#define _DIR_H_INCLUDED

#include <dirent.h>

/* This header is for compat only.  New code should use dirent.h directly */

/*
 * backwards compat from when scandir() and
 * alphsort() operated on struct direct as
 * defined below rather than struct dirent.
	struct direct {
		unsigned long d_fileno;
		unsigned short d_reclen;
		unsigned short d_namlen;
		char d_name[1];
	};
 */
#define direct		dirent	/* map struct direct to struct dirent	*/
#define d_fileno	d_ino 	/* map (struct direct *)->d_fileno to	*/
				/* (struct dirent *)->d_ino		*/

#endif

/* __SRCVERSION("dir.h $Rev: 163784 $"); */
