/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * dev_mode.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <mig4nto.h>

static void __set(tcflag_t *ptr, int flag, unsigned set);

unsigned dev_mode(int fd, unsigned mode, unsigned mask)
{
	struct termios tios;
	unsigned omode = 0;

	if(tcgetattr(fd, &tios) == -1)
		return -1;

	// Convert current flags into a mode
	if(tios.c_lflag & ECHO)   omode |= _DEV_ECHO;
	if(tios.c_lflag & ICANON) omode |= _DEV_EDIT;
	if(tios.c_lflag & ISIG)   omode |= _DEV_ISIG;
	if(tios.c_oflag & OPOST)  omode |= _DEV_OPOST;

	// Modify selected flags
	if(mask & _DEV_ECHO)  __set(&tios.c_lflag, ECHO|ECHOE|ECHOK|ECHONL, mode & _DEV_ECHO);
	if(mask & _DEV_EDIT)  __set(&tios.c_lflag, ICANON|IEXTEN, mode & _DEV_EDIT);
	if(mask & _DEV_EDIT)  __set(&tios.c_iflag, ICRNL,  mode & _DEV_EDIT);
	if(mask & _DEV_ISIG)  __set(&tios.c_lflag, ISIG,   mode & _DEV_ISIG);
	if(mask & _DEV_OPOST) __set(&tios.c_oflag, OPOST,  mode & _DEV_OPOST);

	if(tcsetattr(fd, TCSANOW, &tios) == -1)
		return -1;

	return omode;
}

static void
__set(tcflag_t *ptr, int flag, unsigned set)
{
	if(set)
		*ptr |= flag;
	else
		*ptr &= ~flag;
}
