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





#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <process.h>
#include <errno.h>

#ifdef __QNXNTO__
#include <sched.h>
#else
#include <sys/sched.h>
#endif


int main(int argc, char **argv) {
    int increment=0;
	int error=0, rc, c;
	int current_priority, min_increment, max_increment;
	char *nump=NULL, *endp;


	if (argc<2) {
		fprintf(stderr,"nice: no command line arguments supplied\n");
		exit(EXIT_FAILURE);
	}

	opterr=0;	/* disable getopt error for bad option */

	/* there may be only one option specified */
	c=getopt(argc,argv,"n:");

	switch(c) {
		case 'n':
			increment=strtol(nump=optarg,&endp,10);
			break;
		case '?':
		case -1:
        	if (argv[1][0]!='-')
        		break;
		/* if we got a negative number --something which getopt saw as
		   end-of-options, fall through and let strtol see the
		   negative number - don't forget to increment optind */
		if(argv[1][1] == '-')
		     optind++;
		default:
			increment=strtol(nump=&argv[1][1],&endp,10);
			break;
	}	

	if (optind>=argc) {
		fprintf(stderr,"nice: no command name specified\n");
		error++;
	}

	current_priority=getprio(0);
    max_increment=current_priority-1;
	min_increment=-(sched_get_priority_max(sched_getscheduler(0))-current_priority);

	if (nump!=NULL) {	/* an increment was specified */
		if (*endp) {
            increment=0;
			fprintf(stderr,"nice: bad increment (%s) must be a decimal number between %d and %d\n",nump,min_increment,max_increment);
            error++;
		}
	} else increment = 1; /* default nice value is 1 priority level */

    if (increment>max_increment || increment<min_increment) {
	    fprintf(stderr,"nice: current priority is %d. The increment must be %d<=n<=%d.\n",
                       current_priority,min_increment,max_increment
               );
        error++;
	}

	if (error) exit(EXIT_FAILURE);

	if (-1==setprio(0, current_priority - increment)) {
		fprintf(stderr,"nice: could not change priority to %d from %d (%s)\n",current_priority-increment,current_priority,strerror(errno));
		exit(EXIT_FAILURE);
	}

   	execvp(argv[optind], &argv[optind]);

	if (errno==ENOENT) rc=127;
	else rc=126;

    fprintf(stderr, "nice: %s (%s)\n", argv[optind], strerror(errno));

    return(rc);
}

