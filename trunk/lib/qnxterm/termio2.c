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
/*  History: termio.c, V0.0, 21-Feb-90  8:56:19pm, Dan Hildebrand, Baseline	*/
/*----------------------------------------------------------------------------
dan: I added some possible changes marked as: jtl
----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <termios.h>
#include <sys/term.h>
#include <errno.h>

#ifndef __STDC__
char *calloc();
char *getenv();
#endif

#define ct	__cur_term

void __vidputs( attrs, putc )
	int attrs;
	int (*putc)();
	{
	putc = putc;						/* TEMP for compiler (until code works) */
	attrs = attrs;						/* shut up the compiler */
	}

void __vidattr( attrs )
	int attrs;
	{
	/* Use putchar()	*/
	attrs = attrs;						/* shut up the compiler */
	}

/* Termcap compatibility functions	*/
/* @@@ Macro candidate	*/
void __tgetent( bp, name )
	char *bp;			/* Unused	*/
	char *name;
	{
	bp = bp;							/* shut up the compiler */
	__setupterm( name, 1, (int *) NULL );
	}

/* @@@ Macro candidate	*/
int __tgetflag( codename )
	int codename;
	{
	return( codename );
	}

/* @@@ Macro candidate	*/
int __tgetnum( codename )
	int codename;
	{
	return( codename );
	}

char *__tgetstr( codename, area )
	register char *codename;
	char *area;
	{
	register char *p;

	p = area;
	strcpy( p, codename );
	p += strlen( codename );
	return( p + 1 );		/* Skip over the null byte	*/
	}

int  __baudrate() {
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	return( (int) tios.c_ospeed );
	}

void
__nodelay( win, mode )
	int win, mode;
	{
	win = 0;
	if ( mode ) {
		ct->_inpmode = _NODELAY;
		ct->_nonblock = 1;
		}
	else {
		ct->_inpmode = _DELAY;
		ct->_nonblock = 0;
		}
	}

#ifdef __NO_CURSES

/* @@@ Also pass through breaks and flow control characters	*/
int __raw() {
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_cc[VMIN]  =  1;
	tios.c_cc[VTIME] =  0;
	tios.c_lflag		&= ~(ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
	tios.c_oflag		&= ~(OPOST);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __unraw() {
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_lflag		|= (ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
#ifdef ONLCR
	tios.c_oflag		|= (OPOST|ONLCR);
#else
	tios.c_oflag		|= (OPOST);
#endif
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __cbreak() {							/* equivalent of noedit() */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_cc[VMIN]  =  1;
	tios.c_cc[VTIME] =  0;
	tios.c_lflag	&= ~(ICANON);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __nocbreak() {						/* equivalent of edit() */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	ct->_inpmode = _NODELAY;
	tios.c_lflag |= (ICANON);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __nl() {							/* enable post-processing of output */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

#ifdef ONLCR
	tios.c_oflag |= (OPOST|ONLCR);
#else
	tios.c_oflag |= (OPOST|);
#endif
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __nonl() {							/* disable post-processing of output */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_oflag &= ~(OPOST);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __echo() {							/* enable echo of input chars */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_lflag |= (ECHO);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

int __noecho() {							/* disable echo of input chars */
	struct termios tios;

	if ( tcgetattr( 0, &tios ) )
		return( -1 );

	tios.c_lflag &= ~(ECHO);
	return( tcsetattr( 0, TCSADRAIN, &tios ) );
	}

#endif
