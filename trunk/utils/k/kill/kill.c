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





#ifdef __USAGE
%C - terminate or signal processes (POSIX)

%C	[-n nid] [-signal_name|-signal_number] pid ...
%C	-l
Options:
 -signal_name   Symbolic name of signal to send
 -signal_number Integer representing a signal type
 -l             List symbolic signal names
 -n nid         (/bin/kill only) Kill processes on the specified node.
Where:
 Valid signal names are:

 SIGNULL     SIGHUP      SIGINT      SIGQUIT     SIGILL      SIGTRAP
 SIGIOT      SIGABRT     SIGEMT      SIGFPE      SIGKILL     SIGBUS
 SIGSEGV     SIGSYS      SIGPIPE     SIGALRM     SIGTERM     SIGUSR1
 SIGUSR2     SIGCHLD     SIGPWR      SIGWINCH    SIGURG      SIGPOLL
 SIGSTOP     SIGTSTP     SIGCONT     SIGDEV      SIGTTIN     SIGTTOU

Note:
 kill is also available as a shell builtin. The -n nid option is NOT
 available in the shell builtin version.
#endif


#ifdef __USAGENTO
%C - terminate or signal processes (POSIX)

%C	[-signal_name|-signal_number] pid ...
%C	-l
Options:
 -signal_name   Symbolic name of signal to send
 -signal_number Integer representing a signal type
 -l             List symbolic signal names
 -n node        Kill processes on the specified node. (/bin/kill only)
Where:
 Valid signal names are:

 SIGNULL     SIGHUP      SIGINT      SIGQUIT     SIGILL      SIGTRAP
 SIGIOT      SIGABRT     SIGEMT      SIGFPE      SIGKILL     SIGBUS
 SIGSEGV     SIGSYS      SIGPIPE     SIGALRM     SIGTERM     SIGUSR1
 SIGUSR2     SIGCHLD     SIGPWR      SIGWINCH    SIGURG      SIGPOLL
 SIGSTOP     SIGTSTP     SIGCONT     SIGTTIN     SIGTTOU     SIGVTALRM
 SIGPROF     SIGXCPU     SIGXFSZ 

Note:
 kill is also available as a shell builtin.
#endif

/*
 * Description:
 *			The kill utility sends a signal to the process(es) specified by
 *			each pid operand.                          
 *
 *			For each pid operand, the kill utility performs actions equivalent
 *			to the kill() 3.3.2 Posix 1003.1 function called with the following
 *			arguments.
 *				(1) The value of thie pid operand is used as the pid argument
 *				(2) The sig argument is the value specified by the -signal_name
 *					or -signal_number option, or by SIGTERM, if neither is 
 *					specified
 *			Implementations may omit the verson of this utility that is not
 *			built-in.  See Restrictions on Built-in Utilities 2.4 Posix 1003.2
 *
 * Note:	See POSIX 1003.2 Draft 9 Section 4.34
 *
 * Args:
 *			-signal_name				One of the symbolic names defined for
 *										Required Signals defined in Signal
 *										Names 3.3.1.1 Posix 1003.1.   In
 *										addition, the symbolic name SIGNULL
 *										is recognized, and represents the
 *										signal calue 0.  Values of signal_name
 *										are recognized in case-independant 
 *										fashion, with or without the SIG prefix.
 *										The corresponding signal is send instead
 *										of SIGTERM.
 *			-l							Do not send signals. List the values
 *										of signal_name supported by the
 *										implementation.  The first value shall
 *										be NULL
 *			-signal_number				A nonnegative decimal integer,
 *										representing the signal to be used
 *										instead of SIGTERM as teh sig argument
 *										in the effective call to kill().  The  
 *										correspondence between integer values
 *										and the sig value used is show below
 *										Number			sig Value
 *										0				0
 *										1				SIGHUP
 *										2				SIGINT
 *										3				SIGQUIT
 *										6				SIGABRT
 *										9				SIGKILL
 *										14				SIGALRM
 *										15				SIGTERM
 *                                      NOTE:  This option is declared deprecated
 *			pid							A decimal integer specifying a process
 *										or process group to be signalled.  The
 *										process(es) selected by positive, zero
 *										and negative values of the pid operand
 *										are described for kill() 3.3.2 Posix1003.1
 *
 * Include declarations:
 */
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <libgen.h>
#include <util/stdutil.h>
#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#endif

#define CMDLINE_OPTS  "ln:"

