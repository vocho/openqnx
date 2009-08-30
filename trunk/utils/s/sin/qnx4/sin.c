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
%C	[options]... [command]

Options:
 -g groupname    Get information on this groupname only.
 -h              Supress the printing of headers.
 -l              Keep sin vc requests local.
 -m              Get information on current session (me).
 -n node         Get information from this node.
 -p pid          Get information on this process id only.
 -P program      Get information on this program only.
 -s sid          Get information on this session only.
 -S {bginpstuU}  Sort on this format_str letter.
 -u username     Get information on this username only.
 -U              Display userid for sin files.

Commands (you may abbreviate to the first 2 characters):
 args      fds            freemem   info     net         rtimers    tree
 dir       files          gdt       irqs     proxies     sessions   users   
 env       flags          gnames    memory   registers   signals    vcs     
 family    format {fmt}   idt       names    root        times      versions

Note:
 {fmt} is any combination of the following characters:
 a..Arguments    F..Flags       m..Size       R..Root          u..Eff. UID/GID
 b..Blocked      g..Group       n..Name       s..Session ID    U..Real UID/GID
 d..Directory    i..Process ID  P..Mpass      S..Signals       
 e..Environment  L..Logname     p..Priority   t..CPU time
 f..Family       M..Magic       r..Registers  T..File/Start time
#endif

/*---------------------------------------------------------------------



	$Id: sin.c 153052 2008-08-13 01:17:50Z coreos $

	$Log$
	Revision 1.4  2005/06/03 01:38:00  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.3  1999/06/07 21:12:01  steve
	Attempt #3 to get an initial working setup....
	
	Revision 1.85  1999/04/20 15:21:29  jun
	"sin fds" now supports UNIX domain sockets, Routing sockets, as well
	as INET domain sockets.

	Revision 1.84  1999/02/08 20:34:16  eric
	blat

	Revision 1.83  1998/04/23 18:50:26  eric
	> Steve McPolin wrote:
	> >
	> > I changed the print_freemem() to handle a: a new option from proc to
	> > announce when there are too many items to print properly, and
	> > b: to print the proper error message.

	Revision 1.82  1997/07/14 12:48:22  steve
	tentative; uses zero fields iff nonzero for private memory count.

	Revision 1.81  1997/02/17 18:24:19  dabrown
	changed sin fd to print out the proper laddr and faddr was printing
	laddr for both when there was a tcpip connection

	Revision 1.80  1997/01/06 20:25:49  rajackson
	changed 'sin mem' so that columns line up.

	Revision 1.79  1996/12/13 14:23:22  dabrown
	added space between name and pid in sin fd output

	Revision 1.78  1996/12/12 21:33:11  eric
	added diagnostics for 'sin net'; disabled by default
	To compile-in, define 'DIAG'.

	Revision 1.77  1996/11/14 18:32:53  dabrown
	removed some unreferenced variables

	Revision 1.76  1996/11/14 17:42:34  dabrown
	Added sin fd output for sockets and VC's with local server fd open

	Revision 1.75  1996/09/18 15:55:07  eric
	would sigsegv when printing some process names which started
	with a single slash (seems to be lengthy names).

	Revision 1.74  1996/07/03 15:49:32  eric
	fix for sin -n nid gdt/idt in 32-bit version of sin.

	Revision 1.73  1995/12/11 17:06:27  eric
	fixed problem when sin -n nid net would SIGSEGV because the
	max_nodes was being mistaken for end_nid..

 * Revision 1.72  1995/12/11  16:23:49  eric
 * added TCP detection for sin net
 *
 * Revision 1.71  1995/11/23  19:29:14  eric
 * *** empty log message ***
 *
 * Revision 1.70  1995/11/23  15:42:58  eric
 * added detection of cd-rom, tape, removable hard disk for
 * sin net.
 *
 * Revision 1.68  1995/11/21  20:54:22  eric
 * modified a comment which suggested an optimization which
 * would be too hairy to implement..
 *
 * Revision 1.67  1995/11/21  20:22:11  eric
 * sin net now will count any hard device at 'dev/hdNNNN', as
 * well as the device at fd0 if it exists. It checks the prefix
 * list before stating the device. Only if it is in the prefix
 * list is the stat attempted. This nearly doubles the speed of
 * sin net.
 *
 * Revision 1.66  1995/11/14  20:15:35  garry
 * accepts ENOTSUP, EOPNOTSUP for fsysinfo call...
 *
 * Revision 1.65  1995/10/10  14:42:53  steve
 * made utils header files go through stdutils.h
 * removed 16bit ism from print_irqs
 *
 * Revision 1.64  1995/06/07  01:30:16  dtdodge
 * Fixed "sin net" to print memsize correctly.
 *
 * Revision 1.63  1995/04/19  17:00:00  brianc
 * Don't query held servers for versions
 *
 * Revision 1.62  1995/04/18  15:39:12  dtdodge
 * Checked for a divide by zero.
 *
 * Revision 1.61  1995/01/11  20:00:04  garry
 * The X servers can put bad data in fddata (non-existant pid)
 * which causes the vc_attach to fail
 * which causes fsys_fdinfo to fail with ESRCH on -1...
 *
 * Revision 1.60  1995/01/09  16:00:51  miket
 * added a small check in print_open(): sin files now displays open files
 * that are open by a remote shell
 *
 * Revision 1.59  1995/01/03  14:26:46  miket
 * added qnx_vc_detach() in str_fname() to prevent using all proc entries
 *     "
 *
 * Revision 1.58  1994/12/01  20:49:40  miket
 * simple change to print_proc_header to set sp="" after ':' in fmtstr
 * to keep PID from being printed twice with fds and mem commands
 *
 * Revision 1.57  1994/10/18  14:01:48  dtdodge
 * Added a new system flag for _PSF_PCI_BIOS.
 *
 * Revision 1.56  1994/09/23  13:33:00  garry
 * int10 returns EINVAL to fsys_fdinfo() causes infinite loop in printing sin files
 *
 * Revision 1.55  1994/09/09  14:15:46  dtdodge
 * Masked registers displayed via "sin reg" so they don't get sign extended
 * and mess up the display.
 *
 * Revision 1.54  1994/08/11  19:26:28  dtdodge
 * Fixed a typo which will not effect the code produced but was
 * incorrect.
 *
 * Revision 1.53  1994/08/10  19:33:24  peterv
 * Added flag characters for XIP & APM & rom booting
 *
 * Revision 1.52  1994/08/03  01:38:15  dtdodge
 * Fixed (with some help in Proc32) to print out the size of code and data
 * for flat model programs. If multiple programs shared code then the size
 * of the code is divided by the number of programs.
 *
 * Revision 1.51  1994/07/11  15:38:46  steve
 * forgot to check in.  Dan added something about semaphores....
 *
 * Revision 1.50  1994/06/14  13:51:04  steve
 * changed to ensure PPF_FLAT was set before subtracting sp
 *
 * Revision 1.49  1994/06/08  15:21:17  steve
 * Adjusted size of 32bit flat programs with a borrowed code segment by
 * subtracting the current value of the stack pointer from the data segment
 * size.
 *
 * Revision 1.47  1994/05/09  13:45:45  brianc
 * Allow huge process times (clock_t is unsigned long; time_t is signed long)
 *
 * Revision 1.46  1994/03/29  15:47:50  brianc
 * Added title for format R
 *
 * Revision 1.45  1994/02/15  18:15:52  waflowers
 * Fixed "sin fi" so it won't spit out wrong info over, and over, and over.
 *
 * Revision 1.44  1993/11/24  15:39:22  dtdodge
 * Fixed a SIGSEGV on  sin -Un61 fi
 * If the remote owner was not knowed it tried to use a null pointer.
 *
 * Revision 1.43  1993/11/15  22:57:51  peterv
 * Added "APM" flag to "sin in"
 *
 * Revision 1.42  1993/09/20  14:11:02  brianc
 * Now supports -P option with proxies and vcs
 *
 * Revision 1.41  1993/08/13  14:21:51  brianc
 * Report errno on failures
 *
 * Revision 1.40  1993/07/30  17:09:48  dtdodge
 * Changed str_nbytes to create a 6 character field for more precision
 * in printing sizes.
 *
 * Revision 1.39  1993/03/18  16:49:05  brianc
 * Display proxy count as unsigned
 * Use debug_xfer() to display IDT or GDT on remote node
 *
 * Revision 1.38  1993/03/12  17:26:09  waflowers
 * Bug in print_open prevented 'sin fi' from printing all files open.
 *
 * Revision 1.37  1993/02/09  14:42:30  dtdodge
 * {aboyd} : added nice strings for output of sin gdt
 *
 * Revision 1.36  1992/12/03  20:40:58  dtdodge
 * aboyd checking in sin for dtdodge.
 * The executable version for this is in //61/bin
 *
 * Revision 1.35  1992/11/05  18:27:02  dtdodge
 * Finished the work on sin files.
 *
 * Revision 1.34  1992/10/29  19:57:45  peterv
 * More efficient "sin tree" stuff.
 *
 * Revision 1.33  1992/10/29  18:38:28  peterv
 * Added ansi terminal support to "sin tree"
 *
 * Revision 1.32  1992/10/29  16:50:34  dtdodge
 * Added a  sin files  sink.
 *
 * Revision 1.30  1992/09/30  16:22:26  dtdodge
 * Added sin gnames
 *
 * Revision 1.29  1992/09/16  20:14:14  dtdodge
 * Misc changes for 32 bit.
 *
 * Revision 1.28  1992/07/08  16:59:19  dtdodge
 * *** empty log message ***
 *
 * Revision 1.27  1992/05/29  15:38:20  dtdodge
 * Extended offset on sig sig to 6 digits for 32 bit.
 *
 * Revision 1.26  1992/05/26  19:37:54  dtdodge
 * -h now supresses the header on  sin proxy
 *
 * Revision 1.25  1992/05/06  19:18:47  dtdodge
 * (eric) fixed insufficient memory to sin problem with sin -n x ver.
 * sin ver was trying to look at _all_ process entry slots instead of
 * skipping to the next one in use. This blew its data segment. So
 * I changed this, plus reduced the stack to 2k to make room for a
 * few more processes that sin can look at (it was blowing up at
 * the 198th of 200).
 *
 * Revision 1.24  1992/04/20  15:45:24  dtdodge
 * Adjusted fudge factors so it would print out new DOS ticksize of 54.9msec.
 *
 * Revision 1.23  1992/04/15  14:04:29  dtdodge
 * sin args/env now prints ... to terminate very longs lists. Used to
 * print garbage.
 *
 * Revision 1.22  1992/03/28  06:35:44  brianc
 * Fixed spelling mistake
 *
 * Revision 1.21  1992/03/25  19:34:53  dtdodge
 * Fixed usage to agree with docs.
 *
 * Revision 1.20  1992/03/23  21:35:01  dtdodge
 * sin fds now prints  //nid (pid)  instead of unknown.
 *
 * Revision 1.19  1992/03/14  16:36:53  dtdodge
 * Now prints .5 msec ticksize properly.
 *
 * Revision 1.18  1992/02/23  18:47:49  dtdodge
 * sin proxy
 *   Now prints the proxies priority.
 *
 * Revision 1.17  1992/02/18  17:48:20  dtdodge
 * sin fds now prints out device classes as //nid [clasname]
 *
 * Revision 1.16  1992/02/13  17:47:40  dtdodge
 * Now prints size of raw hard drives.
 *
 * Revision 1.15  1992/02/12  20:50:24  dtdodge
 * Now prints fd and ldt heap space with sin info.
 *
 * Revision 1.14  1992/01/30  16:02:30  root
 * Checks for super use on sin idt/gdt.
 *
 * Revision 1.13  1992/01/23  15:36:20  dtdodge
 * Added a flags display to sin net.
 *
 * Revision 1.12  1992/01/23  14:14:51  root
 * Added a new flag to show you are running the 32 bit version.
 * sin info
 *
 * Revision 1.11  1991/12/11  03:27:11  dtdodge
 * Fixed a bug where  sin rt  would print non-existant timers pending.
 *
 * Revision 1.9  1991/11/01  21:37:41  eric
 * *** empty log message ***
 *
 * Revision 1.8  1991/10/24  19:53:08  waflowers
 * Don't add size of /dev/fd1 to /dev/fd0 when reporting 'sin net'.
 *
 * Revision 1.7  1991/10/10  21:36:19  brianc
 * Modified sin vc to display "proxy" for vids to proxies
 *
 * Revision 1.6  1991/10/10  21:14:33  waflowers
 * Fixed 'sin net' so even non-super users can get disk size info.
 *
 * Revision 1.5  1991/10/10  13:31:06  brianc
 * Added 'e' to format_str for showing env (oops)
 * Added space between locators list
 *
 * Revision 1.4  1991/09/27  15:18:18  brianc
 * Added header for environment format
 *
 * Revision 1.3  1991/09/26  16:01:23  brianc
 * Added env command for showing process environments
 *
 * Revision 1.2  1991/09/26  12:48:14  dtdodge
 * WAIT_BLOCKED now prints out blocked on pids.
 * Increased treeline to 512 so we could read 512 bytes of env vars
 * out of a process.
 *
 * Revision 1.1  1991/07/10  17:21:02  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <pwd.h>
#include <i86.h>
#include <grp.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include <sys/debug.h>
#include <sys/dev.h>
#include <sys/fd.h>
#include <sys/irqinfo.h>
#include <sys/kernel.h>
#include <sys/magic.h>
#include <sys/name.h>
#define _NAMELOC_NODES_LEN 10
#define _NAMELOC_QUERY_LEN	1
#include <sys/_nameloc.h>
#include <sys/osinfo.h>
#include <sys/proc_msg.h>
#include <sys/psinfo.h>
#include <sys/seginfo.h>
#include <sys/sidinfo.h>
#include <sys/stat.h>
#include <sys/sys_msg.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/vc.h>
#include <sys/fsys_msg.h>
#include <sys/fsys.h>
#include <sys/prfx.h>
#include <sys/disk.h>
#include <sys/io_msg.h>
#include <sys/sock_msg.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>


//#include "stdutil.h"
#include "qnx_term.h"

#define PINDEX(pid)		((pid) & osdata.pidmask)

char *format_str = "abdefFgilLmMnNpPrRsStTuU";

/*
 * Some simple protos to make the compiler happy.
 */
