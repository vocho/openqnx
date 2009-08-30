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




#ifndef __UTMP_H_INCLUDED
#define __UTMP_H_INCLUDED

#ifndef _PATHS_H
#include <paths.h>
#endif

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#define UTMP_FILE   _PATH_UTMP
#define WTMP_FILE   _PATH_WTMP

#define UT_NAMESIZE 14 
#define UT_LINESIZE 14 

struct utmp {
	char    ut_user[UT_NAMESIZE]; /*  login name */
#define ut_name ut_user
	char    ut_id[4];             /*  line # */
	char    ut_line[UT_LINESIZE]; /*  device name (console) */
	pid_t   ut_pid;               /*  process id */
	short   ut_type;              /*  entry type */
	struct exit_status {
		short   e_termination;      /* termination status */
		short   e_exit;             /* exit status */
	} ut_exit;
	short	ut_spare;
	time_t  ut_time;
};

#define EMPTY           0
#define RUN_LVL         1
#define BOOT_TIME       2
#define OLD_TIME        3
#define NEW_TIME        4
#define INIT_PROCESS    5
#define LOGIN_PROCESS   6
#define USER_PROCESS    7
#define DEAD_PROCESS    8
#define ACCOUNTING      9


__BEGIN_DECLS

/* Sys V type utmp access functions */
extern struct utmp *getutent(void);
extern struct utmp *getutid(struct utmp *__id);
extern struct utmp *getutline(struct utmp *__line);
extern void pututline(struct utmp *__utmp);
extern void setutent(void);
extern void endutent(void);
extern void utmpname(char *__filename);

/* BSD type access functions */
void login(struct utmp *__utmp);
int  logout(const char *__line);
void logwtmp(const char *__line, const char *__name, const char *__host);

__END_DECLS

#endif

/* __SRCVERSION("utmp.h $Rev: 153052 $"); */
