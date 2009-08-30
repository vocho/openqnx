/* vms_misc.c -- sustitute code for missing/different run-time library routines.

   Copyright (C) 1991-1993, 1996, 1997 the Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#define creat creat_dummy	/* one of gcc-vms's headers has bad prototype */
#include "awk.h"
#undef creat
#include <fab.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif
#include <rmsdef.h>
#include <ssdef.h>
#include <stsdef.h>

    /*
     * VMS uses a completely different status scheme (odd => success,
     * even => failure), so we'll trap calls to exit() and alter the
     * exit status code.  [VAXC can't handle this as a macro.]
     */
#ifdef exit
# undef exit
#endif
void
vms_exit( int final_status )
{
    exit(final_status == 0 ? SS$_NORMAL : (SS$_ABORT | STS$M_INHIB_MSG));
}
#define exit(v) vms_exit(v)

    /*
     * In VMS's VAXCRTL, strerror() takes an optional second argument.
     *  #define strerror(errnum) strerror(errnum,vaxc$errno)
     * is all that's needed, but VAXC can't handle that (gcc can).
     * [The 2nd arg is used iff errnum == EVMSERR.]
     */
#ifdef strerror
# undef strerror
#endif
extern char *strerror P((int,...));

/* vms_strerror() -- convert numeric error code into text string */
char *
vms_strerror( int errnum )
{
    return ( errnum != EVMSERR ? strerror(errnum)
			       : strerror(EVMSERR, vaxc$errno) );
}
# define strerror(v) vms_strerror(v)

    /*
     * Miscellaneous utility routine, not part of the run-time library.
     */
/* vms_strdup() - allocate some new memory and copy a string into it */
char *
vms_strdup( const char *str )
{
    char *result;
    int len = strlen(str);

    emalloc(result, char *, len+1, "strdup");
    return strcpy(result, str);
}

    /*
     * VAXCRTL does not contain unlink().  This replacement has limited
     * functionality which is sufficient for GAWK's needs.  It works as
     * desired even when we have the file open.
     */
/* unlink(file) -- delete a file (ignore soft links) */
int
unlink( const char *file_spec ) {
    char tmp[255+1];			/*(should use alloca(len+2+1)) */
    extern int delete(const char *);

    strcpy(tmp, file_spec);		/* copy file name */
    if (strchr(tmp, ';') == NULL)
	strcat(tmp, ";0");		/* append version number */
    return delete(tmp);
}

    /*
     * Work-around an open(O_CREAT+O_TRUNC) bug (screwed up modification
     * and creation dates when new version is created), and also use some
     * VMS-specific file options.  Note:  optional 'prot' arg is completely
     * ignored; gawk doesn't need it.
     */
#ifdef open
# undef open
#endif
extern int creat P((const char *,int,...));
extern int open  P((const char *,int,unsigned,...));

/* vms_open() - open a file, possibly creating it */
int
vms_open( const char *name, int mode, ... )
{
    int result;

    if (mode == (O_WRONLY|O_CREAT|O_TRUNC)) {
	/* explicitly force stream_lf record format to override DECC$SHR's
	   defaulting of RFM to earlier file version's when one is present */
	result = creat(name, 0, "rfm=stmlf", "shr=nil", "mbc=32");
    } else {
	struct stat stb;
	const char *mbc, *shr = "shr=get", *ctx = "ctx=stm";

	if (stat((char *)name, &stb) < 0) {	/* assume DECnet */
	    mbc = "mbc=8";
	} else {    /* ordinary file; allow full sharing iff record format */
	    mbc = "mbc=32";
	    if ((stb.st_fab_rfm & 0x0F) < FAB$C_STM) shr = "shr=get,put,upd";
	}
	result = open(name, mode, 0, shr, mbc, "mbf=2");
    }

    /* This is only approximate; the ACP -> RMS -> VAXCRTL interface
       discards too much potentially useful status information...  */
    if (result < 0 && errno == EVMSERR
		   && (vaxc$errno == RMS$_ACC || vaxc$errno == RMS$_CRE))
	errno = EMFILE;	/* redirect() should close 1 file & try again */

    return result;
}

    /*
     * Check for attempt to (re-)open known file.
     */
/* vms_devopen() - check for "SYS$INPUT" or "SYS$OUTPUT" or "SYS$ERROR" */
int
vms_devopen( const char *name, int mode )
{
    FILE *file = NULL;

    if (STREQ(name, "/dev/null"))
	return open("NL:", mode, 0);	/* "/dev/null" => "NL:" */
    else if (STREQ(name, "/dev/tty"))
	return open("TT:", mode, 0);	/* "/dev/tty" => "TT:" */
    else if (strncasecmp(name, "SYS$", 4) == 0) {
	name += 4;		/* skip "SYS$" */
	if (strncasecmp(name, "INPUT", 5) == 0 && (mode & O_WRONLY) == 0)
	    file = stdin,  name += 5;
	else if (strncasecmp(name, "OUTPUT", 6) == 0 && (mode & O_WRONLY) != 0)
	    file = stdout,  name += 6;
	else if (strncasecmp(name, "ERROR", 5) == 0 && (mode & O_WRONLY) != 0)
	    file = stderr,  name += 5;
	if (*name == ':')  name++;	/* treat trailing colon as optional */
    }
    /* note: VAXCRTL stdio has extra level of indirection (*file) */
    return (file && *file && *name == '\0') ? fileno(file) : -1;
}


#ifndef VMS_V7
    /*
     * VMS prior to V7.x has no timezone support unless DECnet/OSI is used.
     */
/* these are global for use by missing/strftime.c */
char   *tzname[2] = { "local", "" };
int     daylight = 0, timezone = 0, altzone = 0;

/* tzset() -- dummy to satisfy linker */
void tzset(void)
{
    return;
}
#endif	/*VMS_V7*/


/* getpgrp() -- there's no such thing as process group under VMS;
 *		job tree might be close enough to be useful though.
 */
int getpgrp(void)
{
    return 0;
}

#ifndef __GNUC__
void vms_bcopy( const char *src, char *dst, int len )
{
    (void) memcpy(dst, src, len);
}
#endif /*!__GNUC__*/

/*----------------------------------------------------------------------*/
#ifdef NO_VMS_ARGS      /* real code is in "vms/vms_args.c" */
void vms_arg_fixup( int *argc, char ***argv ) { return; }	/* dummy */
#endif

#ifdef NO_VMS_PIPES     /* real code is in "vms/vms_popen.c" */
FILE *popen( const char *command, const char *mode ) {
    fatal(" Cannot open pipe `%s' (not implemented)", command);
    return NULL;
}
int pclose( FILE *current ) {
    fatal(" Cannot close pipe #%d (not implemented)", fileno(current));
    return -1;
}
int fork( void ) {
    fatal(" Cannot fork process (not implemented)");
    return -1;
}
#endif /*NO_VMS_PIPES*/
