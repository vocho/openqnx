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




#define _FILE_OFFSET_BITS	64
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

#define OPTS			"r" "w" "x" "f" "b" "c" "d" "p" "u" "g" "k" "s"
#define READABLE		0x00000001	/* r */
#define WRITEABLE		0x00000002	/* w */
#define EXECUTABLE		0x00000004	/* x */
#define FILE			0x00000008	/* f */
#define BLOCKSPEC		0x00000010	/* b */
#define CHARSPEC		0x00000020	/* c */
#define DIR				0x00000040	/* d */
#define FIFO			0x00000080	/* p */
#define SUID			0x00000100	/* u */
#define SGID			0x00000200	/* g */
#define STICKY			0x00000400	/* k */
#define HAS_SIZE		0x00000800	/* s */

static int check(const char *path, unsigned flags) {
	struct stat				buff;

	if(flags & (READABLE | WRITEABLE | EXECUTABLE)) {
		int						amode = 0;

		if(flags & READABLE) {
			amode |= R_OK;
		}
		if(flags & WRITEABLE) {
			amode |= W_OK;
		}
		if(flags & EXECUTABLE) {
			amode |= X_OK;
		}
		if(access(path, amode) == -1) {
			return 0;
		}
		if(!(flags & ~(READABLE | WRITEABLE))) {
			return 1;
		}
	}
	if(stat(path, &buff) == -1) {
		return 0;
	}
	if((flags & FILE) && !S_ISREG(buff.st_mode)) {
		return 0;
	}
	if((flags & BLOCKSPEC) && !S_ISBLK(buff.st_mode)) {
		return 0;
	}
	if((flags & CHARSPEC) && !S_ISCHR(buff.st_mode)) {
		return 0;
	}
	if((flags & DIR) && !S_ISDIR(buff.st_mode)) {
		return 0;
	}
	if((flags & (EXECUTABLE | DIR)) == EXECUTABLE && S_ISDIR(buff.st_mode)) {
		return 0;
	}
	if((flags & FIFO) && !S_ISFIFO(buff.st_mode)) {
		return 0;
	}
	if((flags & SUID) && !(buff.st_mode & S_ISUID)) {
		return 0;
	}
	if((flags & SGID) && !(buff.st_mode & S_ISGID)) {
		return 0;
	}
	if((flags & STICKY) && !(buff.st_mode & S_ISVTX)) {
		return 0;
	}
	if((flags & HAS_SIZE) && buff.st_size == 0) {
		return 0;
	}
	return 1;
}
	
char *pathfind_r(const char *path, const char *name, const char *mode, char *buff, size_t buff_size) {
	unsigned					flags;
	char						*p;
	int							n;

	flags = 0;
	--mode;
	while(*++mode) {
		unsigned					bit;
		const char					*ptr;
		static const char FIXCONST	opts[] = OPTS;

		for(ptr = opts, bit = 1; *ptr; ptr++, bit <<= 1) {
			if(*mode == *ptr) {
				flags |= bit;
				break;
			}
		}
	}

	n = strlen(name);
	if(*name == '/') {
		return (n < buff_size && check(name, flags)) ? strcpy(buff, name) : 0;
	}

	buff_size -= n;
	do {
		for(n = buff_size, *(p = buff) = 0; path && *path && *path != ':'; n--, path++) {
			if(n > 0) {
				*p++ = *path;
			}
		}
		if(n > 0) {
			if(*buff && p[-1] != '/') {
				*p++ = '/';
			}
			strcpy(p, name);
			if(check(buff, flags)) {
				return buff;
			}
		}
		if(path && *path == ':') {
			path++;
		}
	} while(path && *path);

	*buff = 0;
	return 0;
}

char *pathfind(const char *path, const char *name, const char *mode) {
	static char 				buff[PATH_MAX + 1];

	return pathfind_r(path, name, mode, buff, sizeof buff);
}

__SRCVERSION("pathfind.c $Rev: 153052 $");
