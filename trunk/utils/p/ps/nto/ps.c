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





#include "ps.h"
#include "output.h"
#include "filter.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/procfs.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/dcmd_chr.h>
#include <sys/mman.h>
#include <dirent.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <pwd.h>

#ifdef __USAGE
%C - report process status (POSIX)

%C -[aAdEfl][-[gG] grp][-o format]... [-n name][-p proc][-t term][-[uU] usr]

Options
-a              Write information for all processes associated with terminals.
-A              Write information for all processes.
-d              Write information for all processes, except session leaders.
-e              Write information for all processes.
-f              Generate full listing.
-g grplist      Write information for processes whose session leader is given
                in the comma- and space-delimited 'grouplist'.
-G grplist      Write information for processes whose real group ID is given
                in the comma- and space-delimited 'grouplist'.
-l              Generate long listing.
-n namelist     Name of an alternate system namelist file. (Not supported)
-o format       Write information according to the specifications given in
                'format'. When -o options are specified, they are treated as a
                concatentation of all the 'format' arguments.
-p proclist     Write information for processes whose ID is given in the
                comma and space-delimited 'proclist'.
-t termlist     Write information for processes whose terminal identifier is
                given in the comma and space-delimited 'termlist'.
-u usrlist      Write information for processes whose user ID or login 
                name is given in the comma and space-delimited 'userlist'.
-U usrlist      Write information for processes whose real user ID or login 
                name is given in the comma and space-delimited 'userlist'.

If a process meets any of the selection criteria its information is displayed.

Output
If a format is not supplied with the -o option, a default one is used. The 
'format' argument is a comma- or space-delimited list of field names. These
field names represent information to be displayed about processes and threads.
The header for a field can be overridden by appending an equals sign and the 
new header. The rest of the characters in the argument shall be used as the 
header text. The 'ps' utility automatically determines the width of all 
columns.

Field Names
The following field names are recognized by 'ps':
ruser    The real user name or real user ID of the process.
user     The effective user name or effective user ID of the process.
rgroup   The real group name or real group ID of the process.
group    The effective user name or effective group ID of the process.
pid      The process ID.
ppid     The parent process ID.
pgid     The process group ID.
pcpu     The ratio of CPU time used to CPU time available in a recent
         period of time.
vsz      The size of the process.
nice     The effective priority of the process.
etime    The elapsed time since the process was started.
time     The cumulative CPU time of the process.
tty      The name of the controlling terminal. (Currently not supported).
comm     The command being executed.
args     The command with all of its arguments.

The following field names are supported, even though they are not
required by POSIX.

uid      The real user name or real user ID of the process.
env      The environment in a space-delimited list.
f        The process flags.
pri      The real priority of the process.
sid      The process ID of the session leader.
suid     The user ID of the session owner.
sgid     The group ID of the session owner.
umask    The process file creation mask.
sigign   The list of signals ignored by the process.
sigpend  The list of signals pending on the process and threads.
sigqueue The list of signals queued on the process.
threads  The number of threads for the process.
stime    The process Start time.
cmd      The command being executed.
sz       The size of memory used.
          
Note: some of the above fields are thread-specific. In such
cases, the value for the first thread in the process is used.
The following field names display information about threads
           
tid      The thread ID.    
tflags   The thread flags.      
dflags   The thread debug flags.
tsigblk  The list of signals blocked on the thread.
cpu      The last cpu the thread ran on.
state    The state of the thread.
wchan    The wait channel.
addr     The wait address.
psched   The process scheduling algorithm.


The comm, args and env fields are the only fields that can
contain blanks. 

NOTE: The default output has changed to match posix. To get
the old output use the options "-A -o pid,pgid,sid,args"

Return Values
0       On success.
-1      On error.
#endif

/* Apparently the going rate for buffers */
#define BUFFER_SIZE 512

const char *delimiters = ", ";
time_t current_time;

