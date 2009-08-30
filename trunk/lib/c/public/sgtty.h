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



/*
 *  sgtty.h
 *

 */
#ifndef _SGTTY_H_INCLUDED
#define _SGTTY_H_INCLUDED
#include <sys/termio.h>

#include <_pack64.h>

/*
 * TIOCGETP/TIOCSETP structure
 */
struct sgttyb {
    char sg_ispeed; /* input speed */
    char sg_ospeed; /* output speed */
    char sg_erase;  /* erase character */
    char sg_kill;   /* kill character */
    int  sg_flags;  /* mode flags */
};

/*
 * TIOCGETC/TIOCSETC structure
 */
struct tchars {
    char t_intrc;   /* interrupt */
    char t_quitc;   /* quit */
    char t_startc;  /* start output */
    char t_stopc;   /* stop output */
    char t_eofc;    /* end-of-file */
    char t_brkc;    /* input delimiter (like nl) */
};

/*
 * TIOCGLTC/TIOCSLTC structure
 */
struct ltchars {
    char    t_suspc;    /* stop process signal */
    char    t_dsuspc;   /* delayed stop process signal */
    char    t_rprntc;   /* reprint line */
    char    t_flushc;   /* flush output (toggles) */
    char    t_werasc;   /* word erase */
    char    t_lnextc;   /* literal next character */
};

#include <_packpop.h>

#define gtty(fd, sg)    ioctl(fd, TIOCGETP, sg)
#define stty(fd, sg)    ioctl(fd, TIOCSETP, sg)
#endif

/* __SRCVERSION("sgtty.h $Rev: 153052 $"); */
