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
 * April 1980 by  D. T. Dodge
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/qnxterm.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

struct line *getptr( int laddr ) {
	struct line *bp;
	int i;

	if(laddr <= (lastln-laddr)) {
		bp = firstp;
		for(i = 0 ; i < laddr ; ++i)
			bp = bp->next;
		}
	else {
		bp = lastp;
		for(i = lastln ; i > laddr ; --i)
			bp = bp->prev;
		}
	return( bp );
	}



int addline( char *tp, char replace_flag, char lflags ) {
	struct line *p, *p1;
	int n;

	unbreakable();

	do {
		if((p = ccalloc(sizeof(struct line) + (n = len(tp)))) == 0) {
			breakable();
			return( ERROR0 );
			}
		tp = store_line(tp, p->textp, n);
		p1 = getptr(curln);
		relink(p, p1->next);
		relink(p1, p);
		++curln;
		if(!replace_flag  ||  *tp)
			mark_line(curln);
		p->lflags = DIRTY_FLAG | lflags | ( replace_flag ? 0 : NEW_FLAG );
		replace_flag = 0;	/*	After the 1st line, all others are new	*/
		++lastln;
		if(curln == lastln)
			lastp = p;
		} while(*tp);

	breakable();

	return( OK );
	}


void
relink( struct line *bp1, struct line *bp2) {
	dirty = 1;
	bp1->next = bp2;
	bp2->prev = bp1;
	}


char *
store_line( char *from, char *to, int n) {
	char *p1, *p2;

	p1 = from;
	p2 = to;
	while(--n > 0)
		*p2++ = *p1++;
	*p2 = '\0';

	return(*p1 ? ++p1 : p1);
	}



int initbuffer() {
	init_externs();

	if((firstp = lastp = calloc(1, sizeof(*firstp) - 1 + LINE_LENGTH + 2)) == 0)
		return(ERROR0);

	strcpy(firstp->textp, "");
	firstp->lflags = DIRTY_FLAG;
	relink(firstp, lastp);
	if((purge_ptr = calloc(1, sizeof(*purge_ptr))) == 0)
		return(ERROR0);

	purge_end_ptr = purge_ptr;
	relink(purge_ptr, purge_ptr);
	curln = lastln = 0;

	if((rbuff = calloc(1, LINE_LENGTH + 2*PAT_SIZE + 10)) == 0)
		return(ERROR0);

	tbuff = &rbuff[LINE_LENGTH + 4];
	pattern = &tbuff[PAT_SIZE + 2];

/*	memory_mapped = 1;	*/
	device_setup();

	clear_screen1(0);	/* formfeed to clear screen */

/*	errno = 0; */
	if(exec_file(macro_file, 0, 0) != OK) {
		default_macros();
		}
	return(OK);
	}

void
init_externs() {

	left_margin = curcol = screen_row = screen_col = 1;
	right_margin = 80;
	opt.opt_a = opt.opt_m = opt.opt_w = opt.state = clr_flag = 1;
	strcpy(opt_chars, "abcdfijlmnstw");
	fill_char = ' ';
	view_quick = 1;
	macro_flags = 1;
	msgrow = 1;
	strcpy(stat_str, "last=nnnnn0(nnnnn0,nnn0");

	error_msgs[0] = "out of memory";
	error_msgs[1] = "no tagged lines";
	error_msgs[2] = "unknown command";
	error_msgs[3] = "invalid pattern specification";
	error_msgs[4] = "buffer has been modified; delete all lines or use qq to quit or ee to edit";
	error_msgs[5] = "invalid line number or line range";
	error_msgs[6] = "nested x or l command";
	error_msgs[7] = "current file not defined";
	error_msgs[8] = "unable to access file";
	error_msgs[9] = "syntax error";
	error_msgs[10] = "file name too long";
	error_msgs[11] = "unknown option";
	error_msgs[12] = "line greater than 512 chars";
	error_msgs[13] = "pattern not found";
	error_msgs[14] = "disk error";
	error_msgs[15] = "command aborted";
	error_msgs[16] = "Attempt to write to a file which is not the current file. Use ww to force";
	}



int len(char *_s)
	{
	char *s;

	s = _s;
	while(*s != '\n'  &&  *s != '\r'  &&  *s != '\0') ++s;

	return(++s - _s);
	}



