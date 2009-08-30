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





/* expr(1)  --  author Erik Baalbergen */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <regex.h>

#define REGEX_WORKS

#ifdef __QNXNTO__
/* to cover differences in regmatch_t members between qnx4 and nto */
#define rm_sp rm_so
#define rm_ep rm_eo
#endif

/* expr accepts the following grammar:
	expr ::= primary | primary operator expr ;
	primary ::= '(' expr ')' | signed-integer ;
  where the priority of the operators is taken care of.
  The resulting value is printed on stdout.
  Note that the ":"-operator is not implemented.
*/

enum
{
	// we return 0 if the expr didn't evaluated to 0 or null
	// we return 1 if the expr evaluated to 0 or null
	// ...go figure, huh?
	EXPR_TRUE 		= 0,
	EXPR_FALSE 		= 1, 
	EXPR_INVALID	 	= 2, // the expr was invalid
	EXPR_OTHER_ERROR	= 3  // some other error occured
};

enum token
{
	EOI	=	 0,
	OR	=	 1,
	AND	=	 2,
	LT	=	 3,
	LE	=	 4,
	EQ	=	 5,
	NE	=	 6,
	GE	=	 7,
	GT	=	 8,
	PLUS	=	 9,
	MINUS	=	10,
	TIMES	=	11,
	DIV	=	12,
	MOD	=	13,
	COLON	=	14,
	LPAREN	=	15,
	RPAREN	=	16,
	OPERAND	=	20
};

#define MAXPRIO	6

char *expr(int, int);
char *eval(char *, int, char *);
char *tostr(long);

struct op 
{
	char *op_text;
	short op_num, op_prio;
} ops[] = 
	{
		{"|",	OR,	6},
		{"&",	AND,	5},
		{"<",	LT,	4},
		{"<=",	LE,	4},
		{"=",	EQ,	4},
		{"!=",	NE,	4},
		{">=",	GE,	4},
		{">",	GT,	4},
		{"+",	PLUS,	3},
		{"-",	MINUS,	3},
		{"*",	TIMES,	2},
		{"/",	DIV,	2},
		{"%",	MOD,	2},
		{":",	COLON,	1},
		{"(",	LPAREN,	-1},
		{")",	RPAREN,	-1},
		{0, 0, 0}
	} ;

char **ip;
struct op *ip_op;
int  stringFLAG;

char *tostr(long l) 
{
	static char buf[26];
	char *p;

	ltoa (l, buf, 10);
	p = strdup (buf);
	if (p == 0)
	{
		fprintf (stderr, "expr error: out of memory.\n");
		exit (EXPR_OTHER_ERROR);	
	}
	return p;
}


enum token lex(char *s) 
{
	struct op *op = ops;

	if (s == 0) {
		ip_op = 0;
		return EOI;
		}
	while (op->op_text) {
		if (strcmp(s, op->op_text) == 0) {
			ip_op = op;
			return op->op_num;
			}
		op++;
		}
	ip_op = 0;
	return OPERAND;
}

void syntax() 
{
	fprintf (stderr, "expr error: invalid syntax\n");
	exit(EXPR_INVALID);
}

long num(char *s)
{
	long l = 0;
	long sign = 1;

	if (*s == '\0')
		syntax();
	if (*s == '-') {
		sign = -1;
		s++;
		}
	while (*s >= '0' && *s <= '9')
		l = l * 10 + *s++ - '0';
	if (*s != '\0')
		syntax();
	return sign * l;
}

char *expr(int n, int prio) 
{
	char *res = ""; // to shutup the compiler

	if (n == EOI)
		syntax();
	if (n == LPAREN) {
		if (prio == 0) {
			res = expr(lex(*++ip), MAXPRIO);
			if (lex(*++ip) != RPAREN)
				syntax();
			}
		else
			res = expr(n, prio - 1);
		}
	else if (n == OPERAND) 
		{
		if (prio == 0)
			return(*ip);
		res = expr(n, prio - 1);
		}
	else
		syntax();

	while ((n = lex(*++ip)) && ip_op && ip_op->op_prio == prio)
	{
		res = eval(res, n, expr(lex(*++ip), prio - 1));
#ifdef DEBUG
fprintf( stderr, "res: %s\n", res );
#endif
	}
	ip--;

	return res;
}


