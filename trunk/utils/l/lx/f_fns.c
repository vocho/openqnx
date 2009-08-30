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





#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/disk.h>
#include <malloc.h>
#include <errno.h>
#include <strings.h>
#include <util/diskman.h>

extern char *Prog;

void f_fsys_stat(const char *path, struct _fsys_stat *buf) 
{
	if (-1==qnx4fs_fsys_stat(path, buf)) {
		fprintf(stderr,"%s: qnx4fs_fsys_stat of %s failed (%s)\n",
				Prog, path, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void f_lstat(const char *path, struct stat *buf)
{
	if (-1==lstat(path, buf)) {
		fprintf(stderr,"%s: lstat of %s failed (%s)\n",
				Prog, path, strerror(errno));
		exit(EXIT_FAILURE);
	}
}


void f_block_read(int fildes, long block, unsigned nblock, void *buf)
{
	if (nblock!=block_read(fildes,block,nblock,buf)) {
		fprintf(stderr,"%s: block_read() failed (%s)\n",
				Prog, strerror(errno));
		exit(EXIT_FAILURE);
	}
}


void *f_calloc(size_t n, size_t size)
{
	void *p;

	if (NULL==(p=calloc(n, size))) {
		fprintf(stderr,"%s: out of memory (heap) (%s)\n",
				Prog, strerror(errno));
		exit(EXIT_FAILURE);
	}

	return p;
}
