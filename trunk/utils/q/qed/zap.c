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
#include <sys/types.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int zap()
	{
	register char *p, *lp_save = lp;
	register int laddr, status = OK, line_save = curln, index_save = curin;
	int n;

	for(laddr = laddr1 ; status > ERROR  &&  laddr <= laddr2 ; ++laddr) {
		lp = lp_save;
		switch(*lp++) {

		case 'c':
			status = zap_char(laddr);
			break;

		case 'f':
			zap_func(getint());
			break;

		case 'h':
			if(nmarks) {
				curln = marker1;
				curcol = limit1;
				}
			return(OK);

		case 'k':
			if((status = get_laddr_expr(&n)) == OK)
				status = replace(n, getptr(laddr)->textp, "");
			break;
				
		case 'l':
			status = zap_line(laddr);
			break;

		case 'm':
			msgcol = getint();
			p = rbuff;
			while((*p = *lp++)  &&  *p != '\n')
				++p;
			*p = '\0';

			change_state(CMD);
			disp_line_image(1, rbuff, attributes[CMD_AREA], 0);
			firstp->lflags &= ~DIRTY_FLAG;
			term_flush();
			break;

		case 'p':
			if(npurge_lns) {
				unbreakable();
				purge(purge_ptr->next, npurge_lns);
				purge_end_ptr = purge_ptr;
				npurge_lns = 0;
				breakable();
				}
			break;

		case 'q':
			if((n = *lp++) == '#') {
				cc_reg = (nmarks != 0);
				break;
				}
			if(n == '!') {
				cc_reg = (cmd_status == 0);
				break;
				}

		case 'v':
			cc_reg = VERSION >= *lp++;
			break;

		default:
			return(ERROR2);
			}

		if(*lp_save == 'l' ||  *lp_save == 'p') {
			curln = line_save;
			curin = index_save;
			curcol = -1;
			}
		else if(laddr)
			curln = laddr;
		}

	return(status);
	}



int zap_char(int laddr)
	{
	register int status, index, length, n;
	register struct line *p;

	status = OK;
	index = laddr ? curin : cmdin;
	length = strlen((p = getptr(laddr))->textp);

	switch(*lp++) {

	case 'c':
		strcpy(rbuff, p->textp);
		if(laddr) {
			rbuff[length] = '\n';
			rbuff[length + 1] = '\0';
			}
		zchar_change(index, length, lp, 1, laddr);
		if(*lp != '\n')
			++lp;
		status = replace(laddr, rbuff, p->textp);
		break;

	case 'd':
		strcpy(rbuff, p->textp);
		if(zchar_delete(index, length, 1) != length)
			status = replace(laddr, rbuff, p->textp);
		break;

	case 'D':
		n = horizontal(index, length, p->textp);
		strcpy(rbuff, p->textp);
		if(zchar_delete(index, length, n - index + 1) != length)
			status = replace(laddr, rbuff, p->textp);
		break;

	case 'e':
		strcpy(rbuff, p->textp);
		n = horizontal(index, length, rbuff);
		zchar_erase(index, length, n - index + 1);
		status = replace(laddr, rbuff, p->textp);
		break;

	case 'f':
		fill_char = *lp;
		if(*lp != '\n') ++lp;
		status = OK;
		break;

	case 'h':
		index = horizontal(index, length, p->textp);
		if((status = index) < ERROR)
			break;
		status = OK;

		if(laddr) {
			curin = index;
			curcol = 0;
			}
		else
			cmdin = index;

		break;

	case 'l':
		lock_cursor = 1;
		status = OK;
		break;

	case 'p':
		del_cnt = del_index = 0;
		status = OK;
		break;

	case 'R':
		strcpy(rbuff, p->textp);
		n = min(getint(), del_cnt);
		zchar_change(index, length, dbuff, n, laddr);
		status = replace(laddr, rbuff, p->textp);
		break;

	case 'r':
		if(del_cnt <= 0) {
			cc_reg = FALSE;
			return(OK);
			}

		cc_reg = TRUE;
		--del_cnt;
		if(--del_index < 0)
			del_index = LINE_LENGTH - 1;
		strcpy(rbuff, p->textp);
		zchar_change(index, length, &dbuff[del_index], 1, laddr);
		status = replace(laddr, rbuff, p->textp);
		break;

	case 's':
		if(index >= length) {
			cc_reg = FALSE;
			return(OK);
			}
		cc_reg = TRUE;

		save_char(p->textp[index]);
		status = OK;
		break;

	default:
		status = ERROR2;
		}

	return(status);
	}



