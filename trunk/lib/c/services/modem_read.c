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




#include <unistd.h>
#include <errno.h>
#include <sys/modem.h>

#ifndef __QNXNTO__
	#define readcond(a,b,c,d,e,f) dev_read(a,b,c,d,e,f,0,0)
#endif

//#define DEBUG


static void strip(char *str, int flags) {
	char	*sp, *dp, c;

	for(sp = dp = str; (c = *sp++) ; ) {

		// Strip top bit
		if((flags & MODEM_ALLOW8BIT) == 0)
			c &= ~0x80;

		// Ignore control characters
		if((flags & MODEM_ALLOWCTRL) == 0)
			if(c < ' ')
				continue;

		// Convert to lower case.
		if((flags & MODEM_ALLOWCASE) == 0)
			if(c >= 'A'  &&  c <= 'Z')
				c |= 0x20;
		*dp++ = c;
		}

	*dp = '\0';
	}

//
// Read a response from the device.
//
int modem_read(int fd, char *buf, int bufsize, int quiet, int timeout, int flags, int (*cancel)(void)) {
	int		n;
	char	*cp = buf;

	quiet += 2;
	for(;;) {
		n = readcond(fd, cp, 1, 1, 0, quiet);
		if(n <= 0) {
			if(cp > buf) {
				*cp = '\0';
				strip(buf, flags);
#ifdef DEBUG
				printf("read 0 \"%s\"\n", buf);
#endif
				return(0);
				}

			if (n < 0) {
				return -1;
				}

			if((cancel  &&  (*cancel)())  ||
			   (timeout -= quiet) <= 0) {
				errno = ETIMEDOUT;
				return(-1);
				}

			continue;
			}

		if(*cp == '\n') {
			if(flags & MODEM_LASTLINE) {
				if((n = readcond(fd, cp, 1, 1, 0, 2)) > 0) {
					buf[0] = *cp;
					cp = buf;
					continue;
					}
				else if (n < 0) {
					return -1;
					}
				}

			*cp = '\0';
			strip(buf, flags);
#ifdef DEBUG
			printf("read %d \"%s\"\n", n, buf);
#endif
			return(0);
			}

		if(cp < &buf[bufsize - 1])
			++cp;
		}
	}

__SRCVERSION("modem_read.c $Rev: 153052 $");
