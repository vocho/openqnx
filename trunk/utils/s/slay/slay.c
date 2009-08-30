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





/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.32  2007/03/23 14:23:57  bstecher
	PR: 45971.

	Removed unneeded include of <term.h>

	Revision 1.31  2007/02/07 16:17:41  thaupt
	PR: 21836, 44029
	CI: rmansfield
	Implemented ttymatch() for NTO to support -t option.
	
	Revision 1.30  2006/09/06 17:42:53  sberthiaume
	
	PR:41198
	CI:thomasf
	
	fixed slay so that it would find processes from non-stripped binaries generated on Windows systems.
	
	Revision 1.29  2006/04/11 16:16:22  rmansfield
	
	PR: 23548
	CI: kewarken
	
	Stage 1 of the warnings/errors cleanup.
	
	Revision 1.28  2006/03/02 16:08:11  seanb
	
	slay across nodes on different domains got busted with the
	fix for PR 29117.
	PR:29840
	CI:cburgess
	
	Revision 1.27  2006/01/23 18:38:09  seanb
	- Don't assume qnet is using /net as a mountpoint.
	PR: 29117
	CI: rcraig
	
	Revision 1.26  2006/01/05 20:45:06  jgarvey
	PR/28890
	Re-design of this.  Now by default the given process will be matched by
	either name or pid.  A new option can specify to match only one format
	("-m name" or "-m pid").  This should be abckwards-compatable yet still
	allow scripts to explicitly identify a process (to runmask/priority it).
	CI: sboucher
	
	Revision 1.25  2005/12/23 22:11:20  jgarvey
	PR/28890
	Allow operands to "slay" to be either process names or pids.
	Precedence is given to names (in the unlikely case of having both
	a process with a fully numeric pathname and that pid slot used).
	CI: seanb
	
	Revision 1.24  2005/12/08 16:41:09  seanb
	- 64 bit paranioa.  The masks themselves are in natively
	  sized masks (unsigned).  Always take 32 bits per -R to
	  avoid change in behaviour when we imminently run on 64
	  bit machines with > 32 cores.
	CI:rcraig
	PR:28499
	
	Revision 1.23  2005/11/16 19:06:45  seanb
	- Update to new _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT
	  semantics after changes for PR 28083.
	PR:28083
	CI:rcraig
	
	Revision 1.22  2005/11/16 17:11:35  seanb
	- Update to new _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT
	  semantics after changes for PR 28083.
	PR:28083
	CI:rcraig
	
	Revision 1.21  2005/11/08 18:30:44  sboucher
	CR636 add rcsid IQ
	
	Revision 1.20  2005/10/17 19:43:42  seanb
	- -P without -T now affects all threads.
	- PR 27817.
	- CI: rcraig.
	
	Revision 1.19  2005/10/17 16:39:38  seanb
	- Raise a STOP if the inherit mask is being
	  altered for every thread in a process so that
	  none are missed.
	- Change came out of code review for PR 27716.
	
	Revision 1.18  2005/10/17 14:17:58  seanb
	- -[CR] without -T now sets runmask of all threads in a
	  process.
	- -i added to set the inherit mask when used in
	  conjunction with -[CR].
	- PR 27716.
	- CI: pending rcraig.
	
	Revision 1.17  2005/09/30 16:24:33  seanb
	- Check return from open() correctly.
	- use SCHED_NOCHANGE to preserve policy rather
	  than pulling it out of procfs.
	- PR 27546
	- Code review: cburgess
	
	Revision 1.16  2005/09/28 22:46:43  seanb
	- Buffer management issues.
	  - Overrun with -n
	  - -n and -C didn't mix.
	- PR 27526
	- Code review: rcraig
	
	Revision 1.15  2005/06/03 01:38:00  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.14  2003/11/17 18:19:51  cburgess
	fix for PR17667 - slay (REGRESSION): can't set priorities with slay
	
	need to set tid=1 in setprio_remote - can't use tid=0
	
	Revision 1.13  2003/08/29 20:27:51  martin
	Update QSSL Copyright.
	
	Revision 1.12  2003/06/06 20:26:30  cburgess
	tid should have been initialized to 0
	
	Revision 1.11  2003/05/27 13:26:30  cburgess
	updated a little to reports errors better
	
	Revision 1.10  2003/02/07 16:10:12  cburgess
	add selecting thread, and setting cpu/runmask
	
	Revision 1.9  2001/12/14 15:34:23  kewarken
	fix for PR:9633 - slay -P not working.  It was calling sched_setscheduler
	
	Revision 1.8  2001/11/15 16:51:53  kewarken
	PR:9140 - slay will now set priorities properly.
	Also fixed bug where 'slay -#' didn't work
	
	Revision 1.7  2001/07/09 19:38:56  kewarken
	fixed option parsing bug (p.r.5750)
	
	Revision 1.6.2.1  2001/07/09 19:36:33  kewarken
	fixed option parsing bug (p.r. 5750)
	
	Revision 1.6  2001/04/25 16:32:01  xtang
	Oops, the change to support cross network slay break the local case
	(causing SIGSEGV on local case). Fixed.
	
	Revision 1.5  2001/04/20 19:49:52  xtang
	kill_remote() now working.
	
	Revision 1.4  2000/04/10 13:22:40  thomasf
	Fixed problem with NTO setprio returning the old priority instead
	of a -1/0 value as was expected.
	
	Revision 1.3  1999/01/04 19:45:15  peterv
	Made priority range testing be done by kernel.
	
	Revision 1.2  1998/10/25 02:48:47  dtdodge
	Added network support. At least I think I did based upon how it is supposed
	to work....
	
	Revision 1.1  1998/10/25 02:29:20  dtdodge
	Initial cvs checkin.
	
	Revision 1.8  1997/07/22 14:10:29  eric
	added new -p option which prints pids to stdout.
	Process name match with slay is exact;

	sin -hP program format i

	is therefore not a replacement, since it matches
	"*program*". You could do

	sin -hP program format ni | grep...

	but is somewhat painful/less usable.

	Revision 1.7  1997/07/22 14:02:02  glen
	changed usage message

	Revision 1.6  1996/07/10 13:50:14  glen
	remove -p from the usage message

	Revision 1.5  1996/05/09 18:45:13  glen
	make -t work a little better

	Revision 1.4  1996/05/09 18:39:06  brianc
	*** empty log message ***

 * Revision 1.3  1992/08/07  22:34:50  brianc
 * Allow modifying scheduling algorithm along with priority
 *
 * Revision 1.2  1992/05/12  18:40:17  eric
 * fixed slay -SI null pointer error message
 *
 * Revision 1.1  1991/08/16  17:11:48  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#ifdef __QNXNTO__
