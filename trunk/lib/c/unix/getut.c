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
Routines for manipulating the "utmp" files.
Designed to be compatible with the sysV routines of the same name.

Changes:
If you pututline() before you 'getutline', it appends it to the file.
*/

#include <utmp.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

/*
 * file descriptor for utmp file, and flag to keep track of validity.
 */
static int fd_ok = 0;
static int utfd = -1;

/*
 * name of utmp file
 */
static char *utfname = NULL;

/*
 * I/O buffer for comparisons, etc...
 * and flag for valid information...
 */
static int buf_ok = 0;
static struct utmp utbuf;


void utmpname(char *filename)
{
	if (utfname) {
		free(utfname);
	}
    utfname = strdup(filename);
    if (fd_ok) {
		close(utfd);
	}
    fd_ok = 0;
    buf_ok = 0;
}

static int open_utfd()
{
    utfd = open((utfname) ? utfname : _PATH_UTMP, O_RDONLY);
    fd_ok = (utfd != -1);
	return fd_ok;
}


void endutent(void)
{
    buf_ok = 0;
    if (fd_ok) {
		close(utfd);
	}
    fd_ok = 0;
}

void setutent(void)
{
    endutent();
    (void)open_utfd();
}


/* utmp access functions */
struct utmp *getutent(void)
{
    if (!fd_ok && open_utfd() == 0) {
		return NULL;
    }
    if (read(utfd, &utbuf, sizeof utbuf) != sizeof utbuf) {
		buf_ok = 0;
		return NULL;
    }
    buf_ok = 1;
    return &utbuf;
}

struct utmp *getutid(struct utmp * id)
{
    while (1) {
		if (buf_ok == 0) {
		    if (getutent() == NULL) {
				return NULL;		/* end of file */
			}
		}
		switch (id->ut_type) {
		  case EMPTY:
		    break;
		  case RUN_LVL:
		  case BOOT_TIME:
		  case OLD_TIME:
		  case NEW_TIME:
		    if (utbuf.ut_type == id->ut_type) {
				return &utbuf;
		    }
		    break;
		  case INIT_PROCESS:
		  case LOGIN_PROCESS:
		  case USER_PROCESS:
		  case DEAD_PROCESS:
		    if (utbuf.ut_id == id->ut_id) {
				return &utbuf;
			}
		    break;
		  case ACCOUNTING:
		  default:
		    break;
		}
		buf_ok = 0;
    }
}

struct utmp *getutline(struct utmp * line)
{
    while (1) {
		if (buf_ok == 0) {
		    if (getutent() == NULL) {
				return NULL;		/* end of file */
			}
		}
		if ((utbuf.ut_type == LOGIN_PROCESS || utbuf.ut_type == USER_PROCESS)
			&& strcmp(utbuf.ut_line, line->ut_line) == 0) {
		    return &utbuf;
		}
		buf_ok = 0;
    }
}

/*
 This is an undocumented call to replace the line 
 that you are currently pointing at, or append if
 the buffer is null.
*/
void _reputline(struct utmp * utmp) 
{
	int fd;

	if (buf_ok == 0 || fd_ok <= 0) {
		if ((fd = open((utfname) ? utfname : _PATH_UTMP, O_WRONLY | O_APPEND)) == -1) {
		    return;
		}
	}
	else {
		int inspos;

		if ((inspos = lseek(utfd, 0L, SEEK_CUR)) == -1) {
			return;
		}
		else if ((fd = open((utfname) ? utfname : _PATH_UTMP, O_WRONLY)) == -1) {
			return;
		}
		lseek(fd, inspos - sizeof *utmp, SEEK_SET);
	}

	write(fd, utmp, sizeof(*utmp));
	close(fd);
}

void pututline(struct utmp * utmp)
{
	(void)getutline(utmp);
	_reputline(utmp);	
}

/**** BSD Functions for manipulating utmp type files ****/
/*NOTE: These functions reset the fd's used by the ut functions */

/*
 This assumes that you have filled out all of the fields as
 you desire, and want the type set to LOGIN_PROCESS (otherwise
 why call login!) and want the information propagated to the
 wtmp and utmp files.
*/
void login(struct utmp *ut) {
	int saved;

	saved = ut->ut_type;

	/* Write out to the utmp file, overwriting the last entry */
	utmpname(_PATH_UTMP);
	setutent();
	ut->ut_type = LOGIN_PROCESS;
	pututline(ut);
	endutent();	

	/* Write out to the wtmp file, appending to the end */
	utmpname(_PATH_WTMP);
	setutent();

	while(getutent()) { ; } //Skip to the end of the file

	pututline(ut);
	endutent(); 

	ut->ut_type = saved;
}

int logout(const char *line) {
	int			gotone;
	struct utmp *ut;

	/* We construct a bogus entry and write it to the utmp stream */
	gotone = 0;
	utmpname(_PATH_UTMP);
	setutent();
	while ((ut = getutent())) {
		if (!ut->ut_name[0] || strncmp(ut->ut_line, line, UT_LINESIZE)) {
			continue;
		}
		memset(ut, 0, sizeof(ut));
		strncpy(ut->ut_line, line, UT_LINESIZE); 
		(void)time(&ut->ut_time);
		_reputline(ut);
		gotone++;
	}
	endutent();	

	return gotone;
}

void logwtmp(const char *line, const char *name, const char *host) {
	struct utmp ut;

	memset(&ut, 0, sizeof(ut));
	strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	strncpy(ut.ut_name, name, sizeof(ut.ut_name));
	//strncpy(ut.ut_host, host, sizeof(ut.ut_host));
	(void)time(&ut.ut_time);

	utmpname(WTMP_FILE);
	setutent();

	while(getutent()) { ; } //Skip to the end of the file

	pututline(&ut);
	endutent(); 
}



__SRCVERSION("getut.c $Rev: 153052 $");
