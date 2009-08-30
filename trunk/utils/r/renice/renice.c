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
	Revision 1.5  2006/04/11 16:16:15  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.4  2005/06/03 01:37:58  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.3  2003/08/28 20:23:47  martin
	Update QSSL Copyright notice.
	
	Revision 1.2  2000/02/14 20:46:44  jbaker
	fixed memory fault under neutrino 2.
	Also added some error checking code.
	
	Re: PR 1957
	
	Revision 1.1  1998/10/23 01:37:57  dtdodge
	Initial cvs checkin.
	
 * Revision 1.4  1996/06/05  16:04:36  brianc
 * blat
 *
 * Revision 1.3  1992/10/27  17:47:26  eric
 * added usage one-liner, cleaned up usage
 *
 * Revision 1.2  1992/07/09  14:17:53  garry
 * Editted usage
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <process.h>
#include <pwd.h>
#include <errno.h>
#ifdef __QNXNTO__
#include <sys/procfs.h>
#include <fcntl.h>
#include <dirent.h>
#include <sched.h>
#include <sys/neutrino.h>
#else
#include <sys/sched.h>
#include <sys/psinfo.h>
#include <sys/kernel.h>
#define MAX_NICE	31
#define MIN_NICE	0
#endif
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>

int		nice_value = 0;
int		Npids, nuser;
pid_t	Pidlist[150];
int		uid[150];

int		status = EXIT_SUCCESS;

#ifdef __QNXNTO__
#define PRIO_MAX sched_get_priority_max( SCHED_RR )
#define PRIO_MIN sched_get_priority_min( SCHED_RR )
#endif


void
add_pid( pid_t pid ) {
	int		i;

	for( i = 0; i < Npids; i++ )
		if ( Pidlist[i] == pid )
			break;

	if ( i == Npids )
		Pidlist[Npids++] = pid;
	}

int
notopt( char *x ) {
    if (x!=NULL) {
		if ( *x == '-' )
			switch( *(x+1) ) {
				case 'p':
				case 'g':
				case 'u':	return( 0 );
				default:	return( 1 );
				}
	}
	return( 1 );
	}

int
AtoI( char *s ) {
	char	*p;
	long	I;

	I = strtol( s, &p, 10 ); 
	if ( ((I == LONG_MAX || I==LONG_MIN)&&errno)  ||  *p ) {
		fprintf( stderr, "Bad numeric argument '%s'\n", s );
		exit( EXIT_FAILURE );
		}
	return I;
	}

int
look4user( int Uid ) {
	int		i;

	for( i = 0; i < nuser; i++ ) {
		if ( uid[i] == Uid )
			return( 1 );
		}
	return( 0 );
	}

int
look4pid( pid_t pid, pid_t pid_group ) {
	int		i;

	for( i = 0; i < Npids; i++ ) {
		if ( Pidlist[i] > 0 ) {
			if ( Pidlist[i] == pid )
				return( 1 );
			}
		else {
			if ( -Pidlist[i] == pid_group )
				return( 1 );
			}
		}
	return( 0 );
	}

int
main( argc, argv )
	int		argc;
	char	*argv[];
	{
	char	*p;
	long	I;
	struct	passwd	*pass;
        int ind=2;
        int current=PRIO_PROCESS;
        long testid;
#ifdef __QNXNTO__
	DIR				*dp;
	struct dirent	*dirp;
	int				fd;
	char			buf[32];
	procfs_status	pstatus;
	procfs_info		pinfo;
	sched_param_t	param;
#else
	pid_t	id;
	int		npri;
	struct	_psinfo	psinfo;
#endif


  if (argc<3)
  {
    fprintf(stderr, "Usage: renice increment [-p pid ...] [-g gid ...] [-u user ...]\n");
    exit(1);
  }
  if ( !(nice_value=atoi(argv[1])) && (strcmp(argv[1], "0") !=0) )
  {
    fprintf(stderr, "Invalid priority, '%s'\n", argv[1]);
    exit(1);
  }
  for (;ind<argc;ind++)
  {
    if(strcmp(argv[ind], "-p"))
      if(strcmp(argv[ind], "-g"))
        if(strcmp(argv[ind], "-u"))
          if(!(testid=strtol(argv[ind], NULL, 0))&&(strcmp(argv[ind], "0")!=0))
          {
            fprintf(stderr, "Invalid Option, '%s'\n", argv[ind]);
            exit(1);
          }
  }
  ind=2;
  for (;ind<argc;ind++)
  {
    if(!strcmp(argv[ind], "-p"))
    {
      current=PRIO_PROCESS;
    }
    else
    {
      if(!strcmp(argv[ind], "-g"))
      {
        current=PRIO_PGRP;
      }
      else
      {
        if(!strcmp(argv[ind], "-u"))
        {
          current=PRIO_USER;
        }
        else
        {
          if (current==PRIO_PROCESS) add_pid(strtol(argv[ind], NULL, 0)); 
          else
            if (current==PRIO_PGRP) break;  
            else
              if (current==PRIO_USER)
              { 
                if (!(pass=getpwnam( argv[ind])))
                {
                  I=strtol(argv[ind], &p, 10);
                  if (((I==LONG_MAX || I==LONG_MIN)&&errno) || *p) 
                  {
                    fprintf(stderr, "No user '%s'\n", argv[ind] );
                  }
                  uid[nuser]=(unsigned)I;
                }  
                else
                {
                  uid[nuser]=pass->pw_uid;
                }
                nuser++;
              }
        }
      }
    }
  }

#ifdef __QNXNTO__

  if((dp = opendir("/proc")) == NULL)
    return(errno);
  fd = -1;
  while((dirp = readdir(dp))) 
  {

    if(fd != -1)
      close(fd);
    sprintf( buf, "/proc/%s", dirp->d_name);
    if((fd = open(buf, O_RDONLY)) == -1)
      continue;
    if(devctl(fd, DCMD_PROC_INFO, &pinfo, sizeof(pinfo), 0) != EOK)
      continue;
    if(!look4user(pinfo.uid)  &&  !look4pid(pinfo.pid, pinfo.pgrp))
      continue;
    for(pstatus.tid = 1 ; ; ++pstatus.tid) 
    {
      if(devctl(fd, DCMD_PROC_TIDSTATUS, &pstatus, sizeof(pstatus), 0) != EOK)
        break;
      param.sched_priority = pstatus.priority - nice_value;
      if (param.sched_priority<PRIO_MIN) param.sched_priority=PRIO_MIN;
      if (param.sched_priority>PRIO_MAX) param.sched_priority=PRIO_MAX;
      if(SchedSet(pstatus.pid, pstatus.tid, SCHED_NOCHANGE, &param) == -1) 
      {
        fprintf(stderr, "SchedSet on pid.tid %d.%d: ", pstatus.pid, pstatus.tid);
        perror( "" );
        status = EXIT_FAILURE;
      }
    }

  }
#else
	for(id = 1 ; ( id = qnx_psinfo( PROC_PID, id, &psinfo, 0, 0 ) ) != -1; id++ ) {
		if ( !look4user( psinfo.ruid )  &&  !look4pid( psinfo.pid, psinfo.pid_group ) )
			continue;
		npri = psinfo.priority - nice_value;
		if ( npri < MIN_NICE  ||  npri > MAX_NICE ) {
			fprintf( stderr, "Priority out of range for pid %d\n", id );
			continue;
			}
		if ( setprio( id, npri ) == -1 ) {
			fprintf( stderr, "setprio on pid %d: ", id );
			perror( "" );
			status = EXIT_FAILURE;
			}
		}
#endif

	return( EXIT_SUCCESS );
	}
