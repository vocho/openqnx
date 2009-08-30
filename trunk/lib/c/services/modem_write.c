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




#include <stdio.h>
#include <unistd.h>
#include <sys/modem.h>
#include <time.h>
#include <sys/neutrino.h>

//#define DEBUG

#ifndef __QNXNTO__
	#define readcond(a,b,c,d,e,f) dev_read(a,b,c,d,e,f,0,0)
#endif

static void waitfor( unsigned tenths ) {
	struct timespec tim;

	tim.tv_sec = tenths/10;
	tim.tv_nsec = 100000000 * (tenths%10);
	(void)clock_nanosleep(CLOCK_MONOTONIC, 0, &tim, &tim);
}

static int hex(char c) {

	if(c >= '0'  &&  c <= '9')
		return(c - '0');

	c |= 0x20;					// Convert to lower case
	if(c >= 'a'  &&  c <= 'f')
		return(10 + (c - 'a'));

	return(0);
	}

static int hexstr(char *str) {

	return((hex(str[0]) << 4) | hex(str[1]));
	}

//
// Write a response to the device expanding special \ sequences.
//
int modem_write(int fd, char *str) {
	char	c;

	if(str == 0)
		return(0);

	(void)tcflush(fd, TCIFLUSH);		// Flush all input
#ifdef DEBUG
	printf("write \"%s\"\n", str);
#endif
	while((c = *str++)) {
		if(c == '\\')
			switch(c = *str++) {
			case '\0':	return(0);
			case 'r':	c = '\r';							break;
			case 'n':	c = '\n';							break;
			case 'x':	c = hexstr(str); str += 2;			break;
			case 'P':	waitfor(hexstr(str)); str += 2;		continue;
			case 'D':	(void)tcdropline(fd, 1000);				continue;
			case 'B':	(void)tcsendbreak(fd, 500);				continue;
			default:	break;
			}

		write(fd, &c, 1);
		(void)readcond(fd, &c, 1, 1, 0, 1);	// Try and eat an echo
		}

	write(fd, "\r", 1);
// Next line added for cjbrown
//	readcond(fd, &c, 1, 1, 0, 1);	// Try and eat an echo

	return(0);
	}

__SRCVERSION("modem_write.c $Rev: 153052 $");
