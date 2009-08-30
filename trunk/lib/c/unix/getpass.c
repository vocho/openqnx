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




#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

char *getpass(const char *prompt) {
    static char		pw[32 + 1];
    int				tty, i;
    char			c;
    struct termios	tios;

    if((tty = open(ctermid(0), O_RDWR)) == -1) {
		tty = fileno(stdout);
	}
    (void)tcgetattr(tty, &tios);
    tios.c_lflag &= ~ECHO;
    tcsetattr(tty, TCSANOW, &tios);
    write(tty, prompt, strlen(prompt));
    for (i = 0; read(tty, &c, sizeof c) > 0 && c != '\n';) {
		if (i < (sizeof pw - 1)) {
		    pw[i++] = c;
		}
	}
    pw[i] = 0;
    tios.c_lflag |= ECHO;
    tcsetattr(tty, TCSANOW, &tios);
    c = '\n';
	write(tty, &c, sizeof c);
    if(tty != fileno(stdout)) {
		close(tty);
	}
    return pw;
}

__SRCVERSION("getpass.c $Rev: 153052 $");
