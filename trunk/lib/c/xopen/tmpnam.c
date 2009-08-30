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




#include <unix.h>
#include <unistd.h>
#include <sys/stat.h>
#include <paths.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>

static char* _mktmpnam(char *start, const char* path, const char* prefix)
{
	static char  seed[]="AAAXXXXXX";
	struct stat sbuf;
	const char *tmp;
	char *tmp2;
	size_t dlen, plen;

	/* determine directory */
	/* search order is: path, TMPDIR, P_tmpdir, _PATH_TMP, . */
	if (path == NULL || stat(tmp = path, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode)) {
		if ((tmp = getenv("TMPDIR")) == NULL || stat(tmp, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode))
			if (stat(tmp = P_tmpdir, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode))
				if (stat(tmp = _PATH_TMP, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode))
					tmp = "./";
	}

	/* determine lengths of string */
	dlen = strlen(tmp);
	plen = (prefix) ? min(strlen(prefix), 5) : 0;

	/* allocate space for path */
	if ((start == NULL) &&
		((start = malloc(dlen + plen + sizeof(seed) + 8))) == NULL) {
		errno = ENOMEM;
		return NULL;
	} else if ((dlen + plen) > ((L_tmpnam - sizeof(seed)) - 4)) {
		/* not enough space to create unique name */
		goto failed;
	}

	/* add directory path */
	strcpy(start, tmp);
	if (start[dlen-1] != '/') {
		strcpy(&start[dlen], "/");
		dlen++;
	}

	/* add prefix, if necessary */
	if (prefix) {
		memcpy(&start[dlen], prefix, plen);
	}

	/* add trailer */
	memcpy(&start[dlen+plen], seed, sizeof(seed));

	/* increment seed */
	tmp2 = seed;
	while ((tmp2 <= &seed[2]) && (*tmp2 == 'Z')) *tmp2++ = 'A';
	++*tmp2;

	/* call core routine */
	if (mktemp(start))	return start;

failed:
	/* failed, but we must return something */
	strcpy(start, _PATH_TMP"/000000");
	return start;
}

char* tmpnam(char* s)
{
	static char buffer[L_tmpnam];

	return _mktmpnam((s) ? s : buffer, NULL, NULL);
}

char* tempnam(const char* dir, const char* pfx)
{
	return _mktmpnam(NULL, dir, pfx);
}


__SRCVERSION("tmpnam.c $Rev: 153052 $");
