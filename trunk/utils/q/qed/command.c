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
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int exec_cmd(int under_glob, int under_until)
	{
	register int status;
	register char *p;
	int n;

	if((status = getrange()) <= ERROR)
		return( status );
	status = ERROR9;

	switch( *lp++ ) {

	case 'i':
		laddr2 = prevln(laddr2);

	case 'a':
		status = append(laddr2, under_glob);
		break;

	case 'b':
			if(!under_glob  &&  !under_until)
				status = branch();
		break;

	case 'c':
		if((status = delete(laddr1, laddr2, SAVE)) == OK)
			status = append(prevln(laddr1), under_glob);
		break;

	case 'd':
		if((status = delete(laddr1, laddr2, SAVE)) == OK && nextln(curln) != 0)
			curln = nextln(curln);
		break;

	case 'e':
		if(lastln  &&  dirty  &&  *lp != 'e') {
			status = ERROR4;
			break;
			}

		if(*lp == 'e')
			++lp;

		if(nladdrs == 0  &&  !under_glob  &&  !under_until  &&
			(status = getfn()) == OK) {
			set_fn(curfile, lp);
			if(lastln != 0)
				delete(1, lastln, NOSAVE);
			num_delete_lines = 0;
			if((status = _read( lp, 0, 0)) == OK) {
				dirty = 0;
				if(lastln)
					curln = 1;
				}
			}
		lp = "\n";
		break;

	case 'f':
		if(nladdrs == 0  &&  (status = getfn()) == OK) {
			set_fn(curfile, lp);
			putmsg(curfile);
			lp = "\n";
			}
		change_state(CMD);
		break;

	case 'g':
		if(!under_glob) {
			if(*lp == '^') {
				++lp;
				n = 0;
				}
			else
				n = 1;
			status = exec_glob(n, under_until);
			}
		break;

	case 'h':
		n = getint();
#ifndef __STDC__
		while(n--)
			for(n1 = 0; n1 < 10; ++n1)
				time_slice();
#endif
		status = OK;
		break;

	case 'j':
		status = join(laddr2);
		break;

	case 'k':
		if((status = get_laddr_expr(&n)) == OK)
			status = kopy(n);
		break;

	case 'l':
		if(nladdrs == 0)
			status = learn();
		break;

	case 'm':
		if((status = get_laddr_expr(&n)) == OK)
			status = move(n);
		break;

	case 'o':
		status = option();
		break;

	case 'p':
	case 'P':
		status = prnt(laddr1, laddr2);
		break;

	case 'q':
		if(nladdrs==0 && !under_glob) {
			if((*lp=='\n' && !dirty) || *lp=='q'  ||  lastln == 0)
				status = EOF;
			else
				status = ERROR4;
		}
		break;

	case 'r':
		if(!under_glob  &&  !under_until  &&  (status = getfn()) == OK)
			status = _read(lp, laddr2, 0);
		lp = "\n";
		break;

	case 's':
		n = getint(); /* read occurance if present */
		if((status=getpat()) == OK  &&  (status=getsubst_str(tbuff)) == OK)
			status = substitute(tbuff, under_glob, n);
		break;

	case 't':
	case 'T':
		if(nladdrs == 0)
			status = translate(*(lp - 1) == 't');
		break;

	case 'u':
		status = until(laddr1, laddr2, under_glob);
		break;

	case 'v':
		if(nladdrs <= 1)
			status = view(laddr1);
		break;

	case 'w':
		n = 0;
		if(*lp == 'w') {
			n |= 2;
			++lp;
			}
		if(*lp == 'a') {
			n |= 1;
			++lp;
			}

		if((status = getfn()) == OK) {
			if(nladdrs == 0) {
				if(curfile[0] == '\0')
					set_fn(curfile, lp);
				else {
					if((n & 2) == 0  &&  strcmp(curfile, lp) != 0) {
						for(p = lp ; *p ; ++p)
							if(*p == '$')
								goto ok;
						puterr(-19);
						lp = "\n";
						break;
						}
					}
				ok:
				if((status = _write(lp, 1, lastln, n & 1, 0)) == OK)
					dirty = 0;
				}
			} else {
				status = _write(lp, laddr1, laddr2, n & 1, 0);
			}
		lp = "\n";
		break;

    case 'x':
		if(!under_glob  &&  !under_until  &&  (status = getfn()) == OK)
			if(nladdrs == 0)
				status = exec_file(lp, under_glob, under_until);
		lp = "\n";
		break;

	case 'y':
		status = yut();
		break;

	case 'z':
		status = zap();
		break;

	case '\n':
		--lp; /* put back newline for main */
		if(laddr2 < 0  ||  laddr2 > lastln)
			status = lastln ? ERROR5 : OK;
		else {
			curln = laddr2;
			status = OK;
			}
		break;

	case ' ':
	case '\t':
	case CMD_CHAR:
		status = OK;
		break;

	case '=':
		dtoc(rbuff, laddr2);
		if(under_glob)
			prnt_screen(rbuff, 1);
		else
			putmsg(rbuff);
		status = OK;
		break;

	case '"':
		lp = "\n";	/* Ignore rest of line */
		status = OK;
		break;

	case '!':
		if(escape_char == 0  ||  restrict_flag) {
			putmsg("Escape from ED inhibited");
			status = NOTHING;
			}
		else
			status = exec_sys_cmd(under_glob, under_until);
		break;

	default:
		status = ERROR2;
		}
	return(status);
	}



