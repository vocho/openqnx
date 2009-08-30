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
 * extract.c - Extract files from a tar archive. 
 *
 * DESCRIPTION
 *
 * AUTHOR
 *
 *	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
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
 * Revision 1.6  2005/06/03 01:37:53  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.5  2003/08/27 18:16:57  martin
 * Add QSSL Copyright to cover QNX contributions.
 *
 * Revision 1.4  2001/05/30 15:05:55  mikhailk
 * Long file names (>100) processing added to provide compatibility with GNU tar.
 *
 * Revision 1.3  2001/05/25 15:17:57  kewarken
 * test fix for end of file problem
 *
 * Revision 1.2  1999/08/05 18:48:51  adrianj
 * Remove dependance on struct stat types in scanf call to read
 * ascii header for cpio format.
 *
 * Revision 1.1  1998/12/03 18:54:43  eric
 * Initial revision
 *
 * Revision 1.3  89/02/12  10:29:43  mark
 * Fixed misspelling of Replstr
 * 
 * Revision 1.2  89/02/12  10:04:24  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:07  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: extract.c 153052 2008-08-13 01:17:50Z coreos $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Defines */

/*
 * Swap bytes. 
 */
#define	SWAB(n)	((((ushort)(n) >> 8) & 0xff) | (((ushort)(n) << 8) & 0xff00))


/* Function Prototypes */

#ifdef __STDC__

static int inbinary(char *, char *, Stat *);
static int inascii(char *, char *, Stat *);
static int inswab(char *, char *, Stat *);
static int readtar(char *, Stat *);
static int readcpio(char *, Stat *);

#else /* !__STDC__ */

static int inbinary();
static int inascii();
static int inswab();
static int readtar();
static int readcpio();

#endif /* __STDC__ */


/* read_archive - read in an archive
 *
 * DESCRIPTION
 *
 *	Read_archive is the central entry point for reading archives.
 *	Read_archive determines the proper archive functions to call
 *	based upon the archive type being processed.
 *
 * RETURNS
 *
 */

#ifdef __STDC__

void read_archive(void)

#else
    
int read_archive()

#endif
{
    Stat            sb;
/*
 * BLOCKSIZE replaced by PATH_MAX + 1 to provide compatibility
 * with GNU tar.
 */
    char            name[PATH_MAX + 1];
    int             match;
    int		    	pad;

    name_gather();		/* get names from command line */
    name[0] = '\0';
    while (get_header(name, &sb) == 0) {
	match = name_match(name) ^ f_reverse_match;
	if (f_list) {		/* only wanted a table of contents */
	    if (match) {
		print_entry(name, &sb);
	    }
	    if (((ar_format == TAR) 
		? buf_skip(ROUNDUP((OFFSET) sb.sb_size, BLOCKSIZE)) 
		: buf_skip((OFFSET) sb.sb_size)) < 0) {
		warn(name, "File data is corrupt");
	    }
	} else if (match) {
	    if (rplhead != (Replstr *)NULL) {
		rpl_name(name);
		if (strlen(name) == 0) {
		    continue;
		}
	    }
	    if (get_disposition("extract", name) || 
                get_newname(name, sizeof(name))) {
		/* skip file... */
		if (((ar_format == TAR) 
		    ? buf_skip(ROUNDUP((OFFSET) sb.sb_size, BLOCKSIZE)) 
		    : buf_skip((OFFSET) sb.sb_size)) < 0) {
		    warn(name, "File data is corrupt");
		}
		continue;
	    } 
	    if (inentry(name, &sb) < 0) {
		warn(name, "File data is corrupt");
	    }
	    if (f_verbose) {
		print_entry(name, &sb);
	    }
	    if (ar_format == TAR && sb.sb_nlink > 1) {
		/*
		 * This kludge makes sure that the link table is cleared
		 * before attempting to process any other links.
		 */
		if (sb.sb_nlink > 1) {
		    linkfrom(name, &sb);
		}
	    }
	    if (ar_format == TAR && (pad = sb.sb_size % BLOCKSIZE) != 0) {
		pad = BLOCKSIZE - pad;
		buf_skip((OFFSET) pad);
	    }
	} else {
	    if (((ar_format == TAR) ? buf_skip(ROUNDUP((OFFSET) sb.sb_size, BLOCKSIZE)) 
		: buf_skip((OFFSET) sb.sb_size)) < 0) {
		warn(name, "File data is corrupt");
	    }
	}
    }

    close_archive();
}



