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



#include "sendnto.h"

#define MAX_RESEND		3


static int
writelap(int fd, const void *buf, int n) {
	int		 		i;
	int				j;
	const uint8_t	*sp = buf;
	static uint8_t	*lapbuf = NULL;
	static unsigned	bufsize;
	unsigned		newsize;
	uint8_t			*dp;

	//
	// Expand a buffer into a target protocol suitable for sending over
	// a LAPLINK parallel cable. There are only 5 data lines available
	// in the cable. We use the bottem 4 bits for a nibble of data
	// (low nibble then high nibble). The 5th bit is used as a data strobe
	// which is active low.
	// 
	// The target will set busy after the next byte. However,
	// if the target is slow (about 25 usec) then the host
	// may send another byte before seeing the busy. We solve
	// this by sending the bytes multiple times on slow machines.
	// Fast machines need only a single byte. Code executing out
	// of an 8 bit ROM may require 2 or 3. 
	//
	newsize = n * (4 * MAX_RESEND);
	if(newsize > bufsize) {
		bufsize = newsize;
		lapbuf = realloc(lapbuf, newsize);
	}
	dp = lapbuf;
	for(i = 0 ; i < n ; ++i) {
		for(j = 0 ; j < laplink ; ++j) {
			*dp++ = *sp & 0xf;					// Data no strobe
		}
		for(j = 0 ; j < laplink ; ++j) {
			*dp++ = (*sp & 0x0f) | 0x10;		// Data with strobe
		}
		for(j = 0 ; j < laplink ; ++j) {
			*dp++ = (*sp >> 4) & 0xf;			// Data no stobe
		}
		for(j = 0 ; j < laplink ; ++j) {
			*dp++ = ((*sp >> 4)  & 0xf) | 0x10;	// Data with strobe
		}

		++sp;
	}

	writedevice(fd, lapbuf, laplink*4*n);

	return(0);
}


static int
writelapfirst(int fd, const void *buf, int n) {
	// send the start sync
	writedevice(fd, "\x1f\x1f\x0a\x15", 4);
	output.write = writelap;
	return writelap(fd, buf, n);
}


static int
getlap(int fd, int timeout) {
	if(timeout != -1) {
		return getdevice(fd, timeout);
	}
	// abort check
	return checklapdevice(fd) ? -2 : 0;
}


static void
initdevice(void) {
	if(laplink) {
		if((laplink < 1) || (laplink > MAX_RESEND)) {
			fprintf(stderr,"Laplink count must be 1, 2 or 3.\n");
			exit(EXIT_FAILURE);
		}
		output.check_rate = 1;
		output.write = writelapfirst;
		output.get = getlap;
	} else {
		output.check_rate = 8;
		output.write = writedevice;
		output.get = getdevice;
	}
}


static void
closedevice(int fd) {
}


void
outputdevice(void) {
	output.init = initdevice;
	output.open = opendevice;
	output.flush = flushdevice;
	output.close = closedevice;
}
