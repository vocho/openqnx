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




/*				26-Feb-90 10:33:30am										*/

/*--------------------------------------------------------------------------*/
/*  History: tparm.c, V0.0, 21-Feb-90  8:56:19pm, Dan Hildebrand, Baseline	*/
/*----------------------------------------------------------------------------

     Function  to  substitute  parameters  into  termio  strings using the
     termio RPN language.

@@@ This code does not handle pointers pass in large model, since it
	assumes that sizeof(int) = sizeof(char *) so that it can push and
	pop arguments on the stack without concern for their type. A change
	needs to be made to force all data types to 32 bit to accomodate this.

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/term.h>

#define	MAX_DEPTH	20
#define	BOTTOM		3

/*
     These macros can only be used in an expression  where  the  order  of
     pre/post increment  is  not important. ie: Don't use more than one of
     these per expression where order of evaluation is important.

*/
#define	pop()		*(tos--)
#define	push( i )	*(++tos) = i

static	char	lbuf[100];

#ifdef __STDC__
char *__tparm( char *fmt, ... )
#else
char *__tparm( fmt )
	char *fmt;
#endif
	{
	va_list ap;
	register char *s = fmt;
	int		stack[ MAX_DEPTH ];
	char	pbuf[30];
	register char *d = lbuf, *p;
	register int i, j=0, k, ch, p1, p2, conditional;
	register int *tos = &stack[ BOTTOM ];

	va_start( ap, fmt );
	/* At least two arguments for the high-runner case	*/
	p1 = va_arg( ap, int );
	p2 = va_arg( ap, int );	/* Garbage if only 1 arg passed	*/

	*d = '\0';
	conditional = 0;
	while( *s ) {
#if 0	/* Let tputs() deal with these characters	*/
		if ( *s == '$' ) {		/* Strip padding information from strings	*/
			s += 2;				/* Skip over '$<' characters				*/
			i = 0;
			while( *s >= '0' && *s <= '9' ) i = i*10 + (*s++ - '0');
			++s;					/* Skip over the '>'		*/
			if ( !no_pad_char && pad_char )
				while ( i-- ) *d++ = *pad_char;
			continue;
			}
#endif
		if ( *s != '%' ) {
			*d++ = *s++;
			continue;
			}
		++s;						/* Found a '%', skip over it	*/
     	ch = (int) *s++;
		switch( ch ) {
			case '%':				/* A real '%' character			*/
				*d++ = '%';
				continue;
			case 'd':				/* A frequent case, don't use sprintf	*/
				i = pop();
				k = 0;
				if ( i >= 1000) {
					*d++ = (char)((j=i/1000) + '0');
					i-=j*1000;
					k = 1;			/* Flag to indicate a char was output	*/
					}               /*  and that following '0's are needed.	*/
				if ( i >= 100 ) {
					*d++ = (char)((j=i/100 ) + '0');
					i-=j*100;
					k = 1;
					}
				else if ( k ) *d++ = '0';
				if ( i >= 10  ) {
					*d++ = (char)((j=i/10  ) + '0');
					i-=j*10;
					}
				else if ( k ) *d++ = '0';
				*d++ = (char)(i + '0');
/*		This is the sprintf equivalent to the above if's
				d += sprintf( d, "%d", pop() );
*/
				continue;

			case ':':			/* A colon preceding the '-' or '+' format	*/
				if ( !(*s == '-'  ||  *s == '+') ) continue;
			case '#':				/* printf flags [-+# ] get us to here	*/
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				p = pbuf;
				*p++ = '%';
				if ( ch != ':' ) *p++ = *(s-1);
				/* Copy %[[:]flags[width[.precision]][doxXs] into printf buf*/
				while( *s  &&	*s != 'd'  &&  *s != 'o'  &&  *s != 'x'  &&
								*s != 'X'  &&  *s != 's' )
					*p++ = *s++;
				*p++ = *s++;		/* Put the last character into place	*/
				*p = '\0';
				d += sprintf( d, pbuf, pop() );
				continue;

			case 'c':	*d++ = (char) pop(); continue;	/* Pop char off stack*/

			case 'p':				/* Push parameter onto stack			*/
				i = *s - '0';
				/* Optimize most frequent case	*/
				if ( i == 1 ) { push( p1 ); ++s; continue;	}
				if ( i == 2 ) { push( p2 ); ++s; continue;	}
				va_start( ap, fmt );
				while( i-- ) j = (int) va_arg( ap, int );
				va_end( ap );
				push( j );
				++s;				/* Skip over parameter number			*/
				continue;

			case 'P':				/* Set variable a-z to pop()			*/
				i = *s - 'a';
				if ( i >= 0  &&  i < 26 )	__cur_term->_regs[ i ] = pop();
				++s;
				continue;

			case 'g':				/* Get variable a-z and push()			*/
				i = *s - 'a';
				if ( i >= 0  &&  i < 26 )	push( __cur_term->_regs[ i ] );
				++s;
				continue;

			case '\'':				/* Quoted character						*/
				push( (int) *s );
				s += 2;				/* Skip over trailing quote				*/
				continue;

			case '{':				/* Push decimal constant				*/
				i = 0;
				while( *s >= '0'  &&  *s <= '9' ) i = i*10 + (*s++ - '0');
				++s;				/* Skip over the '}'					*/
				push( i );
				continue;

			case 'l':	*tos = strlen( (char *) *tos );		continue;
			case '+':	--tos; *tos += *(tos+1);			continue;
			case '-':	--tos; *tos -= *(tos+1);			continue;
			case '*':	--tos; *tos *= *(tos+1);			continue;
			case '/':	if ( !(i=pop())) i = 1; *tos /= i;	continue;
			case 'm':	if ( !(i=pop())) i = 1; *tos %= i;	continue;
			case '&':	--tos; *tos &= *(tos+1);			continue;
			case '|':	--tos; *tos |= *(tos+1);			continue;
			case '^':	--tos; *tos ^= *(tos+1);			continue;
			case '=':	--tos; *tos = *tos == *(tos+1);		continue;
			case '>':	--tos; *tos = *tos > *(tos+1);		continue;
			case '<':	--tos; *tos = *tos < *(tos+1);		continue;
			case 'A':	--tos; *tos = *tos && *(tos+1);		continue;
			case 'O':	--tos; *tos = *tos || *(tos+1);		continue;
			case '!':	*tos = !*tos;						continue;
			case '~':	*tos = ~*tos;						continue;
			case 'i':	++p1; ++p2;							continue;
			case '?':										continue;
			case 't':	if ( pop() )
							conditional = 1;
						else {
							/* Skip over then until the else or end clause	*/
							while(	  *s  &&
									!(*s == '%'  &&  (*(s+1) == 'e') ||
													 (*(s+1) == ';' ) ) ) ++s;
							conditional = 0;
							}
						continue;
			case 'e':	if ( conditional ) {
							/* Then clause was done, skip over the else	*/
							while( *s  &&  !(*s == '%' && *(s+1) == ';') ) ++s;
							conditional = 0;
							}
						continue;
			case ';':	conditional = 0;					continue;
			}
		}
	*d = '\0';
	return( lbuf );
	}

char *__tgoto( cap, col, row )
	char *cap;
	int col, row;
	{
	static char addr[ 15 ];

	if ( cap == cursor_address ) {
		if ( __cur_term->_cup_type == _VT52_CUP ) {
			addr[0] = 27;
			addr[1] = 'Y';
			addr[2] = (char) (row + ' ');
			addr[3] = (char) (col + ' ');
			addr[4] = '\0';
			return( &addr[0] );
			}
		if ( __cur_term->_cup_type == _VT100_CUP ) {
			sprintf( addr, "\033[%d;%dH", ++row, ++col );
			return( &addr[0] );
			}
		}
	return( __tparm( cap, row, col ) );
	}