#include <sched.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/procfs.h>
#include <sys/debug.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
typedef char * node_t;
#else
#include <sys/psinfo.h>
#include <sys/sidinfo.h>
#include <sys/kernel.h>
#include <sys/vc.h>
#include <sys/sched.h>
typedef nid_t node_t;
#endif

typedef unsigned short 		uint;
typedef unsigned char		uchar;

#ifndef TRUE
#	define TRUE		(1==1)
#	define FALSE	(1==0)
#endif

#define MAX_PIDS	400					/* Max # of names we'll kill */

#define DEAD		0		// process state

#ifdef __QNXNTO__
enum	{ MATCH_NAME, MATCH_PID };
int		match[MATCH_PID - MATCH_NAME + 1];
#endif

int		query, verbose, epid, killson, force, quiet;
int		prio;
#ifdef __QNXNTO__
int		sched = SCHED_NOCHANGE;
#else
int		sched = -1;
#endif
pid_t	pids[MAX_PIDS];
char	*names[MAX_PIDS];
char	*ttys[MAX_PIDS];
char	ttynambuf[100];		// Lots of room


struct sig_entry {
	int signum;
	char *signame;
	} sigtable[] ={
		{0,			"NULL"},
		{SIGHUP,	"HUP"},
		{SIGINT,	"INT"},
		{SIGQUIT,	"QUIT"},
		{SIGILL,	"ILL"},
		{SIGTRAP,	"TRAP"},
		{SIGIOT,	"IOT"},
		{SIGABRT,	"ABRT"},
		{SIGEMT,	"EMT"},
		{SIGFPE,	"FPE"},
		{SIGKILL,	"KILL"},
		{SIGBUS,	"BUS"},
		{SIGSEGV,	"SEGV"},
		{SIGSYS,	"SYS"},
		{SIGPIPE,	"PIPE"},
		{SIGALRM,	"ALRM"},
		{SIGTERM,	"TERM"},
		{SIGUSR1,	"USR1"},
		{SIGUSR2,	"USR2"},
		{SIGCHLD,	"CHLD"},
		{SIGPWR,	"PWR"},
		{SIGWINCH,	"WINCH"},
		{SIGURG,	"URG"},
		{SIGPOLL,	"POLL"},
		{SIGSTOP,	"STOP"},
		{SIGCONT,	"CONT"},
#ifndef __QNXNTO__
		{SIGDEV,	"DEV"},
#endif
		{0,			NULL},
		} ;

