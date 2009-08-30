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



#include <sys/syspage.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <libgen.h>
#include <signal.h>
#include <spawn.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <login.h>
#include <sched.h>
#include <sys/sched_aps.h>
#include <util/util_limits.h>

#ifdef __USAGE
%-on
%C - execute a command on another node or tty (QNX)
%C [-d][-h][-n|f node][-p prio[r|f|o]][-s][-t tty][-u|l user][-C cpu][-R runmask][-X sched][-W nsec][-w name] command
Options:
-d        detach (nozombie)
-h        spawn held (for debugging)
-n node   spawn on remote node
-f node	  spawn from remote node 
-p prio   spawn with priority; optionally, can follow with
          changed scheduling policy (r|f|o).
-P        spawn with SPAWN_PADDR64_SAFE
-s        spawn new process group
-t tty    spawn new session on tty
-u user   change to be user
-l user   change to be user like login
-C CPU    set CPU affinity to <CPU>.  First CPU is 0.  If specified
          multiple times or in conjunction with -R, the resultant mask
          is the bitwise oring of all -[CR] options.
-R mask   set CPU Affinity to <mask>.  May be used multiple times to
          specify masks > 32 bits wide (32 bits per -R).  The lower 32
          bits (bits 31-0) are always taken from the first -R, bits
          63-32 from the second...  If used in conjuction with -C, the
          resultant mask is the bitwise oring of all -[CR] options.
-X sched  set extended scheduling parameters, current formats are:
          aps=partition
-W nsec   number of seconds to wait for -w
-w name   wait for name to exist.

%-waitfor
%C - wait for a name to exist. (QNX)

%C /path/to/name [seconds]

Example:
    # Wait up to 10 seconds for /dev/ser2 to exist
    waitfor /dev/ser2 10

#endif

extern char *__progname;

char *qnx_crypt(const char *, const char *);

int device(const char *name, int needfd) {
	char * dev_prefix = "/dev/";
	char			buff[UTIL_PATH_MAX+1];
	struct stat		st;

	if(!strchr(name, '/')) {
		if ((strlen(name)+strlen(dev_prefix)) > UTIL_PATH_MAX) {
			errno=ENAMETOOLONG;
			return -1;
		}
		name = strcat(strcpy(buff, dev_prefix), name);
	}
	return needfd ? open(name, O_RDWR) : stat(name, &st);
}

int waitfor_device( const char *name, void *handle )
{
	int ret;
	ret = device(optarg, 0);
   	if ( ret == -1 ) {
		if (errno == ENAMETOOLONG) {
			fprintf(stderr, "%s: device wait for \'%s\': %s\n", __progname, optarg, strerror(errno));
		}
		switch(errno) {
		case ENOENT:
		case ENOSYS:
		case EAGAIN:
		case ENXIO:
			return WAITFOR_CHECK_CONTINUE;
		default:
			if (errno >= ENETDOWN && errno <= EHOSTUNREACH) {
				return WAITFOR_CHECK_CONTINUE;
			}
			return WAITFOR_CHECK_ABORT;
		}
	}
	return ret;
}

int check_pass(struct passwd *pw) {
	int tries;
	enum pwdbstat_e x;

	if(pw->pw_passwd[0] == '\0') {
		return 1;
	}

	if ((x=auth_pwdb()) != PdbOk) {
		if(x != NoShadow){
			fprintf(stderr,"authorization error: %s\n",pwdb_errstr(x));
			return 0;
		}
		fprintf(stderr,"Warning: no shadow file\n");
	}

	for(tries = 0; tries < 3; tries++) {
		if (chklogin(0,pw->pw_name)) 
			return 1;
	}
	fprintf(stderr, "%s: Invalid passwd\n", __progname);
	return 0;
}

struct {
	int nd;
	pid_t pid;
	siginfo_t info;
} net_spawn;

void net_signal_record(int signo, siginfo_t* siginfo, void* data) {
	memcpy((void*)&net_spawn.info, (void*)siginfo, sizeof(siginfo_t));
}

void net_signal_deliver(int signo, siginfo_t* siginfo, void* data) {
	SignalKill(net_spawn.nd, -net_spawn.pid, 0, signo, siginfo->si_code, siginfo->si_value.sival_int);
}


