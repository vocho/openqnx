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
#include <stdlib.h>
#include <fnmatch.h>
#include <sys/modem.h>

//#define DEBUG

//
// Execute a script.
// The passed table is terminated by a NULL pointer to a pattern.
// Baud will be set to a number if it can be extracted. Otherwise 0.
//
int modem_script(int fd, struct modem_script *table, speed_t *baud, void (*io)(char *progress, char *in, char *out), int (*cancel)(void)) {
	int					quiet;
	int					timeout;
	int					state;
	int					flags;
	char				buf[200];
	char				*cp;
	struct modem_script	*csp;

	csp = table;
	quiet = timeout = flags = state = 0;
	*baud = 0;

	// On entry we always write the first response.
	if(io)
		(*io)(csp->progress, 0, csp->response);
	(void)modem_write(fd, csp->response);

	for(;;) {
		// Check for a state change.
		if(state != csp->newstate) {
			
#ifdef DEBUG
			printf("goto %d\n", csp->newstate);
#endif
			if(csp->newstate == 0)					// Return on newstate of 0
				return(csp->retvalue);

			timeout = csp->newtimeout * 10;			// Convert to 1/10 sec
			quiet = csp->newquiet;
			state = csp->newstate;
			flags = csp->newflags;
			}

		// Collect input until a timeout or a match.
		do {
			// Wait for input
			if(modem_read(fd, buf, sizeof(buf), quiet, timeout, flags, cancel) == -1)
				return(-1);							// Timeout

			if(io)
				(*io)(0, buf, 0);

			// Try and match input
			for(csp = &table[1]; csp->pattern ; ++csp) {
				if((csp->curstate == state || csp->curstate == 0)  &&  fnmatch(csp->pattern, buf, FNM_QUOTE) == 0) {
#ifdef DEBUG
					printf("match \"%s\"\n", csp->pattern);
#endif
					if(io) {
						(*io)(csp->progress, 0, 0);
						if((csp->curflags & MODEM_NOECHO) == 0)
							(*io)(0, 0, csp->response);
						}
					(void)modem_write(fd, csp->response);
					break;
					}
				}
			} while(csp->pattern == 0);

		// Found a match. Should we try and extract the baud?
		if((csp->curflags & MODEM_BAUD)  &&  baud) {
			for(cp = buf; *cp  &&  (*cp < '0'  ||  *cp > '9') ; ++cp)
				;

			*baud = atoi(cp);
			}
		}
	}

__SRCVERSION("modem_script.c $Rev: 153052 $");
