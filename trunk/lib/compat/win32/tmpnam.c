/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#if defined(__MINGW32__)
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int win32_tmpnam_g = 1; // set to 0 to disable file existence checking

static int tmpnam_GetCanonicalPath(char *tmppath, unsigned int N, const char *tp);

//
// The win32 version of tmpnam (which is called by the mingw32 lib) creates
// names in the root of the current drive. This is bad since the user might
// not have permission to write there, especially if it happens to be a
// CD rom drive!! We will override this function with one that uses the much
// better tempnam() function instead.
//
// I am aware that this is not a perfect solution, but I think it will cover
// the cases well enough. Problems with this overriding implementation are:
//
// 1) We assume all our mingw32 utilities link with -lcompat.
// 2) We don't handle the (less common) case where a non-null argument is 
//    passed in to tmpnam().
// 3) In the NULL argument case, we may return names longer than MAX_PATH, but
//    I think this will rarely cause a problem.
//
// Creates temp file name of the following structure:

char *tmpnam(char *buffer) {
	static char tmppath[MAX_PATH-17] = {'\0'}; // -17 for the filename
	static char tmpfname[MAX_PATH]; // temporary file name; if buffer is null, this buffer will be returned
	DWORD res;
	static unsigned int cnt = 0;
	int retries = 1000;
	struct _stat st;

	if (!cnt)
		cnt = GetTickCount() & 0x0000000F;
	// cnt - counter, to avoid name collision when the same process asks for tmp name very fast;
	// it is initialized to GetTickCount to avoid collisions when a pid is reused.
	// Collisions can not be guaranteed to be avoided but this will minimize the
	// chances. See bellow for further collision avoidance

	if (!tmppath[0]) {
		res = GetTempPathA(sizeof(tmppath), tmppath);
		if (!res || res > sizeof(tmppath)) {
			fprintf(stderr, "Temporary path too long or invalid.\n"
							"Please check your TMP environment variable "
							"(path should not contain more than %d characters including separators)\n", 
					sizeof(tmppath)/sizeof(tmppath[0])-2);
			return NULL; // not enough space for tmppath
		}
	}

tryname:
	cnt++;
	snprintf(tmpfname, MAX_PATH, "%sQ%X%08x.tmp", 
					tmppath,
					getpid() & 0x00000FFF, 
					(GetTickCount() + cnt));
	if (win32_tmpnam_g) {
		if ((!_stat(tmpfname, &st) || (errno != ENOENT))) {
			if (retries--)
				goto tryname; // try again.
			else {
				fprintf(stderr, "Error: unable to create temporary file name.\n"
								"Please check your TMP environment variable and perform "
								"temporary directory cleanup if neccessary.\n");
				return NULL; // fail. Couldn't find the non-existing name.
			}
		}
	}

	if (buffer) {
		strcpy(buffer, tmpfname);
		return buffer;
	}

	return tmpfname;
}

#endif
