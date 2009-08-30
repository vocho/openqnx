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





/*-
 *
 * fregex.c:	Fixed Expression Matching Library.
 *
 * Description: fregex provides routines for high-speed matching of keywords
 * within strings.  The routines build tables out of the keywords, which
 * exploit any similiarity between the strings being searched for.  Even if
 * such similiarity is limited to the last matched character matching the
 * first char of another string, it prevents the need to rescan the original,
 * thus the processing is single pass.
 *
 * The calling convention is modelled after the "regex" library defined by
 * POSIX.  This is simply for consistency.
 *
 *
 * The basic algorithms were derived from "the dragon book" by Aho, Sethi &
 * Ullman, (Exercises 3.31 & 3.32 in particular).
 *
 *
 * These are elements from the fegex_t structure:
 * fr_dtrans fr_maxst fr_nst fr_fstate - 
 *	dtrans is the transition table, which has fr_nst entries.
 *	In state N with input C, the next state in fd_dtrans[N][C].
 * 	fr_maxst entries have been allocated, and fr_fstate is
 * 	the first, or start state in the table.
 * fr_fail -
 *	the state to move to when a failure occurs in the transition
 *	table.
 * fr_final -
 *	set of states designated as final, or accepting.
 *
 * fr_which fr_nstrings -
 *	an index, corresponding to the initial set of strings,
 *	which indicates the 'final' state for each string.Useful
 *	for later determining which string matched.
 *
 *
 * Steve.
 *
 */

#include	<stdio.h>
#include	<ctype.h>
#ifdef __STDC__
#include	<stdlib.h>
#include	<string.h>
#endif

#include	"fregex.h"

#include	<setjmp.h>



#ifndef INITIAL
#define	INITIAL 0
#endif


#ifndef DEBUGGING
#define DEBUGGING 0
#endif

#define debuglevel DEBUGGING

#if debuglevel
void
print_bvect(FILE *f, char *prompt, BitVect p, int n)
{
	int	i;
	fputs(prompt,f);
	for (i=0; i < n; i++) {
		putc(INSET(p,i) ? '1' : '0', f);
	}
	putc('\n',f);
}

void
print_line(FILE *f, char *prompt, int *fline, int n)
{
	fputs(prompt,f);
	while (n--) {
		fprintf(f,"%2d, ",*fline++);
	}
	putc('\n',f);
}

void
print_dtrans(FILE *f, fregex_t *F)
{
	int	i;
	for (i=0; i < F->fr_nst; i++) {
		fprintf(f,"[%2d] ",i);
		print_line(f, "", F->fr_dtrans[i], F->fr_nchars);
	}
}
#endif


/*
 primative error handling -- if anything goes wrong, we longjmp back.
*/
static char     *fr_emsg = "unknown error\n";
static jmp_buf   fr_recover;
#define	frerr_init()	(setjmp(fr_recover))

static int fr_matchexact(fregex_t *, char *, int );

static void
fr_error(s)
	char           *s;
{
	fr_emsg = s;
	longjmp(fr_recover, 1);
}



static int
isempty(BitVect v, int l)
{
	int             r = BIT_LEN(l);
	while (r--) {
		if (*v++)
			return 0;
	}
	return 1;
}

/*
 * what is the transition from state s on input a
 */
#define Dtrans(F,s,a)	((F)->fr_dtrans[(s)][(a)])


static int
alloc_st(fregex_t *F, int def)
{
	int             i;

	if ((i = F->fr_nst) >= F->fr_maxst) {
		long		newsize;
		newsize = ((long)F->fr_maxst+ST_INCR) * sizeof(int *);
		if (newsize > INT_MAX) {
			fr_error("alloc_st(too big)\n");
		}
		if ((F->fr_dtrans = realloc(F->fr_dtrans, newsize)) == 0) {
			fr_error("alloc_st(no memory)\n");
		}
		if ((F->fr_final = realloc(F->fr_final,
			BIT_LEN(F->fr_maxst + ST_INCR))) == 0) {
			fr_error("alloc_st(no memory)\n");
		}
		/*
		 * set the values as if calloc'd.
		 */
		for (i=0; i < ST_INCR; i++) {
			F->fr_dtrans[F->fr_maxst+i] = 0;
			DELSET(F->fr_final, F->fr_maxst+i);
		}
		F->fr_maxst += i;
	}
	F->fr_dtrans[F->fr_nst] = malloc(F->fr_nchars * sizeof(int));
	for (i=0; i < F->fr_nchars; i++) {
		Dtrans(F, F->fr_nst, i) = def;
	}
	return F->fr_nst++;
}
/*-
 * followset
 *     Generate a set containing the states reachable from this state.
 * -note that the INITIAL state is not considered 'reachable'.
 */