int 
getArgv (int fd, procfs_info *pInfo, struct _ps *pPs, char *path, size_t pathLength)
{
	int i, l, argc = 0;
	char *argv;
	char *p = path;

	//todo: get the full, long path of processes

	// read in argc and argv from the bottom of the stack
	if (lseek64 (fd, pInfo->initial_stack, SEEK_SET) == -1)
		return -1;

	if (read (fd, &argc, sizeof (argc)) < sizeof (argc))
		return -1;

	pathLength--; // leave room for the terminating byte
	for (i = 0; (i < argc) && (pathLength > 0); i++)
	{
		// iterate through argv[0..argc-1] by reading argv[i] and seeking to 
		// location argv[i] in the addr space, and then reading the string
		// at that location in the addr space
		if (lseek64 (fd, 
			pInfo->initial_stack + (1 + i) * sizeof (void *), SEEK_SET) == -1)
			return -1;
		if (read (fd, &argv, sizeof (argv)) < sizeof (argv))
			return -1;
		if (lseek64 (fd, (uintptr_t)argv, SEEK_SET) == -1)
			return -1;

		l = read (fd, p, pathLength);
		if (l == -1)
			return -1;

		if (pPs->comm[0] == 0)	 // argv[0] is the command name
			strncpy (pPs->comm, p, l);

		// we need to find the end of the string because read() will
		// read past the '\0'
		while (*p && l)	
		{
			p++;
			l--;
			pathLength--;
		}

		if(pathLength > 0){
			*p++ = ' ';   // add a space between arguments
			pathLength--;
		}
		if(pathLength > 0)
			*p = 0; // preserve our command line now in case we have an error
	}
	p[-1] = 0; 	// remove the trailing space
	return argc;
}

int 
getEnv (int fd, procfs_info *pInfo, struct _ps *pPS, int argc, char *env, int envLength)
{
	int i, l;
	char *argv;
	char *p = env;

	i = 0;
	envLength--; // leave room for the terminating byte
	// read the environment for the process; it follows argc and argv on the stack
	while (1)
	{	
		// reading in char **env is the same as reading in char **argv, except that 
		// we read until we reach a null-pointer sentinel; also, when reading
		// from the bottom of the stack, we must skip argc (1 integer), argv (an 
		// argc number of pointers), and the null pointer sentinel for argv
		if (lseek64 (fd, pInfo->initial_stack 
			+ (argc + 2 + i) * sizeof (void *), SEEK_SET) == -1)
			return -1;
		if (read (fd, &argv, sizeof (argv)) < sizeof (argv))
			return -1;
		if (argv == 0) // a NULL-pointer terminates the enivronment list
			break;
		if (lseek64 (fd, (uintptr_t)argv, SEEK_SET) == -1)
			return -1;

		//opt:read straight into our env buffer, so we don't copy things twice
		l = read (fd, p, envLength);
		if (l == -1)
			return -1;

		while (*p && l)
		{
			p++;
			l--;
			envLength--;
		}

		*p++ = ' ';
		envLength--;
		*p = 0; // preserve our env now in case we have an error later
		i++;
	}
	p[-1] = 0;	// remove the trailing space
	return 0;
}