/*
 * Prototypes
 */
struct sig_entry	*lookup(char *);
char				*tail(char *);
#ifdef __QNXNTO__
int ttymatch( char *dev, int sid );
int					 setprio_remote(node_t, pid_t, int, int);
int					 setrunmask_remote(node_t, pid_t, int, int *, int, int);
char *				prepare_path(char **, node_t, char *, ...);
int					prepare_node(char **, int *);
#else
char				*ttymatch(pid_t, int);
int					 setprio_remote(node_t, pid_t, int);
int					 kill_remote(node_t, pid_t, int);
#endif
int					 find(node_t, char *);
int					 yesno(void);
void				 _raw(void);
void				 _unraw(void);


int
main( argc, argv )
int		argc;
char	*argv[];
	{
	char				*p;
	struct sig_entry	*sigptr = NULL;
	long				 lval;
	int					 i, npids = 0, tpids = 0, need_newline, status;
	int					tid = 0, inherit, node_nd = ND_LOCAL_NODE;
	unsigned			*runmask = NULL, rmax = 0, rsize = 0, rdone = 0, val;
	unsigned			rdone_32 = 0;
	node_t				 node = 0;

	epid = verbose = query = FALSE;
	killson = TRUE;
#ifdef __QNXNTO__
	match[MATCH_NAME] = match[MATCH_PID] = TRUE;
#endif
	prio = -1;

	opterr = 0;
	inherit = 0;

#ifdef __QNXNTO__
#define NTO_OPTS "T:R:C:im:"
#else
#define NTO_OPTS /* Nothing */
#endif
	while ((i = getopt(argc, argv, "fhupQqvSs:n:t:P:" NTO_OPTS)) != -1) {
		switch( i ) {
			case 'f':	force = TRUE;			break;
			case 'h':	sigptr = lookup("STOP");break;
#ifdef __QNXNTO__
			case 'T':	tid = atoi(optarg); break;
			case 'C':
			case 'R':
				if (runmask == NULL) {
					rmax = _syspage_ptr->num_cpu;
					rsize = RMSK_SIZE(rmax);
					rdone = 0;
					if ((runmask = alloca(rsize *
					    sizeof(*runmask))) == NULL) {
						fprintf(stderr, "No memory\n");
						exit(0);
					}
					memset(runmask, 0x00, rsize *
					    sizeof(*runmask));
				}
				
				val = strtoul(optarg, NULL, 0);
				if (i == 'C') {
					if (val < rmax)
						RMSK_SET(val, runmask);
				}
				else if (rdone < rsize) {
					/* i == 'R' */
					
					/*
					 * Always take 32 bits worth of mask
					 * per -R.  This assumes __INT_BITS__
					 * is always a multiple of 32. ie.
					 * sizeof(unsigned) %
					 * sizeof(uint32_t) == 0.
					 */
					val &= 0xffffffff;
					val <<= 32 * rdone_32;
					runmask[rdone] |= val;
					if (++rdone_32 >= sizeof(unsigned) /
					    sizeof(uint32_t)) {
						rdone_32 = 0;
						rdone++;
					}

				}
				break;
			case 'i':	inherit = 1; break;
			case 'm':	if(!stricmp(optarg, "pid")) match[MATCH_NAME] = FALSE;
						else if(!stricmp(optarg, "name")) match[MATCH_PID] = FALSE;
						else {
							fprintf(stderr, "slay: Invalid match rule (-m)\n");
							exit(0);
						}
						break;
			case 'n':	node = optarg; break;
#else
			case 'n':	lval = strtol( optarg, &p, 10 );
						if ( *p != NULL || lval > 65535L || lval < 0L )	{
							fprintf(stderr, "slay: Invalid node id (-n)\n");
							exit(0);
							}
						node = (node_t)lval;
						break;
#endif
			case 'p':	epid = TRUE;			break;
			case 'q':	query = TRUE;			break;
			case 'Q':	quiet = TRUE;			break;
			case 't':	if( *optarg != '/')
						strcpy( ttynambuf, "/dev/" );
					if (strlen(ttynambuf) + strlen(optarg) >= sizeof(ttynambuf)) {
						fprintf(stderr, "slay: %s: ttyname too long\n", optarg);
						exit(0);
					}	
					strncat( ttynambuf, optarg, sizeof(ttynambuf) - strlen(ttynambuf) - 1);
					break;
			case 'u':	sigptr = lookup("CONT");break;
			case 'v':	verbose = TRUE;			break;
			case 'P':	lval = strtol( optarg, &p, 10 );
#ifndef __QNXNTO__
						if ( lval >= _NUM_PRIORITIES || lval < -1 )	{
							fprintf(stderr, "slay: Invalid priority (-P)\n");
							exit(0);
							}
#endif
						switch (*p) {
							case 'f': sched = SCHED_FIFO;	break;
							case 'o': sched = SCHED_OTHER;	break;
							case 'r': sched = SCHED_RR;		break;
						}
						prio = (int) lval;
						break;
			case 'S':	killson = FALSE;
						break;
			case 's':	sigptr = lookup(strupr(optarg));
						break;
			default:	if(isdigit(optopt)){
							long val = 0;
							val = strtol(&argv[optind - 1][1], NULL, 10);
							if(val)
								sigptr = lookup(&argv[optind - 1][1]);
							break;
						}
						fprintf(stderr, "slay: Invalid option or missing argument (-%c)\n", optopt);
						exit(0);
						break;
				}
			}

	if ( optind == argc )	{
		fprintf( stderr, "slay: No process name specified.\n");
		exit(0);
		}
#ifdef __QNXNTO__
	if (node != NULL && prepare_node(&node, &node_nd) == -1) {
		fprintf(stderr, "slay: Invalid node: %s: %s\n",
		    node, strerror(errno));
		exit(0);
	}
#endif

	if(sigptr == NULL  &&  prio == -1 
#ifdef __QNXNTO__
	&& runmask == NULL
#endif
	)
		sigptr = lookup("TERM");

	for( ; optind < argc; optind++ ) {
		memset( pids, 0, sizeof( pids ) );
		npids = find( node, argv[optind] );     	

		if(npids == 0  &&  !quiet) {
			if(!force) {
				fprintf( stderr, "slay: Unable to find process '%s'", argv[optind] );
				if ( ttynambuf[0] )
					fprintf(stderr, " on tty '%s'\n", ttynambuf );
				else
					fprintf(stderr, "\n");
				}
			}

		if ( epid ) {
			for (i=0;i<npids;++i) {
				fprintf(stdout,"%d\n",pids[i]);
			}
			exit( npids );
		}

		for( i = 0; i < npids; ++i ) {
			need_newline = FALSE;
			if((verbose || query || npids > 1)  &&  isatty(0)  &&  !quiet) {
				printf( "slay: %s %d on %s", names[i], pids[i], ttys[i]);
				need_newline = TRUE;
				}

			free( names[i] );
			free( ttys[i] );

			if((query || (npids > 1  &&  !force))  &&  isatty(0)  &&  !quiet) {
				printf( " (y/N)? " );
				fflush( stdout );
				if (yesno() != 'Y')
					continue;
				}
			else if(need_newline)
				printf("\n");

			if( sigptr) {
#ifdef __QNXNTO__
				status = SignalKill(node_nd, pids[i], tid, sigptr->signum, SI_USER, 0);
#else
				status = kill_remote( node, pids[i], sigptr->signum );
#endif
				if(status == 0)
					++tpids;
				if(!quiet  &&  status)
					printf("%skill(%d, %d) : %s\n",
							need_newline ? "      " : "",
							pids[i], sigptr->signum, strerror(errno));
				}

			if ( prio != -1 ) {
#ifdef __QNXNTO__
				status = setprio_remote( node, pids[i], tid, prio );
#else
				status = setprio_remote( node, pids[i], prio );
#endif
				if(status == 0)
					++tpids;
				if(!quiet  &&  status)
#ifndef __QNXNTO__
					printf("%ssetprio(%d, %d) : %s\n",
							need_newline ? "      " : "",
							pids[i], prio, strerror(errno));
#else
					printf("%ssetprio(nd=%s, pid=%d, tid=%d, prio=%d) : %s\n",
							need_newline ? "      " : "",
							node != NULL ? node : "local",
							pids[i], tid, prio, strerror(errno));
#endif
				}

#ifdef __QNXNTO__
			if (runmask != NULL) {
				int k;
				status = setrunmask_remote(node, pids[i], tid, runmask, inherit, rsize);
				if(status == 0)
					++tpids;
				if(!quiet  &&  status) {
					printf("%ssetrunmask_remote(nd=%s, pid=%d, tid=%d, runmask=",
							need_newline ? "      " : "",
							node != NULL ? node : "local",
							pids[i], tid);
					for (k = 0; k < rsize; k++)
						printf("%#x ", runmask[k]);
					printf(", inherit=%d) : %s\n", inherit, strerror(errno));
					}
				}
#endif
			}
		}

	return(npids);
	}