static void
followset(fregex_t *F, BitVect prev, BitVect next)
{
	int	st;
	int	a;
	clearset(next, F->fr_nst + 1);
	for (st = 0; st < F->fr_nst; st++) {
		if (!INSET(prev, st)) {
			continue;
		}
		for (a=0; a < F->fr_nchars; a++) {
			int t = Dtrans(F, st, a);
			if (t != FAILED && t != INITIAL) {
				ADDSET(next,t);
			}
		}
	}
}

/*-
 * build_fail builds a failure function for the transition table in F.
 *
 * The failure function encodes what state you must enter when a mismatch is
 * encountered.   We examine the dfa by a breadth-first traversal, starting
 * at depth 1 and continuing until all paths are visited.  Each state
 * corresponds to the prefix of one or more strings.  At each state, the
 * failure function defines the longest suffix recognized which is a prefix
 * of some other string(s).
 *
 */

static int      *
build_fail(F)
	register fregex_t *F;
{
	register int   *fail;
	BitVect         prev, next;
	//int             i;
	int             sd;

	if ((fail = calloc(sizeof(int), F->fr_nst + 1)) == NULL) {
		return NULL;
	}
	if ((prev = calloc(sizeof(BitEl), 2 * BIT_LEN(F->fr_nst + 1))) == NULL) {
		free(fail);
		return NULL;
	}
	fail++;
	next = prev + BIT_LEN(F->fr_nst + 1);

	ADDSET(prev, INITIAL);/* just to start us off */

	while (!isempty(prev, F->fr_nst)) {
#if debuglevel
		if (debuglevel) {
			print_bvect(stderr, "prev=", prev, F->fr_nst);
		}
#endif
		followset(F, prev, next);
		/*
		 * for each state reachable, calculate the failure function
		 */
		for (sd = 0; sd < F->fr_nst; sd++) {
			int             c;
			if (!INSET(next, sd))
				continue;
#if debuglevel
			if (debuglevel) fprintf(stderr, "select %d\n", sd);
#endif
			for (c=0; c < F->fr_nchars; c++) {
				int             s, snew, r;
				s = fail[sd];
				snew = Dtrans(F, sd, c);
				r = Dtrans(F, s, c);
				while ((r=Dtrans(F,s,c)) == FAILED) {
					s = fail[s];
				}
#if debuglevel
				if (debuglevel > 1)
					fprintf(stderr, "Fail[%d]=%d\n",snew,r);
#endif
				fail[snew] = r;
			}
		}
		memcpy(prev, next, BIT_LEN(F->fr_nst + 1));
	}
	free(prev);
	return fail;
}

/*-
 * fregex incremental compiller -- allows user to build fregex without
 * remembering strings.
 */

int 
fregadd(fregex_t *feg, char *string)
{
	int             st;
	int             r;
	char           *s;

	if (frerr_init()) {	/* handle all errors... */
#if debuglevel
		if (debuglevel) fprintf(stderr, "fregadd:%s\n", fr_emsg);
#endif
		fregfree(feg);
		return 1;
	}

	st = INITIAL;
	s = string;

	while (*s) {
		int c = feg->fr_equiv[(int)*s++];
		int	t;
		if ((t=Dtrans(feg, st, c)) == INITIAL || t == FAILED) {
			/*-
			 * alloc_st() can change fr_dtrans
			 */
			r = alloc_st(feg, FAILED);
			st = Dtrans(feg,st,c) = r;
		} else {
			st = t;
//		} else  {
//			if (debuglevel) {
//				fprintf(stderr,"would have assigned to -1\n");
//			}
		}
	}
	ADDSET(feg->fr_final, st);	/* mark as a F->fr_final state */

#if 0
	if ((feg->fr_cflags & REG_NOSUB) == 0) {
		void           *p;
		int             i;
		if ((i = feg->fr_nstrings++) == 0) {	/* one more string */
			p = calloc(sizeof(int), 2);
		} else {
			p = realloc(feg->fr_which, feg->fr_nstrings * 2 * sizeof(int));
		}
		if (p == NULL) {
			fregfree(feg);
			return 1;
		}
		feg->fr_which = p;
		feg->fr_which[i * 2] = st;	/* mark which string it's
						 * from */
		feg->fr_which[i * 2 + 1] = s - string;	/* and how long it was */
	}
#endif
	return 0;
}