int
do_ps ()
// this function collects info for each process by iterating through /proc
// errors are handled by just ignoring them and moving onto the next process
{
	int fd;
	procfs_info info;	
	char buffer[512];
	char comm[512];	
	char path[1024];
	char env[1024];
	int i, argc;
	struct _ps ps;
	procfs_status tinfo;
	procfs_mapinfo *map = 0;
	int tcount;
	int mapCount;
	struct
	{
		procfs_debuginfo debug;
		char path[1024];
	} mapdebug;
	DIR *dir;
	struct dirent *dirent;

	dir = opendir ("/proc");
	if (dir == 0)
	{
		fprintf (stderr, "Unable to open /proc.\n");
		return -1;
	}

	while (dirent = readdir (dir))
	{
		if(!isdigit(dirent->d_name[0])) continue;
		// open the addr space of the process
		sprintf (buffer, "/proc/%s/as", dirent->d_name);
		fd = open64 (buffer, O_RDONLY);
		if (fd < 0)
			continue;

		// get the basic stats on the process 
		if (devctl (fd, DCMD_PROC_INFO, &info, sizeof (info), 0) != EOK)
			continue; 	
		
		ps.pid    = info.pid;
		ps.ppid   = info.parent;
		ps.pgid   = info.pgrp;
		ps.user   = info.euid;
		ps.ruser  = info.uid;	//todo:get the user name as well
		ps.group  = info.egid;
		ps.rgroup = info.gid;	//todo:get the group name as well
		ps.pflags = info.flags;
		ps.sid    = info.sid;
		ps.suid   = info.suid;
		ps.sgid   = info.sgid;
		ps.umask  = info.umask;
		ps.sigign = info.sig_ignore;
		ps.sigpend = info.sig_pending;
		ps.sigqueue = info.sig_queue;
		ps.threads = info.num_threads;
		ps.stime = ps.etime  = info.start_time / 1000000000;
		if(ps.etime > current_time) {
			ps.etime = 0;
		} else {
			ps.etime = current_time - ps.etime;
		}
		ps.time   = (info.utime + info.stime + 500000000) / 1000000000;
		ps.pcpu   = ps.etime ? (ps.time / ps.etime) * 100 : 0;

		//todo: the following stats are not yet stored in the kernel
		//todo: ps.tty = -1;

		// get the command line for the process
		ps.comm = comm;	ps.comm[0] = 0;
		ps.args = path;	ps.args[0] = 0;
		ps.env  = env;	ps.env[0]  = 0;
		
		argc = -1;
		if(needArgs)
			argc = getArgv (fd, &info, &ps, &path[0], sizeof (path));
		if (needEnv && argc != -1)
			getEnv (fd, &info, &ps, argc, &env[0], sizeof (env));

		// iter through all the mem blocks in the process
		// first we call DCMD_PROC_MAPINFO with no buffer to get the number
		// of memory blocks in mapCount, then we get map and iter through it
		if (1) {
			if (devctl (fd, DCMD_PROC_MAPINFO, 0, 0, &mapCount) != EOK)
				continue;
			map = malloc (sizeof (*map) * mapCount); 		
			if (map == 0)
				continue;
			if (devctl (fd, DCMD_PROC_MAPINFO, map, sizeof (*map) * mapCount, &mapCount) != EOK)
				continue;
			ps.vsz = 0;
			ps.sz = 0;
			for (i = 0; i < mapCount; i++)
			{	
				// we need to give DCMD_PROC_MAPDEBUG the vaddr of the memory block
				// whose name we would like
				mapdebug.debug.vaddr = map[i].vaddr; 
				if (devctl (fd, DCMD_PROC_MAPDEBUG, &mapdebug, sizeof (mapdebug), 0) < 0)
					continue;
				ps.vsz += map[i].size; 	
				if(map[i].flags & MAP_SYSRAM)
					ps.sz += map[i].size;
	
				// process 1 doesn't have any command line args, so we cheat a little
				// and use the name of its base memory block as its command line
				if ((ps.pid == 1) && (map[i].flags & MAP_ELF) && (ps.comm[0] == '\0'))
				{
					strcpy (ps.comm, mapdebug.debug.path);
					strcpy (ps.args, mapdebug.debug.path);
				}
				
			}
			free (map);	
		}

		// this code gets info for each thread in the process
		tcount = 0;    // how many threads we have looked at
		tinfo.tid = 1; // start at the first thread
			
		switch (filter_ps (&ps))
		{
		case 0:
			close (fd);	
			continue;

		case -1: // insufficient memory, so quit
			return -1;

		case 1:
			// continue on w/ displaying the process info
			break;
		}

		while (1)
		{
			// there can be holes in thread numbers, so skip missing threads
			if (devctl (fd, DCMD_PROC_TIDSTATUS, &tinfo, sizeof (tinfo), 0) == EOK)
			{	
				ps.tid 		= tinfo.tid;
				ps.tflags 	= tinfo.tid_flags;
				ps.dflags 	= tinfo.flags;
				ps.nice		= tinfo.priority;
				ps.pri 		= tinfo.real_priority;
				ps.tsigpend 	= tinfo.sig_pending;
				ps.tsigblk 	= tinfo.sig_blocked;
				ps.cpu 		= tinfo.last_cpu;
				
				print_ps (&ps);

				tcount++;
				// stop when we have gone through each thread, or when
				// the user only want process info
				if ((usingThreads == 0) || (tcount == info.num_threads))
					break;
			}
			tinfo.tid++;
		}
		close (fd);	
	}
	closedir (dir);
	return 0;
}

void displayHelp (char *msg)
{
	fprintf (stderr, "%s", msg);
	fprintf (stderr, "For help on running the 'ps' utility, enter the command line \"use ps\".\n");
}