int wait_for( int argc, char *argv[] )
{
    unsigned timeout_ms = 5000;
    unsigned sample_ms  = 100;
    int ret;

    if( argc < 2 )
        return EXIT_FAILURE;

    if( argc == 3 ) {
    	char *cp;

        timeout_ms = strtoul( argv[2], &cp, 0 ) * 1000;
        if( cp[0] == '.' && isdigit(cp[1]) ) {
        	timeout_ms += (cp[1] - '0') * 100;
        }
	}

    if( timeout_ms < 0 )
        timeout_ms = 5000;

	ret = waitfor( argv[1], timeout_ms, sample_ms );
	if ( ret == -1 ) {
        fprintf( stderr, "Unable to access %s\n", argv[1] );
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char *setup_extsched(char *sched, spawn_inheritance_type *inherit)
{
struct sched_query		query;

	if (inherit->flags & SPAWN_SETND && ND_NODE_CMP(inherit->nd, ND_LOCAL_NODE))
		return("Can only set extended scheduler on local node");
	switch ((SchedCtl(SCHED_QUERY_SCHED_EXT, &query, sizeof(query)) == -1) ? SCHED_EXT_NONE : query.extsched) {
	case SCHED_EXT_NONE:
		return("No extended scheduler installed");
	case SCHED_EXT_APS: {
		char					*cp;
		sched_aps_lookup_parms	aps_lookup;
		sched_aps_join_parms	aps_join;

		APS_INIT_DATA(&aps_lookup);
		APS_INIT_DATA(&aps_join);
		if (strncmp(sched, "aps=", 4))
			return("APS scheduler installed; use \"aps=partition\"");
		aps_join.pid = aps_join.tid = 0;
		aps_join.id = strtol(aps_lookup.name = &sched[4], &cp, 10);
		if (SchedCtl(SCHED_APS_LOOKUP, &aps_lookup, sizeof(aps_lookup)) != -1)
			aps_join.id = aps_lookup.id;
		else if (cp == &sched[4] || *cp != '\0')
			return("Invalid APS partition");
		if (SchedCtl(SCHED_APS_JOIN_PARTITION, &aps_join, sizeof(aps_join)) == -1)
			return("Unable to join APS partition");
		} break;
	}
	return(NULL);
}

int main(int argc, char *argv[]) {
	pid_t					pid;
	int						status;
	int						i;
	spawn_inheritance_type	inherit;
	int						count = INT_MAX;
	int						fds[3] = { 0, 1, 2 };
	int						nfds = 0;
	char					*cmd = 0;
	char					*extsched = NULL;
	unsigned				*inherit_mask, val;
	int						idone, idone_32, bigmask, *rsizep;

	rsizep = NULL;
	inherit_mask = NULL;
	idone = idone_32 = bigmask = 0;


    if( argv[0] && strcmp( basename( argv[0] ), "waitfor" ) == 0 )
        return wait_for( argc, argv );

	status = -1;
	inherit.flags = SPAWN_EXEC;
	while((i = getopt(argc, argv, "Ddhsl:f:n:p:Pu:t:C:R:X:w:W:")) != -1) {
		switch(i) {
		case 'D':	/* debug */
			inherit.flags |= SPAWN_DEBUG;
			break;

		case 'd':	/* detach */
			inherit.flags |= SPAWN_NOZOMBIE;
			break;

		case 'h':	/* hold */
			inherit.flags |= SPAWN_HOLD;
			break;

		case 's':	/* session */
			inherit.flags |= SPAWN_SETGROUP;
			inherit.pgroup = 0;
			break;

		case 'f': 
		{
			char path[PATH_MAX];

			if(netmgr_path(optarg, NULL, path, PATH_MAX) == -1) {
				fprintf(stderr, "%s: Invalid node \"%s\" (%s).\n", __progname, optarg, strerror(errno));
				return EXIT_FAILURE;
			}
			if(chroot(path) != 0) {
				fprintf(stderr, "%s: Can not chroot \"%s\" (%s).\n", __progname, path, strerror(errno));
				return EXIT_FAILURE;
			}
		}
			/* full through */

		case 'n':	/* remote */
			if((inherit.nd = netmgr_strtond(optarg, 0)) == -1) {
				fprintf(stderr, "%s: Invalid node \"%s\" (%s).\n", __progname, optarg, strerror(errno));
				return EXIT_FAILURE;
			}
			inherit.flags |= SPAWN_SETND;
			if(ND_NODE_CMP(inherit.nd, ND_LOCAL_NODE)) { 
				inherit.flags &= ~SPAWN_EXEC;
				net_spawn.nd = inherit.nd;
			}
			break;

		case 'p':	/* priority */
			inherit.param.sched_priority = strtol(optarg, &optarg, 10);
			switch (*optarg) {
			case 'f':
				inherit.policy = SCHED_FIFO;
				break;
			case 'o':
				inherit.policy = SCHED_OTHER;
				break;
			case 'r':
				inherit.policy = SCHED_RR;
				break;
			default:
				inherit.policy = sched_getscheduler(0);
				break;
			}
			inherit.flags |= SPAWN_EXPLICIT_SCHED;
			status = 0;
			break;
		case 'P':  /* paddr64 safe */
			inherit.flags |= SPAWN_PADDR64_SAFE;
			break;
		case 't':	/* tty */
			close(0);
			if(device(optarg, 1) == -1) {
				fprintf(stderr, "%s: %s (%s)\n", __progname, strerror(errno), optarg);
				return EXIT_FAILURE;
			}
			dup2(0, 1);
			dup2(1, 2);
			inherit.flags &= ~SPAWN_SETGROUP;
			inherit.flags |= SPAWN_SETSID | SPAWN_TCSETPGROUP;
			inherit.flags &= ~SPAWN_EXEC;
			nfds = 3;
			break;

		case 'l':
		case 'u': {
			struct passwd			*pw;
			uid_t					uid;
			char					*name;

			name = 0;
			if(pw = getpwnam(optarg)) {
				if(i == 'l' || getuid() != geteuid() || getgid() != getegid()) {
					if(!check_pass(pw)) {
						return EXIT_FAILURE;
					}
				}
				name = optarg;
				if(initgroups(optarg, pw->pw_gid) == -1 || setgid(pw->pw_gid) == -1 ||
						setegid(pw->pw_gid) == -1 || setuid(pw->pw_uid) == -1) {
					fprintf(stderr, "%s: Unable to init user %s (%s)\n", __progname, optarg, strerror(errno));
					return EXIT_FAILURE;
				}
				seteuid(uid = pw->pw_uid);
			} else {
				if(*optarg < '0' || *optarg > '9') {
					fprintf(stderr, "%s: Unable to find user %s (%s)\n", __progname, optarg, strerror(errno));
					return EXIT_FAILURE;
				}
				if((getuid() != geteuid() || getgid() != getegid()) && i == 'u') {
					fprintf(stderr, "%s: Not allowed to switch to user %s (%s)\n", __progname, optarg, strerror(errno));
					return EXIT_FAILURE;
				}
				uid = strtol(optarg, &optarg, 0);
				if(*optarg++ == ':') {
					gid_t					groups[NGROUPS_MAX];
					int						size = 0;
					gid_t					gid;
	
					if(setgid((gid = strtol(optarg, &optarg, 0))) == -1 || setegid(gid) == -1) {
						fprintf(stderr, "%s: Unable to set gid (%s)\n", __progname, strerror(errno));
						return EXIT_FAILURE;
					}
					while(size < sizeof groups / sizeof *groups && *optarg++ == ',') {
						groups[size++] = strtol(optarg, &optarg, 0);
					}
					if(size) {
						if(setgroups(size, groups) == -1) {
							fprintf(stderr, "%s: Unable to set groups (%s)\n", __progname, strerror(errno));
							return EXIT_FAILURE;
						}
					}
				}
				if(setuid(uid) == -1) {
					fprintf(stderr, "%s: Unable to set uid to %d (%s)\n", __progname, uid, strerror(errno));
					return EXIT_FAILURE;
				}
				seteuid(uid);
			}
			if(i == 'l') {
				if(!pw) {
					if(!(pw = getpwuid(uid))) {
						fprintf(stderr, "%s: Unable to find uid %d (%s)\n", __progname, uid, strerror(errno));
						return EXIT_FAILURE;
					}
					if(!check_pass(pw)) {
						return EXIT_FAILURE;
					}
				}
				setenv("LOGNAME", name ? name : pw->pw_name, 1);
				setenv("HOME", pw->pw_dir, 1);

				if(!pw->pw_dir || !pw->pw_dir[0] || (strcmp(pw->pw_passwd, "*") ? chdir(pw->pw_dir) : chroot(pw->pw_dir)) == -1) {
					chdir("/");
				}

				if(pw->pw_shell && pw->pw_shell[0]) {
					cmd = strdup(pw->pw_shell);
				} else {
					cmd = "/bin/sh";
				}
				setenv("SHELL", cmd, 1);
				unsetenv("ENV");
				umask(022);
			}
			break;
		}

		case 'C':
		case 'R':
			if (inherit_mask == NULL) {
				int size;
				int rsize;

				rsize = RMSK_SIZE(_syspage_ptr->num_cpu);
				size = sizeof(int) ;              /* rsize */
				size += rsize * sizeof(unsigned); /* runmask */
				size += rsize * sizeof(unsigned); /* inherit_mask */
				if ((rsizep = alloca(size)) == NULL) {
				    fprintf(stderr, "No memory\n");
					return EXIT_FAILURE;
				}
				memset(rsizep, 0x00, size);
				*rsizep = rsize;
				/* Point inherit_mask at runmask */
				inherit_mask = (unsigned *)(rsizep + 1);
				/* Skip runmask, point at inherit_mask */
				inherit_mask += rsize;
			}
			val = strtoul(optarg, NULL, 0);
			if (i == 'C') {
				/*
				 * The runmask member of
				 * spawn_inheritance_type is a uint32_t.
				 */
				if (val >= 32)
					bigmask = 1;
				if (val < _syspage_ptr->num_cpu)
					RMSK_SET(val, inherit_mask);
			}
			else { /* i == 'R' */
				if (idone || idone_32)
					bigmask = 1;
				if (idone < *rsizep) {
					/*
					 * Always take 32 bits worth of mask
					 * per -R.  This assumes __INT_BITS__
					 * is always a multiple of 32. ie.
					 * sizeof(unsigned) %
					 * sizeof(uint32_t) == 0.
					 */
					val &= 0xffffffff;
					val <<= 32 * idone_32;
					inherit_mask[idone] |= val;
					if (++idone_32 >= sizeof(unsigned) /
					    sizeof(uint32_t)) {
						idone_32 = 0;
						idone++;
					}
				}
			}
			break;

		case 'X':
			extsched = optarg;
			break;

		case 'w':
			if ( count < INT_MAX ) {
				count *= 100;
			}
			if ( _waitfor( optarg, count, 100, waitfor_device, NULL ) == -1 ) {
				return EXIT_FAILURE;
			}
			status = 0;
			break;

		case 'W':
			count = strtol(optarg, 0, 0) * 10;
			break;
		}
	}

	if(optind == argc) {
		if(cmd) {
			argv[0] = malloc(strlen(basename(cmd)) + 2);
            strcat(strcpy(argv[0], "-"), basename(cmd));
			argv[1] = 0;
		} else {
			if(inherit.flags & SPAWN_EXPLICIT_SCHED) {
				if(sched_setscheduler(getppid(), inherit.policy, &inherit.param) == -1) {
					fprintf(stderr, "%s: Unable to change priority (%s)\n", __progname, strerror(errno));
					return EXIT_FAILURE;
				}
				return EXIT_SUCCESS;
			}
			if(status == -1) {
				fprintf(stderr, "%s: Need arguments\n", __progname);
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}
	} else {
		argv += optind;
		cmd = argv[0];
	}

	if((inherit.flags & SPAWN_SETND) && ND_NODE_CMP(inherit.nd, ND_LOCAL_NODE)) {
		// install signal handler here to block and record the signal
		struct sigaction 		act;

		net_spawn.info.si_signo = 0;

		/* install signal handlers to propergate signals to the remote spawned process */
		act.sa_flags = SA_SIGINFO;
		sigemptyset(&act.sa_mask);
		act.sa_handler = net_signal_record;

		sigaction(SIGHUP, &act, NULL);
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGQUIT, &act, NULL);
		sigaction(SIGTTIN, &act, NULL);
		sigaction(SIGTTOU, &act, NULL);
		sigaction(SIGTERM, &act, NULL);

	}
	if (extsched != NULL) {
	char	*errmsg;

		if ((errmsg = setup_extsched(extsched, &inherit)) != NULL) {
			fprintf(stderr, "%s: %s (for '-X')\n", __progname, errmsg);
			return(EXIT_FAILURE);
		}
	}
	if (inherit_mask != NULL) {
		if (bigmask == 0) {
			inherit.flags |= SPAWN_EXPLICIT_CPU;
			inherit.runmask = inherit_mask[0];
		}
		else if ((inherit.flags & SPAWN_SETND) &&
		    ND_NODE_CMP(inherit.nd, ND_LOCAL_NODE) != 0) {
			fprintf(stderr, "Remote spawn on cpu >= 32 "
			    "not supported\n");
			return EXIT_FAILURE;
		}
		else if (ThreadCtl(_NTO_TCTL_RUNMASK_GET_AND_SET_INHERIT,
		    rsizep) == -1) {
			perror("Unable to set inherit mask");
			return EXIT_FAILURE;
		}
	}
	if((pid = spawnp(cmd, nfds, nfds ? fds : 0, &inherit, argv, 0)) == -1) {
		fprintf(stderr, "%s: %s (%s)\n", __progname, strerror(status = errno), argv[0]);
	} else if((inherit.flags & SPAWN_SETSID) == 0) {
		int ret;
		
//		signal(SIGINT, SIG_IGN);
//		signal(SIGQUIT, SIG_IGN);
		close(0);
		close(1);
		close(2);
		fcloseall();
		if((inherit.flags & SPAWN_SETND) && ND_NODE_CMP(inherit.nd, ND_LOCAL_NODE)) {
			siginfo_t				info;
			struct sigaction 		act;


			/* install signal handlers to propergate signals to the remote spawned process */
			net_spawn.pid = pid;
			
			act.sa_flags = SA_SIGINFO;
			sigemptyset(&act.sa_mask);
			act.sa_handler = net_signal_deliver;

			sigaction(SIGHUP, &act, NULL);
			sigaction(SIGINT, &act, NULL);
			sigaction(SIGQUIT, &act, NULL);
			sigaction(SIGTTIN, &act, NULL);
			sigaction(SIGTTOU, &act, NULL);
			sigaction(SIGTERM, &act, NULL);
		
			if(net_spawn.info.si_signo != 0) {
				net_signal_deliver(net_spawn.info.si_signo, &net_spawn.info, (void*)NULL);
			}	
			do {
				ret = __waitid_net(inherit.nd, P_PID, pid, &info, WEXITED);
			} while(ret == -1 && errno == EINTR);
			if(ret != -1) {
				switch(info.si_code) {
					case CLD_EXITED:
						status = (info.si_status & 0xff) << 8;
						break;
					case CLD_KILLED:
						status = info.si_status & WSIGMASK;
						break;
					case CLD_DUMPED:
						status = (info.si_status & WSIGMASK) | WCOREFLG;
						break;
					default:
						errno = EINVAL;
						ret = -1;
				}
			}
		} else {
			ret = waitpid(pid, &status, 0);
		}
		if(ret == -1) {
			/* stderr has been closed. No need to print out anything. */
			status = errno;
//			fprintf(stderr, "%s: %s (%s)\n", __progname, strerror(status = errno), argv[0]);
		} else if(WIFSIGNALED(status)) {
			status = WTERMSIG(status);
			signal(status, SIG_DFL);
			raise(status);
		} else if(WIFEXITED(status)) {
			status = WEXITSTATUS(status);
		}
	}
	return status;
}

__SRCVERSION("on.c $Rev: 199951 $");