int append(int laddr, char under_glob)
	{
	register int status;

	if(*lp != '\n'  &&  *lp++ != ' ') {
		if(*(lp + -1) == 'd')
			return(undelete(laddr));
		return(ERROR9);
		}

	curln = laddr;
	if((status = addline(esc_line(lp), 0, 0)) == OK) {
		curcol = left_margin;
		change_state(TEXT);
		if(!under_glob)
			opt.opt_n = TRUE;
		if(opt.opt_s  &&  ++auto_save_cnt >= SAVE_THRESHOLD) {
			_write("auto_save", 1, lastln, 0, 0);
			auto_save_cnt = 0;
			}
		}

	lp = "\n";  /* forces main to fetch a new line */
	return( status );
	}



int prnt(int from, int to)
	{
	register int laddr;
	char raw = 0;

	if(from <= 0)
		return( ERROR5 );

	raw = *(lp + -1) == 'P';

	for(laddr = from ; laddr <= to ; ++laddr)
		prnt_screen(getptr(laddr)->textp, raw);

	curln = to;
	return( OK );
	}



int delete(int from, int to, char save)
	{
	register struct line *p1, *p2, *p3;
	register int num;
	char new_purge_type;

	if(from <= 0)
		return( ERROR5 );

	unbreakable();
	p1 = getptr(prevln(from));
	p2 = getptr(to);
	if(to == lastln)
		lastp = p1;
	num = to - from + 1;
	new_purge_type = from==to ? SINGLE : MULTIPLE;
	lastln -= num;
	curln = prevln(from);
	mark_line(imin(lastln, from));

	if(save) {
/*
 * Code added to help display handler figure out a fast way using line ins/del
 * to update the screen.
 */
		if(from <= lastln && (num_delete_lines == 0  ||  from < delete_line)) {
			delete_line = from;
			num_delete_lines = num;
			}

		if((from != to  ||  purge_type != new_purge_type)  &&  npurge_lns) {
			purge(purge_ptr->next, npurge_lns);
			npurge_lns = 0;
			}

		if(npurge_lns == 0)
			purge_end_ptr = p2;

		npurge_lns += num;
		purge_type = new_purge_type;
		p3 = purge_ptr->next;
		relink(purge_ptr, p1->next);
		relink(p1, p2->next);
		relink(p2, p3);
		}
	else
		purge(p1->next, num);
	breakable();
	return( OK );
	}



int undelete(int laddr)
	{
	register struct line *p1, *p2, *p3;

	if(laddr < 0)
		return(ERROR5);

	cc_reg = FALSE;
	if(npurge_lns == 0)
		return(OK);		/* Nothing to restore... */

	p1 = purge_ptr->next;
	p3 = getptr(laddr);

	unbreakable();
	if(purge_type == SINGLE) {
		p2 = p1;
		++lastln;
		--npurge_lns;
		}
	else {
		p2 = purge_ptr->prev;
		lastln += npurge_lns;
		npurge_lns = 0;
		}

	/*
	 * First unlink line(s) from purge list.
	 */

	relink(purge_ptr, p2->next);

	/*
	 * Next link in the line(s) between p1 and p2 into the text.
	 */

	relink(p2, p3->next);
	relink(p3, p1);
	if(lastp == p3)
		lastp = p2;
	mark_line( imax( 1, laddr ) );

	/*
	 * Mark the undeleted lines as new and dirty
	 */

	while( p1 != p2->next ) {
		p1->lflags |= NEW_FLAG | DIRTY_FLAG;
		p1 = p1->next;
		}

	curln = laddr + 1;
	cc_reg = TRUE;
	breakable();
	return(OK);
	}



