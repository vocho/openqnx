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





#include <signal.h>
#include <sys/time.h>
#include <stropts.h>
#include <termio.h>

#include "sendnto.h"

int
opendevice(const char *devicename, int baud) {
	int            fd, baud_index;
	struct termios termio;

	// Open the device to send the image over.
	fd = open(devicename, O_RDWR);
	if(fd != -1) {
		// Go raw, set hw flow.
		if (tcgetattr(fd, &termio)) {
			fprintf(stderr,"tcgetattr() failed, errno = %d\n",errno);
		}

		termio.c_iflag = BRKINT;
		termio.c_lflag = IEXTEN;
		termio.c_oflag = 0;

		if(baud != -1) {
			switch(baud) {
			#ifdef B0	
				case 0	    : baud_index = B0;		break; 
			#endif
			#ifdef B50	
				case 50     : baud_index = B50;		break; 
			#endif
			#ifdef B75	
				case 75     : baud_index = B75;		break; 
			#endif
			#ifdef B110	
				case 110    : baud_index = B110;	break; 
			#endif
			#ifdef B134	
				case 134    : baud_index = B134;	break; 
			#endif
			#ifdef B150	
				case 150    : baud_index = B150;	break; 
			#endif
			#ifdef B200	
				case 200    : baud_index = B200;	break; 
			#endif
			#ifdef B300	
				case 300    : baud_index = B300;	break; 
			#endif
			#ifdef B600	
				case 600    : baud_index = B600;	break; 
			#endif
			#ifdef B1200	
				case 1200   : baud_index = B1200;	break; 
			#endif
			#ifdef B1800	
				case 1800   : baud_index = B1800;	break; 
			#endif
			#ifdef B2400	
				case 2400   : baud_index = B2400;	break; 
			#endif
			#ifdef B4800	
				case 4800   : baud_index = B4800;	break; 
			#endif
			#ifdef B9600	
				case 9600   : baud_index = B9600;	break; 
			#endif
			#ifdef B19200	
				case 19200  : baud_index = B19200;	break; 
			#endif
			#ifdef B38400	
				case 38400  : baud_index = B38400;	break; 
			#endif
			#ifdef B57600	
				case 57600  : baud_index = B57600;	break; 
			#endif
			#ifdef B76800	
				case 76800  : baud_index = B76800;	break; 
			#endif
			#ifdef B115200	
				case 115200 : baud_index = B115200;	break; 
			#endif
			#ifdef B153600	
				case 153600 : baud_index = B153600;	break; 
			#endif
			#ifdef B230400	
				case 230400 : baud_index = B230400;	break; 
			#endif
			#ifdef B307200	
				case 307200 : baud_index = B307200;	break; 
			#endif
			#ifdef B460800	
				case 460800 : baud_index = B460800;	break;
			#endif
				default:
					fprintf(stderr,"Invalid or unsupported baudrate specified.\n");
					fprintf(stderr,"Please use :\n\t0 \n\t50 \n\t75 \n\t110 \n\t134 \n\t150 \n\t200 \n\t300 \n\t600 \n\t1200 \n\t1800 \n\t2400 \n\t4800 \n\t9600 \n\t19200 \n\t38400 \n\t57600 \n\t76800 \n\t115200 \n\t153600 \n\t230400 \n\t307200 \n\t460800\n");
					exit(0);
			}

			if (cfsetospeed(&termio,baud_index)) {
				fprintf(stderr,"cfsetospeed() failed, errno = %d\n",errno);
			}
			if (cfsetispeed(&termio,baud_index)) {
				fprintf(stderr,"cfsetispeed() failed, errno = %d\n",errno);
			}
		}
		if (tcsetattr(fd, TCSANOW, &termio)) {
			fprintf(stderr,"tcsetattr() failed, errno = %d\n",errno);
		}

		if (tcflush(fd, TCIOFLUSH)) {
			fprintf(stderr,"tcflush() failed, errno = %d\n",errno);
		}
    }
    return( fd );
}

void
flushdevice( int devicefd ) {
	tcflush(devicefd, TCIFLUSH);
}

ssize_t
writedevice( int devicefd, const void *buff, int len ) {
	return( write( devicefd, buff, len ) );
}

static void
MyHandler(void) {
	return;
}

int
getdevice(int fd, int timeout) {
	char c;
	int n, flags;
	struct itimerval  itime;
	void (*ofunc)();

	// wait previous tx to complete before doing read
	if (tcdrain(fd)) { 
		fprintf(stderr,"tcdrain() failed, errno = %d\n",errno);
	}

	flags = fcntl(fd, F_GETFL);
	if(flags == -1) {
		fprintf(stderr,"Error getting fd flags for read\n");
		return(-1);
	}
	if(timeout == -1) {
		flags |= O_NONBLOCK;
		if(fcntl(fd, F_SETFL, flags) == -1) {
			fprintf(stderr,"Error setting fd flags for non-blocking read\n");
			return(-1);
		}
		n = read(fd, &c, 1);		// Don't block
	} else {
		flags &= ~O_NONBLOCK;
		flags &= ~O_NDELAY;
		if(fcntl(fd, F_SETFL, flags) == -1) {
		    fprintf(stderr,"Error setting fd flags for blocking read\n");
		    return(-1);
		}
		ofunc = signal(SIGALRM, (void*)MyHandler);

		// timeout is in multiples of .1 seconds, so we have to
		// munge the value to fit the structure
		itime.it_value.tv_sec = timeout / 1000;
		itime.it_value.tv_usec = (timeout % 1000) * 1000000;
		itime.it_interval.tv_sec = 0;
		itime.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &itime, NULL);

		c = 0xff;
		n = read(fd, &c, 1);       // Read with timeout

		itime.it_value.tv_sec = 0;
		itime.it_value.tv_usec = 0;
		itime.it_interval.tv_sec = 0;
		itime.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &itime, NULL);
		signal(SIGALRM, ofunc);
	}
	return(n == 1 ? c & 0xff : -1);
}

int
checklapdevice(int devicefd) {
    return(0);
}
