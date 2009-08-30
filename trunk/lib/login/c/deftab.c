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



/*- getdef.c -- handle defaults database.
 *
 *  This modules provides a simple symbol-table like mechanism for
 *  defaults processing.
 *  Files with lines of the form(s):
 *  ^#.*$ == ignored (comment)
 *  [^=]*=.* == symbol entry {\1,\2}.
 *  [^=]*    == symbol entry {\1,0}.




 * $Log$
 * Revision 1.7  2005/06/03 01:22:46  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.6  2003/10/11 23:57:16  jgarvey
 * Guess the v1.5 "fix" for PR/9478 wasn't tested: a close() of a FILE*
 * would have been an EBADF with the file descriptor left open ...
 * ... actually looking at the compiler warning/output is a good habit.
 *
 * Revision 1.5  2001/11/23 13:22:12  thomasf
 * Fixes to address PR 9478 where we weren't properly closing fds on error.
 *
 * Revision 1.4  1999/11/11 16:59:07  thomasf
 * Removed all that strdup debug message stuff
 *
 * Revision 1.3  1999/05/20 12:10:21  thomasf
 * Added modifications to _not_ include the code that is already present
 * in the NTO libc, tested locally on my configuration.
 *
 * Revision 1.2  1999/04/02 20:15:18  bstecher
 * Clean up some warnings
 *
 * Revision 1.1  1997/02/18 16:50:06  kirk
 * Initial revision
 *
 */


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "login.h" 

struct syment {
	char	*key;
	char	*val;
	struct syment *next;
};

static int
def_hash(char *key, int nsym)
{
	short unsigned tot=0;
	while (*key) {
		tot += (key[1] << 8) + *key;
		key++;
	}
	return tot%nsym;
}

char *
def_find(default_t *dp, char *key)
{
	int	hval = def_hash(key, dp->nsymtab);
	struct	syment *sp;
	for (sp=&dp->symtab[hval]; sp && sp->key && strcmp(sp->key,key);
	     sp = sp->next);
	return sp ? sp->val : 0;
}

char *
def_install(default_t *dp, char *key, char *val)
{
	int	hval = def_hash(key,dp->nsymtab);
	char	*xp;

	if (xp=def_find(dp, key)) {
		return 0;
	}
	if (dp->symtab[hval].key) {
		struct syment *sp = malloc(sizeof *sp);
		if (!sp) {
			return 0;
		}
		/*
		printf("1 STRDUP key %s length %d \n", 
			(key) ? key : "!NULL!", (key) ? strlen(key) : -1);
		fflush(stdout);
		*/
		if (!(sp->key = strdup(key))) {
			free(sp);
			return 0;
		}
		/*
		printf("1 STRDUP val %s length %d \n", 
			(val) ? val : "!NULL!", (val) ? strlen(val) : -1);
		fflush(stdout);
		*/
		if (!(sp->val = strdup(val))) {
			free(sp->key); free(sp);
			return 0;
		}
		sp->next = dp->symtab[hval].next;
		dp->symtab[hval].next = sp;
		return sp->val;
	}

	/*
	printf("2 STRDUP key %s length %d \n", 
		(key) ? key : "!NULL!", (key) ? strlen(key) : -1);
	fflush(stdout);
	*/
	if (!(dp->symtab[hval].key = strdup(key))) {
		return 0;
	}
	
	/*
	printf("2 STRDUP val %s length %d \n", 
		(val) ? val : "!NULL!", (val) ? strlen(val) : -1);
	fflush(stdout);
	*/
	if (!(dp->symtab[hval].val = strdup(val))) {
		free(dp->symtab[hval].key);
		dp->symtab[hval].key = 0;
		return 0;
	}
	return dp->symtab[hval].val;
}

default_t *
def_open(char *fname, int nentries)
{
	struct	defobj	*dp;
	FILE	*fp;
	char	lbuf[256];
	if (nentries == 0) {
		nentries = 211;
	}
	if ((fp=fopen(fname,"r")) == 0) {
		return 0;
	}
	if (dp=malloc(sizeof *dp)) {
		if (dp->symtab = calloc(sizeof *dp->symtab, nentries)) {
			dp->nsymtab = nentries;
		} else {
			fclose(fp);
			free(dp);
			errno = ENOMEM;
			return 0;
		}
	} else {
		fclose(fp);
		return 0;
	}
	while (fgets(lbuf,sizeof lbuf, fp)) {
		char	*p;
		if (lbuf[0] == '#' || lbuf[0] == '\n')
			continue;
		if (p=strchr(lbuf,'\n'))
			*p='\0';

		if (!(p=strtok(lbuf,"=")))
			p = lbuf+strlen(lbuf);
		else
			p = lbuf+strlen(lbuf)+1;
		def_install(dp, lbuf, p);
	}
	fclose(fp);
	return dp;
}

int
def_close(default_t *dp)
{
	int	i;
	for (i=0; i < dp->nsymtab; i++) {
		struct	syment *sp;
		if (sp=dp->symtab[i].next) {
			struct syment *osp = sp;
			free(osp->key);
			if (osp->val) free(osp->val);
			sp = osp->next;
			free(osp);
		}
		free(dp->symtab[i].key);
		if (dp->symtab[i].val) free(dp->symtab[i].val);
	}
	free(dp->symtab);
	free(dp);
	return 0;
}
