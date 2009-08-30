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
#include <errno.h>
#include <sys/modem.h>
#include <unistd.h>

#ifndef __QNXNTO__
#include <sys/dev.h>
#endif


//
// Open a device in a mode suitable for raw communications such as PPP.
// If baud == 0 then leave it as is.
//
int modem_open(char *device, speed_t baud) {
	int						fd;
	struct termios			termio;

	// Open the device
	if((fd = open(device, O_RDWR)) == -1)
		return(fd);

#ifndef __QNXNTO__
	{
	struct _dev_info_entry	dinfo;
	// Check if anyone else has the device open.
	dev_info(fd, &dinfo);
	if(dinfo.open_count > 1) {
		close(fd);
		errno = EBUSY;
		return(-1);
		}
	}
#endif

	// Go raw, set hw flow.
	(void)tcgetattr(fd, &termio);
	termio.c_cflag = CS8|IHFLOW|OHFLOW|CREAD|HUPCL;
	termio.c_iflag = BRKINT;
	termio.c_lflag = IEXTEN;
	termio.c_oflag = 0;

	// Set baud rate if requested
	if(baud) {
		(void)cfsetispeed(&termio, baud);
		(void)cfsetospeed(&termio, baud);
		}

	(void)tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &termio);

	return(fd);
	}

__SRCVERSION("modem_open.c $Rev: 153052 $");
