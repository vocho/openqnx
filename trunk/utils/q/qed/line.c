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





/*
 *  April 1980 by  D. T. Dodge
 */

#include <stdio.h>
#include <string.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int getladdr() {
	char c;
	int num;

	switch(c = *lp++) {

	case '.':
		return(curln);

	case '@':
		return(imin(lastln, screen_row + center_line));

	case '&':
		return(screen_row);

	case '$':
		return(lastln);

	case '%':
		return(opt.state == CMD ? 0 : curln);

	case '~':
		return(screen_height - 2);

	case '/':
	case '?':
		--lp;
		if(getpat() <= ERROR)
			return( ERROR3 );
		++lp; /* move over pattern delim */
		if((num = patscan(c == '/' ? FORWARD : BACKWARD)) == ERROR13)
			if(*lp == '|')
				num = (c == '/') ? lastln + 1 : 0;
		return(num);

	case '^':
		num = prevln(curln);
		while(*lp == '^') {
			num = prevln(num);
			++lp;
			}
		return( num );

	default:
		--lp;
		if(c <= '9'  &&  c>= '0') {
			num = getint();
			if(num >= 0)
				return(num);
			}

		return( NOTHING );
		}
	}



int get_laddr_expr(int *lnum) {
	int num;
	char *lp_save;
	char c;

	lp_save = lp;
	*lnum = num = getladdr();
	while(num >=  0  &&  ((c = *lp)=='+' || c=='-')) {
		++lp;
		if((num = getladdr()) >= 0)
			*lnum += (c=='+' ? num : -num);
		}

	if(lp == lp_save)
		return( NOTHING );

	if(num < 0)
		return( num );

	if(*lp == '|') {	/* limit number to 1 or lastln */
		*lnum = imin(imax(1, num = *lnum), lastln);
		cc_reg = (num == *lnum);
		++lp;
		}

	if(*lnum < 0  ||  *lnum > lastln)
		return( ERROR5);

	return( OK );
	}



int getrange() {
	int status;
	int laddr;

	if(*lp == '*') {
		laddr1 = 1;
		laddr2 = lastln;
		nladdrs = 2;
		++lp;
		return(OK);
		}

	if(*lp == '#') {
		if(nmarks == 0)
			return(ERROR1);

		mark_line(laddr1 = marker1);
		laddr2 = marker2;
		nladdrs = nmarks;
		nmarks_save = nmarks;
		nmarks = 0;
		++lp;
		status = laddr1 > lastln || laddr2 > lastln ? ERROR5 : OK;
		}
	else
		for(nladdrs = 0 ; (status=get_laddr_expr(&laddr)) == OK ;) {
			laddr1 = laddr2;
			laddr2 = laddr;
			++nladdrs;
			if(*lp != ';'  &&  *lp != ',')
				break;
			if(*lp++ == ';')
				curln = laddr;
			}

	if(nladdrs == 0)
		laddr2 = curln;
	if(nladdrs <= 1)
		laddr1 = laddr2;
	else
		nladdrs = 2;
	if(status >  ERROR  &&  laddr1 <= laddr2)
		return( OK );
	return(status > ERROR ? ERROR5 : status);
	}



int nextln(int laddr) {

	if(++laddr > lastln)
		return( 0 );
	else
		return(laddr);
	}




int prevln( int laddr) 	{

	if(--laddr < 0)
		return(lastln);
	else
		return(laddr);
	}



int patscan(char dir)
	{
	int laddr;
	struct line *p;
	int index;

	index = curin;
	laddr = curln;
	p = getptr(curln);
	if(index > strlen(p->textp))
		if(dir == FORWARD)
			goto newline;
		else
			index = strlen(p->textp) + 1;

	do {
		if( laddr!=0 && (index = match(p->textp, pattern, dir, index))) {
			curin = index - 1;
			curcol = 0;
			return(laddr);
			}

		if(dir == FORWARD) {
newline:
			p = p->next;
			index = -1;
			laddr = nextln(laddr);
			}
		else {
			p = p->prev;
			index = strlen(p->textp) + 1;
			laddr = prevln(laddr);
			}
		} while(laddr != curln  &&  (laddr != 0  ||  opt.opt_w));
	return( ERROR13 );
	}
