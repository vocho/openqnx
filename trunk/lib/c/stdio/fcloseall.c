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




#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <share.h>
#include <malloc.h>
#include <errno.h>
#include <string.h>
#include "file.h"
#include "xstdio.h"

int fcloseall(void) {
	FILE							*fp;
	int								again;

	_Locksyslock(_LOCK_STREAM);
	for(again = 1; again;) {
		again = 0;
		for(fp = _Files[0]; fp; fp = fp->_NextFile) {
			if(fp->_Mode & (_MOPENR | _MOPENW)) {
				_Unlocksyslock(_LOCK_STREAM);
				fclose(fp);
				again = 1;
				_Locksyslock(_LOCK_STREAM);
				break;
			}
		}
	}
	_Unlocksyslock(_LOCK_STREAM);
	return 0;
}

__SRCVERSION("fcloseall.c $Rev: 153052 $");