#ifdef __QNXNTO__

int find(node_t node, char *proc) {
	int					npids = 0;
	static DIR			*dp;
	struct dirent		*dirp;
	int					fd;
	procfs_info			pinfo;
	procfs_debuginfo	*dinfop;
	char				*mtty = NULL;
	char				*endp, *procname;
	union {
						procfs_debuginfo dbg;
						char	buf[200];
	} dbg_u;

	endp = NULL;
	if ((procname = prepare_path(&endp, node, "/proc")) == NULL)
		return 0;

	if((dp = opendir(procname)) == NULL)
		return(0);

	while((dirp = readdir(dp))) {
		if ((procname = prepare_path(&endp, NULL, "/%s", dirp->d_name)) == NULL)
			continue;

		if((fd = open(procname, O_RDONLY)) == -1)
			continue;

		dinfop = &dbg_u.dbg;
		if(devctl(fd, DCMD_PROC_INFO, &pinfo, sizeof(pinfo), 0) != EOK
		|| devctl(fd, DCMD_PROC_MAPDEBUG_BASE, dinfop, sizeof(dbg_u), 0) != EOK) {
			close(fd);
			continue;
			}
		close(fd);

		if(!((match[MATCH_NAME] && !strcmp(proc, tail(dinfop->path))) || (match[MATCH_PID] && !strcmp(proc, dirp->d_name))))
			continue;

		if ( ttynambuf[0] && ( 1 != ttymatch( ttynambuf, pinfo.sid ) ) )
			continue;

		if(killson == FALSE  &&  pinfo.child)
			continue;

		if(npids > MAX_PIDS) {
			fprintf( stderr, "slay: Too many pids, working with first %d processes found!\n", MAX_PIDS );
			break;
			}

		names[npids] = strdup(dinfop->path);
		ttys[npids] = strdup( mtty ? mtty : "(tty not known)");
		pids[npids] = pinfo.pid;
		++npids;
		}

	closedir(dp);
	return(npids);
	}




