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
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

void
disp_stats() {
	register char *p1, *p2;

	if(lastln != lastln_save  ||  clr_flag) {
		dtoc(&stat_str[5], lastln_save = lastln);
		put_stat(0, stat_str, 10);
		}

	if(curln != curln_save  ||  clr_flag) {
		dtoc(&stat_str[12], curln_save = curln);
		put_stat(10, &stat_str[11], 6);
		}

	if(lastln == 0)
		curin = 0;

	if(curcol != curcol_save  || clr_flag) {
		dtoc(&stat_str[19], curcol_save = curcol);
		put_stat(16, strcat(&stat_str[18], ")"), 5);
		}

	p1 = (char *) &opt.opt_a;
	p2 = (char *) &opt_save;

	while(p1 < ( (char *)&opt + sizeof(opt) ) ) {
		if(*p1 != *p2  ||  clr_flag) {
			*p2 = *p1;
			put_option(p1 - (char* ) &opt, *p1);
			}
		++p1;
		++p2;
		}
	}



int col_to_index(char *buf, int col)
	{
	register char *p;
	register int n;

	p = buf;
	n = 0;
	--col;

	while(n < col  &&  *p) {
		if(*p++ == '\t')
			n |= tab_len;
		++n;
		}

	if(n < col)
		p += col-n;
	else
		curcol = n + 1;		/* force it to line up with tabstop */

	if(p - buf >= LINE_LENGTH) {
		curcol = index_to_col(buf, LINE_LENGTH - 1);
		return(LINE_LENGTH - 1);
		}

	return(p-buf);
	}




int index_to_col( char *buf, int index)
	{
	register char *p, *endp;
	register int col;

	p = buf;
	endp = p + index;
	col = 0;

	while(p < endp  &&  *p) {
		if(*p++ == '\t')
			col |= tab_len;
		++col;
		}

	col += endp - p;

	return(col + 1);
	}



char *
recall_line(int c, char *buf) {

	switch(c) {
	case 'f':
		recall_lineno = 0;
		break;

	case 'n':
		if(recall_lineno < MAX_RECALL_LINES-1  &&
			recall_buf[(MAX_RECALL_LINES-1) - (recall_lineno+1)][0])
			++recall_lineno;
		break;

	case 'p':
		if(recall_lineno > 0)
			--recall_lineno;
		break;
		}

	strcpy(buf, recall_buf[(MAX_RECALL_LINES-1) - recall_lineno]);

	return(buf);
	}


char *
cmd_input( char *buffer)
	{
	register char *p;
	register struct line *p1;
	unsigned c;
	static char fill_used = 0;

	if(fill_used) {
		if(cc_reg)
			strcpy(buffer, ".+1|zch$\n");
		else
			strcpy(buffer, "zch$\n");
		fill_used = 0;
		return(buffer);
		}

	if(prnt_row) {
		prnt_row = 0;
		mark_line(0);
		term_flush();
		term_key();
		term_init();
		clear_screen1(0);
		term_flush();
		clr_flag = 1;
		}

	if(lastln == 0)
		change_state(CMD);	/* force state to cmd if no text */
	update_screen();

	for(;;)
		switch(c = mgetchar()) {
	
		case CMD_CHAR:
			p = buffer;
			while((c = mgetchar()) != '\r'  &&  c != '\n')
				if(p < &buffer[LINE_LENGTH - 2]  &&  c != CMD_CHAR)
					*p++ = c;
	
			*p++ = '\n';
			*p = '\0';
			return(buffer);
	
		case RECALL_CHAR:
			recall_line('f', firstp->textp);
			firstp->lflags |= DIRTY_FLAG;
			continue;

		case RECALL2_CHAR:
			recall_line(mgetchar(), firstp->textp);
			firstp->lflags |= DIRTY_FLAG;
			continue;

		case '\r':
		case '\n':
			if(opt.state == CMD) {
				p1 = firstp;
				if(strlen(p1->textp)  &&  strcmp(recall_buf[MAX_RECALL_LINES - 1], p1->textp) != 0) {
					memcpy(recall_buf[0], recall_buf[1], (MAX_RECALL_LINES-1)*sizeof(recall_buf[0]));
					strcpy(recall_buf[MAX_RECALL_LINES - 1], p1->textp);
					}

				strcat(strcpy(buffer, p1->textp), "\n");
				p1->textp[0] = '\0';
				p1->lflags |= DIRTY_FLAG;
				cmdin = 0;
				}
			else
				strcpy(buffer, opt.opt_n ? "zcla\n" : "zcl.+1|zchl\n");
	
			change_state(TEXT);
			return(buffer);
	
		case EOF:
			return(0);

		default:
			if(curcol == right_margin+1  &&  opt.state == TEXT  &&  opt.opt_f) {
/*
				strcpy(buffer, " zcc zlfb1f.+1|\nzchlzch$\n");
 */
				strcpy(buffer, ".zcc zlf\n");
				fill_used = 1;
				}
			else {
				strcpy(buffer, "%zcc %zch+1\n");
				}
			buffer[4] = c;
			return(buffer);
			}
	}


void
change_state(char new_state)
	{
	int ctype1, ctype2;
	char one;

	if(lastln == 0)		/* Force user to command state */
		new_state = CMD;

	if(opt.state != new_state) {
		firstp->lflags |= DIRTY_FLAG;

		if(opt.state == CMD) {
			ctype1 = ACTIVE;
			ctype2 = INACTIVE;
			}
		else {
			ctype1 = INACTIVE;
			ctype2 = ACTIVE;
			}

		move_cursor(cursor_row, cursor_col, ctype1,
					&cursor_row, &cursor_col, ctype2);

		one = 1;
		move_cursor(one, cmd_cursor_col, ctype2,
					&one, &cmd_cursor_col, ctype1);

		opt.state = new_state;
		}
	}
