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
    getdef.c -- setup defaults and retrieve by types.



    $Log$
    Revision 1.8  2006/05/29 15:59:16  rmansfield
    PR: 29454
    CI: kewarken

    Fix off by one error.

    Revision 1.7  2006/05/10 12:13:39  rmansfield

    PR: 29454
    CI: kewarken

    Buffer overflow fix in su and passwd.

    Revision 1.6  2005/06/03 01:22:46  adanko

    Replace existing QNX copyright licence headers with macros as specified by
    the QNX Coding Standard. This is a change to source files in the head branch
    only.

    Note: only comments were changed.

    PR25328

    Revision 1.5  2003/10/15 21:22:40  jgarvey
    Tidy up lib/login.  There were two almost-identical header files,
    "h/login.h" (used during build because it came earlier in -I list)
    and "public/login.h" (which was the one installed for clients to
    compile against).  This duplication could led to coherence/
    consistency problems.  So take all duplication out of "h/login.h",
    and what is left is very private/internal use only, put that in
    a new "h/liblogin.h" which can be included by the (3) source files
    in this library that use it.  Now builds clean QNX4 and QNX6.

    Revision 1.4  2001/09/05 18:51:42  thomasf
    Added in a conditional compile for NTO to up the high end limit if
    no limit is defined for the ranges.  This addresses PR 8419 where
    passwd wasn't making a big enough default group entry if only the
    starting GID was in the /etc/default/passwd.

    Revision 1.3  1998/10/14 21:20:04  eric
    Modified for nto, now uses some support/util includes.

    Revision 1.2  1998/09/26 16:23:00  eric
    *** empty log message ***

    Revision 1.1  1997/02/18 16:50:06  kirk
    Initial revision

 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <util/stdutil.h>

#include "liblogin.h"
#include "login.h"



static char *deflist[][2] = {
	{	"BASEDIR",	"/usr"	},
	{	"SHELL",	"/bin/sh" },
	{	"UIDRANGE",	"100-30000" },
	{	"GIDRANGE",	"1-30000" },
	{	"PROFILE",	".profile" },
	{	"DEFPROFILE",	"/etc/default/profile" },
	{       "TZ",           "EST5EDT4" },
	{       "HZ",           "1000" },
	{       "BAUD",         ""     },
        {       "SULOG",        "/usr/adm/sulog" }
/*	{	"NOPASSWORDOK",	"" }, */
/*      {       "INSISTANT",    "32000" }    */
};

#define MAX_DEFAULT (sizeof deflist/sizeof *deflist)

static default_t *defp;

int
init_defaults(char *prognam, char *alt)
{
	char	buf[UTIL_PATH_MAX + 1];
	int	i;
	strncpy(buf,DEFDIR, UTIL_PATH_MAX);
	strncat(buf,prognam, UTIL_PATH_MAX - strlen(buf));
	buf[UTIL_PATH_MAX] = '\0';
	if (access(buf,R_OK) == -1) {
		strncpy(buf,DEFDIR, UTIL_PATH_MAX);
		strncat(buf,alt, UTIL_PATH_MAX - strlen(buf));
		buf[UTIL_PATH_MAX] = '\0';
		if (access(buf,R_OK) == -1) {
			strcpy(buf,"/dev/null"); /* none */
		}
	} 
	if (!(defp=def_open(buf,0)))
		return -1;
	for (i=0; i < MAX_DEFAULT; i++) {
		if (def_find(defp,deflist[i][0]) == 0) {
			def_install(defp,deflist[i][0],deflist[i][1]);
		}
	}
	return 0;
}

char *
getdef_str(char *str)
{
	char	*p;
	if (!defp) return "";
	return (p=def_find(defp,str)) ? p : "";
}

int
getdef_bool(char *str)
{
	if (!defp) return 0;
	return def_find(defp,str) != 0;
}

long
getdef_int(char *str)
{
	return strtol(getdef_str(str),0,10);
}

int
getdef_range(char *str, long *low, long *high)
{
	char	*p;
	if (!defp) return -1;
	if (!(p = getdef_str(str)) || !*p)
		return -1;
	*low = strtol(p,&p,10);
	if (*p == '-') {
		if (p[1]) {
			*high = strtol(p+1,&p, 10);
		} else {
#if defined(__QNXNTO__)
			*high = LONG_MAX;
#else
			*high = SHRT_MAX;
#endif
		}
		return 2;
	}
	return 1;
}