int getfn() {
	char *p;

	p = lp;

	if(*p != ' '  &&  *p != '\n')
		return(ERROR9);

	if(*p == ' ') {
		if(restrict_flag)
			return(ERROR8);
		lp = ++p;
		while(*p  &&  *p != '\n') {
			if(p++ >= &lp[FNAME_SIZE])
				return( ERROR10 );
			}
		*p = '\0';
		return( OK );
		}

	if(curfile[0] != '\0') {
		lp = curfile;
		return( OK );
		}

	return( ERROR7 );
	}

void
set_fn(char *dst, char *src)
	{
#if 0
#ifdef __STDC__
	if(*src == '/'  &&  qnx_fullpath(rbuff, src)) { // gkm
#else
	if(*src == '/'  &&  full_path(rbuff, src)) {
#endif
		rbuff[FNAME_SIZE] = 0;
		strcpy(dst, rbuff);
		}
	else
#endif
		strcpy(dst, src);
	}


int getint() {
	int num;

	num = 0;
	while(*lp <= '9'  &&  *lp >= '0')
		num = num*10 + (*lp++ - '0');
	return(num);
	}


void
putln(FILE *fd, char *s, char raw)
	{
	char *p;

	p = s;

#if 0			/* @@@	*/
	if(ftty(fd))
		options = set_option(fd, get_option(fd) | ETAB|ERS);
#endif

	while(*p)
		if(raw)
			putc(*p++, fd);
		else
			expand(*p++);

	fprintf( fd, "\r\012" );

#if 0			/* @@@	*/
	if(ftty(fd)) {
		fflush(fd);
		set_option(fd, options);
		}
#endif
	}


void
dtoc(char *buff, int n)
	{
	char *p;

	*(p = buff + 5) = '\0';

	do
		*--p = n%10 + '0';
	while(n /= 10);

	strcpy(buff, p);
	}


void
puterr(int err)
	{
	extern char *error_msgs[];

	macro_flags = 1;
	macro_ptr = 0;
	macro_level = 0;
	if(err != ERROR13)
		putc(007, stderr);	/* make some noise... */
	putmsg(error_msgs[-3 - err]);

	while(msg_tid == 0  &&  msg_char != '\r'  &&  msg_char != '\n')
		msg_char = term_key();

	cmdin = 0;
	change_state(CMD);
	}


void
putmsg(char *msg)
	{
	char ostate;

	if(msg_tid == 0) {
		firstp->lflags |= DIRTY_FLAG;
		/*
		 *	Fake command mode so the message offset is correct.
		 */
		ostate = opt.state;
		opt.state = CMD;
		disp_line_image(1, msg, attributes[CMD_AREA], 0);
		opt.state = ostate;
/*		term_flush();	*/
		if((msg_char = term_key()) == 0xFF)
			msg_char = term_key();
		}
	}


void
mark_line(int ln)
	{
	if(ln < redisp_line)
		redisp_line = ln;
	}


void
prnt_screen(char *text, char raw)
	{

	if(prnt_row <= 0) {
		clear_screen1(0);
		prnt_row = 1;
		}

	if(prnt_row++ >= screen_height-1) {
		if(msg_tid == 0) term_key();
		clear_screen1(0);
		prnt_row = 1;
		}

	putln(stdout, text, raw);
	}


void
purge( struct line *line_ptr, int num_lines) {
	struct line *p1, *p2, *temp;
	int n;

	if(num_lines == 0)
		return;

	n = num_lines;
	temp = line_ptr->prev;

	for(p2 = p1 = line_ptr ; --n >= 0 ; p1 = p2) {
		p2 = p1->next;
		free(p1);
		}

	relink(temp, p2);
	}


void *
ccalloc( int size ) {
	void *p;

	if(p = malloc(size))
		return(p);

	if(npurge_lns) {
		unbreakable();
		purge(purge_ptr->next, npurge_lns);
		purge_end_ptr = purge_ptr;
		npurge_lns = 0;
		breakable();
		putmsg("Delete buffer lost. Space reclaimed.");
		if(p = malloc(size))
			return(p);
		}

	return(NULL);
	}

int imin(int a, int b) 	{	return(a <= b ? a : b);
	}

int imax(int a, int b) 	{	return(a >= b ? a : b);
	}