/* get_header - figures which type of header needs to be read.
 *
 * DESCRIPTION
 *
 *	This is merely a single entry point for the two types of archive
 *	headers which are supported.  The correct header is selected
 *	depending on the archive type.
 *
 * PARAMETERS
 *
 *	char	*name	- name of the file (passed to header routine)
 *	Stat	*asb	- Stat block for the file (passed to header routine)
 *
 * RETURNS
 *
 *	Returns the value which was returned by the proper header
 *	function.
 */

#ifdef __STDC__

int get_header(char *name, Stat *asb)

#else
    
int get_header(name, asb)
char *name;
Stat *asb;

#endif
{
    if (ar_format == TAR) {
	return(readtar(name, asb));
    } else {
	return(readcpio(name, asb));
    }
}


/* readtar - read a tar header
 *
 * DESCRIPTION
 *
 *	Tar_head read a tar format header from the archive.  The name
 *	and asb parameters are modified as appropriate for the file listed
 *	in the header.   Name is assumed to be a pointer to an array of
 *	at least PATH_MAX bytes.
 *
 * PARAMETERS
 *
 *	char	*name 	- name of the file for which the header is
 *			  for.  This is modified and passed back to
 *			  the caller.
 *	Stat	*asb	- Stat block for the file for which the header
 *			  is for.  The fields of the stat structure are
 *			  extracted from the archive header.  This is
 *			  also passed back to the caller.
 *
 * RETURNS
 *
 *	Returns 0 if a valid header was found, or -1 if EOF is
 *	encountered.
 */

#ifdef __STDC__

static int readtar(char *name, Stat *asb)

#else
    
static int readtar(name, asb)
char	*name;
Stat    *asb;

#endif
{
    int             status = 3;	/* Initial status at start of archive */
    static int      prev_status;

    for (;;) {
	prev_status = status;
	status = read_header(name, asb);
	switch (status) {
		case 1:		/* Valid header */
			return(0);
		case 0:		/* Invalid header */
		    switch (prev_status) {
		  	  case 3:		/* Error on first record */
				warn(ar_file, "This doesn't look like a tar archive");
				/* FALLTHRU */
		  	  case 2:		/* Error after record of zeroes */
		  	  case 1:		/* Error after header rec */
				warn(ar_file, "Skipping to next file...");
				/* FALLTHRU */
		  	  default:
		  	  case 0:		/* Error after error */
				break;
		    }
	    	break;
		case 2:			/* Record of zeroes */
		    if (f_append)
			lseek(archivefd, -BLOCKSIZE, SEEK_CUR);
	       //     else	
		//	read_header(name,asb);
		case EOF:		/* End of archive */
		default:
		    return(-1);
	}
    }
}


/* readcpio - read a CPIO header 
 *
 * DESCRIPTION
 *
 *	Read in a cpio header.  Understands how to determine and read ASCII, 
 *	binary and byte-swapped binary headers.  Quietly translates 
 *	old-fashioned binary cpio headers (and arranges to skip the possible 
 *	alignment byte). Returns zero if successful, -1 upon archive trailer. 
 *
 * PARAMETERS
 *
 *	char	*name 	- name of the file for which the header is
 *			  for.  This is modified and passed back to
 *			  the caller.
 *	Stat	*asb	- Stat block for the file for which the header
 *			  is for.  The fields of the stat structure are
 *			  extracted from the archive header.  This is
 *			  also passed back to the caller.
 *
 * RETURNS
 *
 *	Returns 0 if a valid header was found, or -1 if EOF is
 *	encountered.
 */

#ifdef __STDC__

static int readcpio(char *name, Stat *asb)

#else
    
static int readcpio(name, asb)
char           *name;
Stat           *asb;