int kopy(int laddr3)
	{
	register int laddr, status;
	register struct line *bp;

	if(laddr1 <= 0  ||  (laddr1 <= laddr3  &&  laddr3 < laddr2))
		return(ERROR5);

	status = OK;
	curln = laddr3;
	for(bp = getptr(laddr = laddr1) ; laddr <= laddr2 ; ++laddr) {
		if((status = addline(bp->textp, 0, bp->lflags)) != OK)
			break;
		bp = bp->next;
		}
	curln = imin(laddr3 + 1, lastln);

	return(status);
	}



int move(int laddr3)
	{
	register struct line *p1, *p2, *p3, *p4, *p5, *p6;

	if(laddr1<=0 || (laddr1<=laddr3 && laddr3<=laddr2))
		return( ERROR5 );
	if(nextln(laddr3) == laddr1)
		return( OK );  /* no move necessary */
	p1 = getptr(prevln(laddr1));
	p2 = getptr(nextln(laddr2));
	p3 = getptr(laddr1);
	p4 = getptr(laddr2);
	p5 = getptr(laddr3);
	p6 = getptr(nextln(laddr3));

	unbreakable();

	relink(p1, p2);
	relink(p5, p3);
	relink(p4, p6);
	if(laddr3 > laddr1) {
		curln = laddr3 - (laddr2-laddr1);
		mark_line(laddr1);
		if(laddr3 == lastln)
			lastp = p4;
		}
	else {
		curln = laddr3 + 1;
		mark_line( imax( 1, laddr3 ) );
		if(laddr2 == lastln)
			lastp = p1;
		}

	/*
	 * Mark the moved lines as new and dirty
	 */

	while( p3 != p6 ) {
		p3->lflags |= NEW_FLAG | DIRTY_FLAG;
		p3 = p3->next;
		}
/*
 * Code added to help display handler figure out a fast way using line ins/del
 * of updating the screen.
 */
	if( num_delete_lines == 0  ||  laddr1 < delete_line ) {
		delete_line = laddr1;
		num_delete_lines = laddr2 - laddr1 + 1;
		}

	breakable();

	return( OK );
	}



int _read( char *file, int laddr, char bufnum) {
	register struct line *p, *tp;
	char c, first = 0;
	int n;

	if(strcmp(file, "@") == 0) {
		sprintf(lbuff, "/tmp/ed_%u.%u", getgid(), getuid() );
		file = lbuff;
		}

	if(!(ed_open(file, "r", 1)))
		return( NOTHING );

	file_type = 0;			/* Native posix file type	*/
	mark_line(curln = laddr);
	while((n = fgetline(ed_fp, lbuff, LINE_LENGTH)) > 0) {
		/* DOS file with ^z handling	*/
		if(file_type == 1  &&  (p = (struct line *)strchr(lbuff, 0x1a))) {
			forward(ed_fp);
			if((n = ((char *)p) - lbuff) == 0)
				break;
			lbuff[n++] = '\n';
			}

		c = lbuff[n - 1];			/* Examine last char on line	*/
		if(c != RS_  &&  c != LF_  &&  c != CR_ )
			++n;
		else {
			lbuff[n - 1] = '\0';
			if(c == CR_) {
				while((c = getc(ed_fp)) == CR_)
					;
				if(c == LF_  &&  (first == 0  || file_type == 1)) {
					file_type = 1;
					c = LF_;
					}
				else {
					ungetc(c, ed_fp);
					c = CR_;
					}
				first = 1;
				}
			else if(c == RS_) {
				if(first != 0  &&  file_type == 2) {
					lbuff[n - 1] = 0;
					c = LF_;
					}
				else {
					if(first == 0)
						file_type = 2;
					first = 1;
					c = LF_;
					}
				}
			else
				first = 1;
			}

		unbreakable();
		if((p = malloc(sizeof(struct line) - 1 + n)) == NULL) {
			ed_close();
			breakable();
			return( ERROR0 );
			}

		if(c == LF_)
			p->lflags = 0;
		else if(c == CR_)
			p->lflags = OVER_FLAG;
		else
			p->lflags = CONT_FLAG;
		
		p->lflags |= NEW_FLAG | DIRTY_FLAG;
		memcpy(p->textp, lbuff, n);
		if(bufnum) {
			tp = npurge_lns ? purge_end_ptr : (purge_end_ptr = purge_ptr);
			++npurge_lns;
			purge_end_ptr = p;
			}
		else {
			tp = getptr(curln);
			++curln;
			++lastln;
			if(curln == lastln)
				lastp = p;
			}
		relink(p, tp->next);
		relink(tp, p);
		breakable();
		}

	curln = imin(lastln, laddr + 1);

	ed_close();

	if(bufnum == 0)
		lp = "\n"; /* forces main to fetch a new line */

	if(errno) {
		putmsg(strerror(errno));
		return(NOTHING);
		}

	if (!S_ISREG(file_attrs)  ||  !(file_attrs & S_IWRITE))
		putmsg("File is read only. Press enter to continue.");

	return(OK);
	}



