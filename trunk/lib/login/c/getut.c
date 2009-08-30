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
	routines for manipulating the "utmp" files.
	designed to be compatable with the sysV routines of the same name.
	changes:
		if you pututline() before you 'getutline', it appends it to the
		file.
*/

#include <utmp.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


#include "login.h"

#if !defined(__QNXNTO__)

/*
 * file descriptor for utmp file, and flag to keep track of validity.
 */
static	int fd_ok = 0;
static	int	utfd=-1;
/*
 * name of utmp file
 */
static char *utfname = UTMP_FILE;
/*
 * io buffer for comparisons, etc...
 * and flag for valid information...
 */
static	int buf_ok=0;
static	struct	utmp	utbuf;


void utmpname(char *filename)
{
	utfname = strdup(filename);
	if (fd_ok)
		close(utfd);
	fd_ok = 0;
	buf_ok = 0;
}

static int open_utfd()
{
	return fd_ok =  (utfd=open(utfname,O_RDONLY)) != -1;
}
	

void endutent(void)
{
	buf_ok = 0;
	if (fd_ok)
		close(utfd);
	fd_ok = 0;
}

void setutent(void)
{
	endutent();
	open_utfd();
}


/* utmp access functions */
struct utmp *getutent(void)
{
	if (!fd_ok && open_utfd() == 0) {
		return NULL;
	}
	if (read(utfd,&utbuf,sizeof utbuf) != sizeof utbuf) {
		buf_ok =0;
		return NULL;
	}
	buf_ok = 1;
	return &utbuf;
}

struct utmp *getutid(struct utmp *id)
{
	while (1) {
		if (buf_ok == 0) {
			if (getutent() == NULL)
				return NULL;	/* end of file */
		}
		switch (id->ut_type) {
		case	EMPTY:
			break;
		case	RUN_LVL:
		case	BOOT_TIME:
		case	OLD_TIME:
		case	NEW_TIME:
			if (utbuf.ut_type == id->ut_type) {
				return &utbuf;
			}
			break;
		case	INIT_PROCESS:
		case	LOGIN_PROCESS:
		case	USER_PROCESS:
		case	DEAD_PROCESS:
			if (utbuf.ut_id == id->ut_id)
				return &utbuf;
			break;
		case	ACCOUNTING:
		default:
			break;
		}
		buf_ok = 0;
	}
}

struct utmp *getutline(struct utmp *line)
{
	while (1) {
		if (buf_ok == 0) {
			if (getutent() == NULL)
				return NULL;	/* end of file */
		}
		if ((utbuf.ut_type == LOGIN_PROCESS || utbuf.ut_type == USER_PROCESS)
			&& strcmp(utbuf.ut_line,line->ut_line) == 0) {
			return &utbuf;
		}
		buf_ok = 0;
	}
}

void pututline(struct utmp *utmp)
{
int	fd;
long	seekpos;
	if (getutline(utmp) == NULL) {
		if ((fd=open(utfname,O_WRONLY|O_APPEND)) == -1)
			return;
	} else {
		if ((seekpos = lseek(utfd,0L,SEEK_CUR)) == -1)
			return;
		if ((fd=open(utfname,O_WRONLY)) == -1)
			return;
		lseek(fd,seekpos-sizeof *utmp,SEEK_SET);
	}
	write(fd,utmp,sizeof *utmp);
	close(fd);
}

#endif