struct sigs {
	int signo;
	char *sigstr;
} signals[] = {
	{NULL,		"NULL"},	{SIGHUP,	"HUP"},	{SIGINT,	"INT"},  
	{SIGQUIT,	"QUIT"},	{SIGILL,	"ILL"},	{SIGTRAP,	"TRAP"},
	{SIGIOT,	"IOT"}, 	{SIGABRT,	"ABRT"},{SIGEMT,	"EMT"},
	{SIGFPE,	"FPE"},		{SIGKILL,	"KILL"},{SIGBUS,	"BUS"},
	{SIGSEGV,	"SEGV"},	{SIGSYS,	"SYS"},	{SIGPIPE,	"PIPE"},
	{SIGALRM,	"ALRM"},	{SIGTERM,	"TERM"},{SIGUSR1,	"USR1"},
	{SIGUSR2,	"USR2"},	{SIGCHLD,	"CHLD"},{SIGPWR,	"PWR"},
	{SIGWINCH,	"WINCH"},	{SIGURG,	"URG"},	{SIGPOLL,	"POLL"},
	{SIGSTOP,	"STOP"},	{SIGCONT,       "CONT"},
#ifdef __QNXNTO__
{SIGTSTP, "TSTP"},
{SIGTTIN, "TTIN"},
{SIGTTOU, "TTOU"},
{SIGVTALRM, "VTALRM"},
{SIGPROF, "PROF"},
{SIGXCPU, "XCPU"},
{SIGXFSZ, "XFSZ"},
#else
{SIGDEV,	"DEV"},
#endif
	{0,			0} 	
};

int kill_remote(int nid, pid_t pid, int signum);
	
int main(int argc, char **argv) {
	char		*pname = basename(argv[0]);
	int			sigval = -1, i;
#ifdef __QNXNTO__
	int			nid = 0;
#else
	nid_t		nid = 0;
#endif
	struct		sigs *ss;
	long		lval;			/* temporary for checking argument ranges */
	int         err=0;    		/* exit status: errors encountered in processing */
	int			success=0;      /* exit status: successful signal deliveries */
	int 		ind=optind;	

	opterr = 0;
	while (sigval == -1 && (i = getopt(argc, argv, CMDLINE_OPTS)) != -1) {
		switch(i) {
			case 'l':{
					for(ss = signals, i = 0; ss->sigstr; ss++) {
						printf("%-8s", ss->sigstr);
						if (++i % 10 == 0) printf("\n");
					}
					if (i % 12) printf("\n");
					exit(EXIT_SUCCESS);
					}

			case 'n':
#ifdef __QNXNTO__
					nid = netmgr_strtond(optarg, 0);
					if(nid == -1){
						fprintf(stderr, "Unable to open %s: %s\n", optarg, strerror(errno));
						return 1;
					}
#else
					nid = qnx_strtonid(optarg, 0);
#endif
					break;

			default:
					if(isdigit(optopt)) {
						char *p = &argv[ind][1];

						optind=ind+1; /* make getopt skip over this arg */

						if ((lval = strtol(p, 0, 0)) <= _SIGMAX) {
							sigval = lval;
						} else {
							fprintf(stderr,"%s: invalid signal number (%s)\n",pname,p);
							exit(EXIT_FAILURE);
						}
					} else {
						char *p = &argv[ind][1];

						optind=ind+1; /* make getopt skip over this arg */

						i = strnicmp(p, "sig", 3) ? 0 : 3;
						for (ss = signals; ss->sigstr; ++ss) {
							if (stricmp(&p[i], ss->sigstr) == 0) {
								sigval = ss->signo;
								break;
							}
						}
						if (!ss->sigstr) {
							fprintf(stderr, "%s: invalid signal name (%s)\n", pname, p);
							exit(EXIT_FAILURE);
						}
					}
					break;
		}
		ind=optind;
	}
	if (sigval == -1)
		sigval = SIGTERM;

	if (sigval < 0) {
		fprintf(stderr, "%s: invalid signal value (%d)\n", pname, sigval);
		exit(EXIT_FAILURE);
	}

	for (i = optind; i < argc; i++) {
		char *endptr;

		errno=0;

		lval = strtol(argv[i], &endptr, 0);
		if (errno) {
			fprintf(stderr,"%s: bad PID number '%s' (%s)\n",
				pname, argv[i], strerror(errno));
			err++;
			continue;
		}
		if (endptr==argv[i]) {
			fprintf(stderr,"%s: bad PID number '%s' (must be integer)\n",
				pname, argv[i]);
			err++;
			continue;
		}
		if (!(isxdigit(*(endptr-1)))){
			fprintf(stderr,"%s: bad PID number '%s' (must be integer)\n",
				pname, argv[i]);
			err++;
			continue;
		}
				
		if (kill_remote(nid, (pid_t) lval, sigval) != 0) {
			fprintf(stderr, "%s: %s (%s)\n", pname, strerror(errno), argv[i]);
			if (errno==ESRCH) err++; /* if process doesn't exist, exit non-zero */
		} else success++; /* successful delivery of signal */
	}

	/* must have had a process matching every operand, and for at least one
       of these the kill() call must have been successful (permission) */

	if (err || success==0) exit(EXIT_FAILURE);
	exit(EXIT_SUCCESS);
}

#ifdef __QNXNTO__
int
kill_remote(int nid, pid_t pid, int signum)
{
	return SignalKill(nid, pid, 0, signum, SI_USER, 0) == -1;
}

#else

int
kill_remote(nid_t nid, pid_t pid, int signum) {
	int status;

    if (nid && pid < 0) {
		errno = ENOREMOTE;
		return -1;
    }
	if (pid < 0) {
		status = kill(pid, signum);
	} else {
		pid_t vid;
	
		if ((vid = qnx_vc_attach(nid, pid, 0, 0)) == -1)
			return -1;

		status = kill(vid, signum);
		qnx_vc_detach(vid);
    }

	return status;	
}
#endif

