/*
 * $QNXtpLicenseC:
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





/* $Source$
 *
 * $Revision: 153052 $
 *
 * create.c - Create a tape archive. 
 *
 * DESCRIPTION
 *
 *	These functions are used to create/write and archive from an set of
 *	named files.
 *
 * AUTHOR
 *
 *     	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log$
 * Revision 1.10  2005/06/03 01:37:53  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.9  2003/09/24 19:51:53  thomasf
 * Updates to make the build work with the mingw platform.
 *
 * Revision 1.8  2003/08/27 18:16:57  martin
 * Add QSSL Copyright to cover QNX contributions.
 *
 * Revision 1.7  2003/06/27 17:56:13  hsbrown
 * Fixed a problem with pax adding extra characters to path names.
 * PR#15557
 *
 * Revision 1.6  2000/12/06 19:49:15  kewarken
 * fixed obscure bug with long pathnames to tar
 *
 * Revision 1.5  1999/08/05 18:47:24  adrianj
 * Correct printf call for correct types.
 *
 * Revision 1.4  1998/12/03 18:57:44  bstecher
 * tweaked for win32
 *
 * Revision 1.3  1995/01/03 19:12:03  garry
 * Added -Wa option to reset access times.
 *
 * Revision 1.1  1994/12/22  16:06:35  garry
 * Initial revision
 *
 * Revision 1.2  1994/01/17  19:33:37  garry
 * if linkname is too long to fit in 'linkname' field,
 * don't store it.
 *
 * Revision 1.3  89/02/12  10:29:37  mark
 * Fixed misspelling of Replstr
 * 
 * Revision 1.2  89/02/12  10:04:17  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:06  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: create.c 153052 2008-08-13 01:17:50Z coreos $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"
#if defined(__NT__) || defined(__MINGW32__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
extern short f_restore_access;


/* Function Prototypes */

#ifdef __STDC__

static void writetar(char *, Stat *);
static void writecpio(char *, Stat *);
static char tartype(int);

#else /* !__STDC__ */

static void writetar();
static void writecpio();
static char tartype();

#endif /* __STDC__ */


/* create_archive - create a tar archive.
 *
 * DESCRIPTION
 *
 *	Create_archive is used as an entry point to both create and append
 *	archives.  Create archive goes through the files specified by the
 *	user and writes each one to the archive if it can.  Create_archive
 *	knows how to write both cpio and tar headers and the padding which
 *	is needed for each type of archive.
 *
 * RETURNS
 *
 *	Always returns 0
 */

#ifdef __STDC__

int create_archive(void)

#else

int create_archive()

#endif
{
    char            name[PATH_MAX + 1];
    Stat            sb;
    int             fd;

    while (memset(name,0,PATH_MAX+1) && name_next(name, &sb) != -1) {
	if ((fd = openin(name, &sb)) < 0) {
	    /* FIXME: pax wants to exit here??? */
	    continue;
	}

	if (rplhead != (Replstr *)NULL) {
	    rpl_name(name);
	    if (strlen(name) == 0) {
		continue;
	    }
	}
	if (get_disposition("add", name) || get_newname(name, sizeof(name))) {
	    /* skip file... */
	    if (fd) {
		close(fd);
	    }
	    continue;
	} 

	if (!f_link && sb.sb_nlink > 1) {
	    if (islink(name, &sb)) {
		sb.sb_size = 0;
	    }
	    linkto(name, &sb);
	}

	if (ar_format == TAR) {
	    writetar(name, &sb);
	} else {
	    writecpio(name, &sb);
	}
	if (fd) {
	    outdata(fd, name, sb.sb_size);
	    if (f_restore_access) {
	        struct utimbuf times;
	        times.actime = sb.sb_atime;
	        times.modtime = sb.sb_mtime;
	        if (utime(name,&times) != 0) {
	            warn(name,"Unable to restore access time");    
	            // continue processing, however pax
	            // should return error code on exit
	        }
	    }
	}
	if (f_verbose) {
	    print_entry(name, &sb);
	}
    }

    write_eot();
    close_archive();
    return (0);
}


/* writetar - write a header block for a tar file
 *
 * DESCRIPTION
 *
 * 	Make a header block for the file name whose stat info is in st.  
 *	Return header pointer for success, NULL if the name is too long.
 *
 * 	The tar header block is structured as follows:
 *
 *		FIELD NAME	OFFSET		SIZE
 *      	-------------|---------------|------
 *		name		  0		100
 *		mode		100		  8
 *		uid		108		  8
 *		gid		116		  8
 *		size		124		 12
 *		mtime		136		 12
 *		chksum		148		  8
 *		typeflag	156		  1
 *		linkname	157		100
 *		magic		257		  6
 *		version		263		  2
 *		uname		265		 32
 *		gname		297		 32
 *		devmajor	329		  8
 *		devminor	337		  8
 *		prefix		345		155
 *
 * PARAMETERS
 *
 *	char	*name	- name of file to create a header block for
 *	Stat	*asb	- pointer to the stat structure for the named file
 *
 */

#ifdef __STDC__

static void writetar(char *name, Stat *asb)

#else
    
static void writetar(name, asb)
char           *name;
Stat           *asb;

