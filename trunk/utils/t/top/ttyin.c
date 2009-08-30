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





#include <termios.h>
#include "ttyin.h"
#include <unix.h>

static int tty = 0;

int input_to_read()
{
    return rdchk(tty);
}

int prior_term_set = 0;
struct termios prior_term;

int enter_cbreak_mode()
{
    if (prior_term_set == 0) {
	if (tcgetattr(tty, &prior_term) != 0) {
	    perror("Unable to enter cbreak mode.");
	    return -1;
	} else {
	    struct termios term;

	    term = prior_term;
	    term.c_lflag &= ~(ICANON | ECHO);
	    term.c_iflag &= ~ICRNL;
	    term.c_oflag |= ONLCR;
	    term.c_lflag |= ISIG;
	    term.c_cc[VMIN] = 1;
	    term.c_cc[VTIME] = 0;
	    prior_term_set = 1;
	    return tcsetattr(tty, TCSANOW, &term);
	}
    }
    return 0;
}

int leave_cbreak_mode()
{
    if (prior_term_set) {
	prior_term_set = 0;
	return tcsetattr(tty, TCSANOW, &prior_term);
    }
    return 0;
}