extern struct _psinfo	*get_proc(), *get_remproc();
extern struct _irqinfo	*get_irq();
extern struct _nameinfo	*get_name();
extern struct _sidinfo	*get_session();

extern char *str_flags(), *str_cmd(), *str_nbytes(), *str_state(), *str_fname();
extern char *make_task_header(), *str_addr(), *trim_spaces(), *str_time(time_t);
extern char *str_seconds(clock_t), *str_path(nid_t, char *, int);
extern void *salloc();
extern unsigned long disk_size(nid_t, char *, unsigned long *);
extern long number();
extern void get_disk_space(nid_t, unsigned long *, unsigned long *, unsigned long *);
extern int cmp();
extern void print_proc(), print_open();
int qnx_timer_query(pid_t, timer_t, struct _itimerspec *);
nid_t qnx_nameloc_query(pid_t, nid_t, int, char *);
int get_disk_entry (struct _disk_entry *, char *);
node(nid_t nid, nid_t max_nodes);

/*New functions to print Socket info*/

int print_socket_info(pid_t pid, int fd);
void print_state (int state);
int print_sock_info(struct _socket_fdinfo_reply fdinfo);
int get_sock_info(int pid, int fd, int server_pid, struct _socket_fdinfo_reply *fdinfo);

/*
 * Some data strings.
 */
char const	*schedstrs[4] ={ "fifo", "robin", "other", "????" };
char const	*monitorstrs[] ={
			"None",
			"Mono",
			"CGA",
			"Unknown",
			"EGA Color",
			"EGA Mono",
			"PGS",
			"VGA Mono",
			"VGA Color",
			"Unknown",
			"Unknown",
			"PS/2 30 M",
			"PS/2 30 C",
			"Unknown",
			"Unknown",
			"Unknown"
			} ;



/*
 * Option vars
 */
nid_t	nid;		/* Target nid					*/
pid_t	vid;		/* Vid to proc on target nid	*/
int		sid = -1; 	/* Session id to match			*/
char	*fmtstr = "sinpbm";
char	*program;
int		header = 1, physical, hflag, lflag, mflag, uflag, gflag, Uflag;
int		last_sid, last_nameid, last_irqid;
int		sort_letter = 'i';
int		secret;
uid_t	uid;
gid_t	gid;
void	(*print_func)() = &print_proc;
long	skip_mask = _PPF_VID | _PPF_MID;


/*
 * Buffers.
 */
struct _osinfo		 osdata;
struct _psinfo		*(*proc_table)[], *(*sort_table)[];
struct _nameinfo	*(*name_table)[];
struct _irqinfo		*(*irq_table)[];
struct _sidinfo		*(*session_table)[];
char				treeline[512], treeflag;


struct _psinfo			psdata;
struct _seginfo			segdata[16];
struct _reg_struct		reg;
struct _fd_entry		fddata;
struct _fsys_info_entry fsysdata;
struct _totals {
	long	speed,
			memk,
			diskm;
	} 				netdata;

/*
 * Same as itimerspec with a pid added at the end.
 */
struct _itimerspec {
    struct timespec it_value,
                    it_interval;
    int             notify_type,
                    timer_type;
    long            data;           /* Used by gettimer only */
	pid_t			pid;			/* Used by gettimer only */
    } ;


debug_read(pid_t vid, struct _psinfo *proc, char *buf, unsigned maxbytes,
           short unsigned segment, long unsigned off);

#ifdef DIAG
#define DELTA (Start_Time==0)?(Start_Time=time(NULL)):(time(NULL)-Start_Time)
time_t Start_Time=0;

//#nclude "qnx_name_locate2.c"
//#define qnx_name_locate qnx_name_locate2
#endif

main(argc, argv)
int argc;
char *argv[];
	{
	struct _psinfo	*pp;
	struct group	*grp;
	struct passwd	*pwp;
	unsigned type, opt, i, n;
	pid_t pid = 0;

	if(need_usage(argv))
		print_usage(argv);

	while ((opt = getopt(argc, argv, "g:hlmn:p:P:s:S:u:U\\")) != -1) {
		switch(opt) {
		case 'g':
			gflag = 1;
			if(*optarg >= '0'  &&  *optarg <= '9')
				gid = number("-g", optarg);
			else if(grp = getgrnam(optarg))
				gid = grp->gr_gid;
			else {
				fprintf(stderr, "sin: unknown group name.\n");
				exit(EXIT_FAILURE);
				}
			break;

		case 'h':
			hflag = 1;
			break;

		case 'l':
			lflag = 1;
			break;

		case 'm':
			mflag = 1;
			break;

		case 'n':
			nid = qnx_strtonid(optarg, NULL);
			break;

		case 'p':
			pid = (pid_t) number("-p", optarg);
			break;

		case 'P':
			program = optarg;
			break;

		case 's':
			sid = (pid_t) number("-s", optarg);
			break;

		case 'S':
			sort_letter = *optarg;
			break;

		case 'u':
			uflag = 1;
			if(*optarg >= '0'  &&  *optarg <= '9')
				uid = number("-u", optarg);
			else if(pwp = getpwnam(optarg))
				uid = pwp->pw_uid;
			else {
				fprintf(stderr, "sin: unknown group name.\n");
				exit(EXIT_FAILURE);
				}
			break;

		case 'U':
			Uflag = 1;
			break;

		case '\\':
			secret = 1;
			break;

		default:
			exit(EXIT_FAILURE);
			}
		}

	if(optind == argc)
		type = 'xx';
	else {
		type = (argv[optind][0] << 8) | argv[optind][1] | 0x2020;
		if(argc != optind + (type == 'fo' ? 2 : 1)) {
			fprintf(stderr, "sin: incorrect number of operands.\n");
			exit(EXIT_FAILURE);
			}
		}

	if((vid = qnx_vc_attach(nid, PROC_PID, sizeof(psdata), 1)) == -1) {
		fprintf(stderr, "sin: unable to connect to node %ld (%s).\n", nid, strerror(errno));
		exit(EXIT_FAILURE);
		}

	if(qnx_osinfo(nid, &osdata) == -1) {
		fprintf(stderr, "sin: unable to connect to node %ld (%s).\n", nid, strerror(errno));
		exit(EXIT_FAILURE);
		}

	if(osdata.pidmask == 0) osdata.pidmask = 0xff;

	if(mflag) {
		if(vid != PROC_PID) {
			fprintf(stderr, "sin: you cannot use -m with a remote node request.\n");
			exit(EXIT_FAILURE);
			}
		sid = get_proc(getpid())->sid;
		}

	switch(type) {
	case 'fr':
		print_freemem();
		break;

	case 'gd':
		print_gdtidt(1);
		break;

	case 'gn':
		print_gnames();
		break;

	case 'id':
		print_gdtidt(0);
		break;

	case 'in':
		print_info();
		break;

	case 'ir':
		print_irqs();
		break;

	case 'na':
		print_names();
		break;

	case 'ne':
		print_net();
		break;

	case 'pr':
		print_proxies();
		break;

	case 'se':
		print_session();
		break;

	case 'rt':
		print_timers();
		break;

	case 'tr':
		print_tree(PROC_PID, 0);
		fix_treeline(treeline);
		if(treeflag == 0) {
			qnx_term_fputs(treeline, stdout);
			fputc('\n', stdout);
			}
		break;

	case 'vc':
		print_vcs();
		break;

	case 've':
		print_versions();
		break;


	case 'ar':
		fmtstr = "iLa";
		goto xx;
	case 'di':
		fmtstr = "iLnd";
		goto xx;
	case 'en':
		fmtstr = "iLne";
		goto xx;
	case 'fa':
		fmtstr = "snpbif";
		goto xx;
	case 'fd':
		fmtstr = "v:f";
								/*Get VC also for Server process fd's*/	
		skip_mask = _PPF_MID;
		goto xx;
	case 'fi':
		header = 0;
		skip_mask = _PPF_MID;
		print_func = &print_open;
		goto xx;
	case 'fl':
		fmtstr = "sinbF";
		goto xx;
	case 'fo':
		for(fmtstr = argv[optind+1] ; *fmtstr ; ++fmtstr)
			if(strchr(format_str, *fmtstr) == NULL) {
				fprintf(stderr, "sin: invalid format option '%c'.\n", *fmtstr);
				exit(EXIT_FAILURE);
				}
		fmtstr = argv[optind+1];
		goto xx;
	case 'me':
		fmtstr = "ni:m";
		goto xx;
	case 're':
		fmtstr = "ir";
		goto xx;
	case 'ro':
		fmtstr = "iLnR";
		goto xx;
	case 'si':
		fmtstr = "sinpS";
		goto xx;
	case 'ti':
		fmtstr = "sinpt";
		goto xx;
	case 'us':
		fmtstr = "signUu";
		goto xx;
	case 'xx':
	xx:
		if(header) {
			print_proc_header();
			if((type == 'xx')&&!(pid||(sid!=-1)||program||uflag||gflag))
				printf(" --    -- Microkernel             --- -----   ---  %5u      0\n",
						osdata.microkernel_size);
			}

		for(i = 1, n=0 ; pp = get_proc(i) ; i = PINDEX(pp->pid) + 1) {
			if(pp->flags & skip_mask)
				continue;
			if(pid != 0  &&  pid != pp->pid)
				continue;
			if(sid != -1  &&  sid != pp->sid)
				continue;
			if(gflag  &&  gid != pp->rgid)
				continue;
			if(uflag  &&  uid != pp->ruid)
				continue;
			if(program  &&  strstr(pp->un.proc.name, program) == 0)
				continue;
			(*sort_table)[n++] = pp;
			}

		if (n) {
			if(sort_letter)
				qsort(sort_table, n, sizeof(struct _psinfo *), &cmp);
	
			for(i = 0 ; i < n ; ++i)
				(*print_func)((*sort_table)[i]);
	
		} else {
			fprintf(stderr,"sin: no process(es) found.\n");
			}

		break;

	case 'to':
		if (!strcmp(argv[optind],"toomuch")) {
			printf("You must be punished for your sins my son.\n");
			printf("rm * (Y/n)? ");
			if(getchar() != 'n')
				printf("Not only are you a sinner, you are a stupid sinner!\n");
			printf("You are forgiven my son.  This time...\n");
			break;
		}

	default:
		fprintf(stderr, "sin: unknown command specified.\n");
		exit(EXIT_FAILURE);
		}

	exit(EXIT_SUCCESS);
	}