static char* resolveUsers(const char *listIn){
	char *listOut = malloc(BUFFER_SIZE);
	char *listCopy = malloc(BUFFER_SIZE);
	char *token;
	char *tokbuf;
	char *tmp = malloc(BUFFER_SIZE);
	int dflag = 0;
	int i;
	
	if (listOut == NULL || listCopy == NULL)
		return NULL;

	/* Make a copy of the list */
	strncpy(listCopy, listIn, BUFFER_SIZE);
	listCopy[BUFFER_SIZE-1] = '\0';

	token = strtok_r (listCopy, ",", &tokbuf);
	while (token){
		/* For this user, look up the UID */
		/* Note: UID may be both a username or numerical UID */
		/* Nothing stops a username from being numerical */
		struct passwd *pw = getpwnam(token);
		/* Only add it to the list if we found it */
		if (pw && pw->pw_uid >= 0){
			/* Add the UID to the lookup*/
			snprintf(tmp, BUFFER_SIZE, "%s,%d", listOut, pw->pw_uid);
			strncpy(listOut, tmp, BUFFER_SIZE);
		}
		/* If by some off chance, they added a numerical to the list, */
		/* If they were really adding a UID, the above would not have resolved */
		/* And therefore, it's up to here to add it */
		dflag = 1;
		for(i = 0; i < strlen(token); i++){
			if (!isdigit(token[i])){
				dflag = 0;
				break;
			}
		}
		if (dflag){
			snprintf(tmp, BUFFER_SIZE, "%s,%s", listOut, token);
			strncpy(listOut, tmp, BUFFER_SIZE);
		}
		token = strtok_r (0, ",", &tokbuf);
	}
	/* Free all our temporary buffers */
	free(tmp);
	free(listCopy);
	return listOut;
}

int
main (int argc, char **argv) 
{
	int c;
	struct winsize	ws;
	char *p;

	columns = 0;
	if((p = getenv("COLUMNS")) && isatty(fileno(stdout)))
		columns = atoi(p);
	else if(devctl(fileno(stdout), DCMD_CHR_GETSIZE, &ws, sizeof ws, 0) == EOK)
		columns = ws.ws_col;

	current_time = time(0);

	//todo: display more verbose error messages
	// parse the command line
	opterr = 0;
	while (1)
	{
		c = getopt (argc, argv, "Aadefg:G:ln:o:p:t:u:U:");
		if (c == -1)
			break;
		switch (c)
		{
		case 'A': 
		case 'e': 
			// display information about all processes
			allProcs = 1;
			break;
		case 'a':
			// display information about all processes
			// attached to terminals
			termProcs = 1;
			break;
		case 'd': 
			// display information about all processes
			sessionProcs = 1;
			break;
		case 'f': 
			// display full information
			fullListing = 1;
			break;
		case 'l': 
			// display long information
			longListing = 1;
			break;
		case 'g':
			groupSession = 1;
			// Fall Through
		case 'G':
			// display info about processes whose gid
			// is in the given list
			if (!is_valid_filter_list (optarg))
			{
				displayHelp ("Invalid filter list for option -G.\n");
				return -1;
			}
			groupList = optarg;
			break;
		case 'n':
			break;
		case 'o':
			// get the user's desired layout for the output
			if (parse_header (optarg) == -1)
			{
				displayHelp ("Unrecognized field name for option -o.\n");
				return -1;
			}
			break;
		case 'p':
			// display info about processes whose pid
			// is in the given list
			if (!is_valid_filter_list (optarg))
			{
				displayHelp ("Invalid filter list for option -p.\n");
				return -1;
			}
			procList = optarg;
			break;
		case 't':
			// display info about processes whose 
			// controlling terminal is in the given list
			if (!is_valid_filter_list (optarg))
			{
				displayHelp ("Invalid filter list for option -t.\n");
				return -1;
			}
			termList = optarg;
			break;
		case 'u':
			userEffective = 1;
			// Fall Through
		case 'U':
			// display info about processes whose uid
			// is in the given list
			if (!is_valid_filter_list (optarg))
			{
				displayHelp ("Invalid filter list for option -U.\n");
				return -1;
			}
			userList = resolveUsers(optarg);
		
			// get the user's login name so that we
			// can match it as we search in the user list
			if (userName == 0)
				userName = getlogin ();
			break;
		case '?':			
			displayHelp ("Unrecognized option.\n");
			return -1;
		}
	}
	if (optind != argc)
	{
		// the user specified arguments we don't understand
		displayHelp ("Unrecognized option.\n");
		return -1;
	}

	if (fieldCount == 0)
	{
		// These come from the POSIX 1003.1-200x 
		if(longListing)						parse_header("f,s");
		if(longListing || fullListing) 		parse_header("uid");
											parse_header("pid");
		if(longListing || fullListing) 		parse_header("ppid,c");
		if(longListing)						parse_header("pri,nice,addr,sz,wchan");
		if(fullListing)						parse_header("stime");
											parse_header("tty_tty,time,cmd");
	}
	print_header (); // print the title for each field
	return do_ps ();// display info about process 1
}
