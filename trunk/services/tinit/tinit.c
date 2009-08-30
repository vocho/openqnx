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
%C	[-pt] [env_var=value]...

Options:
 -p         Start Photon. If it fails or terminates start a console login.
 -t         Do not mask suspend signal (SIGTSTP) in spawned process.
 -f file    Specify a ttys file (default /etc/config/ttys)

 You can specify commands to run by placing them in /etc/config/ttys
 in the following format:
	device command terminal
 ie
	ser1 "/bin/ksh" qansi-m
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <process.h>
#include <spawn.h>
#include <termios.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <utmp.h>


// Manifests
#define MAXTTYS			32
#define MAXARGS			32

// Structures
struct tty_entry {
	char		*devname;
	char		*cmdline;
	char		*term;
	char		*extra;		// Might find a use in the future for this arg
	pid_t		 pid;
} ;

// Prototypes
void options(int argc, char *argv[]);
void read_ttys(void);
pid_t start(struct tty_entry *ttp, int session);


// Globals
FILE *DebugFp;
int StartPhoton;
int tstop;
char ttys_file[PATH_MAX + NAME_MAX];
struct tty_entry PhTty[1] = {
	{"/dev/con1",  "/bin/sh -c ph",  "qansi-m",  "", -1}
} ;
struct tty_entry Tty[MAXTTYS];


int main(int argc, char *argv[]) {
	struct tty_entry	*ttp;
	struct tty_entry	*phttp  = NULL;
	char				*phpath = NULL;
	pid_t				pid;
	int					starterr;
	_int64				timeout = 60;

	/* Default */
	strcpy (ttys_file, "/etc/config/ttys");

	timeout *= 1000000000;		// Convert sec to nsec

	options(argc, argv);		// Command line options

	if(DebugFp != NULL) {
		close(0); close(1); close(2);
	} else
		procmgr_daemon(0, 0);		// Make ourselves a daemon

	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	// Read /etc/config/ttys
	read_ttys();

	// Start programs on each tty
	for(ttp = &Tty[0], starterr = 0 ; ttp->devname ; ++ttp) {
		// Don't start the login on photon's console yet
		if((StartPhoton) && (!strcmp(ttp->devname, PhTty[0].devname))) {
			phttp = ttp;
			continue;
		}

		if(start(ttp, 1) == -1) {
			starterr = 1;
		}
	}

	//
	// If requested we start Photon. If Photon finishes starting
	// or it terminates then we start up console logins.
	//
	if(StartPhoton) {
		if((pid = start(&PhTty[0], 0)) != -1) {
			waitpid(pid, NULL, 0);

			phpath = getenv("PHOTON");
			if (phpath == NULL) phpath = "/dev/photon";

			// Wait for photon to start or die
			while ((access(phpath, F_OK) != 0) && (getsid(pid) != -1)) {
				sleep(1);
			}
		}

		// Start the program on photon's console
		if(phttp) {
			if(start(phttp, 1) == -1) {
				starterr = 1;
			}
		}
	}

	for(;;) {
		// The TimerTimeout is probably un-neccessary paranoid code to catch
		// the case where a command was started and failed for some reason.
		// Like a network load with a transient error. This will retry until
		// it starts.
		if(starterr) {
			if(DebugFp)
				fprintf(DebugFp, "TimerTimeout\n");
			TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_SEND|_NTO_TIMEOUT_REPLY,
							NULL, &timeout, NULL);
		}

		if((pid = wait(0)) == -1) {
			if(errno == ECHILD) {
				sleep(10);	// No children. Wait so we dont thrash and restart
			}
		}

		// Restart programs then ended or programs which failed to start
		for(ttp = &Tty[0], starterr = 0; ttp->devname; ++ttp) {
			if(ttp->pid == pid || ttp->pid == -1) {
				if(ttp->pid == pid && strstr(ttp->cmdline, "login")){
					char *dev = strrchr(ttp->devname, '/');
					if(dev && *(++dev))
						logout(dev);
				}
				if(start(ttp, 1) == -1)
					starterr = 1;;
			}
		}
	}
}