#endif
{
    OFFSET          skipped;
    char            magic[M_STRLEN];
    static int      align;

    if (align > 0) {
	buf_skip((OFFSET) align);
    }
    align = 0;
    for (;;) {
	buf_read(magic, M_STRLEN);
	skipped = 0;
	while ((align = inascii(magic, name, asb)) < 0
	       && (align = inbinary(magic, name, asb)) < 0
	       && (align = inswab(magic, name, asb)) < 0) {
	    if (++skipped == 1) {
		if (total - sizeof(magic) == 0) {
		    fatal("Unrecognizable archive");
		}
		warnarch("Bad magic number", (OFFSET) sizeof(magic));
		if (name[0]) {
		    warn(name, "May be corrupt");
		}
	    }
	    memcpy(magic, magic + 1, sizeof(magic) - 1);
	    buf_read(magic + sizeof(magic) - 1, 1);
	}
	if (skipped) {
	    warnarch("Apparently resynchronized", (OFFSET) sizeof(magic));
	    warn(name, "Continuing");
	}
	if (strcmp(name, TRAILER) == 0) {
	    return (-1);
	}
	if (nameopt(name) >= 0) {
	    break;
	}
	buf_skip((OFFSET) asb->sb_size + align);
    }
#ifdef	S_IFLNK
    if ((asb->sb_mode & S_IFMT) == S_IFLNK) {
	if (buf_read(asb->sb_link, (uint) asb->sb_size) < 0) {
	    warn(name, "Corrupt symbolic link");
	    return (readcpio(name, asb));
	}
	asb->sb_link[asb->sb_size] = '\0';
	asb->sb_size = 0;
    }
#endif				/* S_IFLNK */

    /* destroy absolute pathnames for security reasons */
    if (name[0] == '/') {
	int index = 1;

	if (name[index] == '/') {
	    while (name[++index] != '/');
	    if (name[index]) ++index;
	}
	if (name[index]) {
	    while (name[0] = name[index]) {
		++name;
	    }
	} else {
	    name[0] = '.';
	}
    }
    asb->sb_atime = asb->sb_ctime = asb->sb_mtime;
    if (asb->sb_nlink > 1) {
	linkto(name, asb);
    }
    return (0);
}


/* inswab - read a reversed by order binary header
 *
 * DESCRIPTIONS
 *
 *	Reads a byte-swapped CPIO binary archive header
 *
 * PARMAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 *
 * RETURNS
 *
 * 	Returns the number of trailing alignment bytes to skip; -1 if 
 *	unsuccessful. 
 *
 */

#ifdef __STDC__

static int inswab(char *magic, char *name, Stat *asb)

#else
    
static int inswab(magic, name, asb)
char           *magic;
char           *name;
Stat           *asb;