void
print_proc(proc)
struct _psinfo *proc;
	{
	char *cp;
	char *space = "";
	int i, j, si, fd;
	long code_bytes, data_bytes, shared_bytes, dword;
	pid_t remote_proc;
	struct _psinfo psinfo;
	struct _psinfo psinfo2;
	struct _fd_entry fdbuf;

	for(cp = fmtstr ; *cp  &&  *cp != ':' ; ++cp) {
		printf("%s", space);
		space = " ";

		switch(*cp) {
		case 'a':
			if(__qnx_debug_xfer(vid, proc->pid, _DEBUG_MEM_RD,
				&dword, sizeof(dword),
				(unsigned)proc->magic_off + offsetof(struct _magic, sptrs[2]),
				proc->magic_sel) != -1
			&& dword
			&& debug_read(vid, proc, treeline, sizeof(treeline),0, dword) != -1)
				print_args((struct _proc_spawn *)treeline);
			else
				printf("%-32s", "Not available.");
			break;

		case 'b':
			printf("%5s", str_state(proc->state));
			if(proc->state == STATE_SEND_BLOCKED
			|| proc->state == STATE_RECEIVE_BLOCKED
			|| proc->state == STATE_REPLY_BLOCKED
			|| proc->state == STATE_SIGNAL_BLOCKED
			|| proc->state == STATE_WAIT_BLOCKED
			|| proc->state == STATE_SEM_BLOCKED
			|| proc->state == STATE_DEAD)
				printf(" %5d", proc->blocked_on);
			else
				printf("   ---");
			break;

		case 'd':
			if(__qnx_debug_xfer(vid, proc->pid, _DEBUG_MEM_RD,
			   &dword, sizeof(dword),
			   (unsigned)proc->magic_off + offsetof(struct _magic, sptrs[3]),
			   proc->magic_sel) != -1
			&& dword
			&& debug_read(vid, proc, treeline, sizeof(treeline),0, dword) != -1)
				printf("%-32s", treeline);
			else
				printf("%-32s", "Not available.");
			break;

		case 'e':
			if(__qnx_debug_xfer(vid, proc->pid, _DEBUG_MEM_RD,
				&dword, sizeof(dword),
				(unsigned)proc->magic_off + offsetof(struct _magic, sptrs[2]),
				proc->magic_sel) != -1
			&& dword
			&& debug_read(vid, proc, treeline, sizeof(treeline),0, dword) != -1)
				print_env((struct _proc_spawn *)treeline);
			else
				printf("%-32s", "Not available.");
			break;

		case 'f':
			printf(proc->un.proc.father  ? " %5d" : "   ---", proc->un.proc.father);
			printf(proc->un.proc.son	 ? " %5d" : "   ---", proc->un.proc.son);
			printf(proc->un.proc.brother ? " %5d" : "   ---", proc->un.proc.brother);
			break;

		case 'F':
			printf("%s",
					str_flags(proc->flags, "-fSZHTLE-mMVXF3------DSmcsfr-FiI"));
			break;

		case 'g':
			printf("%5d", proc->pid_group);
			break;

		case 'i':
			printf("%5d", proc->pid);
			break;

		case 'l':
			printf("%4d", proc->un.proc.links);
			break;

		case 'L':
			printf("%-12.12s", get_session(proc->sid)->name);
			break;

		case 'm':
			code_bytes = shared_bytes = 0;
#define ps_dpages(p) (*(ulong_t *)(p)->zero1)
#define ps_cpages(p) (*(ulong_t *)(p)->un.proc.zero2)
			if (data_bytes = ps_dpages(proc)*4096) {
				code_bytes = ps_cpages(proc)*4096;
			} else for(si = 0 ; qnx_psinfo(vid, proc->pid, &psdata, si, &segdata[0]) == proc->pid ; si += 16) {
				if(psdata.un.proc.nselectors == 0)
					break;
				for(i = 0 ; i < psdata.un.proc.nselectors ; ++i)
					if(segdata[i].flags & _PMF_INUSE) {
						if((segdata[i].flags & _PMF_BORROWED) == 0)
							continue;
						if((segdata[i].flags & (_PMF_SHARED|_PMF_LINKED)) == (_PMF_SHARED|_PMF_LINKED))
							shared_bytes += segdata[i].nbytes;
						else if(segdata[i].flags & _PMF_CODE_RX)
							code_bytes += segdata[i].nbytes;
						else
							data_bytes += segdata[i].nbytes;
					}
			}

			if(proc->flags & _PPF_FLAT)
				code_bytes /= proc->un.proc.links ? proc->un.proc.links : 1;

			printf("%s", str_nbytes(code_bytes, 100));
			printf(" %s", str_nbytes(data_bytes, 100));
			break;

		case 'M':
			printf("%4.4X:%7.7lX", proc->magic_sel, proc->magic_off);
			break;

		case 'n':
			printf("%-23s", str_path(0L, proc->un.proc.name, 23));
			break;

		case 'N':
			printf("%5ld", proc->sid_nid);
			break;

		case 'p':
			printf("%2d%c", proc->priority,
							 schedstrs[proc->sched_algorithm & 0x3][0]);
			break;

		case 'P':
			printf("%5d %4.4X %4.4X",
					proc->un.proc.mpass_pid,
					proc->un.proc.mpass_sel,
					proc->un.proc.mpass_flags);
			break;

		case 'r':
			#define MASK(n) ((n) & 0xffff)
			if(__qnx_debug_xfer(vid, proc->pid, _DEBUG_REG_RD, &reg, sizeof(reg), 0, 0) != -1) {
				printf("%4.4lX:%4.4lX %4.4lX:%4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX",
				MASK(reg.cs), MASK(reg.ip), MASK(reg.ss), MASK(reg.sp), MASK(reg.ds), MASK(reg.es),
				MASK(reg.ax), MASK(reg.bx), MASK(reg.cx), MASK(reg.dx), MASK(reg.si), MASK(reg.di), MASK(reg.bp), MASK(reg.fl));
				if(proc->flags & _PPF_32BIT)
					printf("\n           %4.4lX      %4.4lX           %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX %4.4lX",
					MASK(reg.ip>>16), MASK(reg.sp>>16),
					MASK(reg.ax>>16), MASK(reg.bx>>16), MASK(reg.cx>>16), MASK(reg.dx>>16), MASK(reg.si>>16), MASK(reg.di>>16), MASK(reg.bp>>16), MASK(reg.fl>>16));
				}
			else
				printf("%-69s", "Not available.");
			break;

		case 'R':
			if(__qnx_debug_xfer(vid, proc->pid, _DEBUG_MEM_RD,
			   &dword, sizeof(dword),
			   (unsigned)proc->magic_off + offsetof(struct _magic, sptrs[4]),
			   proc->magic_sel) != -1
			&& dword
			&& debug_read(vid, proc, treeline, sizeof(treeline),0, dword) != -1)
				if(geteuid() != 0  &&  *treeline == '-')
					printf("//%ld", nid ? nid : getnid());
				else
					printf("%-32s", treeline);
			else
				printf("%-32s", "Not available.");
			break;

		case 's':
			printf("%3d", proc->sid);
			break;

		case 'S':
			printf("%8.8lX %8.8lX %8.8lX %4.4X:%6.6lX",
					proc->signal_ignore,
					proc->signal_mask,
					proc->signal_pending,
					proc->signal_sel,
					proc->signal_off);
			break;

		case 't':
			printf("%s ",  str_time(proc->un.proc.start_time));
			printf("%6s ", str_seconds(proc->un.proc.times.tms_utime));
			printf("%6s ", str_seconds(proc->un.proc.times.tms_stime));
			printf("%6s ", str_seconds(proc->un.proc.times.tms_cutime));
			printf("%6s" , str_seconds(proc->un.proc.times.tms_cstime));
			break;

		case 'T':
			printf("%s ", str_time(proc->un.proc.file_time));
			printf("%s",  str_time(proc->un.proc.start_time));
			break;

		case 'u':
			printf("%5d %5d", proc->euid, proc->egid);
			break;

		case 'U':
			printf("%5d %5d", proc->ruid, proc->rgid);
			break;
		
/*Add case for printing VC to a local server supplying fd's. Only a 
  VC that has an fd open will be printed. */		
		
		case 'v':
			if (proc->flags & _PPF_VID){
				if(qnx_fd_query(vid,proc->pid,0,&fdbuf)!=-1){
					printf("%s", "VC to ");
					remote_proc=qnx_vc_attach(proc->un.vproc.remote_nid,PROC_PID,0,0);
					qnx_psinfo(remote_proc,proc->un.vproc.remote_pid,&psinfo,0,0);
					qnx_vc_detach(remote_proc);
					qnx_psinfo(vid,proc->un.vproc.local_pid,&psinfo2,0,0);
					printf("//%-5d           ",proc->un.vproc.remote_nid);
					printf("%5d", proc->pid);
					printf(" %-22s <->",str_path(0L, psinfo2.un.proc.name, 22));
					printf(" %-22s",str_path(0L, psinfo.un.proc.name, 22));
					}
				}
			
	/*If not a VC, print process name*/		
			
			else{
				printf("%-23s ", str_path(0L, proc->un.proc.name, 23));	
				printf("%5d", proc->pid);
				}
			break;
        	}
		}

	for( ; *cp ; ++cp) {
		switch(*cp) {
			case 'f':
			for(fd = i = 0 ; (fd = qnx_fd_query(vid, proc->pid, fd, &fddata)) != -1; ++fd, ++i) {
				//if(i%2 == 0)
				printf("\n");
				printf("  %2d%c-",fd,(fddata.flags& _FD_CLOSE_ON_EXEC) ? 'C' : ' ');
				
			/* If socket info call fails, treat as regular fd*/	
				
				if (print_socket_info(proc->pid,fd)!=0)
					printf("%-30s",str_fname(proc->pid,fd));
				}
			break;

		case 'm':
			for(si = 0 ; qnx_psinfo(vid, proc->pid, &psdata, si, &segdata[0]) == proc->pid ; si += 16) {
				if(psdata.un.proc.nselectors == 0)
					break;
				for(i = j = 0 ; i < psdata.un.proc.nselectors ; ++i)
					if(segdata[i].flags & _PMF_INUSE) {
						if(j++%2 == 0)
							printf("\n");
						printf("  %4.4X %8.8lX %s %s",
							segdata[i].selector,
							segdata[i].addr, str_nbytes(segdata[i].nbytes, 1),
							str_flags((long)segdata[i].flags, "LBS3---GA-PMDCO"));
						}
				}
			break;
			}
		}
	if(proc->flags& _PPF_VID){
		if(qnx_fd_query(vid,proc->pid,0,&fdbuf)!=-1)
			putchar('\n');
		}
	else
		putchar('\n');
	}