int setprio_remote(node_t node, pid_t pid, int tid, int prio) {
	struct sched_param param;
	char *path;
	procfs_status status;
	procfs_run run;
	int ret, fd, did_stop, found;

	if (node != NULL) {
		fprintf(stderr, "Setting remote priorities not supported\n");
		errno = ENOTSUP;
		return -1;
	}

	if (tid != 0) {
		if ((ret = SchedGet(pid, tid, &param)) == -1) {
			fprintf(stderr, "Unable to determine current "
			    "scheduling paramaters.\n");
		}
		else {
			param.sched_priority = prio;
			if ((ret = SchedSet(pid, tid, sched, &param)) != -1)
				ret = 0;
		}

		return ret;
	}

	/* Setting all threads in a process */
	if ((path = prepare_path(NULL, node, "/proc/%d/as", pid)) == NULL)
		return -1;

	did_stop = 0;
	if ((fd = open(path, O_RDWR)) == -1) {
		/*
		 * Only one O_RDWR at any point in time.  eg. someone might
		 * have it in the debugger.  Try O_RDONLY.
		 */
		if ((fd = open(path, O_RDONLY)) == -1)
			return -1;

	}
	else if (_devctl(fd, DCMD_PROC_STOP, NULL, 0,
	    _DEVCTL_FLAG_NORETVAL) != -1)
		did_stop = 1;

	if (did_stop == 0)
		fprintf(stderr, "Warning: unable to stop process %d prior to "
		    "setting prio (inherited value).  Continuing.\n", pid);

	for (found = 0, status.tid = 1;; status.tid++) {
		if ((ret = _devctl(fd, DCMD_PROC_TIDSTATUS, &status,
		    sizeof(status), _DEVCTL_FLAG_NORETVAL)) == -1) {
			if (errno == ESRCH && found)
				ret = 0;
			break;
		}

		if ((ret = SchedGet(pid, status.tid, &param)) == -1) {
			if (errno == ESRCH)
				continue;
			break;
		}

		param.sched_priority = prio;
		if ((ret = SchedSet(pid, status.tid, sched, &param)) == -1) {
			if (errno == ESRCH)
				continue;
			break;
		}
		found = 1;
	}

	if (did_stop != 0) {
		memset(&run, 0x00, sizeof(run));
		if (_devctl(fd, DCMD_PROC_RUN, &run, sizeof(run),
		    _DEVCTL_FLAG_NORETVAL) == -1) {
			/* Very bad */
			perror("process restart failed");
			ret = -1;
		}
	}
	
	close(fd);
	return ret;
}


