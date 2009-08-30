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




#ifdef __MINGW32__

#include <windows.h>
#include <stdio.h>


//
// This should fix PR# 47519 and related problems.
//
// The win32 version of tmpfile creates files
// in the root of the current drive. This is bad since the user might
// not have permission to write there. We will override this function with one that uses Win32 API
//
// I am aware that this is not a perfect solution, but I think it will cover
// the cases well enough. Problems with this overriding implementation are:
//
// 1) We assume all our mingw32 utilities link with -lcompat.
//
FILE *tmpfile() {
	char fname[MAX_PATH];
	if (!tmpnam(fname)) 
		return NULL;
	return fopen(fname, "w+bTD");
}


#endif