print_proc_header() {
	register char *sp, *cp;
	char *space = "";

	if(hflag)
		return;

	for(cp = fmtstr ; *cp  &&  *cp != ':' ; ++cp) {
		switch(*cp) {
		case 'a':	sp = "ARGUMENTS";
					break;
		case 'b':	sp = "STATE   BLK";
					break;
		case 'd':	sp = "CURRENT DIRECTORY               ";
					break;
		case 'e':	sp = "ENVIRONMENT";
					break;
		case 'f':	sp = "   DAD   SON   BRO";
					break;
		case 'F':   sp = "            FLAGS               ";
					break;
		case 'g':	sp = " PGRP";
					break;
		case 'i':	sp = "  PID";
					break;
		case 'l':	sp = "LINK";
					break;
		case 'L':	sp = "USER NAME   ";
					break;
		case 'm':	sp = "  CODE   DATA";
					break;
		case 'M':	sp = "   MAGIC   ";
					break;
		case 'n':	sp = "PROGRAM                ";
					break;
		case 'N':	sp = " SNID";
					break;
		case 'p':	sp = "PRI";
					break;
		case 'P':	sp = " MPID MSEL MFLG";
					break;
		case 'r':	sp = " CS   IP   SS   SP   DS   ES   AX   BX   CX   DX   SI   DI   BP  FLAG";
					break;
		case 'R':	sp = " ROOT";
					break;
		case 's':	sp = "SID";
					break;
		case 'S':	sp = " SIG IGN SIG MASK SIG PEND  SEG:OFF   ";
					break;
		case 't':	sp = " START TIME   UTIME  STIME CUTIME CSTIME";
					break;
		case 'T':	sp = "  FILE TIME   START TIME ";
					break;
		case 'u':	sp = " EUID  EGID";
					break;
		case 'U':	sp = " RUID  RGID";
					break;
		case 'v':   sp = "PROGRAM                   PID";
					break;
		default:	sp = "";
					break;
			}

		printf("%s%s", space, sp);
		space = " ";
		}

	sp = "";	//miket dec 1/94

	for( ; *cp ; ++cp) {
		switch(*cp) {
		case ':':
			continue;
			}

		printf("%s%s", space, sp);
		space = " ";
		}

	putchar('\n');
	}


print_args(sp)
struct _proc_spawn *sp;
	{
	char *cp = sp->data, *endcp = (char *)sp + sizeof(treeline);

	while (sp->argc--) {
		cp += strlen(cp) + 1;			/* Skip first arg */
		if(cp + strlen(cp) >= endcp) {
			/*
			 * sp may be to small to hold all args so we check
			 * for an overrun and print a ... to let the user know.
			 */
			sp->argc = 0;
			cp = "...";
			}
		printf("%s ", cp);
		}
	}


print_env(sp)
struct _proc_spawn *sp;
	{
	char *cp = sp->data, *endcp = (char *)sp + sizeof(treeline);

	while (sp->argc--  &&  cp < endcp) {
		cp += strlen(cp) + 1;
		}

	cp += strlen(cp) + 1;
	while (sp->envc--) {
		cp += strlen(cp) + 1;
		if(cp + strlen(cp) >= endcp) {
			/*
			 * sp may be to small to hold all args so we check
			 * for an overrun and print a ... to let the user know.
			 */
			sp->envc = 0;
			cp = "...";
			}
		printf("%s ", cp);
		}
	}


print_freemem()
{
	int i;
	union {
		struct _proc_freemem		s;
		struct _proc_freemem_reply	r;
	} msg;

	memset(&msg.s, 0, sizeof(msg.s));
	msg.s.type = _PROC_FREEMEM;
	if (Send(vid, &msg.s, &msg.r, sizeof(msg.s), sizeof(msg.r)) == -1) {
		msg.r.status = errno;
	}
	if (msg.r.status == EOK || msg.r.status == EMORE) {
		for(i = 0 ; i < msg.r.npieces ; ++i)
			printf("%s\n", str_nbytes(msg.r.nbytes[i], 1));
		if (msg.r.status == EMORE) {
			printf("...\n");
		}
	} else {
		fprintf(stderr, "sin: unable to communicate with process manager (%s).\n", strerror(msg.r.status));
		exit(EXIT_FAILURE);
	}
}



print_gdtidt(gdt)
int gdt;
	{
	struct _selector {
		short unsigned	limit,
						base_lo;
		char			base_hi,
						type,
						limflags,
						base_xhi;
	} far *sp;
	unsigned i, nentries;

	if(geteuid() != 0) {
		fprintf(stderr, "You must be a super user to examine the gdt or idt.\n");
		exit(EXIT_FAILURE);
		}

	if (vid == PROC_PID)
		sp = MK_FP(0x08, gdt ? 0x08 : 0x10);
	else {
		sp = malloc(sizeof *sp);
		psdata.pid = 1, psdata.flags = 0;
		debug_read(vid, &psdata, (char *) sp, sizeof sp[0],
                   0x08, gdt ? 0x08 : 0x10);
                  /* (long) MK_FP(0x08, gdt ? 0x08 : 0x10)); bogus */
		sp = realloc(sp, sp[0].limit + 1);
		}

	nentries = (sp[0].limit + 1) / 8;

	if (vid == PROC_PID)
		sp = MK_FP(gdt ? 0x08 : 0x10, 0);
	else
		for(i = 1; i < nentries; i += 512 / sizeof *sp)
			debug_read(vid, &psdata, (char *) &sp[i], 512,
                       gdt ? 0x08 : 0x10, 8 * i);
                       /* (long) MK_FP(gdt ? 0x08 : 0x10, 8 * i));  bogus */

	if(gdt) {
		if(!hflag)
			printf(" SEL   BASE     LIMIT  TYPE DESCRIPTION\n");
		for(i = 1 ; i < nentries ; ++i) 
			if(sp[i].type) {
				printf("%4.4X %8.8lX  %6.6lX   %2.2X ",
					i*8,

					(((long)sp[i].base_xhi) << 24) |
					(((long)sp[i].base_hi)  << 16) |
					sp[i].base_lo,

					(((long)sp[i].limflags & 0x0f) << 16) |
					sp[i].limit,

					(unsigned) sp[i].type);
				
				if (sp[i].type & 0x10) {		
					if (sp[i].type & 0x08) {		
						printf(" code ");
						if (sp[i].type & 0x04) {		
							printf("(conforming)  ");
						} else {
							printf("              ");
						}
					} else {
						printf(" data ");
						if (sp[i].type & 0x02) {		
							printf("(read/write)  ");
						} else {
							printf("(read-only)   ");
						}
					}
				} else {
					switch(sp[i].type & 0x0f) {
						case 0 :	printf(" reserved           ");	break;
						case 1 :	printf(" avail 286 TSS      ");	break;
						case 2 :	printf(" LDT                ");	break;
						case 3 :	printf(" busy 286 TSS       ");	break;
						case 4 :	printf(" call gate          ");	break;
						case 5 :	printf(" task gate          ");	break;
						case 6 :	printf(" 286 irq gate       ");	break;
						case 7 :	printf(" 286 trap gate      ");	break;
						case 8 :	printf(" reserved           ");	break;
						case 9 :	printf(" avail 386 TSS      ");	break;
						case 10:	printf(" reserved           ");	break;
						case 11:	printf(" busy 386 TSS       ");	break;
						case 12:	printf(" 386 call gate      ");	break;
						case 13:	printf(" reserved           ");	break;
						case 14:	printf(" 386 irq gate       ");	break;
						case 15:	printf(" 386 trap gate      ");	break;
					}
				}
				switch((sp[i].type >> 5) & 0x03) {
					case 0 :	printf("Priv 0\n");		break;
					case 1 :	printf("Priv 1\n");		break;
					case 2 :	printf("Priv 2\n");		break;
					case 3 :	printf("Priv 3\n");		break;
				}				

			}
		}
	else {
		if(!hflag)
			printf("INT  OFF  SEL TYPE\n");
		for(i = 0 ; i < nentries ; ++i) 
			if(sp[i].type)
				printf(" %2.2X %6.6X %6.6X  %2.2X\n",
					i,
					sp[i].limit,
					sp[i].base_lo,
					(unsigned) sp[i].type);
		}
	}
	


print_irqs() {
	int intnum;
	register struct _irqinfo *ip;
	register struct _psinfo *pp;

	if(!hflag)
		printf("IRQ   PID  PROGRAM                    CS:IP      DS\n");
	/*
	 * This should be recoded to build a sorted linked list of a vector
	 * of 17 ptrs to an _irqinfo changed by a _link entry.
	 */
	for(intnum = -1 ; intnum < 16 ; ++intnum)
		for(last_irqid = 0 ; ip = get_irq(last_irqid)  ; ++last_irqid)
			if((unsigned short)intnum == ip->intnum &&  (pp = get_proc(ip->pid))  &&  pp->pid == ip->pid)
				printf("%3d %5d  %-23s  %4.4X:%6.6lX %4.4X\n",
						intnum,
						ip->pid,
						str_path(0L, pp->un.proc.name, 23),
						ip->cs,
						ip->offset,
						ip->ds);
	}


print_gnames() {
	nid_t nid, nids[10];
	pid_t vid;
	int n;
	char *cp, *sp;

	if(!hflag)
		printf("NAME               NID\n");
	for(n = qnx_name_locators(&nids) ; n-- > 0 ; ) {
		if((vid = qnx_vc_name_attach(nids[n], sizeof(treeline), "qnx/nameloc")) == -1)
			continue;

		/* Found one to use. */
		for(nid = 1 ; (nid = qnx_nameloc_query(vid, nid, sizeof(treeline), treeline)) != -1; ++nid)
			for(cp = treeline ; *cp ; cp += strlen(cp) + 1) {
				if(*cp != '/')
					if(secret)
						for(sp = cp ; *sp ; ++sp)
							*sp = ((*sp ^ 0x55) | ' ') & 0x7f;
					else
						continue;
				printf("%-16s %5ld\n", cp, nid);
				}
		break;
		}
	}


print_names() {
	register struct _nameinfo *np;
	register struct _psinfo *pp;

	if(!hflag)
		printf("NAME               PID  PROGRAM\n");
	for(last_nameid = 0 ; np = get_name(last_nameid) ; ++last_nameid)
		if((pp = get_proc(np->pid))  &&  pp->pid == np->pid)
			printf("%-16s %5d  %-23s\n",
					np->name,
					np->pid,
					str_path(0L, pp->un.proc.name, 23));
	}


print_net() {
	nid_t	start_nid = 1, end_nid = osdata.max_nodes;
	int		one_node_only;
	char   *nid_stat;


	if (nid) {
		one_node_only = 1;
		start_nid = end_nid = nid;
	} else {
		one_node_only = 0;
		if (nid_stat = calloc(osdata.max_nodes+1,1)) {
			qnx_net_alive(nid_stat,osdata.max_nodes+1);
		}
	}

	if (!hflag) {
		printf("Nid Machine Cpu Fpu Speed Memory   Hard Flop Other         Display   Flags\n");
	}
	for (nid = start_nid ; nid <= end_nid ; ++nid) {
		if (one_node_only || (nid_stat && nid_stat[nid])) {
			node(nid,osdata.max_nodes);
		}
	}

	if (start_nid != end_nid) {
		printf("                    ----- ------  -----\n");
		printf("                   %6ld %5ldM %5ldM\n",
					netdata.speed, netdata.memk/1000, netdata.diskm);
	}
}