int setrunmask_remote(node_t node, pid_t pid, int tid, int *runmask, int inherit, int rsize_unsigned) {
	char *path;
	procfs_threadctl *threadctl;
	procfs_status status;
	int fd, ret, found, cmd, *rsizep, rsize_char, size;
	unsigned *rmaskp, *imaskp;
	procfs_run run;

	/* How many chars in each mask */
	rsize_char = rsize_unsigned * sizeof(unsigned);
	size = offsetof(procfs_threadctl, data);
	/*
	 * Out of _NTO_TCTL_RUNMASK*, a straight _NTO_TCTL_RUNMASK
	 * works with the most old versions of proc.
	 */
	if (inherit == 0 && rsize_unsigned == 1) {
		cmd = _NTO_TCTL_RUNMASK;
		size += sizeof(unsigned); /* The mask */
	}
	else {
		cmd = _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT;
		size += sizeof(int);		/* rsize */
		size += rsize_char;		/* runmask */
		size += rsize_char;		/* inherit_mask */
	}

	if ((threadctl = alloca(size)) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	memset(threadctl, 0x00, size);
	threadctl->cmd = cmd;
	if (cmd == _NTO_TCTL_RUNMASK) {
		rmaskp = (unsigned *)threadctl->data;
		*rmaskp = *runmask;
		imaskp = NULL;
		rsizep = NULL;
	}
	else {
		rsizep = (int *)threadctl->data;
		*rsizep = rsize_unsigned;
		rmaskp = rsizep + 1;
		imaskp = rmaskp + rsize_unsigned;
		memcpy(rmaskp, runmask, rsize_char);
		if (inherit)
			memcpy(imaskp, runmask, rsize_char);
		else
			memset(imaskp, 0x00, rsize_char);
	}
		
	/*
	 * _NTO_TCTL_RUNMASK* always needs O_RDWR so we can't fall
	 * back to O_RDONLY like in setprio_remote().
	 */
	if ((path = prepare_path(NULL, node, "/proc/%d/as", pid)) == NULL ||
	    (fd = open(path, O_RDWR)) == -1)
		return -1;

	if (tid != 0) {
		/* Set the specified thread */
		threadctl->tid = tid;
		ret = _devctl(fd, DCMD_PROC_THREADCTL, threadctl,
		    size, _DEVCTL_FLAG_NORETVAL);
	}
	else if (inherit == 0 || (ret = _devctl(fd, DCMD_PROC_STOP, NULL, 0,
	    _DEVCTL_FLAG_NORETVAL)) != -1) {
		/*
		 * Set all threads.  DCMD_PROC_STOP issued for inherit
		 * above so that none are missed.
		 */
		for (found = 0, status.tid = 1;; status.tid++) {
			if ((ret = _devctl(fd, DCMD_PROC_TIDSTATUS, &status,
			    sizeof(status), _DEVCTL_FLAG_NORETVAL)) == -1) {
				if (errno == ESRCH && found)
					ret = 0;
				break;
			}
			threadctl->tid = status.tid;

			/*
			 * Ignore ESRCH as the thread in question may have
			 * exited since being found above.
			 */
			if ((ret = _devctl(fd, DCMD_PROC_THREADCTL, threadctl,
		    	    size, _DEVCTL_FLAG_NORETVAL)) != -1)
				found = 1;
			else if (errno != ESRCH)
				break;

			if (cmd == _NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT) {
				/* Reset since old masks are returned out */
				memcpy(rmaskp, runmask, rsize_char);
				if (inherit)
					memcpy(imaskp, runmask, rsize_char);
				else
					memset(imaskp, 0x00, rsize_char);
			}
		}

		if (inherit != 0) {
			memset(&run, 0x00, sizeof(run));
			if (_devctl(fd, DCMD_PROC_RUN, &run, sizeof(run),
			    _DEVCTL_FLAG_NORETVAL) == -1) {
				/* Very bad */
				perror("process restart failed");
				ret = -1;
			}
		}
	}

	close(fd);
	return ret;
}

char *
prepare_path(char **endp, node_t node, char *fmt, ...)
{
	int		off, lim, ret;
	char 		*p;
	va_list		ap;
	static char	buf[PATH_MAX];
	static int	max = PATH_MAX;
	/* PATH_MAX hardcoded for now.  Can move to realloc() if desired */

	if (endp != NULL)
		p = *endp;
	else
		p = NULL;

	/* Non NULL p on entry means append */
	if (p != NULL) {
		/* Append and prepend don't mix */
		if (node != NULL) {
			errno = EINVAL;
			return NULL;
		}
		off = p - buf;
		if ((unsigned)off >= max) {
			errno = EINVAL;
			return NULL;
		}
		lim = max - off;
	}
	else {
		p = buf;
		lim = max;
		if (node != NULL) {
			ret = snprintf(p, lim, "%s", node);
			if (ret >= lim) {
				/* Could realloc and start over */
				errno = ENAMETOOLONG;
				return NULL;
			}
			p += ret;
			lim -= ret;
		}
	}

	va_start(ap, fmt);
	ret = vsnprintf(p, lim, fmt, ap);
	va_end(ap);
	if (ret >= lim) {
		/* Could realloc and start over */
		errno = ENAMETOOLONG;
		return NULL;
	}
	/*
	 * If append, don't update *endp
	 * (XXX unless realloc() returns different base)
	 */
	if (endp != NULL && *endp == NULL)
		*endp = p + ret;

	return buf;

}

int
prepare_node(char **np, int *ndp)
{
	char	*node;
	int	len1, len2, node_nd;

	node = *np;
	len2 = -1;

	if ((node_nd = netmgr_strtond(node, NULL)) == -1 ||
	    (len1 = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_NAME_SHOW,
	    node_nd, NULL, 0)) == -1 ||
	    (node = malloc(len1)) == NULL ||
	    (len2 = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_NAME_SHOW,
	    node_nd, node, len1)) != len1) {
		if (node != *np)
			free(node); /* may be NULL */
		if (len2 != -1)
			errno = EINVAL; /* Something weird happened */
		return -1;
	}

	*np = node;
	*ndp = node_nd;
	return 0;
}

