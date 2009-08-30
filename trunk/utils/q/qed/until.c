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
#include <stdlib.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int until(int laddr1, int laddr2, int under_glob)
	{
	register int cnt, status;
	register char *lp_save;
	int condition;

	if(laddr1 <= 0)
		return(ERROR5);

	if(*lp >= '0'  &&  *lp <= '9')
		cnt = getint();
	else
		cnt = -1;

	if((condition = get_cond()) < 0)
		return(ERROR5);

	if(condition != NOT_TRUE_OR_FALSE)
		cc_reg = !condition;

	curln = laddr1;
	lp_save = lp;
	for(curln = laddr1 ; curln <= laddr2 ; ++curln) {
		status = OK;
		while((cnt >= 0 ? --cnt : 1) >= 0  &&  status == OK  &&  cc_reg != condition) {
			lp = lp_save;
			status = exec_line(under_glob, 1);
			}
		}
	--curln;

	lp = "\n"; /* Force main to fetch a new line */
	return(OK);
	}



int branch()
	{
	register int n, c;
	int condition;

	n = getint();
	if((condition = get_cond()) < 0 )
		return(ERROR5);

	if(condition == cc_reg  ||  condition == NOT_TRUE_OR_FALSE)
		if(n == 0)
			lp = lbuff;
		else {
			while(n > 1)
				if((c = x_fp ? getc(x_fp) : mgetchar()) == '\n'  ||  c == '\r'  ||  c == EOF)
					--n;
			lp ="\n";	/* Force main to fetch a new line */
			}

	return(OK);
	}



int get_cond() {
	switch(*lp++) {

	case 'f':
		return(FALSE);

	case 't':
		return(TRUE);

	case '\n':
		--lp;
	case ' ':
		return(NOT_TRUE_OR_FALSE);

	default:
		return(ERROR5);
		}
	}