int zchar_change(int index, int length, char *textptr, int nchars, int laddr)
	{
	register char *p, *tp, fc;
	register int len;

	len = length;
	tp = textptr;
	fc = laddr ? fill_char : ' ';

	while(nchars--) {

		if(index >= len) {
			for(p = rbuff + len ; index >= len ; ++len) {
				if(fc == '\t')
					curin = index = imax(index - 3, len);
				*p++ = fc;
				}
			*p = '\0';
			}
		else if(opt.opt_i  &&  len < LINE_LENGTH)
			memmove(rbuff + index + 1, rbuff + index, len++ - index + 2);

		if(*tp)
			rbuff[index] = *tp;
		++index;
		++tp;
		}

	return(len);
	}



int zchar_delete(int index, int length, int nchars)
	{
	register int n;

	if(index >= length  ||  nchars <= 0) {
		cc_reg = FALSE;
		return(length);
		}

	cc_reg = TRUE;
	n = min(index + nchars, length);
	strcpy(rbuff + index, rbuff + n);

	return(length - (n - index));
	}



int zchar_erase(int index, int length, int nchars)
	{
	register int end;

	if(index >= length  ||  nchars <= 0) {
		cc_reg = FALSE;
		return(length);
		}

	cc_reg = TRUE;
	if(index + nchars < length)
		for(end = index + nchars; index < end; ++index)
			rbuff[index] = ' ';
	else
		rbuff[length = index] = '\0';

	return(length);
	}



int zchar_save(int index, int length, int nchars)
	{
	register struct line *p1, *p2;
	register int i;

	if(nchars <= 0) {
		cc_reg = FALSE;
		return(NOTHING);
		}

	cc_reg = TRUE;
	unbreakable();
	if((p1 = ccalloc(sizeof(struct line) + nchars)) == 0) {
		breakable();
		return( ERROR0 );
		}

	for(i = 0; i < nchars; ++i)
		p1->textp[i] = index + i < length ? rbuff[index + i] : ' ';
	p1->textp[nchars] = '\0';
	p1->lflags = 0;

	p2 = npurge_lns ? purge_end_ptr : (purge_end_ptr = purge_ptr);
	relink(p1, p2->next);
	relink(p2, p1);
	purge_end_ptr = p1;
	++npurge_lns;
	breakable();

	return(OK);
	}


int replace(int laddr, char *new_text, char *old_text)
	{
	char lflags;
	int line_save;

	if(laddr) {
		line_save = redisp_line;
		lflags = getptr(laddr)->lflags;
		delete(laddr, laddr, NOSAVE);
		redisp_line = line_save;
		
		if(addline(new_text, 1, lflags) != OK) {
			addline(strcpy(rbuff, old_text), 1, lflags);
			return(ERROR0);
			}
		}
	else {
		strcpy(firstp->textp, new_text);
		firstp->lflags |= DIRTY_FLAG;
		}

	return(OK);
	}



int horizontal(int cur_index, int length, char *text)
	{
	register int index;
	int n, c;
	register char *p1, *p2;

	index = cur_index;

	switch(*lp++) {

	case '$':
		if(length == 0)
			index = col_to_index(text, left_margin);
		else
			index = length;
		break;

	case '+':
		index += getint();
		break;

	case '-':
		index -= getint();
		break;

	case 'l':
		index = col_to_index(text, left_margin);
		break;

	case 'r':
		index = col_to_index(text, right_margin);
		break;

	case '/':
	case '?':
		--lp;
		c = *lp;
		if(getpat() <= ERROR)
			return(ERROR3);
		++lp;
		if(index >= length) {
			cc_reg = FALSE;
			return(cur_index);
			}
		p1 = &text[index];
		if(c == '?') {
			for(p2 = rbuff ; p1 >= text ; *p2++ = *p1--);
			*p2 = '\0';
			}
		else
			strcpy(rbuff, p1);
			
		if((n = match(rbuff, pattern, FORWARD, -1)) == 0) {
			cc_reg = FALSE;
			return(cur_index);
			}
		--n;
		if(c == '?') {
/*			if(rbuff + n >= p2) {
				cc_reg = FALSE;
				return(cur_index);
				}
*/			index -= n;
			}
		else
			index += n;
		index = imin(imax(0, index), LINE_LENGTH - 1);	/* make sure it succeeds */
		break;

	default:
		--lp;
		index = col_to_index(text, getint());
		}

	cc_reg = index >= length  ||  index < 0 ? FALSE : TRUE;
	return(imin(imax(0, index), LINE_LENGTH - 1));
	}


void
save_char(char c)
	{

	if(del_cnt < LINE_LENGTH)
		++del_cnt;

	dbuff[del_index++] = c;
	if(del_index >= LINE_LENGTH)
		del_index = 0;
	}



