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
#  include <lib/compat.h>
#endif
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

/*
This function will return the stat information for the named file.
dent will be used (if possible) to extract stat
information gleaned through the readdir optimization present in
the version of the OS the program is being run under. Otherwise,
if path is non-NULL it will use that path and make a regular lstat()
call.
*/

int lstat_optimize(struct dirent *entry, struct stat *statp)
{
#if defined(__QNXNTO__)
    struct dirent_extra_stat        *extra;
	
    for(extra = _DEXTRA_FIRST(entry);_DEXTRA_VALID(extra, entry);
	    extra = _DEXTRA_NEXT(extra))
	{
		switch(extra->d_type) {
		    case _DTYPE_LSTAT:
				memcpy(statp,&(extra->d_stat),sizeof(struct stat));
				return 0;
		        break;
	    }
	}
	return -1;
#elif defined(__QNX__)
	if (entry->d_stat.st_status & _FILE_USED) {
    	memcpy(statp,&(entry->d_stat),sizeof (struct stat));
		return 0;
	}
#endif
	return -1;
}

int stat_optimize(struct dirent *entry, struct stat *statp)
{
#ifdef __QNXNTO__
    struct dirent_extra_stat        *extra;
	
    for(extra = _DEXTRA_FIRST(entry);_DEXTRA_VALID(extra, entry);
	    extra = _DEXTRA_NEXT(extra))
	{
		switch(extra->d_type) {
		    case _DTYPE_STAT:
				memcpy(statp,&(extra->d_stat),sizeof(struct stat));
				return 0;
		        break;
	    }
	}
#endif
	
	/* if we didn't have actual stat info, but the file is not a
       link, the stat info will be the same as the lstat info */

	if (0==lstat_optimize(entry,statp)) {
		if (!S_ISLNK(statp->st_mode)) return 0;
	}

	return -1;
}