int _write(char *file, int from, int to, char append_flag, char bufnum)
	{
	register struct line *bp;
	register int laddr = from;
	int n, status;

	if(strcmp(file, "@") == 0) {
		sprintf(lbuff, "/tmp/ed_%u.%u", getgid(), getuid() );
		file = lbuff;
		}

	if(!(ed_open(file, append_flag ? "a" : "w", 0)))
		return( NOTHING );

	status = OK;
/*	Exc_pending[0] = 0;	@@@	*/

/*	for(bp = bufnum ? purge_ptr->next : getptr(laddr) ; Exc_pending[0] == 0  &&  laddr <= to ; ++laddr) {	@@@	*/
	for(bp = bufnum ? purge_ptr->next : getptr(laddr) ; laddr <= to ; ++laddr) {
		n = strlen(bp->textp);
		if(fput(bp->textp, n, ed_fp) != n) {
			status = ERROR14;
			break;
			}
		if((bp->lflags & CONT_FLAG) == 0) {
			if(bp->lflags & OVER_FLAG)
				n = fput("\r", 1, ed_fp) == 1;
			else if(file_type == 1)
				n = fput("\r\012", 2, ed_fp) == 2;
			else if(file_type == 2)
				n = fput("\036", 1, ed_fp) == 1;
			else
				n = fput("\n", 1, ed_fp) == 1;
			if(n == 0) {
				status = ERROR14;
				break;
				}
			}

		bp = bp->next;
		}

	ed_close();

	return( status);
	}



int substitute(char *subtxt, char under_glob, int occurance)
	{
	register char *bol, *pp, *lastpp;
	register char *rp, *p;
	register struct line *bp;
	register int laddr, nmatches, status, index;

	nsubs = 0;
	if(laddr1 < 0)
		return(ERROR5);

	status = ERROR13;
	cc_reg = FALSE;
	index = 0;

	for(bp = getptr(laddr = laddr1) ; laddr <= laddr2 ; ++laddr) {
		bol = p = bp->textp;
		bp = bp->next;
		rp = &rbuff[0];
		nmatches = 0;
		lastpp = NULL;
		do {
			if(occurance==0 || nmatches<occurance)
				pp = amatch(p, bol, pattern);
			else
				pp = NULL;
			if(pp && lastpp != pp) {
				++nmatches;
				if(occurance == 0 || nmatches == occurance) {
					rp = subst_txt(rp,p,pp,subtxt);
					index = (opt.opt_a ? p : pp) - bol;
					}
				else
					rp = subst_same(rp, p, pp);
				lastpp = pp;
				}
			if(pp==NULL || p==pp)
				*rp++ = *p;
			else
				p = pp - 1;
			} while(*p++ != '\0'  &&  rp <= &rbuff[LINE_LENGTH]);
		nsubs += nmatches;
		if(nmatches != 0  &&  nmatches >= occurance) {
			if(rp > &rbuff[LINE_LENGTH+1])
				return( ERROR12 );
			*rp = '\0';

			replace(laddr, rbuff, bol);
			if(laddr) {
				curin = index;
                curcol = 0;
                }
			else
				cmdin = index;

			status = OK;
			cc_reg = TRUE;
			}
		}
	if(under_glob)
		return( OK );
	return( status );
	}



