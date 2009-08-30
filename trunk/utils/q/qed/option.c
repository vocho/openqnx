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

int option() {
	char *p;

	switch(*lp++) {

	case 'a':
		p = &opt.opt_a;
		break;

	case 'b':
		p = &opt.opt_b;
		mark_line(0);
		break;

	case 'c':
		if(*lp == '+')
			change_state(CMD);
		else if(*lp == '-')
			change_state(TEXT);
		else if(*lp == '~')
			change_state(opt.state==CMD ? TEXT : CMD);
		else {
			p = &opt.state;
			break;
			}
		++lp;
		return(OK);

	case 'd':
		p = &opt.opt_d;
		break;

	case 'e':
		p = &opt_e;
		break;

	case 'f':
		p = &opt.opt_f;
		break;

	case 'i':
		p = &opt.opt_i;
		break;

	case 'j':
		p = &opt.opt_j;
		break;

	case 'l':
		if(nmarks)
			mark_line(marker1);
		p = &opt.opt_l;
		break;

	case 'm':
		p = &opt.opt_m;
		break;

	case 'n':
		p = &opt.opt_n;
		break;

	case 'r':
		firstp->lflags |= DIRTY_FLAG;
		if(*lp == '+'  ||  *lp == '-'  ||  *lp == '~') {
			if(++file_type == 3)
				file_type = 0;
			}
		else {
			p = &opt.state;
			break;
			}
		++lp;
		return(OK);
		break;
		
	case 's':
		p = &opt.opt_s;
		break;

	case 't':
		p = &opt.opt_t;
		mark_line(0);
		break;

	case 'w':
		p = &opt.opt_w;
		break;

	default:
		return(ERROR9);
		}

	switch(*lp++) {

	case '+':
		*p = 1;
		break;

	case '-':
		*p = 0;
		break;

	case '~':
		*p = *p ? 0 : 1;
		break;

	case '?':
		cc_reg = *p;
     	return(OK);

	default:
		return(ERROR9);
		}

	if(p == &opt_e)
		if(opt_e)
			memcpy(&opt_stack, &opt, sizeof(opt));
		else
			memcpy(&opt, &opt_stack, sizeof(opt));

	return(OK);
	}
