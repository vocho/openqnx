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



 
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/vc.h>
#include <string.h>

#include <stdutil.h>

#ifdef __USAGE
%C - update filesystems to match cached data (UNIX)

%C	[node]
Where:
  node      is the node to synchronize (default is the node the command is
            run on).
#endif


main(int argc, char *argv[])
	{
	nid_t	nid = 0;
	int		status = 0;

	if(argc > 2)
		{
		fprintf(stderr, "Use: %s [node]\n", argv[0]);
		exit(0);
		}
	else if(argc == 2) {
		nid = strtonid(argv[1],NULL);

		if (nid==0 || nid==-1) {
			fprintf(stderr,"Invalid node specified (%s).\n",argv[1]);
			exit(EXIT_FAILURE);
		}
	}

	if(qnx_sync(nid) != 0)
		{
		status = EXIT_FAILURE;
		switch(errno)
			{
		case	ESRCH:
			fprintf(stderr, "Cannot locate Fsys on node %s.\n",
						nidtostr(((nid != 0) ? nid : getnid()),NULL,0) );
			break;

		case	ENOSYS:
		case	EOPNOTSUPP:
			fprintf(stderr,
				"Node %s is running an old Fsys which doesn't support sync.\n",
					nidtostr((nid?nid:getnid()),NULL,0));
			break;

		default:
			fprintf(stderr,
				"Cannot sync node %s (%s).\n",
					nidtostr((nid?nid:getnid()),NULL,0),
					strerror(errno)
			);
			}
		}

	exit(status);
	}
