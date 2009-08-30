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




/* input_line.c */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <termios.h>

#define TRUE		(1)
#define FALSE		(0)

#define KEY_CSI		0x009b
#define KEY_UP		'A'
#define KEY_DOWN	'B'

extern char *input_line();

static struct history *newBUF(struct history *);
static int sameCMD( char *s, char *s1 );

int		input_line_max = 20;

static struct history
	{
	struct history	*prev,
					*next;
	long			 time_stamp;
	unsigned int	 bytes;			/* size of writable memory area that
                                       h->string points to. Initialized to
                                       ZERO because string is pointing to
                                       "" which can be in the code segment. */
	char			*string;
	long fill;
	} *h;


char *input_line( fp, lbuf, bufsize )
FILE *fp;
unsigned char *lbuf;
int  bufsize;
/*
 *	Definition:	input a line with recall
 *
 *  Return:     The 'input' line
 */
	{
	register int	 i;
	register char	*p;
	int		 n;
	int		 updn = 0;
	struct history *new;
	static long last;

	if( h == NULL )
		{
		if( input_line_max < 3 )
			input_line_max = 3;
		if( (h = calloc(input_line_max, sizeof(struct history))) == NULL )
			{
			errno = ENOMEM;
			return( NULL );
			}
		for( i = 0; i < input_line_max; ++i )
			{
			(h+i)->prev = h + i - 1;
			(h+i)->next = h + i + 1;
			(h+i)->string = "";
			}
		h->prev = h+input_line_max-1;
		h->prev->next = h;
		}

	lbuf[0] = NULL;

	while( TRUE )
		{
		fflush( stdout );
		if( isatty( fileno( fp ) ) )
			n = read( fileno(fp), lbuf, bufsize );
		else
			n = (fgets( lbuf, bufsize, fp ) == NULL) ? 0 : strlen(lbuf);

		if( n < 1 )
			return( NULL );

		if( isatty(fileno(fp)) && lbuf[0] == KEY_CSI )
			{
			p = NULL;
			if( lbuf[1] == KEY_DOWN )
				p = (h = h->next)->string;
			else if( lbuf[1] == KEY_UP )
				{
				if( updn )
					h = h->prev;
				p = h->string;
				}
			if( p != NULL )
				{
				updn = 1;
				(void)tcinject( fileno(fp), p, strlen(p) );
				continue;
				}
			}

		if(lbuf[n-1] == '\n')			/* trim it */
			--n;
		lbuf[n] = 0;

		if( (lbuf[0] && lbuf[0] != '\f') || lbuf[1] )
			{
			if( strcmp( lbuf, h->string ) != 0 )
				{
				if( updn == 0 || (! sameCMD(h->string,lbuf)) )
					{
					new = newBUF( h );	/* get the oldest buffer */
					new->prev = h;
					new->next = h->next;
					h->next->prev = new;
					h->next = new;
					h = new;
					}
				if( (n+1) > h->bytes ) /* size of line+NULL >size prev malloc'ed? */
					{
					if( h->bytes )     /* if there was an area malloc'ed b4 */
						free( h->string );
					/* malloc a new area and set h->bytes to the malloc'ed size */
					h->string = malloc(h->bytes = (n + 1));
					}
				h->time_stamp = ++last;
				strcpy( h->string, lbuf );
				}
			}
		return( lbuf );
		}
	}


static struct history *newBUF( buf )
struct history *buf;
/*
 *	Definition:	Get the 'next' (oldest) buffer for input.
 *
 *  Return:     The buffer
 */
	{
	struct history		*t,
						*new;
	long	 			 ctr = 1999999999L;

	for( new = buf, t = buf->next; t != buf; t = t->next )
		{
		if( t->time_stamp < ctr )
			{
			ctr = t->time_stamp;
			new = t;
			}
		}

	new->prev->next = new->next;
	new->next->prev = new->prev;

	return( new );
	}


static int sameCMD( char *s, char *s1 ) {
	while( isspace(*s) )
		++s;
	while( isspace(*s1) )
		++s1;
	return( *s == *s1 );
	}

__SRCVERSION("inpline.c $Rev: 153052 $");
