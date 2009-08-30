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



/*-
 * routines for login to adjust the environment for a new user.



 * $Log$
 * Revision 1.5  2005/06/03 01:22:46  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.4  2001/11/23 13:22:12  thomasf
 * Fixes to address PR 9478 where we weren't properly closing fds on error.
 *
 * Revision 1.3  1999/04/02 20:15:18  bstecher
 * Clean up some warnings
 *
 * Revision 1.2  1998/10/14 21:20:04  eric
 * Modified for nto, now uses some support/util includes.
 *
 * Revision 1.1  1997/02/18 16:50:06  kirk
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#ifndef __QNXNTO__
#include <env.h>
#endif
#include <ctype.h>

#include "login.h"

char          **
addstr(char **env, char *key, char *s, int nenv)
{
	char           *lbuf;

	if (nenv == 0) {
		if ((env = calloc(sizeof(char *), 2)) == NULL) {
			return NULL;
		}
	} else {
		if ((env = realloc(env, sizeof(char *) * (nenv + 2))) == NULL) {
			return NULL;
		}
	}
	lbuf = alloca(strlen(key) + strlen(s) + 2);
	strcpy(lbuf, key);
	strcat(lbuf, "=");
	strcat(lbuf, s);
	env[nenv++] = strdup(lbuf);
	env[nenv++] = NULL;
	return env;
}

int
strclean(char *s)
{
	int             i;
	if ((i = strlen(s)) == 0)
		return 0;
	while (--i >= 0) {
		if (!isspace(s[i]))
			return 1;
		s[i] = '\0';
	}
	return 0;
}

int
build_env(char *envsave, int preserve)
{
	FILE           *f;
	char            bufp[256];
	char          **envp = NULL;
	int             save = 0;
	char           *p;
	int             i;

	if ((f = fopen(envsave, "r")) == NULL) {
		if (!preserve)
			clearenv();
		return 0;
	}
	/* extract env-vars we want to keep */
	bufp[sizeof bufp - 1] = '\0';
	while (fgets(bufp, sizeof bufp, f) != NULL) {
		char           *s;
		strclean(bufp);
		if ((s = strchr(bufp, '=')) != NULL) {
			*s++ = '\0';
		}
		if ((p = getenv(bufp)) == NULL) {
			if ((p = s) == NULL)
				continue;
		}
		if (!preserve) {
			if ((envp = addstr(envp, bufp, p, save++)) == NULL) {
				fclose(f);
				return 0;
			}
		} else {
			char           *x;
			if (x = malloc(strlen(bufp) + strlen(p) + 2)) {
				sprintf(x, "%s=%s", bufp, p);
				putenv(x);
				save++;
			}
		}
	}
	fclose(f);
	if (preserve)
		return save;
	/* now, clear out the environment space */
	clearenv();
	/* and add back the things we want.... */
	for (i = 0; i < save; i++) {
		putenv(envp[i]);
	}
	free(envp);
	return save;
}
