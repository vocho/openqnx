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




#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/dcmd_chr.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <sys/procmgr.h>

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

#define LETTERS "pqrstuvwxyzPQRST"

int
openpty(int *amaster, int *aslave, char *name, struct termios *termp, struct winsize *winp) {
	static const char FIXCONST template[] = "/dev/XtyXX";
	char line[sizeof template];
	const char *cp1, *cp2;
	int master, slave;

	strcpy(line, template);
	for (cp1 = LETTERS; *cp1; cp1++) {
		line[8] = *cp1;
		for (cp2 = "0123456789abcdef"; *cp2; cp2++) {
			line[5] = 'p';
			line[9] = *cp2;
			if ((master = open(line, O_RDWR | O_NOCTTY)) == -1) {
				if (errno == ENOENT)
					return (-1);	/* out of ptys */
			} else {
				line[5] = 't';
				(void)chown(line, getuid(), -1);
				(void)chmod(line, S_IRUSR|S_IWUSR|S_IWGRP);
				if ((slave = open(line, O_RDWR | O_NOCTTY)) != -1) {
					*amaster = master;
					*aslave = slave;
					if (name)
						strcpy(name, line);
					if (termp)
						tcsetattr(slave, TCSAFLUSH, termp);
					if (winp)
						devctl(slave, DCMD_CHR_SETSIZE, winp, sizeof *winp, 0);
					return 0;
				}
				(void) close(master);
			}
		}
	}
	errno = ENOENT;	/* out of ptys */
	return -1;
}

int
login_tty(int fd) {
	if(setsid() == -1) {
		return -1;
	}
	/*
	 * Since we are now the session leader, the process group of the
	 * session is the same as out pid.
	 */
	if(tcsetsid(fd, getpid()) == -1) {
		return -1;
	}

	dup2(fd, STDIN_FILENO);
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if(fd > max(STDIN_FILENO, max(STDOUT_FILENO, STDERR_FILENO))) {
		close(fd);
	}
	return 0;
}

pid_t
forkpty(int *amaster, char *name, struct termios *termp, struct winsize *winp) {
	int master, slave;
	pid_t pid;

	if (openpty(&master, &slave, name, termp, winp) == -1)
		return (-1);
	switch (pid = fork()) {
	case -1:
		return (-1);
	case 0:
		/* 
		 * child
		 */
		(void) close(master);
		(void)login_tty(slave);
		return (0);
	default:
		break;
	}
	/*
	 * parent
	 */
	*amaster = master;
	(void) close(slave);
	return (pid);
}

__SRCVERSION("pty.c $Rev: 153052 $");