int fsys_fdinfo(pid_t server, pid_t pid, int fd, struct _fsys_info_entry *info)
	{
	union
		{
		struct _fsys_fdinfo			s;
		struct _fsys_fdinfo_reply	r;
		}	msg;

	msg.s.type  = _FSYS_FDINFO;
	msg.s.pid   = pid;
	msg.s.zero1 = 0;
	msg.s.fd    = fd;
	msg.s.zero2 = 0;
    if(Send(server, &msg.s, &msg.r, sizeof(msg.s), sizeof(msg.r)) == -1)
        return(-1);

    if(msg.r.status != EOK)
        {
        errno = msg.r.status;
        return(-1);
        }

	memcpy(info, &msg.r.info, sizeof(struct _fsys_info_entry));
	return(0);
	}


void
print_open(proc)
	register struct _psinfo *proc;
	{
	char buf[30];
	char *indent = buf;
	int fd;
	nid_t io_nid;
	pid_t io_pid, io_vid = -1, fsys_pid, print_pid;
	struct _psinfo *rproc = proc;
	struct _sidinfo *sp;

	if(!hflag) {
		printf("PROGRAM               PID FD  MODE  %s   FILE\n",
										Uflag ? "USERID    " : "   POSITION");
		hflag = 1;
		}

	if(proc->flags & _PPF_VID) {
		char *name;
		rproc = get_remproc(proc->un.vproc.remote_nid, proc->un.vproc.remote_pid);
		name = rproc ? rproc->un.proc.name : "unknown";
		sprintf(buf, "%3ld %-15s %5d",	proc->un.vproc.remote_nid,
										str_path(0L, name, 15),
										proc->pid);
		}
	else
		sprintf(buf, "%-19s %5d", str_path(0L, proc->un.proc.name, 19), proc->pid);

	print_pid = proc->pid;
	for(fd = 0 ; (fd = qnx_fd_query(vid, print_pid, fd, &fddata)) != -1; ++fd) {
		if(fddata.nid == 0)	/* This is a bug in early Proc's */
			fddata.nid = nid;
#if 0
		if(fddata.nid != nid  &&  lflag)
			continue;
#endif
		if(io_nid != fddata.nid  ||  io_pid != fddata.pid  ||  io_vid == -1) {
			if(io_vid != -1)
				qnx_vc_detach(io_vid);
			io_vid = qnx_vc_attach(io_nid = fddata.nid, io_pid = fddata.pid, 0, 1);
			if(io_vid == -1) continue;
			}

		/* If fsys is remote we must give it the remote vid for the lookup */
		if((fddata.pid != fddata.vid) && (fddata.vid != 0))
			if(proc = get_proc(fddata.vid))
				fsys_pid = proc->un.vproc.remote_vid;
			else
				fsys_pid = 0;
		else
			fsys_pid = print_pid;

		if(fsys_fdinfo(io_vid, fsys_pid, fd, &fsysdata) == -1) {
			if((errno == ENOSYS)||(errno == EINVAL)||
			   (errno == ENOTSUP)||(errno == EOPNOTSUPP))   //int10 returns EINVAL
				continue;
			fsysdata.fflags = fsysdata.offset = fsysdata.fsize = 0;
			strcpy(fsysdata.path, "unknown");
			}

		if(fsysdata.pid != fsys_pid)
    		break;

		fd = fsysdata.fd;
		printf("%s %2d %s ",
				indent,
				fd,
				str_flags((long) fsysdata.fflags, "E    FDA WR")
				);

		if(!Uflag)
			printf("%6s/%-6s %s\n",
					str_nbytes(fsysdata.offset, 1),
					str_nbytes(fsysdata.fsize, 1),
					str_path(io_nid, fsysdata.path, 29)
					);
		else
			printf("%-12.12s %s\n",
					rproc ?	(sp = get_session(rproc->sid)) ? sp->name : "unknown"
						  : "unknown",
					str_path(io_nid, fsysdata.path, 29)
					);

		indent = "                         ";
		}
	}


print_proxies() {
	int i;
	register struct _psinfo *p1, *p2;

	if(!hflag)
		printf("PROXY  PROGRAM                  PRI STATE COUNT\n");
	for(i = 1 ; p1 = get_proc(i) ; i = p1->pid + 1)
		if((p1->flags & _PPF_MID)  &&  (p2 = get_proc(p1->blocked_on))  &&  p2->pid == p1->blocked_on
			&& (program == 0  ||  strstr(p2->un.proc.name, program) != 0))
			printf("%5d  %-23s  %3d %5s %5u\n",
											p1->pid,
											str_path(0L, p2->un.proc.name, 23),
											p1->priority,
											str_state(p1->state),
											p1->un.mproc.count);
	}



print_session() {
	register struct _sidinfo *sp;
	register struct _psinfo *pp;

	if(!hflag)
		printf("SID USERID           DEVICE             PID  MEMBERS\n");  

	for(last_sid = 0 ; sp = get_session(last_sid) ; ++last_sid)
		if((pp = get_proc(sp->pid))  &&  pp->pid == sp->pid)
			printf("%3d %-16s %-16s %5d  %3d\n",
					sp->sid,
					sp->name,
					sp->tty_name,
					sp->pid,
					sp->links);
	}



print_timers() {
	register int i;
	register struct _psinfo *pp;
	struct _itimerspec tbuf;
	static char *timer_strs[3] ={ "sleep ", "proxy ", "signal" };

	if(!hflag)
		printf(" ID   PID PROGRAM                 ACTION          TRIGGER     REPEAT\n");  

	for(i = 0 ; (i = qnx_timer_query(vid, i, &tbuf)) != -1 ; ++i) {
		pp = get_proc(tbuf.pid);
		printf("%3d %5d %-23s %s ",
				i,
				tbuf.pid,
				str_path(0L, pp->un.proc.name, 23),
				timer_strs[tbuf.notify_type]
				);

		if(tbuf.notify_type)
			printf("%-5ld ", tbuf.data);
		else
			printf("      ");

		if((tbuf.timer_type & 0x80) == 0)
			printf("%6ld.%3.3ld %6ld.%3.3ld\n",
				tbuf.it_value.tv_sec,
				tbuf.it_value.tv_nsec/1000000,
				tbuf.it_interval.tv_sec,
				tbuf.it_interval.tv_nsec/1000000
				);
		else
			printf("     -.---      -.---\n");
		}
	}




print_tree(pid, offset)
pid_t pid;
int offset;
	{
	register struct _psinfo *pp;
	register pid_t brother, son;
	register char *np;
	unsigned n;

	qnx_term_load(_QNX_T_LINES, 0, 0);
	while(pid) {
		if((pp = get_proc(pid)) == NULL  ||  pp->pid != pid)
			return;

		brother = pp->un.proc.brother;
		son = pp->un.proc.son;

		if(sid == -1  ||  pp->sid == sid) {
			if((np = strrchr(pp->un.proc.name, '/')) == NULL)
				np = pp->un.proc.name - 1;
			n = sprintf(treeline + offset, "%s%s",
					treeflag ? (brother ? " ÃÄ" : " ÀÄ") : (brother ? "ÄÂÄ" : "ÄÄÄ"),
					np + 1);
			}
		else
			n = 0;

		treeflag = 0;
		
		if(son)
			print_tree(son, offset + n);

		if((pid = brother)  &&  n) {
			fix_treeline(treeline);
			qnx_term_fputs(treeline, stdout);
			fputc('\n', stdout);
			sprintf(treeline, "%*s", offset, "");
			treeflag = 1;
			}
		}
	}


fix_treeline(cur_line)
char *cur_line;
	{
	static char last_line[200];
	register unsigned char *cp = cur_line;
	register unsigned char *lp = last_line;

	while(*lp  &&  *cp == ' ') {
		if(*lp == 'Â'  ||  *lp == '³'  ||  *lp == 'Ã')
			*cp = '³';
		++lp;
		++cp;
		}

	strcpy(last_line, cur_line);
	}



print_vcs() {
	int i;
	register struct _psinfo *p1, *p2;

	if(!hflag) {
		printf("  LOCAL                                     REMOTE\n");
		printf("  VID   PID LK   STATE PROGRAM              NID   VID   PID PROGRAM\n");
		}
	for(i = 1 ; p1 = get_proc(i) ; i = PINDEX(p1->pid) + 1)
		if(p1->flags & _PPF_VID) {
			if((p2 = get_proc(p1->un.vproc.local_pid))  &&  p2->pid != p1->un.vproc.local_pid)
				continue;
			if (program  &&  strstr(p2->un.proc.name, program) == 0)
				continue;
			printf("%5d %5d %2d %s/%d %-17s  %5ld %5d %5d ",
					p1->pid,
					p1->un.vproc.local_pid,
					p1->un.vproc.links,
					str_state(p1->state),
					p1->un.vproc.substate,
					(p2->flags & _PPF_MID) ? "proxy" :
						p2 ? str_path(0L, p2->un.proc.name, 17) : "unknown",
					p1->un.vproc.remote_nid,
					p1->un.vproc.remote_vid,
					p1->un.vproc.remote_pid);

			p2 = get_remproc(p1->un.vproc.remote_nid, p1->un.vproc.remote_pid);
			printf("%-17s\n", p2 ? (p2->flags & _PPF_MID) ? "proxy" :
						str_path(0L, p2->un.proc.name, 17) : "unknown");
			}
	}


print_versions() {
	int i, unit;
	pid_t vid;
	register struct _psinfo *pp;
	union {
		struct {
			struct _sysmsg_hdr				hdr;
			struct _sysmsg_version			data;
			} s;
		struct {
			struct _sysmsg_hdr_reply		hdr;
			struct _sysmsg_version_reply	data;
			} r;
		} msg;

	if(!hflag)
		printf("PROGRAM                 NAME         VERSION DATE\n");

	for(i = 1 ; pp = get_proc(i) ; ++i) {
		unit = 0;
		i = PINDEX(pp->pid);
		if((pp->flags & (_PPF_SERVER|_PPF_TO_BE_HELD|_PPF_VID)) == _PPF_SERVER) {
			do {
				msg.s.hdr.type = _SYSMSG;
				msg.s.hdr.subtype = _SYSMSG_SUBTYPE_VERSION;
				msg.s.data.unit = unit;
				vid = qnx_vc_attach(nid, pp->pid, sizeof(msg), 0);
				Send(vid, &msg.s, &msg.r, sizeof(msg.s), sizeof(msg.r));
				qnx_vc_detach(vid);
				if(msg.r.hdr.status == EOK)
					printf("%-23s %-12.12s %d.%2.2d%1.1s   %s\n",
							str_path(0L, pp->un.proc.name, 23),
							msg.r.data.name,
							msg.r.data.version/100,
							msg.r.data.version%100,
							msg.r.data.letter ? &msg.r.data.letter : "",
							msg.r.data.date
							);

				++unit;
				} while(msg.r.hdr.status == EOK  &&  msg.r.data.more) ;
			}
		}
	}



char *str_flags(flags, flag_str)
long flags;
char *flag_str;
	{
	static char buf[33];
	int bit;
	register char *sp = flag_str, *dp = buf;

	for(bit = strlen(sp) - 1 ; *sp ; --bit, ++sp)
		if(*sp != ' ')
			*dp++ = ((1L << bit) & flags) ? (*sp == '-' ? '+' : *sp) : '-';

	return(buf);
	}