int ttymatch( char *dev, int sid ) {
	int  fd, tsid, r = 0;

	if ( -1 == ( fd = open( dev, O_RDONLY ) ) )
		return -1;

	r = ( -1 == ( tsid = tcgetsid( fd ) ) ) ? -1 : ( sid == tsid );
	close( fd );

	return r;
}

#else

int find( node_t node, char *proc ) {
	int		npids = 0;
	struct	_psinfo	psinfo;
	pid_t	pid, vid;
	char	*tail();
	char	*mtty, *cp;

	mtty = (char *)0;
	if((vid = qnx_vc_attach(node, PROC_PID, sizeof(struct _psinfo), 1)) == -1) {
		fprintf(stderr, "Unable to connect to node %ld.\n", node);
		return( 0 );
		}

	for( pid = 1 ; (pid = qnx_psinfo(vid, pid, &psinfo, 0, NULL)) != -1  ; ++pid ) {
		if ( psinfo.state == DEAD     ||
			 psinfo.flags & _PPF_MID  ||
			 psinfo.flags & _PPF_VID  ||
			 psinfo.flags & _PPF_VMID ) {
			continue;
			}

		if ( strcmp( proc, tail( psinfo.un.proc.name) )  )
			continue;

		mtty = ttymatch( vid, psinfo.sid );
		/* if we're matching ttys, try the full name, then skip the node */
		if(ttynambuf[0])
			if(mtty == NULL || (strcmp(mtty, ttynambuf) != 0 &&
			   ((cp = strchr(mtty + 2, '/')) == NULL || strcmp(cp, ttynambuf) != 0)))
				continue;

		if ( killson == FALSE  &&  psinfo.un.proc.son  )
			continue;

		if ( npids > MAX_PIDS ) {
			fprintf( stderr, "slay: Too many pids, working with first %d processes found!\n", MAX_PIDS );
			break;
			}

		names[npids] = strdup( psinfo.un.proc.name );
		ttys[npids] = strdup( mtty ? mtty : "(tty not known)");
		pids[npids++] = pid;
		}

	qnx_vc_detach( vid );

	return( npids );
	}


