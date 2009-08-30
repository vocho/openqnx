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



/*
 * Accept a password and verify it.
 * Attempt to annihilate the raw password as quickly as possible --
 * in case anyone is looking.
 */

#include <stdio.h>
#include <string.h>
#ifdef __MINGW32__
#include <lib/compat.h>
#else
#include <pwd.h>
#endif

#include "passwd.h"


int
password(char *prompt,struct passwd *pw)
{
char	pwstr[MAX_PWLEN+1];
struct	spwd *sp = NULL;
char *p;
	if (pw && (pw->pw_passwd == NULL || pw->pw_passwd[0] == '\0')) {
    	   return 1;
    	}
	if (pw && !(sp = getspnam(pw->pw_name))) {
	   return 0;
	}
	if (!(p=getpass(prompt))) {
	   return 0;
	}
	if (!pw || !sp->sp_pwdp) {
	/* do a phony crypt to waste time */
		strcpy(pwstr,crypt(pwstr,"xx"));
		return 0;    /* fail */
	} 
	strcpy(pwstr,crypt(p,sp->sp_pwdp));
	return !strcmp(pwstr,sp->sp_pwdp);
}