/*
 * Limit name to width chars. Drop excess from the front.
 * IE: Keep the last components and the node component.
 * //3/dir1/dir2/dir3/abc  ->  //3/(star)/dir2/dir3/abc
 */
char *str_path(nid, path, width)
nid_t nid;
char *path;
int width;		/* Max of 100 */
	{
	int n;
	static char buf[101];
	char *cp = buf, *cp2, *markcp = NULL;

	if(nid/*  ||  cp[0] != '/'  ||  cp[1] != '/'*/) {
		cp = path;
		if(cp[0] == '/'  &&  cp[1] == '/')
			for(cp = path + 2 ; *cp  &&  *cp != '/' ; ++cp)
				;	/* Strip any existing node */

		sprintf(buf, "//%ld%s%s", nid, *cp == '/' ? "" : " ", cp);
		}
	else
		strcpy(buf, path);

	if(strlen(buf) < width)
		return(buf);

// sin fi calls str_path(,,15), sys/Proc32.32.X causes SEGV
// (if buf does not start with //, there is no nodeid and we can
// simply return the last <width> characters
if (buf[0] != '/' || buf[1]!='/') return(buf+strlen(buf)-width);

	/* We now have in form  //node/path  OR  something if nid == 0 */
	for(cp = buf + 2 ; *cp  &&  *cp != '/'  &&  *cp != ' ' ; ++cp)
		;	/* We always keep the //node part */

	*++cp = '*';
	n = width - (++cp - buf);	/* Number of chars free for last component */
	for(cp2 = buf + strlen(buf) - 1 ; n ; --n, --cp2)
		if(*cp2 == '/')
			markcp = cp2;

	strcpy(cp, markcp ? markcp : cp2);
	return(buf);
	}

char *str_cmd(cmd, width)
char *cmd;
int width;		/* Max of 100 */
	{
	register int n;
	static char buf[101];

	strcpy(buf, cmd);
	if((n = strlen(buf)) <= width)
		return(buf);

	buf[0] = '*';
	strcpy(&buf[1], &buf[(n - width) + 1]);
	return(buf);
	}



char *str_nbytes(num_bytes, scale)
long unsigned num_bytes;
int scale;	/* Added to force xxxxx to print as xxk to make it prettier */
	{
	static char *buf, buf1[8], buf2[8];

	buf = (buf == buf2) ? buf1 : buf2;
/*
	if(num_bytes >= 10000000L)
		sprintf(buf, "%4uM", num_bytes/1000000L);
	else if(num_bytes >= 1000000L)
		sprintf(buf, "%2ld.%1ldM",
			num_bytes / 1000000L, (num_bytes % 1000000L) / 100000L );
	else if(num_bytes >= 10000)
		sprintf(buf, "%4uK", num_bytes/1024);
	else
		sprintf(buf, "%5u", num_bytes);
*/
	if(num_bytes < 1000000/scale)
		sprintf(buf, "%6lu", num_bytes);
	else if(num_bytes < 100000000)
		sprintf(buf, "%5luk", num_bytes/1000);
	else
		sprintf(buf, "%5luM", num_bytes/(1000*1000L));

	return(buf);
	}



char *str_state(state)
unsigned state;
	{

	switch(state) {
		case STATE_DEAD:				return(" DEAD");
		case STATE_READY:				return("READY");
		case STATE_SEND_BLOCKED:		return(" SEND");
		case STATE_RECEIVE_BLOCKED:		return(" RECV");
		case STATE_REPLY_BLOCKED:		return("REPLY");
		case STATE_HELD:				return(" HELD");
		case STATE_SIGNAL_BLOCKED:		return("SIGBL");
		case STATE_WAIT_BLOCKED:		return(" WAIT");
		case STATE_SEM_BLOCKED:			return("  SEM");
		}

	return("???? ");
	}



char *str_seconds(t)
clock_t t;
	{
	static char buf[20];

	if(t < 100000L)
		sprintf(buf, "%2lu.%3.3lu", t/CLK_TCK, t%CLK_TCK);
	else
		sprintf(buf, "%6lu", t/CLK_TCK);

	return(buf);
	}


char *str_time(t)
time_t t;
	{
	static char buf[20];

	if(t)
		strftime(buf, sizeof(buf), "%b %d %H:%M", localtime(&t));
	else
		strcpy(buf, "--- -- --:--");
	return(buf);
	}


print_info() {
	nid_t nids[_PNAME_NIDS];
	int i, n;
	long unsigned freepmem, totpmem;

	freepmem = osdata.freepmem;
	if((totpmem = osdata.totpmem) == 0) {
		totpmem = osdata.totmemk * 1000L;
		freepmem = osdata.freememk * 1000L;
		}

	printf(" Node    CPU    Machine Speed     Memory    Ticksize   Display            Flags\n");

		{
		long int divby=1000L;
		char freememstr[8],totpmemstr[8];

		if (((freepmem/divby)>99999L) || ((totpmem/divby)>99999L))
			divby*=1024L;
		sprintf(freememstr,"%lu%c",	freepmem/divby, (divby<=1000)?'k':'M');
		sprintf(totpmemstr,"%lu%c",totpmem/divby, (divby<=1000)?'k':'M');

		printf("%5ld %4ld/%-4ld %7.7s %5u %6s/%-6s %2u.%ums %11s %s\n\n",
				osdata.nodename,
				osdata.cpu,
				osdata.fpu,
				osdata.machine,
				osdata.cpu_speed,
				freememstr,
				totpmemstr,
				(osdata.tick_size + 10)/1000,
				((osdata.tick_size + 1)/100)%10,
				monitorstrs[osdata.primary_monitor],
				str_flags((long)osdata.sflags, "D3P--------AEE8P")
				);
		}

	printf("Heapp Heapf Heapl Heapn Hands Names Sessions Procs Timers Nodes Virtual\n");
	if(osdata.proc_freemem & 1)
		printf("%4uk ", osdata.proc_freemem >> 1);
	else
		printf("%5u ", osdata.proc_freemem);

	if(osdata.fd_freemem & 1)
		printf("%4uk ", osdata.fd_freemem >> 1);
	else
		printf("%5u ", osdata.fd_freemem);

	if(osdata.ldt_freemem & 1)
		printf("%4uk ", osdata.ldt_freemem >> 1);
	else
		printf("%5u ", osdata.ldt_freemem);

	if(osdata.name_freemem & 1)
		printf("%4uk ", osdata.name_freemem >> 1);
	else
		printf("%5u ", osdata.name_freemem);

	printf("%5u %5u %8u %5u %6u %5u",
			osdata.num_handlers,
			osdata.num_names,
			osdata.num_sessions,
			osdata.num_procs,
			osdata.num_timers,
			osdata.max_nodes
			);

	if(osdata.totvmem)
		printf(" %5luM/%5luM\n\n", osdata.freevmem/1000000, osdata.totvmem/1000000);
	else
		printf("   ---\n\n");

	printf("Boot from %s at %s   ",
			osdata.bootsrc == 'H' ? " Hard" :
			osdata.bootsrc == 'h' ? "*Hard" :
			osdata.bootsrc == 'F' ? " Flop" :
			osdata.bootsrc == 'f' ? "*Flop" :
			osdata.bootsrc == 'M' ? "MemDsk" :
			osdata.bootsrc == 'm' ? "*MemDsk" :
			osdata.bootsrc == 'N' ? "  Net" :
			osdata.bootsrc == 'R' ? "  Rom" : " ????",
			str_time(get_proc(PROC_PID)->un.proc.start_time));

	printf("Locators:");
	for(i = 0, n = qnx_name_locators(&nids) ; i < n ; ++i)
		printf(" %ld", nids[i]);
	printf("\n");

#if 0
	printf("Booted From: ");
	switch(osdata.bootsrc) {
		case 'F':	printf("Floppy Disk");		break;
		case 'H':	printf("Hard Disk  ");		break;
		default:	printf("Network    ");		break;
		}
#endif
	}

int node_has_tcp(nid_t nid, nid_t max_nodes)
{
	static char *tcp_node_map=NULL;

	if (NULL==tcp_node_map) {
		nid_t tmp, nids[10];
		pid_t vid;
		int n;
		char *cp;

		if (NULL==(tcp_node_map=calloc(max_nodes+1,sizeof(char)))) return 0;

		/* load the map based on namelocator global name info */
		for(n = qnx_name_locators(&nids) ; n-- > 0 ; ) {
#ifdef DIAG
	printf("[%04d] node_has_tcp: trying to get vid to nameloc on //%d\n",DELTA,nids[n]);
#endif
			if((vid = qnx_vc_name_attach(nids[n], sizeof(treeline), "qnx/nameloc")) == -1)
				continue;	/* look for another name locator */
#ifdef DIAG
	printf("[%04d] node_has_tcp: got it; getting all names, looking for 'qnx/socket'\n",DELTA);
#endif
			/* Found one to use. */
			for(tmp = 1 ;
                (tmp = qnx_nameloc_query(vid, tmp, sizeof(treeline), treeline)) != -1;
                ++tmp)
            {
#ifdef DIAG
	printf("[%04d] node_has_tcp: getting global names reg'd by node %d\n",DELTA,tmp);
#endif
				for(cp = treeline ; *cp ; cp += strlen(cp) + 1) {
#ifdef DIAG
	if (*cp=='/')
		printf("[%04d] node_has_tcp: got name '%s'\n",DELTA,cp);
#endif
					if (!strcmp(cp,"/qnx/socket")) {
#ifdef DIAG
	printf("[%04d] node_has_tcp: node %d has tcp (memorized)\n",DELTA,tmp);
#endif
						tcp_node_map[tmp]=1;
					}
				}
			}

#ifdef DIAG
	printf("[%04d] node_has_tcp: detaching VC\n",DELTA);
#endif
	
			qnx_vc_detach(vid);
			break;  /* out of nameloc loop */
		}
	}    

#ifdef DIAG
	printf("[%04d] node_has_tcp: done\n",DELTA);
#endif

	return tcp_node_map[nid];	
}

