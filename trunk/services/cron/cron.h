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





#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <time.h>
#include <paths.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#ifndef __WATCOMC__
#include <libgen.h>
#endif
#include <util/util_limits.h> /* for UTIL_PATH_MAX */

/*  Configure variables */
#define TAB_BUFFER_SIZE		UTIL_PATH_MAX
#define IO_WIDTH		50

/* Bit maps, where set bits repesent valid times */
#define TIMES_ELEMENTS 6 /* Not a config var */
typedef struct cron_job {
	unsigned long time[TIMES_ELEMENTS];
	uid_t     user;
	char *    command;
	struct cron_job * next;
} cron_job;

/* called by sys dependant routines */
void message(int stream, char *fmt, ...);
long do_jobs();
void ctab_load(int signo);
char * cronstrtime();

/* sys-dependant routines in *"/"sys_cron */
/* int main(int *, char **); */
void cleanup(int signo);
void trigger(cron_job * job);

/* Names for the different kinds of message() */
#define LOG   1        /* Only with -v           */
#define FATAL 2        /* Fatal                  */
#define FATAL_FORKED 3 /* Fatal without shutdown */
#define EVENT 4        /* Non-fatal              */

