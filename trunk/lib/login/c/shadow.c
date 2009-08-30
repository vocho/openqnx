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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "login.h"

#define	FIELDS	5


int parse_sp(char *buf, struct spwd *sp)
{
char	*fields[FIELDS];
char	*cp;
char	*cpp;
int	i;

	if (cp = strrchr (buf, '\n'))
		*cp = '\0';

	for (cp = buf, i = 0;*cp && i < FIELDS;i++) {
		fields[i] = cp;
		while (*cp && *cp != ':')
			cp++;

		if (*cp)
			*cp++ = '\0';
	}
	if (*cp || i != FIELDS)
		return 0;

	sp->sp_namp = fields[0];
	sp->sp_pwdp = fields[1];

	if ((sp->sp_lstchg = strtol (fields[2], &cpp, 10)) == 0 && *cpp)
		return 0;

	if ((sp->sp_min = strtol (fields[3], &cpp, 10)) == 0 && *cpp)
		return 0;

	if ((sp->sp_max = strtol (fields[4], &cpp, 10)) == 0 && *cpp)
		return 0;
	return 1;
}

#if !defined(__QNXNTO__)
static	char	buf[BUFSIZ];
static	struct	spwd	spwd;

struct spwd	*getspnam(char *name)
{
FILE	*f;
int	found = 0;

	if ((f=fopen(SHADOW,"r")) == NULL)
		return NULL;
	while (fgets(buf,sizeof(buf)-1,f)) {
		if (parse_sp(buf,&spwd)) {
			if ((found=!strcmp(spwd.sp_namp,name)))
				break;
		}
	}
	fclose(f);
	return found ? &spwd : NULL;
}
#endif
