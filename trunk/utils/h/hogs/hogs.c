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
%C	[options] [pids ...]

Options:
 -n            Show names of processes.
 -p config     Run hogs at this priority (default: same as creator)
 -s sec        Sleep this long between updates (default: 3).
 -% num        Only show processes which consume this percentage or more cpu
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/debug.h>
#include <sys/procfs.h>
#include <sched.h>

#define BUFFER_GROW		10

char *get_name(int fd, procfs_info *info) {
	char			buf[200];
	char			*args[1];
	int				argc;
	int				valid;

	// Seek to bottom of stack
	if(lseek64(fd, info->initial_stack, SEEK_SET) == -1  ||
	   (valid = read(fd, &argc, sizeof(char *))) < sizeof(char *))
		return(NULL);

	// Read argv. Only the first entry which is the command name.
	if(read(fd, args, 1 * sizeof args[0]) < 1 * sizeof args[0])
		return(NULL);

	if(lseek64(fd, (int) args[0], SEEK_SET) == -1)
		return(NULL);

	if((valid = read(fd, buf, sizeof(buf) - 1)) == -1)
		return(NULL);

	if(memchr(buf, 0, valid) == 0)
		return(NULL);

	return(strdup(basename(buf)));
}


int main(int argc, char *argv[]) {
	int		i;
	int		fd;
	int		name;
	int		sec;
	int		pri;
	char		buf[30];
	char		**pids;
	char		**names;
	int		percentage;
	int		first;
	int		sum;
	int		*new, *old, *dif;
	DIR		*dir;
	procfs_info	info;
	int		num_entries;

	// Parse options.
	name = 0;
	sec = 3;
	percentage = 0;
	pri = getprio(0);

	// allocate memory for BUFFER_GROW processes
	pids = (char **) malloc(sizeof(*pids) * BUFFER_GROW +1 ); 
	names = (char **) malloc(sizeof(*names) * BUFFER_GROW +1 ); 
	new = (int *) malloc( sizeof (int *) * BUFFER_GROW +1 );
	old = (int *) malloc( sizeof (int *) * BUFFER_GROW +1 );
	dif = (int *) malloc( sizeof (int *) * BUFFER_GROW +1 );
	num_entries = BUFFER_GROW;
 
 		while ((i = getopt(argc, argv, "np:s:%:")) != -1)
		switch(i) {
		case 'n':
			name = 1;
			break;

		case 'p':
			pri = atoi(optarg);
			break;

		case 's':
			sec = atoi(optarg);
			break;

		case '%':
			percentage = atoi(optarg);
			break;

		default:
			fprintf(stderr, "Invalid option '%c'\n", i);
			exit(EXIT_FAILURE);
		}

	if(sec<1)
		sec = 1;
	printf("PID             NAME  MSEC  PIDS SYSTEM\n");

	// Collect pids
	for(i = 0 ; optind < argc ; ++optind, ++i) {
		pids[i] = argv[optind];
	}
	pids[i] = 0;

	// If no pids were given assume you want all of them.
	dir = NULL;
	if(i == 0)
		if(!(dir = opendir("/proc"))) {
			fprintf(stderr, "Couldn't open /proc: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

	setprio(0, pri);
	for(first = 1;;) {
		if(dir && first) {
			struct dirent		*dirent;

			// Remove names which were strdup from last run.
			for(i = 0 ; pids[i] ; ++i) {
				free(pids[i]);
				if(names[i]) 
					free(names[i]);
				names[i] = 0;
			}

			rewinddir(dir);
			i = 0;
			while((dirent = readdir(dir))) {
				if(dirent->d_name[0] <= '0' || dirent->d_name[0] > '9')
					continue;
				if ( i >= num_entries ) {
					// allocate memory for more processes
					num_entries += BUFFER_GROW;
					pids = (char **) realloc( pids, sizeof(*pids) * num_entries );
					names = (char **) realloc( names, sizeof(*names) * num_entries);
					new = (int *) realloc( new, sizeof(int) * num_entries );
					old = (int *) realloc( old, sizeof(int) * num_entries );
					dif = (int *) realloc( dif, sizeof(int) * num_entries );
				}
				names[i] = 0;
				pids[i++] = strdup(dirent->d_name);
			}
			pids[i] = 0;
		}
	
		for(i = 0 ; pids[i] ; ++i) {
			// Open pid and get basic process info.
			sprintf(buf, "/proc/%s/as", pids[i]);
			if((fd = open(buf, O_RDONLY)) == -1 ||
			   devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0) != EOK) {
				info.stime = info.utime = 0;
			}
	
			// Save in msec
			new[i] = (info.stime + info.utime)/1000000;
			if(name) {	
				// don´t allocate the name twice
				if (!names[i]) {
					names[i] = get_name(fd, &info);
				}
			}
			else {
				names[i] = 0;
			}

			close(fd);
		}

		// Calculate sum and diffrences
		for(i = sum = 0 ; pids[i] ; ++i) {
			dif[i] = max(1, new[i] - old[i]);
			old[i] = new[i];
			sum += dif[i];
			}

		// Don't print on first loop since we need a difference over sec
		if(!first) {
			for(i = 0 ; pids[i] ; ++i) {
				if((dif[i]*100)/sum < percentage)
					continue;
				printf("%-7s %12s %5d  %3d%%   %3d%%\n",
					pids[i],
					names[i] ? names[i] : "",
					dif[i],
					(dif[i]*100)/sum,
					(dif[i]*100)/(sec*1000));
			}
			printf("\n");
		}
		first = dir ? !first : 0;

		if(dir == 0 || first == 0)
			sleep(sec);
	}

	exit(EXIT_SUCCESS);
}
