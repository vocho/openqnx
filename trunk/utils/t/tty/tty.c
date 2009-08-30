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





#ifdef __USAGE		/* tty.c (phase 3) */
%C - return users terminal name (POSIX)

%C	[-s]
Options:
 -s        Silent (do not write the terminal name to standard output).
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define EXIT_ERROR 2

/* 
 *	The tty utility writes to the standard output the name of the terminal
 *	attached to standard input.  The name that is used is equivalent to the 
 *	string that would be returned by the ttyname();
 *
 *	exit values : 0  standard input is a terminal
 *                1  standard input is not a terminal
 *	             >1  an error occured.
 */


int main(int argc, char *argv[])
{
	register int i;
	int  error,supress;
	char *name;
		
	error=supress=0;

	while(( i= getopt( argc, argv, "s")) != -1)
	{	switch (i) 	
		{	case 's': supress++;                 
					  break;                   
			case '?': error++;
					  break;
		}
	}
	if (optind > argc)     
		error++;

	if (error)
		exit(EXIT_ERROR);
/*
 *	The -s option does not write the terminal name. Only the exit status is
 * 	affected  by this option.  The terminal status is determined as if the
 *	isatty() function were used. The -s option is depricated.
 */
	if (supress)
	{
		if (isatty(fileno(stdin)) )
			exit(0);
		else
			exit(1);
	}
	/*	While no input is read from standard input, standard input is examined
	 * 	to determine whether or not it is a terminal, and/or to determine the name
	 * 	of the terminal.
	 */
	else
	{
		if ( !isatty(fileno(stdin)) ) {
			fprintf(stdout, "not a tty\n" );
			exit( EXIT_SUCCESS );
			}
		name = ttyname(STDIN_FILENO);
		if (name != NULL)
		{
			fprintf(stdout, "%s\n",name);
			exit(0);
		}
		else
			exit(1);
	}
}
