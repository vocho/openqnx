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
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

int match(char *text, char *pat, char dir, int offset)
	{
	register char *tp, *p;

	tp = text + offset;

	if(dir == FORWARD)
		while(*tp++ != '\0'  ||  tp == text) {
			if(p = amatch(tp, text, pat))
				return((opt.opt_a ? tp : p) - text + 1);
			}
	else
		while(--tp >= text) {
			if(p = amatch(tp, text, pat))
				return((opt.opt_a ? tp : p) - text + 1);
			}

	return( 0 );
	}



char *amatch(char *text, char *bol, char *pat)
	{
	register char *patp;
	char *textp, *p, *tp;

	textp = text;
	p = NULL;
	for(patp = pat ; *patp != '\0' ; patp += patsize(patp))
		if(*patp == '*') {
			++patp;
			tp = textp;
			do
				if(!match_char(&tp, bol, patp))
					break;
			while(*tp != '\0');

			for(patp += patsize(patp) ; tp >= textp ; --tp)
				if(p = amatch(tp, bol, patp))
					break;

			return( p );
			}
		else
			if(!match_char(&textp, bol, patp))
				return(0);
	return(textp);
	}



int match_char(char **textpp, char *bol, char *patp)
	{
	char c, cc;
	int bump;
	register unsigned col;


	c = **textpp;
	bump = -1;
	switch( *patp ) {

	case '^':
		if(*textpp == bol)
			++bump;
		break;

	case '$':
		if(c == '\0')
			++bump;
		break;

	case '@':
		if((col = *++patp) == 255) {
			if((index_to_col(bol, *textpp - bol) & 0x3) == 1)
				++bump;
			break;
			}

		if(col == 0) {
			if(*textpp - bol == curin)
				++bump;
			}
		else if(index_to_col(bol, *textpp - bol) == col)
			++bump;
		break;

	case '.':
		if(c != '\0'  &&  c != '\n')
			bump = 1;
		break;

	case '[':
		if(c  &&  member_of(patp+1, c))
			bump = 1;
		break;

	case '\\':
		patp = esc_char(++patp, &cc);
		if(c == cc)
			bump = 1;
		break;


	default:
		if(comp(*patp, c))
			bump = 1;
		}

	if(bump >= 0) {
		*textpp += bump;
		return( 1 );
		}
	else
		return( 0 );
	}



int member_of(char *_p, char c)
	{
	register char *p;
	char found, notflg, n;

	p = _p;
	n = *p++ - 1;
	notflg = found = 0;
	if(*p == '^'  &&  n > 2) {
		notflg = 1;
		++p;
		--n;
		}

	while(--n)
		if(comp(*p++, c)) {
			found = 1;
			break;
			}

	return( found != notflg );
	}



int patsize(char *_p)
	{
	register char *p;


	p = _p;
	if(*p == '[')
		return(*(p+1));

	if(*p == '@')
		return(2);

	if(*p == '\\'  &&  *(p + 1) != '\n'  &&  *(p + 1))
		return(hex(*(p + 1)) < 16 ? 3 : 2);

	return( 1 );
	}



int getpat() {
	int status;

	if(*lp == '\0'  ||  *(lp+1) == '\0')
		return( ERROR3 );
	if(*lp == *(lp+1)) {
		++lp; /* use current pattern */
		status = OK;
		}
	else
		status = makepat(*lp++);
	if(pattern[0] == '\0'  ||  status <= ERROR)
		return( ERROR3 );
	return( OK );
	}



int makepat(char delim)
	{
	char c;
	register char *lastp;
	char *tp, *pp, *holdp;

	lastp = pp = pattern;
	for(tp=lp ; *tp!=delim && *tp!='\0' && pp < &pattern[PAT_SIZE] ; ++tp) {
		holdp = pp;
		switch( c = *tp ) {

		case '.':
			if(!opt.opt_m)
				c = '\\';
			break;

		case '^':
			if(tp != lp || !opt.opt_m)
				c = '\\';
			break;


		case '$':
			if(*(tp+1) != delim  ||  !opt.opt_m)
				c = '\\';
			break;

		case '\\':
			++tp;
			break;

		case '@':
			if(opt.opt_m && *(tp + 1) == '(') {
				*pp++ = '@';
				tp += 2;
				if(*tp == 't') {
					++tp;
					c = 0xff;
					goto chk;
					}

				if(*tp == '.')
					++tp;

				for(c = 0; *tp >= '0'  &&  *tp <= '9' ; ++tp)
					c = c*10 + (*tp - '0');
chk:
				if(*tp != ')')
					return( ERROR3 );
				}
			else
				c = '\\';
			break;

		case '[':
			if(opt.opt_m) {
				if(getccl(&tp, &pp) != OK)
					return( ERROR3 );
				lastp = holdp;
				continue;
				}
			else
				c = '\\';
			break;

		case '*':
			if(tp > lp  &&  opt.opt_m) {
				if(*lastp=='^' || *lastp=='$' || *lastp=='*' || *lastp=='@')
					return( ERROR3 );
				insert_clos(pp++, lastp);
				lastp = holdp;
				continue;
				}
			else
				c = '\\';
			break;
			}
		if((*pp++ = c) == '\\')
			*pp++ = *tp;
		lastp = holdp;
		}

	if(*tp != delim)
		return( ERROR3 );

	*pp = '\0';
	lp = tp;
	return( OK );
	}



int getccl(char **tpp, char **ppp)
	{
	register char *tp, *pp;
	char c1, c2;

	tp = *tpp;
	pp = *ppp;
	*pp++ = *tp++; /* save away '[' */
	++pp; /* leave room for count */
	while(*tp != ']'  &&  *tp != '\0' &&  pp < &pattern[PAT_SIZE]) {
		if((*pp++ = *tp++) == '\\')
			tp = esc_char(tp, pp - 1);
		if(*tp=='-' && *(tp+1)!='\0' && *(tp+1)!=']') {
			c1 = *(pp + -1);
			if(*++tp == '\\')
				tp = esc_char(++tp, &c2);
			else
				c2 = *tp;
			while( c1 <= c2 && pp < &pattern[PAT_SIZE]) {
				*pp++ = c1;
				++c1;
				}
			++tp;
			}
		}
	if(*tp != ']')
		return( ERROR3 );
	*(*ppp+1) = pp - *ppp;
	*tpp = tp;
	*ppp = pp;
	return( OK );
	}


void
insert_clos(char *pp, char *lastp)
	{
	register char *p;

	for(p = pp - 1 ; p >= lastp ; --p)
		*(p + 1) = *p;
	*lastp = '*';
	}



int comp(char c1, char c2)
	{

	if(!opt.opt_d) {
		c1 = (c1 >= 'A'  &&  c1 <= 'Z') ? c1 | ' ' : c1;
		c2 = (c2 >= 'A'  &&  c2 <= 'Z') ? c2 | ' ' : c2;
		}
	return(c1 == c2);
	}