int	kill_remote ( node_t node, pid_t pid, int signum ) {
	pid_t	vid;
	int		status;

	if((vid = qnx_vc_attach( node, pid, 0, 0 )) == -1)
		return(-1);

	status = kill( vid, signum );
	if (signum==NULL && status!=0 && errno==EPERM) {
		/* in slay, signal of NULL is used only to test process existence,
           never to check permission */
		status=0;
	}
	qnx_vc_detach( vid );

	return( status );	
	}


int setprio_remote ( node_t node, pid_t pid, int prio ) {
	pid_t	vid;
	int		status;

	if((vid = qnx_vc_attach( node, PROC_PID, 0, 0 )) == -1)
		return(-1);

	status = qnx_scheduler( vid, pid, sched, prio, 0 );
	qnx_vc_detach( vid );

	return( status == -1 ? -1 : 0);
	}


char *ttymatch ( pid_t proc_pid, int sid ) {
	static struct _sidinfo sid_data;

	if (qnx_sid_query(proc_pid, sid, &sid_data) != -1)
		return(sid_data.tty_name);
	
	return(NULL);
	}

#endif


char *
tail ( name )
char *name;
	{
	register char *p;
	
	p = name + strlen(name);
	while(p > name  &&  *p != '/' && *p != '\\' )	--p;

	return( (*p == '/' || *p == '\\') ? p + 1 : p );
	}
	
	
struct sig_entry *
lookup(signame)
char *signame;
	{
	struct sig_entry	*sp;
	char				*name;
	int					 num;

	if(isdigit(*signame)) {
		num = atoi(signame);

		for( sp = &sigtable[0] ; sp->signame ; ++sp )
			if( sp->signum == num )
				return(sp);
		}
	else {
		if ( strncmp( name = signame, "SIG", 3 ) == 0 )
			name += 3;

		for( sp = &sigtable[0] ; sp->signame ; ++sp )
			if( strcmp( sp->signame, name ) == 0 )
				return(sp);
		}

	fprintf(stderr, "slay: Signal %s does not exist.\n", signame);

	exit(0);
	return(NULL);
	}


int yesno() {	int c;

	_raw();
	c = toupper(getchar());
	putchar((c == 'Y') ? 'Y' : 'N');
	_unraw();
	printf("\n");

	return(c);
	}


struct termios	termsave;

void _raw() {
	struct termios termbuf;

	tcgetattr(0, &termbuf);
	termsave = termbuf;
	termbuf.c_iflag = termbuf.c_oflag = termbuf.c_lflag = 0;
	tcsetattr(0, TCSANOW, &termbuf);
	}


void _unraw() {

	tcsetattr(0, TCSANOW, &termsave);
	}

__SRCVERSION("slay.c $Rev: 154696 $");
