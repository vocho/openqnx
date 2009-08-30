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
#include <string.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

/*
 * I would not wish this code upon anyone! It works and there
 * is credit in that....BUT it is not elagent and messy code
 * bothers me a lot.
 */

int zap_fill(int laddr1, int laddr2)
	{
	register char *p1, *p2, *p1save, *p2save;
	register struct line *l1, *bp;
	char lflags, next_lflags, start, para_flag;
	int nholes, margin_width = right_margin - left_margin + 1;
	int width, width_save, status = OK, indent;

	cc_reg = FALSE;
	para_flag = 0;
	indent = 0;
	for(l1 = getptr(curln = laddr1); laddr1 < laddr2; ++laddr1) {
		l1->lflags &= ~STOP_FLAG;
		l1 = l1->next;
		}

	l1->lflags |= STOP_FLAG;

loop:
	if(lastln == 0)
		return(NULL);

	p2 = rbuff;
	p2save = NULL;
	nholes = width = width_save = 0;
	start = 1;

line_joined:
	p1save = p1 = (bp = getptr(curln))->textp;
	lflags = bp->lflags;
	next_lflags = bp->next->lflags;
	if(*p1 == '.'
			||  bp->next->textp[0] == '.'
			||  bp->next->textp[col_to_index(bp->next->textp, left_margin)]
																	== '\t')
		next_lflags |= PARA_FLAG;

	if(start) {
		width = indent = ((lflags & PARA_FLAG)  &&  margin_width > 5) ? 5 : 0;
		para_flag = lflags & PARA_FLAG;
		start = 0;
		}

	/*
	 *	Skip blanks before the left margin.
	 */
	while(*p1 == ' '  &&  (p1 - p1save) < left_margin - 1)
		++p1;

	if(*p1 == '\0') {		/* Leave the line alone */
		nholes = 0;
		width = margin_width + 1;
		if(p2 == rbuff) {
			if(lflags & STOP_FLAG)
				return(OK);
			curln = nextln(curln);
			goto loop;
			}
		goto leave_alone;
		}

	while(*p2++ = *p1) {
		if(*p1++ == ' ') {
			if(width <= margin_width) {
				p2save = p2 - 1;
				p1save = p1;
				width_save = width;
				}
			else
				break;

			if(*(p1 + -2) != '.') {
				++nholes;
				while(*p1 == ' ')
					++p1;
				}
			}
		else if(*(p1 + -1) == '\t')
			width = width + (3 - (width+left_margin-1)%4);
		else if(*(p1 + -1) == 0x1b  &&  width)
			width -= 2;

		++width;
		}
	*p2 = '\0';

	if(width <= margin_width  &&  (lflags & STOP_FLAG) == 0
					&&  (next_lflags & PARA_FLAG) == 0  &&  curln != lastln) {
		*(p2save = p2 + -1) = ' ';
		width_save = width++;
		++nholes;
		laddr1 = curln;
		delete(curln, curln, NOSAVE);
		/*
		 * Code added to help display handler figure out a fast way using
		 * line ins/del to update the screen.
		 */
		if(laddr1 <= lastln
				&&  (num_delete_lines == 0  ||  laddr1 < delete_line)) {
			delete_line = laddr1;
			num_delete_lines = 1;
			}

		curln = nextln(curln);
		cc_reg = TRUE;
		goto line_joined;
		}

leave_alone:
	if(left_margin + indent > 1) {
		memmove(rbuff + left_margin + indent - 1, rbuff, p2 - rbuff + 1);

		for(p1 = rbuff; p1 <= &rbuff[left_margin + indent - 2] ; ++p1)
			*p1 = ' ';

		if(p2save)
			p2save += left_margin + indent - 1;

		}

	if(opt.opt_j && ((p2save && width > margin_width) || ((lflags&STOP_FLAG) == 0
										&&  (next_lflags&PARA_FLAG) == 0))) {
		*p2save = '\0';
		justify(p2save, margin_width - width_save, --nholes);
		if(nholes >= 1)
			p2save += margin_width - width_save;
		}

	if(p2save  &&  width > margin_width) {
		strcpy(bp->textp, p1save);
    	bp->lflags &= ~(PARA_FLAG);
		mark_line(curln);
		bp->lflags |= DIRTY_FLAG;
		curln = prevln(curln);
		*p2save = '\0';
		status = addline(rbuff, 1, (lflags | para_flag | NEW_FLAG)
										& ~(CONT_FLAG|OVER_FLAG|STOP_FLAG));
		if(status != OK)
			return(status);
		curln = nextln(curln);
		cc_reg = TRUE;
		goto loop;
		}
	else {
		laddr1 = curln;
		delete(curln, curln, NOSAVE);
		/*
		 * Code added to help display handler figure out a fast way using
		 * line ins/del to update the screen.
		 */
		if(laddr1 <= lastln
				&&  (num_delete_lines == 0  ||  laddr1 < delete_line)) {
			delete_line = laddr1;
			num_delete_lines = 1;
			}

		if((lflags & STOP_FLAG) == 0) {
			addline(rbuff, 1, lflags | para_flag | NEW_FLAG);
			if(curln != lastln)
				curln = nextln(curln);
			goto loop;
			}
		return(addline(rbuff, 1, lflags | NEW_FLAG));
		}
	}


void
justify(char *endp, int nextra, int nholes)
	{
	register char *p1, *p2;
	int nb;
	static char dir;

	if(nextra <= 0  ||  nholes <= 0)
		return;

	dir = 1 - dir;		/* reverse fill direction */

	p1 = endp;
	p2 = p1 + nextra;

	if(p2 > &rbuff[LINE_LENGTH])
		p2 = &rbuff[LINE_LENGTH];

	while(p1 < p2) {
		*p2 = *p1;
		if(*p1 == ' ') {
			if(dir)
				nb = (nextra - 1)/nholes + 1;
			else
				nb = nextra/nholes;
			nextra -= nb;
			--nholes;

			while(nb--> 0)
				*--p2 = ' ';
			}
		--p1;
		--p2;
		}
	}



int zap_center(int laddr)
	{
	struct line *l1;
	register char *p, *startp, flags;
	register int width = left_margin;

	flags = (l1 = getptr(laddr))->lflags;
	p = l1->textp;
	while(*p == ' ')
		++p;

	startp = p;
	while(*p) {
		if(*p++ == '\t')
			width |= 0x03;
		++width;
		}

	width = imax((right_margin - width)/2, -1) + left_margin;

	memmove(rbuff + width, startp, strlen(startp) + 1);

	for(p = rbuff; p <= &rbuff[width-1] ; ++p)
		*p = ' ';

	delete(laddr, laddr, NOSAVE);
	return(addline(rbuff, 1, flags));
	}

void dummy() {;}
