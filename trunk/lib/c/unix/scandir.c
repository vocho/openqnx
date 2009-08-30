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




#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>

int alphasort(const void* a, const void* b)
{
    return strcmp((*(struct dirent **)a)->d_name, (*(struct dirent **)b)->d_name);
}

int scandir(char *dirname, struct dirent ***namelist,
    int (*select)(const struct dirent *), int (*compar)(const void *, const void *))
{
    DIR *dirp;
    struct dirent *de;
    struct dirent *dp;
    int nentries = 0;
    int nalloc = 0;

    if ((dirp = opendir(dirname)) == NULL) {
	return -1;
    }
    *namelist = NULL;
    while ((de = readdir(dirp)) != NULL) {
	if (select && (*select) (de) == 0)
	    continue;
	if (nalloc == nentries) {
	    nalloc += 32;		/* grab descent sized chunks */
	    *namelist = realloc(*namelist, sizeof dp * nalloc);
	    if (*namelist == NULL) {
		errno = ENOMEM;
		return -1;
	    }
	}
	if ((dp = malloc(de->d_reclen)) == NULL) {
	    errno = ENOMEM;
	    return -1;
	}
	memcpy(dp, de, de->d_reclen);
	(*namelist)[nentries++] = dp;
    }
    closedir(dirp);
    if ((*namelist = realloc(*namelist, nentries * sizeof dp)) == NULL) {
	errno = ENOMEM;
	return -1;
    }

	if (compar)
		qsort(*namelist, nentries, sizeof *namelist, compar);

    return nentries;
}

__SRCVERSION("scandir.c $Rev: 163784 $");