node(nid_t nid, nid_t max_nodes)
{
	long unsigned	hdk,fdk, totpmem, otherstorage;
	long unsigned	megs = 0;
	struct _osinfo	osdata;
	char            buffer[20]; /* for building 'other' column text */

#ifdef DIAG
	printf("\n[%04d] node(%d)--------------------------------------------\n",DELTA,nid);
	printf("[%04d] qnx_osinfo(%d,..)\n",DELTA,nid);
#endif
	if(qnx_osinfo(nid, &osdata) == -1) {
#ifdef DIAG
	printf("[%04d] qnx_osinfo: failed\n",DELTA);
	printf("[%04d] node: failed\n",DELTA);
#endif
		return;
	}
#ifdef DIAG
	printf("[%04d] qnx_osinfo: OK\n",DELTA);
#endif

	if((totpmem = osdata.totpmem) == 0)
		totpmem = osdata.totmemk * 1000L;

	printf("%3ld %-7s %3ld", osdata.nodename, osdata.machine, osdata.cpu);
	if(osdata.fpu)
		printf(" %3ld", osdata.fpu);
	else
		printf("    ");
	printf(" %5u %3lu.%luM", osdata.cpu_speed, totpmem/1000000, (totpmem%1000000)/100000);
#ifdef DIAG
	printf("\n[%04d] get_disk_space(%d,..,..,..)\n",DELTA,osdata.nodename);
#endif
	get_disk_space(osdata.nodename,&hdk,&fdk, &otherstorage); /* get hd and fd capacity */
#ifdef DIAG
	printf("[%04d] get_disk_space: OK\n",DELTA);
#endif

	if(hdk)
		printf(" %5ldM", megs = hdk/1000);
	else
		printf("       ");

	if(fdk) {
		if (fdk>999)
			printf(" %1ld.%ldM", fdk/1000, (fdk%1000)/100);
		else 
			printf(" %3ldk", fdk);
	} else
		printf("     ");

	buffer[0]=0;
#ifdef DIAG
	printf("\n[%04d] node_has_tcp(%d,..)\n",DELTA,nid);
#endif
    if (node_has_tcp(nid,max_nodes)) strcat(buffer,"TCP"); 
#ifdef DIAG
	printf("[%04d] node_has_tcp: OK\n",DELTA);
#endif

	/* indicate cd, rhd (removable hard disk) or tape */
	if (otherstorage&(1L<<_CDROM)) {
		if (buffer[0]) strcat(buffer,",");
		strcat(buffer,"cd");
	}

	if (otherstorage&(1L<<_REMOVABLE)) {
		if (buffer[0]) strcat(buffer,",");
		strcat(buffer,"rhd");
	}

	if (otherstorage&(1L<<_TAPE)) {
		if (buffer[0]) strcat(buffer,",");
		strcat(buffer,"tp");
	}

	printf(" %-13s",buffer);

	printf(" %-9s %s\n",	monitorstrs[osdata.primary_monitor & 0x0f],
							str_flags((long)osdata.sflags, "D3--      -AEE8P"));

	netdata.speed += osdata.cpu_speed;
	netdata.memk  += (totpmem+499)/1000;  /* round */
	netdata.diskm += megs;

#ifdef DIAG
	printf("[%04d] node: OK\n",DELTA); 
#endif
	}

int get_disk_entry (struct _disk_entry *disk_entry_ptr, char *full_path)
{
    int     fd, rc;

    union {
        struct _io_open         s;
        struct _io_open_reply   r;
    } *iomsg;

    /* GET IO HANDLE FOR THE FILE */
    if((iomsg = alloca(sizeof(struct _io_open) + _POSIX_PATH_MAX)) == NULL)
    {
        fprintf(stderr,"sin: out of memory (stack)\n");
        exit(EXIT_FAILURE);
    }

	/* this would be better replaced by a version of __resolve_net and
       __resolve_local which take advantage of our existing vc to the
       actual IO manager involved. All we really need is a qnx_fd_attach
       and an open message to the remote Fsys!! */
    iomsg->s.oflag = _IO_HNDL_INFO;
    fd = __resolve_net(_IO_HANDLE, 1, iomsg, full_path, sizeof(iomsg->r), 0);

    if(fd == -1) {
        return -1;
    }

	rc=disk_get_entry(fd, disk_entry_ptr);

	close(fd);

    return rc;
}

unsigned long
disk_size(nid_t nid, char *name, unsigned long *otherstorage )
	{
	unsigned long		numk = 0;
	char		buffer[30];
	struct _disk_entry diskentry;

	sprintf(buffer, "//%ld/dev/%s", nid, name);

#ifdef DIAG
	printf("[%04d] disk_size: nid=%d name=%s\n",DELTA,nid,name);
	printf("[%04d] disk_size: get_disk_entry(.., %s)\n",DELTA,buffer);
#endif

	/* used to stat this file to determine size from st_size */

	/* now it is more sophisticated  :-) */

	if (get_disk_entry(&diskentry, buffer)==-1) return 0;

	switch (diskentry.disk_type) {
		case _HARD:
		case _FLOPPY: numk=diskentry.disk_sectors/2L; break;
		default: *otherstorage|=(1L<<diskentry.disk_type); break;
	}

#ifdef DIAG
	printf("[%04d] disk_size: done (%uk)\n",DELTA,numk);
#endif

	return(numk);
	}


void
get_disk_space(nid_t nid, unsigned long *hdk, unsigned long *fdk, unsigned long *otherstorage)

{
	/* allocate a fairly big buffer for prefixes to handle 'reasonable'
       prefix sizes in remote nodes without having to send a message to
       the remote proc to determine its actual prefix size */
	static char	prefix_buffer[4096];
	unsigned long *resultptr;
	char        *tok;
	int         c, i;
	pid_t       holdopen_vid[2], remote_pid[2];

	*hdk=*fdk=*otherstorage=0L;
	
	/* hdk and fdk are measured in megabytes. otherstorage is a set of
       bit flags 1L<<disk_type which indicate mere presence of these other
       types of devices e.g. tape, cd */

#ifdef DIAG
	printf("[%04d] get_disk_space: locating qnx/fsys, qnx/fsys32 on //%d\n",DELTA,nid);
	errno=0;
#endif
	/* get the registered name qnx/fsys and/or qnx/fsys32 on that machine */
	holdopen_vid[0]= qnx_name_locate(nid, "qnx/fsys", 0, NULL);
#ifdef DIAG
	printf("[%04d] get_disk_space: qnx_name_locate(%d, \"qnx/fsys\",..)==%d. %s\n",
            DELTA, nid, strerror(errno));
	errno=0;
#endif
	holdopen_vid[1]= qnx_name_locate(nid, "qnx/fsys32", 0, NULL);
#ifdef DIAG
	printf("[%04d] get_disk_space: qnx_name_locate(%d, \"qnx/fsys32\",..)==%d. %s\n",
            DELTA, nid, strerror(errno));
	errno=0;
#endif

	if (holdopen_vid[0]==-1 && holdopen_vid[1]==-1) {
#ifdef DIAG
	printf("[%04d] get_disk_space: no Fsys on //%d\n",DELTA,nid);
#endif
		return;
	}
#ifdef DIAG
	printf("[%04d] get_disk_space: %s %s\n",DELTA,(holdopen_vid[0]!=-1)?"got qnx/fsys":"",
	                (holdopen_vid[1]!=-1)?"got qnx/fsys32":"");
#endif

	/*
		need to find the real process Ids for these Fsys's. Query local
        proc for info on the remote end of the VC
    */
	for (i=0;i<2;i++) {
		remote_pid[i]=qnx_psinfo(PROC_PID, holdopen_vid[i], &psdata, 0, 0);
		if (remote_pid[i]==-1) {
			/* silently close this connection and ignore */
			qnx_vc_detach(holdopen_vid[i]);
			holdopen_vid[i]=-1;
		} else {
			if (psdata.flags & _PPF_VID) {
				remote_pid[i]=psdata.un.vproc.remote_pid;
			} else {
				remote_pid[i]=holdopen_vid[i];
			}
		}
	}


#ifdef DIAG
	printf("[%04d] get_disk_sapce: getting prefixes from //%d\n",DELTA,nid);
#endif
	/* get all the prefixes under /dev/; stat only those starting with
       hd or fd and having all numeric suffixes. */

	/* get entire prefix list */
	if (-1==qnx_prefix_query(nid, "", prefix_buffer, sizeof(prefix_buffer))) {
		return;
	}

	tok=strtok(prefix_buffer, ":");
	while (tok) {

		/* only look at stuff under /dev/ */
		if (!strncmp(tok,"/dev/",5)) {
    	
	        /* check for alpha only prefix */
			for (c=5;isalpha(tok[c]);c++);
	
			/* check for numeric only suffix */
			for (;tok[c]>='0' && tok[c]<='9';c++);
	
			if (tok[c]=='=') {
				char *ptr;
				pid_t manager_pid;
	
				tok[c]=0;
	
				/* good - all alpha (or null) followed by all numeric. */ 
			    c++;	
				
				/* &tok[c] will now be an alias, or a pid,unit pair. If it
	               is an alias, skip it. */
				if (ptr=strchr(&tok[c],',')) {
					*ptr=0;
					/* read pid */
					manager_pid=atol(&tok[c]);

					/* only accept those which are managed by the remote Fsys */
					if ((remote_pid[0]!=-1 && manager_pid==remote_pid[0]) ||
	                    (remote_pid[1]!=-1 && manager_pid==remote_pid[1]))
	                {
#ifdef DIAG
	printf("[%04d] get_disk_space: '%s' looks interesting..\n",DELTA,tok);
#endif
	
			            /* decide where the result is going */
						if (strncmp(tok,"/dev/fd",7)) resultptr=hdk;
						else resultptr=fdk;
						
						/* only count fd0 plus other devices. Do not count fdN where n!=0 */
						if (resultptr==hdk || tok[7]=='0' || tok[7]>='A') {
							/* find the size of that device */
							*resultptr+=disk_size(nid,&tok[5],otherstorage);
						}
					} /* manager was the remote fsys or remote fsys32 */
				} /* was an alias prefix */
			}
	         
		}

		tok=strtok(NULL, ":");
	}

#ifdef DIAG
	printf("[%04d] get_disk_space: detaching from remote Fsys\n",DELTA);
#endif
	/* detach vid to remote Fsys held open */
	for (i=0;i<2;i++) {
	 	if (holdopen_vid[i]!=-1) qnx_vc_detach(holdopen_vid[i]);
	}
#ifdef DIAG
	printf("[%04d] get_disk_space: done\n",DELTA);
#endif

}

char *str_fname(pid, fd)
pid_t pid;
int fd;
	{
	static char				buf[17];
	pid_t					dvid = 0;
	struct _fd_entry		info1;
	struct _dev_info_entry	info2;
	struct _fsys_info_entry	info3;

	if(fd == -1)
		return("none");

	if(qnx_fd_query(vid, pid, fd, &info1) == -1  ||
	  (dvid = qnx_vc_attach(info1.nid, info1.pid, 0, 0)) == -1)
		return("unknown");

	/* If it is on another node we need to use the vid on that end. */
	if(info1.pid != info1.vid) {
		if(qnx_psinfo(vid, info1.vid, &psdata, 0, NULL) != info1.vid) {
			qnx_vc_detach(dvid);
			return("unknown");
			}
		pid = psdata.un.vproc.remote_vid;
		}

	if(dev_fdinfo(dvid, pid, fd, &info2) == -1) {
		if(fsys_fdinfo(dvid, pid, fd, &info3) == -1
					||  info3.pid != pid
					||  info3.fd != fd) {
			qnx_vc_detach(dvid);
			sprintf(buf, "(%d)", info1.pid);
			return(str_path(info1.nid, buf, 30));
			}
		qnx_vc_detach(dvid);
		return(str_path(info1.nid, info3.path, 30));
		}

	qnx_vc_detach(dvid);

	if(info2.tty_name[0] == '\0') {
		sprintf(buf, "[%s]", info2.driver_type); 
		return(str_path(info1.nid, buf, 30));
		}

	return(str_path(0, info2.tty_name, 30));
	}



void *
salloc(n, m)
unsigned n, m;
	{
	void * ptr;

	if(ptr = calloc(n, m))
		return(ptr);

	fprintf(stderr, "sin: insufficient memory to sin.\n");
	exit(EXIT_FAILURE);

	}


/*
 * The following get_xxx routines get information on an object from the
 * OS. To reduce messages for objects which may be requested more than
 * once we cache the data.
 */

struct _psinfo *
get_proc(id)
pid_t id;
	{
	struct _psinfo data;

	if((id = PINDEX(id)) <= 0  ||  id > osdata.num_procs)
		return(NULL);

	if(proc_table == NULL) {
		proc_table = salloc(osdata.num_procs + 1, sizeof(struct _psinfo *));
		sort_table = salloc(osdata.num_procs + 1, sizeof(struct _psinfo *));
		}

	if((*proc_table)[id])
		return((*proc_table)[id]);

	if(qnx_psinfo(vid, id, &data, 0, NULL) != -1) {
		id = PINDEX(data.pid);
		(*proc_table)[id] = salloc(1, sizeof(struct _psinfo));
		*(*proc_table)[id] = data;
		return((*proc_table)[id]);
		}

	return(NULL);
	}


