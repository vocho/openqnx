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
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int exec_glob(char gflag, char under_until)
	{
	char *lp_save;
	register int laddr;
	int status, failures;
	register struct line *p;

	if(nladdrs == 0) {
		laddr1 = 1;
		laddr2 = lastln;
		}

	if(laddr1 <= 0)
		return( ERROR5 );

	if(getpat() != OK)
		return( ERROR3 );

	++lp;
	curin = curcol = 0;
	for(p = getptr(laddr = laddr1) ; laddr <= laddr2 ; ++laddr) {
		if((match(p->textp, pattern, FORWARD, -1) ? 1 : 0) == gflag)
			p->lflags |= GLOB_FLAG;
		else
			p->lflags &= ~GLOB_FLAG;
		p = p->next;
		}

	for(laddr = nextln(laddr2) ; laddr != laddr1 ; laddr = nextln(laddr))
		getptr(laddr)->lflags &= ~GLOB_FLAG;

	lp_save = lp;
	laddr = laddr1;
	failures = 0;
	status = OK;
	do {
		if((p = getptr(laddr))->lflags & GLOB_FLAG) {
			p->lflags &= ~GLOB_FLAG;
			curln = laddr;
			lp = lp_save;
			if((status = exec_line(GLOB, under_until)) == OK)
				failures = 0;
			}
		else {
			++failures;
			laddr = nextln(laddr);
			}
		} while(failures <= lastln  &&  status == OK);

	lp = "\n";	/* Force exec_line to fetch a new line */
	return(status);
	}