int zap_line(int laddr)
	{
	static long last_ticks;
	char flag;
	unsigned n, n1 ,n2, c, status = OK;
	register struct line *p;
	register char *p1;
	struct tms buff;
	long nt;

retry:
	switch(c = *lp++) {

	case 'c':
		status = zap_center(laddr);
		break;

	case 'w':	/* save then write */
	case 's':	/* save in delete buffer */
	case 'd':	/* delete */
	case 'e':	/* erase */
		strcpy(rbuff, p1 = getptr(laddr)->textp);
		n = strlen(rbuff);
		n1 = col_to_index(rbuff, limit1);
		n2 = col_to_index(rbuff, limit2);
		opt.opt_l = FALSE;
		if(c == 'e')
			zchar_erase(n1, n, n2-n1 + 1);
		else if(c == 'd')
			zchar_delete(n1, n, n2-n1 + 1);
		else {
			status = zchar_save(n1, n, n2-n1 + 1);
			if(status == OK  &&  c == 'w'  &&  laddr == laddr2)
				status = _write(getfname(), 1, npurge_lns, 0, 1);
			break;
			}

		status = replace(laddr, rbuff, p1);
		break;

	case 'f':
		status = zap_fill(laddr1, laddr2);
		laddr2 = laddr1;	/* Force zap to stop calling us */
		break;

	case 'j':
		flag = CONT_FLAG;
		goto toggle_flag;

/*
	case 'k':
		status = zap_kopy(laddr);
		opt.opt_l = FALSE;
		break;

	case 'm':
		if((status = zap_kopy(laddr)) == OK) {
			c = 'e';
			goto remove;
			}
		opt.opt_l = FALSE;
		break;

	case 'M':
		if((status = zap_kopy(laddr)) == OK) {
			c = 'd';
			goto remove;
			}
		break;
*/

	case 'o':
		flag = OVER_FLAG;
		goto toggle_flag;

	case 'p':
		flag = PARA_FLAG;

	toggle_flag:
		(p = getptr(laddr))->lflags ^= flag;
		p->lflags |= DIRTY_FLAG;
		break;

	case 'q':
		cc_reg = (nmarks != 0);
		break;

	case 'R':
		if((status = _read(getfname(), 0, 1)) != OK)
			break;
		lp = "\n";
	case 'r':
		status = zap_restore(laddr);
		break;

	case 't':
		if(nmarks == 2  &&  laddr == marker1  &&  opt.opt_l == 0) {
			mark_line(marker1);
			marker1 = marker2;
			--nmarks;
			}
		else if(nmarks  &&  laddr == marker2  &&  opt.opt_l == 0) {
			nt = times( &buff ) - last_ticks;
/*
			if((nt = times( &buff ) - last_ticks ) < 0 )
				nt += 1000;
*/
#ifndef __QNXNTO__
			if(nt <= 333  &&  nmarks == 1)
#else
			if(nt <= 60 * 5  &&  nmarks == 1)
#endif
				limit_save = opt.opt_l = TRUE;
			else {
				limit_save = opt.opt_l = FALSE;
				marker2 = marker1;
				--nmarks;
				}
			}
		else {
			if(nmarks == 0) {
				marker1 = marker2 = laddr;
				limit1 = curcol;
				limit2 = LINE_LENGTH - 1;
				limit_save = opt.opt_l = FALSE;
				last_ticks = times( &buff );
				}
			else {
				if(laddr < marker1)
					marker1 = laddr;
				else
					marker2 = laddr;

				if(curcol < limit1)
					limit1 = curcol;
				else
					limit2 = curcol;
				}

			if(nmarks < 2)
				++nmarks;
			}

		mark_line(marker1);
		break;

	case 'u':
		if(nmarks) {
			nmarks_save = nmarks;
			nmarks = 0;
			opt.opt_l = FALSE;
			}
		else {
			nmarks = nmarks_save;
			opt.opt_l = limit_save;
			}

		mark_line(marker1);
		break;

	case 'y':
		if((status = yut()) == OK) {
			*--lp = msg_char;
			goto retry;
			}
		break;

	default:
		return(ERROR2);
		}

	return(status);
	}

char *
getfname() {
	register char *p;
	static char buf[17];

	sprintf(buf, "/tmp/ed_%u.%u", getgid(), getuid() );

	if(*lp == '"')
		for(p = buf, ++lp ; p < &buf[16] ; ++p, ++lp)
			if(*lp != '"')
				*p = *lp;
			else {
				*p = '\0';
				++lp;
				break;
				}

	return(buf);
	}