#endif
{
    ushort          namesize;
    uint            namefull;
    Binary          binary;

    if (*((ushort *) magic) != SWAB(M_BINARY)) {
	return (-1);
    }
    memcpy((char *) &binary,
		  magic + sizeof(ushort),
		  M_STRLEN - sizeof(ushort));
    if (buf_read((char *) &binary + M_STRLEN - sizeof(ushort),
		 sizeof(binary) - (M_STRLEN - sizeof(ushort))) < 0) {
	warnarch("Corrupt swapped header",
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    asb->sb_dev = (dev_t) SWAB(binary.b_dev);
    asb->sb_ino = (ino_t) SWAB(binary.b_ino);
    asb->sb_mode = SWAB(binary.b_mode);
    asb->sb_uid = SWAB(binary.b_uid);
    asb->sb_gid = SWAB(binary.b_gid);
    asb->sb_nlink = SWAB(binary.b_nlink);
    asb->sb_rdev = (dev_t) SWAB(binary.b_rdev);
    asb->sb_mtime = SWAB(binary.b_mtime[0]) << 16 | SWAB(binary.b_mtime[1]);
    asb->sb_size = SWAB(binary.b_size[0]) << 16 | SWAB(binary.b_size[1]);
    if ((namesize = SWAB(binary.b_name)) == 0 || namesize >= PATH_MAX) {
	warnarch("Bad swapped pathname length",
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    if (buf_read(name, namefull = namesize + namesize % 2) < 0) {
	warnarch("Corrupt swapped pathname", (OFFSET) namefull);
	return (-1);
    }
    if (name[namesize - 1] != '\0') {
	warnarch("Bad swapped pathname", (OFFSET) namefull);
	return (-1);
    }
    return (asb->sb_size % 2);
}


/* inascii - read in an ASCII cpio header
 *
 * DESCRIPTION
 *
 *	Reads an ASCII format cpio header
 *
 * PARAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 * RETURNS
 *
 * 	Returns zero if successful; -1 otherwise. Assumes that  the entire 
 *	magic number has been read. 
 */

#ifdef __STDC__

static int inascii(char *magic, char *name, Stat *asb)

#else
    
static int inascii(magic, name, asb)
char           *magic;
char           *name;
Stat           *asb;

#endif
{
    uint            namelen;
    char            header[H_STRLEN + 1];

    /* Provides known datatypes for the sscanf call */
    /* Args 3-10, 12 */
    long unsigned   head_temp[9];
    long unsigned   * header_ptr = head_temp;

    if (strncmp(magic, M_ASCII, M_STRLEN) != 0) {
	return (-1);
    }
    if (buf_read(header, H_STRLEN) < 0) {
	warnarch("Corrupt ASCII header", (OFFSET) H_STRLEN);
	return (-1);
    }
    header[H_STRLEN] = '\0';

    if (sscanf(header, H_SCAN, head_temp, head_temp+1, head_temp+2, 
               head_temp+3, head_temp+4, head_temp+5, head_temp+6, 
               head_temp+7, &namelen, head_temp+8) != H_COUNT) {
	warnarch("Bad ASCII header", (OFFSET) H_STRLEN);
	return (-1);
    }

    /* A kludge to allow struct stat / scanf formatting independance.
     * Chosen because it does not require compile time conditionals or
     * os specific maintaining.
     */
    asb->sb_dev   = *header_ptr++;
    asb->sb_ino   = *header_ptr++;
    asb->sb_mode  = *header_ptr++;
    asb->sb_uid   = *header_ptr++;
    asb->sb_gid   = *header_ptr++;
    asb->sb_nlink = *header_ptr++;
    asb->sb_rdev  = *header_ptr++;
    asb->sb_mtime = *header_ptr++;
    asb->sb_size  = *header_ptr;

    if (namelen == 0 || namelen >= PATH_MAX) {
	warnarch("Bad ASCII pathname length", (OFFSET) H_STRLEN);
	return (-1);
    }
    if (buf_read(name, namelen) < 0) {
	warnarch("Corrupt ASCII pathname", (OFFSET) namelen);
	return (-1);
    }
    if (name[namelen - 1] != '\0') {
	warnarch("Bad ASCII pathname", (OFFSET) namelen);
	return (-1);
    }
    return (0);
}


/* inbinary - read a binary header
 *
 * DESCRIPTION
 *
 *	Reads a CPIO format binary header.
 *
 * PARAMETERS
 *
 *	char	*magic	- magic number to match
 *	char	*name	- name of the file which is stored in the header.
 *			  (modified and passed back to caller).
 *	Stat	*asb	- stat block for the file (modified and passed back
 *			  to the caller).
 *
 * RETURNS
 *
 * 	Returns the number of trailing alignment bytes to skip; -1 if 
 *	unsuccessful. 
 */

#ifdef __STDC__

static int inbinary(char *magic, char *name, Stat *asb)

#else
    
static int inbinary(magic, name, asb)
char           *magic;
char           *name;
Stat           *asb;

#endif
{
    uint            namefull;
    Binary          binary;

    if (*((ushort *) magic) != M_BINARY) {
	return (-1);
    }
    memcpy((char *) &binary,
		  magic + sizeof(ushort),
		  M_STRLEN - sizeof(ushort));
    if (buf_read((char *) &binary + M_STRLEN - sizeof(ushort),
		 sizeof(binary) - (M_STRLEN - sizeof(ushort))) < 0) {
	warnarch("Corrupt binary header",
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    asb->sb_dev = binary.b_dev;
    asb->sb_ino = binary.b_ino;
    asb->sb_mode = binary.b_mode;
    asb->sb_uid = binary.b_uid;
    asb->sb_gid = binary.b_gid;
    asb->sb_nlink = binary.b_nlink;
    asb->sb_rdev = binary.b_rdev;
    asb->sb_mtime = binary.b_mtime[0] << 16 | binary.b_mtime[1];
    asb->sb_size = binary.b_size[0] << 16 | binary.b_size[1];
    if (binary.b_name == 0 || binary.b_name >= PATH_MAX) {
	warnarch("Bad binary pathname length",
		 (OFFSET) sizeof(binary) - (M_STRLEN - sizeof(ushort)));
	return (-1);
    }
    if (buf_read(name, namefull = binary.b_name + binary.b_name % 2) < 0) {
	warnarch("Corrupt binary pathname", (OFFSET) namefull);
	return (-1);
    }
    if (name[binary.b_name - 1] != '\0') {
	warnarch("Bad binary pathname", (OFFSET) namefull);
	return (-1);
    }
    return (asb->sb_size % 2);
}