char *eval(char *s1, int op, char *s2) 
{
	regex_t re;
	regmatch_t rem[2];
	long l, l1, l2;
	char *end1, *end2;
	char  *buf;
	size_t buflen;
	
	l1 = strtol( s1, &end1, 0 );
	l2 = strtol( s2, &end2, 0 );

	// *end1 is not null if s1 is a string then, the same applies to end2
	// *s1 is null if s1 is the null string, the same applies to s2
	if( *end1 || *end2 || (*s1 == 0) || (*s2 == 0) || (op == COLON))
		{
#ifdef DEBUG
fprintf( stderr, "op:%d strings 1:%s  2:%s\n", op, s1, s2 );
#endif
		switch (op) {
			case OR:
				return (*s1&&!(!*end1 && l1==0))?s1:s2;

			case AND:
				return ((*s1&&!(!*end1 && l1==0)) &&
		                        (*s2&&!(!*end2 && l2==0)))?s1:"0";
			case PLUS:
			case MINUS:
			case TIMES:
			case DIV:
			case MOD:
				fprintf (stderr, "expr error: invalid string operator\n");
				exit (EXPR_INVALID);
			case LT:return((strcmp(s1,s2) <  0) ? "1" : "0");
			case LE:return((strcmp(s1,s2) <= 0) ? "1" : "0");
			case EQ:return((strcmp(s1,s2) == 0) ? "1" : "0");
			case NE:return((strcmp(s1,s2) != 0) ? "1" : "0");
			case GE:return((strcmp(s1,s2) >= 0) ? "1" : "0");
			case GT:return((strcmp(s1,s2) >  0) ? "1" : "0");
			case COLON:
				l = 0;
				memset (&rem, 0, sizeof (rem));
				rem[1].rm_so = -1; // assume we don't match any sub-expr
				buflen = strlen(s1) + 256;
				if ((buf = calloc(buflen, 1)) == NULL) {
					perror("malloc");
					exit(EXIT_FAILURE);
				}
				if (*s2)
				{
					char *p;
					if (*s2 == '^')
					{
						// warn about lack of portability since patterns are 
						// supposed to be anchored at the beginning of a string
						// according to Posix.2
						fprintf (stderr, "expr warning: using '^' to anchor pattern matching is not portable. !%s!%s!\n", s1, s2);
					}
					if ((p = malloc(strlen(s2 + 2))) == NULL) {
						free(buf);
						perror("malloc");
						exit(EXIT_FAILURE);
					}
					*p = '^';
					strcpy(p + 1, s2);
					l = regcomp( &re, p, 0);
					free(p);
					if (l != 0)
					{
						regerror (l, &re, buf, buflen);
						fprintf (stderr, "expr regex error: %s\n", buf);
						free(buf);
						exit (EXPR_INVALID);
					}
					l = (long) regexec( &re, s1, 2, &rem[0], 0 );
					regfree( &re );
				}
				else
				{	// force a "" string match on an empty pattern; regexec won't
					rem[0].rm_eo = rem[0].rm_so = 0;	
				}

#ifdef DEBUG
fprintf( stderr, ":op L=%ld rem[0].rm_sp=%x rem[0].rm_ep=%x rem[1].rm_sp=%x rem[1].rm_ep=%x\n", l, rem[0].rm_sp, rem[0].rm_ep, rem[1].rm_sp, rem[1].rm_ep );
#endif
				if (l && (l != REG_NOMATCH))
				{
					regerror (l, &re, buf, buflen);
					fprintf (stderr, "expr regex error: %s\n", buf);
					free(buf);
					exit (EXPR_OTHER_ERROR);
				}
				if (rem[1].rm_so == -1)
				{
					// if we didn't match a sub-expression, then return the
					// length of whatever we did match
					sprintf (buf, "%d", rem[0].rm_eo - rem[0].rm_so);
				}
				else
				{
					int length = min (buflen - 1, rem[1].rm_eo - rem[1].rm_so);
					// return the string that was matched
					strncpy (buf, s1 + rem[1].rm_so, length);
					buf[length] = '\0';
				}
				return (buf);
	
			default:
				fprintf (stderr, "expr error: invalid string operator.\n");
				exit (EXPR_INVALID);
			}
		}
	else /* else if( integer ) */
		{
#ifdef DEBUG
fprintf( stderr, "op:%d integers 1:%ld:%s  2:%ld:%s\n", op, l1, s1, l2, s2 );
#endif
		switch (op) 
		{
			case OR:	return((l1) ? s1 : s2);
			case AND:	return((l1 && l2) ? s1 : "0");
			case LT:	return((l1 < l2) ? "1" : "0");
			case LE:	return((l1 <= l2) ? "1" : "0");
			case EQ:	return((l1 == l2) ? "1" : "0");
			case NE:	return((l1 != l2) ? "1" : "0");
			case GE:	return((l1 >= l2) ? "1" : "0");
			case GT:	return((l1 > l2) ? "1" : "0");
			case PLUS:	return(tostr(l1 + l2));
			case MINUS:	return(tostr(l1 - l2));
			case TIMES:	return(tostr(l1 * l2));
			case DIV:
				if (l2 == 0)
				{
					fprintf (stderr, "expr error: attempted division by zero.\n");
					exit (EXPR_OTHER_ERROR);
				}
				return(tostr(l1 / l2));
			case MOD:
				if (l2 == 0)
				{
					fprintf (stderr, "expr error: attempted modulus by zero.\n");
					exit (EXPR_OTHER_ERROR);
				}
				return(tostr(l1 % l2));
			default:
				fprintf (stderr, "expr error: invalid integer operator.\n");
				exit (EXPR_INVALID);
			}
		}

	exit(EXPR_INVALID);
}

int 
main(int argc, char *argv[]) 
{
	char *res, *endptr;
	long result;
	enum token token;

	if (argv[1] != NULL && argv[2] != NULL && strcmp(argv[1], "--") == 0) {
		token = lex(argv[2]);
		switch (token) {
		case OPERAND:
		case LPAREN:
		case RPAREN:
			ip = &argv[2];
			break;
		default:
			ip = &argv[1];
			break;
		}
	}
	else {
		ip = &argv[1];
	}

	res = expr(lex(*ip), MAXPRIO);
	if (*++ip != 0)
		syntax();
	printf("%s\n", res);

	// the return value is 1 if the result is
	// either "" or 0; so convert the result to 	
	// a number. if the number is 0 *and* there 
	// are no trailing letters, then the result
	// is 0 or null. we need to check if there
	// are trailing numbers because "0abc" gets
	// converted to a 0, but it should be counted
	// as a string
	result = strtol (res, &endptr, 10);
	if ((result == 0) && (*endptr == 0))
		return EXPR_FALSE;
	return EXPR_TRUE;
}