int getsubst_str( char *buff)
	{
	register char *p;
	char delim;

	p = buff;
	if(*lp == '\0'  ||  *(lp+1) == '\0')
		return( ERROR9 );
	for(delim = *lp++; *lp != delim && *lp != '\0' && p < &buff[PAT_SIZE];)
		if((*p++ = *lp++) == '\\')
			*p++ = *lp++;
	if(*lp != delim)
		return( ERROR9 );
	*p = '\0';
	++lp;
	return( OK );
	}


char *subst_txt(char *rp, char *fromp, char *top, char *subtxt)
	{
	register char *p1, *p2;

	for(p1 = subtxt ; *p1 != '\0' && rp <= &rbuff[LINE_LENGTH];)
		if(*p1 == '&'  &&  opt.opt_m) {
			for(p2 = fromp ; p2 < top && rp <= &rbuff[LINE_LENGTH];)
				*rp++ = *p2++;
			++p1;
			}
		else
			if((*rp++ = *p1++) == '\\')
				p1 = esc_char(p1, rp - 1);
	return( rp );
	}


char *subst_same( char *_rp, char *_pfrom, char *pto)
	{
	register char *rp, *pfrom;

	rp = _rp;
	pfrom = _pfrom;

	while(pfrom < pto  &&  rp <= &rbuff[LINE_LENGTH])
		*rp++ = *pfrom++;
	return( rp );
	}



int join(int laddr)
	{
	register struct line *p;
	register int n, m;
	char temp_flags;

	if(laddr <= 0)
		return( ERROR5 );
	if(laddr == lastln) {
		cc_reg = FALSE;
		return(OK);
		}
	p = getptr(laddr);
	if(((m = strlen(p->textp)) + strlen(p->next->textp)) > LINE_LENGTH)
		return( ERROR12 );
	strcpy(rbuff, p->textp);
	strcpy(rbuff + m, p->next->textp);
	temp_flags = p->next->lflags;
	delete(laddr, n = nextln(laddr), NOSAVE);
	if(addline(rbuff, 1, p->lflags | temp_flags) != OK)
		return(ERROR0);
/*
 * Code added to help display handler figure out a fast way using line ins/del
 * of updating the screen.
 */
	if( num_delete_lines == 0  ||  n < delete_line ) {
		delete_line = n;
		num_delete_lines = 1;
		}
	cc_reg = TRUE;
	return( OK );
	}



int exec_file(char *file, int under_glob, int under_until)
	{
	register int status;

	if(x_fp)
		return(ERROR6);

	unbreakable();
	if((x_fp = fopen(file, "r")) == NULL) {
		breakable();
		return(ERROR8);
		}

	breakable();
	status = OK;
	while(status != EOF  &&  fgetline(x_fp, lp = lbuff, LINE_LENGTH) > 0)
		status = exec_line(under_glob, under_until);

	unbreakable();
    fclose(x_fp);
	x_fp = 0;
	breakable();

	lp = "\n";	/* to force a fetch of a new line */
	return(OK);
	}



int yut() {
	register char *p1 = lp, *p2 = rbuff;
	int n, c, flag;

	if(*p1++ != '"')
		return(ERROR9);

	while(*p1 != '"')
		if((*p2++ = *p1++) == '\0')
			return(ERROR9);

	++p1;
	*p2++ = '\0';

	putmsg(rbuff);

	if(msg_char == 0x1b)
		return(ERROR15);

	dummy();
	flag = n = 0;
	while(*p1  &&  *p1 != '"') {
		if(flag == 0)
			++n;
		if(*p1++ == msg_char)
			flag = 1;
		}

	while(n > 1)
		if((c = x_fp ? getc(x_fp) : mgetchar()) == '\n'  ||  c == '\r'  ||  c == EOF)
			--n;

	lp = "\n";
	return(OK);
	}
