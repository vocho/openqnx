/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems. All Rights Reserved.
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
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <sys/netmgr.h>
#include <sys/shutdown.h>

extern int spawn_remote_shutdown(char *node_name, char **argv);

static int flags = FLAG_VERBOSE | FLAG_NO_DAEMON;
struct termios termattr;

static const char *class_names[] =
{
	"photon apps",
	"apps",
	"daemons",
	"filesystems",
	"display drivers"
};


int main(int argc, char *argv[])
{
	int opt, nd = ND_LOCAL_NODE;
	char *node_name = NULL;
	int shutdown_type = SHUTDOWN_REBOOT;

	tcgetattr(1, &termattr);

	while((opt = getopt(argc, argv, "bDfn:qS:v")) != -1)
		switch(opt)
		{
			case 'b':
				shutdown_type = SHUTDOWN_SYSTEM;
				break;

			case 'S':
				switch(tolower(*optarg))
				{
					case 'r':
						shutdown_type = SHUTDOWN_REBOOT;
						break;
					case 's':
						shutdown_type = SHUTDOWN_SYSTEM;
						break;
					default:
						fprintf(stderr,"Invalid shutdown type (%s).\n",optarg);
						exit(EXIT_FAILURE);
				}

				break;

			case 'D':
				flags |= FLAG_DEBUG;
				break;

			case 'f':
				flags |= FLAG_FAST;
				break;

			case 'n':
				node_name = optarg;
				nd = netmgr_strtond(optarg, NULL);
				if(nd == -1){
					perror("No such node");
					exit(EXIT_FAILURE);
				}
				break;

			case 'q':
				flags &= ~(FLAG_VERBOSE | FLAG_VERY_VERBOSE);
				break;

			case 'v':
				flags |= FLAG_VERBOSE | FLAG_VERY_VERBOSE;
				break;

			default:
				fprintf(stderr,"Invalid option (%c).\n",opt);
				exit(EXIT_FAILURE);
		}

	if(node_name && nd != ND_LOCAL_NODE){
		if(spawn_remote_shutdown(node_name, argv) == -1){
			perror("unable to spawn on remote node");
			exit(EXIT_FAILURE);
		}
	}
	else {
		shutdown_system(shutdown_type,flags);
	}
	exit(EXIT_SUCCESS);
}


void shutdown_display(int type,DisplayData_t const *ddata)
{

	// This seems silly but some apps reprogram the console to raw
	// (no opost) when they die which messes up the verbose output.
	tcsetattr(1, TCSANOW, &termattr);

	switch(type)
	{
		case DISPLAY_CLASS:
			printf("Shutting down %s...\n",class_names[ddata->proc_class]);
			break;

		case DISPLAY_PROC:
			printf("   %s\n",ddata->proc_name);
			break;

		default:
			break;
	}

}


void shutdown_error(char const *msg)
{
	fprintf(stderr,"Error:  %s\n",msg);
}


int shutdown_classify(ProcessInfo_t const *pip)
{
	return(CLASS_APP);
}



int shutdown_prompt(char const *name,pid_t pid)
{
	if(flags & FLAG_VERY_VERBOSE)
		printf("\t%s (%u) not responding, sending SIGKILL...\n",name,pid);

	return(PROMPT_KILL);
}


void shutdown_done(int type)
{
	if(type == SHUTDOWN_SYSTEM)
	{
		printf("\f\n\n\n\n\n\n\n\n\n");
		printf("                             Shutdown Complete\n");
		printf("                  It is now safe to reboot your computer");
		fflush(stdout);
	}
}


void shutdown_process(void)
{
}


void shutdown_notify(void)
{
}


void shutdown_progress(int done,int total)
{
}