/*-
mkmap:: generate a map, which transforms a character value onto a small
	integer or zero if the character is useless.
*/
static int
mkmap(fregex_t *F, char **pats)
{

	char	*s;
	//int	i;
	int	count = 1;

	if ((F->fr_equiv = calloc(UCHAR_MAX+1,sizeof(*F->fr_equiv))) == 0) {
		return -1;
	}
	while (*pats) {
		for (s=*pats++; *s; s++) {
			unsigned c = *s;
			if (F->fr_cflags & REG_ICASE) {
				if (F->fr_equiv[tolower(c)] == 0) {
					F->fr_equiv[tolower(c)] = ++count;
					F->fr_equiv[toupper(c)] = count;
				}
			} else if (F->fr_equiv[c] == 0) {
				F->fr_equiv[c] = count++;
			}
		}
	}
	return F->fr_nchars = count+1;
}

int
fregcomp(feg, pats, cflags)
	register fregex_t *feg;
	char          **pats;
	int             cflags;
{
	int             i;

	memset(feg,0,sizeof *feg);
	if (frerr_init()) {	/* handle all errors... */
#if debuglevel
		if (debuglevel) fprintf(stderr, "freginit:%s\n", fr_emsg);
#endif
		fregfree(feg);
		return 1;
	}

	feg->fr_cflags = cflags;
	mkmap(feg, pats);

/*
 * allocate initial state. The initial state will allways be zero, and
 * this might be relied upon.
 */
	feg->fr_fst = alloc_st(feg, 0);

	for (i = 0; pats[i]; i++) {
		if (fregadd(feg, pats[i]) != 0)
			return 1;
	}
#if debuglevel
	if (debuglevel) print_dtrans(stderr,feg);
#endif
	if ((feg->fr_fail = build_fail(feg)) == NULL) {
		fregfree(feg);
		return 1;
	}
#if debuglevel
	if (debuglevel) {
		print_line(stderr, "fail=", feg->fr_fail, feg->fr_nst);
		print_bvect(stderr,"final=", feg->fr_final, feg->fr_nst+1);
	}
#endif
	return 0;

}







int             last_match = 0;

int
fregexec(feg, string, nmatch, pmatch, eflags)
	register fregex_t *feg;
	register char  *string;
	int             nmatch;
	regmatch_t     *pmatch;
	int             eflags;
{
	int             st;
	int             c;
	int             r;

	if (feg->fr_cflags & FREG_EXACT) {
		return fr_matchexact(feg, string, eflags);
	}
	st = INITIAL;
	while (c = *string++) {
		int	a;
	/* qnx defaults to signed char so binary files cause c to be negative */
		if(c < 0)
			continue;
		a = feg->fr_equiv[c];
		while ((r=Dtrans(feg, st, a)) == FAILED) {
			st = feg->fr_fail[st];
		}
		st = r;
		if (INSET(feg->fr_final, st))
			return 0;
	}
	return 1;
}

static int
fr_matchexact(fregex_t *feg, char *string, int eflags)
{
	int	c;
	int	st;
	st = INITIAL;
	while (c = *string++) {
		int	a;
		if(c < 0)
			continue;
		a = feg->fr_equiv[c];
		if ((st=Dtrans(feg, st, a)) == FAILED || st == INITIAL) {
			return 1;
		}
	}
	return INSET(feg->fr_final, st) ? 0 : 1;
}

#if 0
	if (nmatch) {	/* figure out which one we got */
		for (r = 0; r < feg->fr_nstrings; r++) {
			if (feg->fr_which[r * 2] == st) {
				break;
			}
		}
		if (r < feg->fr_nstrings) {
			last_match = r;
			pmatch->rm_ep = string;
			pmatch->rm_sp = string - feg->fr_which[r * 2 + 1] + 1;
		}
	}
#endif


void
fregfree(feg)
	fregex_t       *feg;
{
	int             i;
	if (feg->fr_dtrans) {
		for (i = 0; i < feg->fr_nst; i++) {
			if (feg->fr_dtrans[i])
				free(feg->fr_dtrans[i]);
		}
		free(feg->fr_dtrans);
	}
	if (feg->fr_equiv)
		free(feg->fr_equiv);
	feg->fr_dtrans = NULL;
	if (feg->fr_fail)
		free(feg->fr_fail);
	feg->fr_fail = NULL;
	if (feg->fr_final)
		free(feg->fr_final);
	feg->fr_final = NULL;
	if (feg->fr_which)
		free(feg->fr_which);
	feg->fr_which = NULL;
	return;
}
