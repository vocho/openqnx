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
 * qnx_term_load() finds the size of the device on stderr.
 * It also sets up the graphics chars for qnx_term_fputs().
 *
 * qnx_term_fputs() outputs all chars between 0x20 and 0x7F.
 * It will map line drawing chars to the current console.
 * All characters it can't map will be displayed as "?"
 */
#ifndef _STDIO_H_INCLUDED
	#include <stdio.h>
#endif

#define _QNX_T_LINES	0x0001		/* load the line_drawing chars */

#ifdef __cplusplus
extern "C" {
#endif
int qnx_term_load(int flags, int *cols, int *rows);
int qnx_term_fputs(char *buf, FILE *fp);

#ifdef __cplusplus
};
#endif