#endif
{
    char	   *p;
    char           *prefix = (char *)NULL;
	char	   *prefix_end = (char *)NULL;  // added to leave *name intact
    int             i;
    int             sum;
    char            hdr[BLOCKSIZE];
    Link           *from;

    memset(hdr, 0, BLOCKSIZE);
    if (strlen(name) > 255) {
	warn(name, "name too long");
	return;
    }

    /* 
     * If the pathname is longer than TNAMLEN, but less than 255, then
     * we can split it up into the prefix and the filename.
     */
    if (strlen(name) > 100) {
	prefix = name;
	name += 155;
	while (name > prefix && *name != '/') {
	    name--;
	}
	/* no slash found....hmmm.... */
	if (name == prefix) {
	    warn(prefix, "Name too long");
	    return;
	}
	prefix_end = name++;
	*prefix_end = '\0';	// will restore to / later
    }

#ifdef S_IFLNK
    if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	if (strlen(asb->sb_link) > 100) {
		if (prefix!=NULL) {
			*prefix_end='/';
			name=prefix;
		}
		warn(name, "linkname too long");
		return;
    	}
	strcpy(&hdr[157], asb->sb_link);
	asb->sb_size = 0;
    }
#endif
    strcpy(hdr, name);
    sprintf(&hdr[100], "%06o \0", asb->sb_mode & ~S_IFMT);
    sprintf(&hdr[108], "%06o \0", asb->sb_uid);
    sprintf(&hdr[116], "%06o \0", asb->sb_gid);
    sprintf(&hdr[124], "%011lo ", (long) asb->sb_size);
    sprintf(&hdr[136], "%011lo ", (long) asb->sb_mtime);
    strncpy(&hdr[148], "        ", 8);
    hdr[156] = tartype(asb->sb_mode);
    if (asb->sb_nlink > 1 && (from = linkfrom(name, asb)) != (Link *)NULL) {
	if (strlen(from->l_name) > 100) {
		if (prefix!=NULL) {
			*prefix_end='/';
			name=prefix;
		}
		warn(name, "linkname too long");
		return;
    	}
	strcpy(&hdr[157], from->l_name);
	hdr[156] = LNKTYPE;
    }
    strcpy(&hdr[257], TMAGIC);
    strncpy(&hdr[263], TVERSION, 2);
    strcpy(&hdr[265], finduname((int) asb->sb_uid));
    strcpy(&hdr[297], findgname((int) asb->sb_gid));
    sprintf(&hdr[329], "%06o \0", major(asb->sb_rdev));
    sprintf(&hdr[337], "%06o \0", minor(asb->sb_rdev));
    if (prefix != (char *)NULL) {
	strncpy(&hdr[345], prefix, 155);
    }

    /* Calculate the checksum */

    sum = 0;
    p = hdr;
    for (i = 0; i < 500; i++) {
	sum += 0xFF & *p++;
    }

    /* Fill in the checksum field. */

    sprintf(&hdr[148], "%06o \0", sum);

    outwrite(hdr, BLOCKSIZE);

	if (prefix!=NULL) {
		*prefix_end='/';
		name=prefix;
	}
}


/* tartype - return tar file type from file mode
 *
 * DESCRIPTION
 *
 *	tartype returns the character which represents the type of file
 *	indicated by "mode". 
 *
 * PARAMETERS
 *
 *	int	mode	- file mode from a stat block
 *
 * RETURNS
 *
 *	The character which represents the particular file type in the 
 *	ustar standard headers.
 */

#ifdef __STDC__

static char tartype(int mode)

#else
    
static char tartype(mode)
int	    mode;

#endif
{
    switch (mode & S_IFMT) {

#ifdef S_IFCTG
    case S_IFCTG:
	return(CONTTYPE);
#endif

    case S_IFDIR:
	return (DIRTYPE);

#ifdef S_IFLNK
    case S_IFLNK:
	return (SYMTYPE);
#endif

#ifdef S_IFIFO
    case S_IFIFO:
	return (FIFOTYPE);
#endif

#ifdef S_IFCHR
    case S_IFCHR:
	return (CHRTYPE);
#endif

#ifdef S_IFBLK
    case S_IFBLK:
	return (BLKTYPE);
#endif

    default:
	return (REGTYPE);
    }
}


/* writecpio - write a cpio archive header
 *
 * DESCRIPTION
 *
 *	Writes a new CPIO style archive header for the file specified.
 *
 * PARAMETERS
 *
 *	char	*name	- name of file to create a header block for
 *	Stat	*asb	- pointer to the stat structure for the named file
 */

#ifdef __STDC__

static void writecpio(char *name, Stat *asb)

#else
    
static void writecpio(name, asb)
char           *name;
Stat           *asb;

#endif
{
    uint            namelen;
    char            header[M_STRLEN + H_STRLEN + 1];

    namelen = (uint) strlen(name) + 1;
    strcpy(header, M_ASCII);
    sprintf(header + M_STRLEN, "%06o%06o%06o%06o%06o",
	    USH(asb->sb_dev), USH(asb->sb_ino), USH(asb->sb_mode), 
	    USH(asb->sb_uid), USH(asb->sb_gid));
    sprintf(header + M_STRLEN + 30, "%06o%06o%011lo%06o%011lo",
	    USH(asb->sb_nlink), USH(asb->sb_rdev),
	    (long unsigned)(f_mtime ? asb->sb_mtime : time((time_t *) 0)),
	    namelen, (long unsigned)asb->sb_size);
    outwrite(header, M_STRLEN + H_STRLEN);
    outwrite(name, namelen);
#ifdef	S_IFLNK
    if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	outwrite(asb->sb_link, (uint) asb->sb_size);
    }
#endif	/* S_IFLNK */
}