void read_ttys(void) {
	struct tty_entry	*ttp = &Tty[0];
	FILE				*fp;
	int					i;
	char				*cp, *argv[4], term;
	char				buf[100];

	if (fp = fopen (ttys_file, "r")) {
		while(fgets(buf, sizeof(buf), fp)) {
			// Strip newline
			if(cp = strrchr(buf, '\n'))
				*cp = '\0';
			else
				break;	// Badly formated file. Bail out.

			// Parse fields
			for(cp = buf, i = 0; *cp  &&  i < 4 ; ++i) {

				if(*cp == '"')
					term = *cp++;
				else
					term = ' ';

				argv[i] = cp;
				while(*cp  &&  *cp != term)
					++cp;
				*cp++ = '\0';

				while(*cp == ' ')
					++cp;
			}
				
			if(i == 2) {		//No terminal, make it qansi
				argv[i++] = (getenv("TERM")) ? getenv("TERM") : "qansi";
			}

			if(i == 3) {		//No extra field, make it null
				argv[i++] = strdup("");
			}

			if(i == 4) {
				char buf[100];

				if(argv[0][0] != '/') {
					sprintf(buf, "/dev/%s", argv[0]);
					ttp->devname = strdup(buf);
				} else
					ttp->devname = strdup(argv[0]);

				ttp->cmdline = strdup(argv[1]);

				sprintf(buf, "TERM=%s", argv[2]);
				ttp->term = strdup(buf);

				ttp->extra = strdup(argv[3]);
				++ttp;
			}
		}

		fclose(fp);
	}

	// Just incase the file does not exist or is a mess we default as follows
	if(ttp == &Tty[0])
		ttp->cmdline = "/bin/login";
}


pid_t start(struct tty_entry *ttp, int session) {
	char				*argv[MAXARGS];
	char				*src, *dst, *cmd;
	char				buf[100];
	struct inheritance	inherit;
	struct termios		tios;
	int					fds[3] = {0, 1, 2}, i;

	close(0);
	if(open(ttp->devname, O_RDWR) == -1)
		return(-1);
	dup2(0, 1);
	dup2(1, 2);

	// Set the perms on the tty to make it usable.
	fchown(fds[0], 0, 0);
	fchmod(fds[0], 0666);

	// Set the terminal into a nice clean edit mode
	tcgetattr(fds[0], &tios);
	tios.c_lflag |= ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL;
	tios.c_oflag |= OPOST;
 	tios.c_iflag |= ICRNL;
	tcsetattr(fds[0], TCSANOW, &tios);

	cmd = ttp->cmdline;

	if(DebugFp)
		fprintf(DebugFp, "On device %s starting %s\n", ttp->devname, cmd);

	// Build an argument list.
	for(src = cmd, dst = buf, i = 0; *src  &&  i < MAXARGS ; ++i) {

		argv[i] = dst;
		while(*src  &&  *src != ' ')
			*dst++ = *src++;
		*dst++ = '\0';

		while(*src == ' ')
			++src;
	}

	argv[i] = NULL;
	cmd = argv[0];
	if(dst = strrchr(cmd, '/')) {
		argv[0] = dst + 1;
	}

	putenv(ttp->term);
	memset(&inherit, 0, sizeof(inherit));
	// We do not want chidren to inherit tinits ignored signals (TERM and HUP).
	inherit.flags = SPAWN_SETSIGDEF;
	sigfillset(&inherit.sigdefault);

	// Add SIGTSTP to spawned process' signal mask
	if(!tstop) {
		inherit.flags |= SPAWN_SETSIGMASK;
		sigemptyset(&inherit.sigmask);
		sigprocmask(SIG_BLOCK, NULL, &inherit.sigmask);
		sigaddset(&inherit.sigmask, SIGTSTP);
	}
			
	if(session != 0)
		inherit.flags |= SPAWN_TCSETPGROUP | SPAWN_SETSID;

	ttp->pid = spawnp(cmd, 3, &fds[0], &inherit, argv, NULL);
	if(DebugFp && ttp->pid == -1)
		fprintf(DebugFp, "Spawn failed: %s\n", strerror(errno));

	close(0);
	close(1);
	close(2);

	return(ttp->pid);
}


void options(int argc, char *argv[]) {
	int		opt;
	char	*cp;

	while((opt = getopt(argc, argv, "D:ptf:")) != -1) {
		switch(opt) {
		case 'p':
			StartPhoton = 1;
			break;

		case 'D':
			DebugFp = fopen(optarg, "w");
			break;

		case 't':
			tstop = 1;
			break;	

		case 'f':
			strcpy (ttys_file, optarg);
			break;
		}
	}

	while(optind < argc) {
		if(strchr(cp = argv[optind], '='))
			putenv(cp);
		else
			fprintf(stderr, "Missing '=' for macro %s\n", cp);
		++optind;
	}
}
