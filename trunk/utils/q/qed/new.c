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
#include <string.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

/*
zap_kopy(int laddr)
	{
	register char *sp, *dp;
	int dindex, dlength, sindex1, sindex2, slength, status;
	static dline;

	if(laddr == laddr1)
		dline = curln;

	if((curln = dline) == 0)
		strcpy(rbuff, " ");
	else
		strcpy(rbuff, dp = getptr(curln)->textp);

	dindex = curin;
	dlength = strlen(rbuff);

	sp = getptr(laddr)->textp;
	sindex1 = col_to_index(sp, limit1);
	sindex2 = col_to_index(sp, limit2);
	slength = strlen(sp);

	if(sindex2 < slength)
		zchar_change(dindex, dlength, &sp[sindex1], sindex2 - sindex1 + 1, 1);
	else {
		if(sindex1 < slength) {
			dlength = zchar_change(dindex, dlength, &sp[sindex1], slength - sindex1 + 1, curln);
			dindex += slength - sindex1;
			}
		else
			slength = sindex1;

		while(slength <= sindex2) {
			dlength = zchar_change(dindex, dlength, " ", 1, 1);
			++dindex;
			++slength;
			}
		}

	if(curln == 0) {
		curln = lastln;
		status = addline(rbuff, 0, 0);
		}
	else
		status = replace(curln, rbuff, dp);

	dline = nextln(curln);
	return(status);
	}
*/


int zap_restore(int laddr)
	{
	register struct line *p1, *p2;
	int status = OK, i;

	if(nladdrs == 2)
		return(ERROR5);

	curln = laddr;
	for(p1 = purge_ptr->next, i = npurge_lns; i ; --i, p1 = p1->next) {
		strcpy(rbuff, (p2 = getptr(curln))->textp);
		zchar_change(curin, strlen(rbuff), p1->textp, strlen(p1->textp), curln);

		if(curln == 0) {		/* Add after last line */
			curln = lastln;
			status = addline(rbuff, 0, 0);
			}
		else
			status = replace(curln, rbuff, p2->textp);

		if(status != OK)
			break;

		curln = nextln(curln);	/* Set to zero after last line */
		}

	curln = laddr;
	return(status);
	}