struct _psinfo *
get_remproc(nid, id)
nid_t nid;
pid_t id;
	{
	pid_t vid;
	static struct _psinfo data;
		
	if(lflag ||  (vid = qnx_vc_attach(nid, PROC_PID, sizeof(data), 0)) == -1)
		return(NULL);

	if(qnx_psinfo(vid, id, &data, 0, NULL) != id)
		return(NULL);

	qnx_vc_detach(vid);
	return(&data);
	}



struct _irqinfo *
get_irq(id)
int id;
	{
	struct _irqinfo data;

	if(id < 0  ||  id >= osdata.num_handlers)
		return(NULL);

	if(irq_table == NULL)
		irq_table = salloc(osdata.num_handlers, sizeof(struct _irqinfo *));

	if((*irq_table)[id])
		return((*irq_table)[id]);

	if((id = qnx_hint_query(vid, id, &data)) != -1) {
		last_irqid = id;
		(*irq_table)[id] = salloc(1, sizeof(struct _irqinfo));
		*(*irq_table)[id] = data;
		return((*irq_table)[id]);
		}

	return(NULL);
	}



struct _nameinfo *
get_name(id)
int id;
	{
	struct _nameinfo data;

	if(id < 0  ||  id >= osdata.num_names)
		return(NULL);

	if(name_table == NULL)
		name_table = salloc(osdata.num_names, sizeof(struct _nameinfo *));

	if((*name_table)[id])
		return((*name_table)[id]);

	if((id = qnx_name_query(vid, id, &data)) != -1) {
		last_nameid = id;
		(*name_table)[id] = salloc(1, sizeof(struct _nameinfo));
		*(*name_table)[id] = data;
		return((*name_table)[id]);
		}

	return(NULL);
	}



struct _sidinfo *
get_session(id)
int id;
	{
	struct _sidinfo data;

	if(id < 0  ||  id >= osdata.num_sessions)
		return(NULL);

	if(session_table == NULL)
		session_table = salloc(osdata.num_sessions, sizeof(struct _sidinfo *));

	if((*session_table)[id])
		return((*session_table)[id]);

	if((id = qnx_sid_query(vid, id, &data)) != -1) {
		last_sid = id;
		(*session_table)[id] = salloc(1, sizeof(struct _sidinfo));
		*(*session_table)[id] = data;
		return((*session_table)[id]);
		}

	return(NULL);
	}


cmp(pp1, pp2)
struct _psinfo **pp1, **pp2;
	{
	struct _psinfo *p1 = *pp1, *p2 = *pp2;
	char *s1, *s2;

	switch(sort_letter) {
	case 'b':	return(p1->blocked_on - p2->blocked_on);
	case 'g':	return(p1->pid_group - p2->pid_group);
	case 'i':	return(p1->pid - p2->pid);
	case 'n':	if((s1 = strrchr(p1->un.proc.name, '/')) == NULL)
					s1 = p1->un.proc.name;
				if((s2 = strrchr(p2->un.proc.name, '/')) == NULL)
					s2 = p2->un.proc.name;
				return(strcmp(s1, s2));
	case 'p':	return(p1->priority - p2->priority);
	case 's':	return(p1->sid - p2->sid);
	case 't':	return((int)(p1->un.proc.times.tms_utime/1000 - p2->un.proc.times.tms_utime/1000));
	case 'u':	return(p1->euid - p2->euid);
	case 'U':	return(p1->ruid - p2->ruid);
		}

	fprintf(stderr, "sin: unsupported sort type (%c).\n", sort_letter);
	exit(EXIT_FAILURE);
	}


long number(opt, str)
char *opt, *str;
	{
	long n;
	char *endptr;

	n = strtol(str, &endptr, 10);

	if(*endptr != '\0') {
		fprintf(stderr, "sin: invalid numeric argument to %s.\n", opt);
		exit(EXIT_FAILURE);
		}

	return(n);
	}



debug_read(pid_t vid, struct _psinfo *proc, char *buf, unsigned maxbytes,
           short unsigned segment, long unsigned off)
	{
	unsigned n, ret, seg;

	/* segment may be 0 ; so that old portions of sin code which
       built up a combined seg/offset in off can pass 0 and keep
       the old functionality */

	if (segment==0) {
		if(proc->flags & _PPF_32BIT)
			seg = proc->magic_sel;
		else {
			seg = off >> 16;
			off &= 0xffff; /* bizarre; why limit to 1st 64k ?? */
			}
	} else {
		if(proc->flags & _PPF_32BIT)
			seg = proc->magic_sel; /* should we keep this here? */
		else {
			seg=segment;
		}
	}

	n = min(maxbytes, 512);		/* Limit to max allowed by the debug msg */

	do {
		ret = __qnx_debug_xfer(vid, proc->pid, _DEBUG_MEM_RD,	
				   buf, n,
				   off, seg);
	} while(ret == -1  &&  errno != EPERM && errno != ESRCH  &&  (n -= 32) >= 64) ;

	buf[n - 1] = '\0';			/* Make sure buf is null terminated. */

	return(ret);
	}



int
(qnx_timer_query) (proc_pid, timerid, value)
pid_t						proc_pid;
timer_t						timerid;
register struct _itimerspec	*value;
	{
	union stimer {
		struct _proc_timer		  s;
		struct _proc_timer_reply	r;
		} msg;

	msg.s.type = _PROC_TIMER;
	msg.s.subtype = _PROC_SUB_QUERY;
	msg.s.cookie = timerid;
	msg.s.flags = 0;
	msg.s.zero1 = 0;

	if(Send(proc_pid, &msg.s, &msg.r, sizeof(msg.s), sizeof(msg.r)) == -1)
		return(-1);

	if(msg.r.status) {
		errno = msg.r.status;
		return(-1);
		}

	value->it_value.tv_sec = msg.r.sec1;
	value->it_value.tv_nsec = msg.r.nsec1;
	value->it_interval.tv_sec = msg.r.sec2;
	value->it_interval.tv_nsec = msg.r.nsec2;
	value->notify_type = (int)msg.r.notify_type;
	value->timer_type = (int)msg.r.timer_type;
	value->data = msg.r.data;
	value->pid = msg.r.pid;
	return(msg.r.cookie);
	}


nid_t
qnx_nameloc_query(vid, nid, len, buf)
pid_t vid;
nid_t nid;
int len;
char *buf;
	{
	union {
		struct _nameloc_query		s;
		struct _nameloc_query_reply	r;
		} msg;
    struct _mxfer_entry mx[2];

	msg.s.type = _NAMELOC_QUERY;
	msg.s.nid = nid;

	_setmx(&mx[0], &msg.s, sizeof(msg.s));
	_setmx(&mx[1], buf, len);

	if(Sendmx(vid, 1, 2, mx, mx) == -1)
		return(-1);

	if(msg.r.status != EOK) {
		errno = msg.r.status;
		return(-1);
		}

	if(msg.r.nid < nid)		/* This catches old namlocs */
		return(-1);

	return(msg.r.nid);
	}


/* Four functions added to print out the socket information. This includes
   print_socket_info()
   get_sock_info()	
   print_sock_info()
   print_state()
*/	


int print_socket_info(pid, fd,)
pid_t pid;
int fd;
	{
	pid_t					dvid = 0;
	struct _fd_entry		info1;
	struct _dev_info_entry	info2;
	struct _socket_fdinfo_reply sock_fdinfo;
	int remote;
	
	remote=0;
	if(fd == -1)
		return(-1);

	if(qnx_fd_query(vid, pid, fd, &info1) == -1  ||
	  (dvid = qnx_vc_attach(info1.nid, info1.pid, 0, 0)) == -1)
		return(-1);

	if(info1.nid!=getnid())
		remote=1;
	

	/* If it is on another node we need to use the vid on that end. */
	if(info1.pid != info1.vid) {
		if(qnx_psinfo(vid, info1.vid, &psdata, 0, NULL) != info1.vid) {
			qnx_vc_detach(dvid);
			return(-1);
			}
		pid = psdata.un.vproc.remote_vid;
		}

	if(dev_fdinfo(dvid, pid, fd, &info2) == -1) {
		qnx_vc_detach(dvid);
		return(-1);
		}

	if(info2.tty_name[0] == '\0') {
		if (info2.driver_type[0]=='@')
			if (get_sock_info(pid,fd,remote ? dvid:info1.pid,&sock_fdinfo)==0){
				printf("//%d ",info1.nid);
				print_sock_info(sock_fdinfo);
				if (remote)
					qnx_vc_detach(dvid);
				return(0);
			}
		
		}

	qnx_vc_detach(dvid);
	return(-1);
	}




int get_sock_info(int pid, int fd, int server_pid, struct _socket_fdinfo_reply *fdinfo)
{
	union
	{
		struct _socket_fdinfo  s;
		struct _socket_fdinfo_reply r;
	} msg;

	memset (&msg,0,sizeof(msg));	
	msg.s.type = _SOCK_FDINFO;
	msg.s.pid = pid;
	msg.s.fd  = fd;

	msg.r.info.sa_family = AF_INET; /* for backward compatible */

	if (Send(server_pid,&msg.s,&msg.r,sizeof(msg.s),sizeof(msg.r))==-1)	
		return(-1);
	if(msg.r.status !=EOK){
		return(msg.r.status);
		}
	else{
		memcpy(fdinfo,&msg.r,sizeof(msg.r));
		}
	return(msg.r.status);	
} 

/* Print the state of the connection */

void print_state (int state)
{
	switch(state){
		case 0: printf("CLOSED");
				break;
		case 1: printf("LISTEN");
				break;
		case 2:	printf("SYN_SENT");
				break;
		case 3: printf("SYN_RECEIVED");
				break;
		case 4: printf("ESTABLISHED");
				break;
		case 5: printf("CLOSE_WAIT");
				break;
		case 6:	printf("FIN_WAIT_1");
				break;
		case 7: printf("CLOSING");
				break;
		case 8: printf("LAST_ACK");
				break;
		case 9:	printf("FIN_WAIT_2");
				break;
		case 10:printf("TIME_WAIT");
				break;
		default:printf(" ");
	
	}
}

/* Print the socket information */

int print_sock_info(struct _socket_fdinfo_reply fdinfo)
{
	if (fdinfo.info.sa_family == AF_INET) {
		if (fdinfo.info.socket_type == SOCK_STREAM)
			printf(" %s ","T");
		else if (fdinfo.info.socket_type == SOCK_DGRAM)
			printf(" %s ","U");
		else
			printf(" %s ","I");
		printf("%16s.%-5d ", inet_ntoa(fdinfo.info.inet.laddr), ntohs(fdinfo.info.inet.lport));
		printf("%16s.%-5d ", inet_ntoa(fdinfo.info.inet.faddr), ntohs(fdinfo.info.inet.fport));
		if (fdinfo.info.socket_type == SOCK_STREAM)
			print_state(fdinfo.info.inet.state);
		else
			printf("N/A");
	} else if (fdinfo.info.sa_family == AF_LOCAL) {
		if (fdinfo.info.socket_type == SOCK_STREAM)
			printf(" %s ","S");
		else if (fdinfo.info.socket_type == SOCK_DGRAM)
			printf(" %s ","D");
		else
			printf(" %s ","L");
		printf("%25s  ", str_path(0L, fdinfo.info.unix.lpath, 25));
		printf("%25s  ", str_path(0L, fdinfo.info.unix.fpath, 25));
	} else if (fdinfo.info.sa_family == AF_ROUTE)
		printf(" %s ","R");
	else
		printf(" %s ","X");

	return(0);	
}
