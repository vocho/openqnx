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






#include <sys/dev.h>
#include <sys/qioctl.h>
#include <termios.h>
#include "sendnto.h"

int
opendevice(const char *devicename, int baud) {
    int		fd;
	struct termios	 termio;

	// Open the device to send the image over.
	fd = open(devicename, O_RDWR);
	if(fd != -1) {
		// Go raw, set hw flow.
		tcgetattr(fd, &termio);
		termio.c_iflag = BRKINT;
		termio.c_lflag = IEXTEN;
		termio.c_oflag = 0;
		if(baud != -1)
			termio.c_ispeed = termio.c_ospeed = baud;
		tcsetattr(fd, TCSANOW, &termio);
		tcflush(fd, TCIOFLUSH);
    }
    return( fd );
}

void
flushdevice( int devicefd ) {
	tcflush(devicefd, TCIFLUSH);
}

int
writedevice( int devicefd, const void *buff, int len ) {
	return( write( devicefd, buff, len ) );
}

int
getdevice(int fd, int timeout) {
	char c;
	int n;

	if(timeout == -1) {
		n = dev_read(fd, &c, 1, 0, 0, 0, 0, NULL);		// Don't block
	} else {
		c = 0xff;
		n = dev_read(fd, &c, 1, 1, 0, timeout, 0, NULL);// Read with timeout
	}

	return(n == 1 ? c & 0xff : -1);
}

int
checklapdevice( int devicefd ) {
	long bits[2];

	bits[0] = bits[1] = 0;	// Don't change anything
	qnx_ioctl(devicefd, QCTL_DEV_CTL, bits, sizeof(bits), bits, sizeof(bits));
	if((bits[0] & (1 << 3)))	//	 Check ERROR line
		return(2);

	if(bits[0] & (1 << 5))		// Check PE line
		return(1);

    return( 0 );
}
