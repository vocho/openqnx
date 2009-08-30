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





#ifdef __USAGE		/* umask.c */
%C - get or set the file mode creation mask (POSIX)

%C	[ -o | -s | mask ]
Options:
 -o    Display the current mask, in octal.
 -s    Display the current mask in symbolic form. (default).
 mask  Set the file mode creation mask to this value.
       (Mask may be octal or symbolic.)
Note:
 See 'chmod' usage for description of symbolic mode syntax.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.9  2005/06/03 01:38:03  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.8  2003/09/02 16:21:33  martin
	Update QSSL Copyright.
	
	Revision 1.7  1998/11/25 23:06:26  eric
	now accommodates gcc
	
	Revision 1.6  1998/09/15 19:00:31  peterv
	cvs
	
	Revision 1.5  1997/02/18 21:45:35  steve
	*** empty log message ***

 * Revision 1.4  1994/11/30  20:59:11  eric
 * changed to not check errno on return from qnx_umask; will not
 * attempt if parent is PROC or is a VID to eliminate (for the most
 * part) the possibility that the call failed.
 *
 * Revision 1.3  1992/10/27  19:50:33  eric
 * added usage one-liner
 *
 * Revision 1.2  1992/07/09  14:44:50  garry
 * Editted usage
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
/*
 * Include declarations:
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __QNXNTO__
#include <sys/kernel.h>
#include <sys/psinfo.h>
#endif

/*
 * Define declarations:
 */
#define TRUE				01
#define FALSE				00

#define MP_USER		1
#define MP_GROUP	2
#define MP_OTHER	4

#define MP_SET		1
#define MP_CLEAR	2
#define MP_ASSIGN	4

#define MP_READ		1
#define MP_WRITE	2
#define MP_EXECUTE	4
#define MP_SETUID	8

/*
 * Global Declarations
 */
int create_mask;

/* 
 *	The umask utility sets the file mode creation mask of the invoking
 *	process to the value specified by teh mask operand. The mask affects
 *	the initial value of the file permission bits of subsequently
 *	created files.
 *	If the mask operand is not specified, the umask utiltity writes to 
 *	standard output the value of the invoking process's file mode 
 *	creation mask.
 */
#define DISP_DISPLAY 1		/* when set in flags, display, don't set */
#define DISP_OCTAL	 2
#define DISP_SYMBOL	 4


void report_error ( estat, fmt )
int		estat;
char 	*fmt;
{
	fprintf( stderr, "%s", fmt );
	if ( estat )
		exit( estat );
}

void print_mode(mask)
int mask;
{
	if (mask & 0x4)
		fprintf(stdout,"r");
	if (mask & 0x2)
		fprintf(stdout,"w");
	if (mask & 0x1)
		fprintf(stdout,"x");
}

/*
 * Parse a mode operand which is in the form of an octal number an
 * calculate the representative integer
 */
int parse_octal_mode( mode )
char *mode;
{
	int	factor, i, value;
	value = 0;
	factor = 1;
	i = strlen( mode )-1;
	while ( i >= 0){
		if ( mode[i] >='0' && mode[i]<'8'){
			value += (mode[i] -'0') * factor;	
			factor *= 8;
			}
		else
			fprintf(stderr, "Invalid character in Octal mode specification\n" );
		i--;
		}
	return( value );
	}

void process_perm( perm, who, op, cur_mode, creat_mask )
int perm, who, *cur_mode, creat_mask, op;
{
int	A, B, C;

    A=B=C=0;

	switch( perm ){
		case MP_READ:	A = S_IRUSR; B = S_IRGRP; C = S_IROTH;	break;
		case MP_WRITE:	A = S_IWUSR; B = S_IWGRP; C = S_IWOTH;	break;
		case MP_EXECUTE:A = S_IXUSR; B = S_IXGRP; C = S_IXOTH;	break;
		case MP_SETUID:	A = S_ISUID; B = S_ISGID; C = 0;		break;
		default:												break;
	}
	
	if ( op == MP_SET || op ==MP_ASSIGN){
		if ( who & MP_USER ) 	*cur_mode |= A;
		if ( who & MP_GROUP )	*cur_mode |= B;
		if ( who & MP_OTHER )	*cur_mode |= C;
		if ( !who ) 			*cur_mode |= (A|B|C)&~creat_mask;
	}
	else{
		if ( who & MP_USER )
			*cur_mode &= ~A;
		if ( who & MP_GROUP )
			*cur_mode &= ~B;
		if ( who & MP_OTHER )
			*cur_mode &= ~C;
		if ( !who )
			*cur_mode &= ~(A|B|C);
	}
}

/*
 * Parse a mode string as follows and calculate a mode integer value
 *
 *		mode	::= clause[,clause]
 *		clause	::= [who] op [perm]
 *		who		::= [u|g|o]...|s
 *		op		::= +|-|=
 *		perm	::= [r|w|x|s]...
 *
 */

int parse_mode( mode_string, creat_mask, initial_val )
char	*mode_string;
int		creat_mask;
int		initial_val;
{
int		i, exit_loop, len;
int		cur_mode, who, op, perm;

	cur_mode = initial_val;
	len = strlen( mode_string );
	i = 0;

	while (i < len ){
		/*
	 	 * Figure out who to affect.  It is optional and there
		 * may be multiple who(s) per clause
		 */
		who = 0;
		for (exit_loop=FALSE, op=0; !exit_loop && i < len ; i++){
			switch( mode_string[i] ){
				case 'u':		who |= MP_USER;					break;
				case 'g':		who |= MP_GROUP;				break;
				case 'o':		who |= MP_OTHER;				break;
				case 'a':		who = MP_USER|MP_GROUP|MP_OTHER;break;
				case '+':		op = MP_SET;	exit_loop=TRUE;		break;
				case '-':		op = MP_CLEAR;	exit_loop=TRUE;		break;
				case '=':		op = MP_ASSIGN;
								exit_loop=TRUE;
								if (who&MP_USER) cur_mode&=~(S_IRWXU|S_ISUID);
								if (who&MP_GROUP) cur_mode&=~(S_IRWXG|S_ISGID);
								if (who&MP_OTHER) cur_mode&=~(S_IRWXO);
								if (!who) cur_mode=0; /* clear all if no who */
								break;
				default:		report_error(EXIT_FAILURE,"Invalid Mode Specification\n");	break;
				}
			}
	
		if (!exit_loop) report_error(EXIT_FAILURE,"Invalid Mode Specification\n");


		/*
	 	 * Figure out the perms to affect.  It is optional and there
		 * may be multiple perms per clause.  As we process each perm
		 * apply the changes to the current mode spec
		 */
		perm = 0;
		exit_loop = FALSE;
		while ( !exit_loop && i< len){
			switch( mode_string[ i ]  ){
				case 'r':	process_perm( MP_READ, who, op, &cur_mode, creat_mask );
							i++;
							break;
				case 'w':	process_perm( MP_WRITE, who, op, &cur_mode, creat_mask );
							i++;
							break;
				case 'x':	process_perm( MP_EXECUTE, who, op, &cur_mode, creat_mask );
							i++;
							break;
#ifdef SETUID_ALLOWED	
				/* not allowed for umask, but allowed in chmod */
				case 's':	process_perm( MP_SETUID, who, op, &cur_mode, creat_mask );
							i++;
							break;
#endif
				case ',':	i++;
							exit_loop = TRUE;
							break;
				default:	report_error(EXIT_FAILURE, "Invalid permission in mode argument\n" );
							break;
				}
			}
		}
	
	/*
	 * Mode Spec Parsing Completed.  Return final mode integer value
	 */
	return( cur_mode );
	}




int main( argc, argv )
int argc;
char *argv[];
{
	int 	i, error,user,group;
	int		flag = DISP_DISPLAY;	/* display, default mode */
	int     create_mode;
	char	*mode_str;
	int		stop_option_parsing;

	stop_option_parsing=error = user = group = FALSE;
	mode_str = NULL;

	/*
	 * Process options and option arguments
	 */
	while((!stop_option_parsing) && (( i= getopt( argc, argv, "osSrwx")) != -1))
		switch ( i ){
			case 'o':	flag |= DISP_OCTAL;	break;
			case 'S':
			case 's':	flag |= DISP_SYMBOL; break;
			case '?':	error = TRUE; break;
			default:	flag&=~DISP_DISPLAY;	/* turn off display flag */
						stop_option_parsing++;
						optind--;
						break;
	}

	if ((argc-optind) && (flag&~DISP_DISPLAY)) error++;

	/* if flag does not have DISP_DISPLAY set, but DOES have other bits set,
	   the user has selected -s, -w, -x in combination with -o or -s, which
	   is not legal syntax. Error msg. */

	if	(error || 
			((flag&DISP_OCTAL) && (flag&DISP_SYMBOL)) ||
			(argc>2) || 
			(flag && !(flag&DISP_DISPLAY))
		)
	{
		fprintf(stderr,"%s: invalid usage\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	/* if the user didn't explicitly specify, default to symbolic display */
	if (flag==DISP_DISPLAY) flag|=DISP_SYMBOL;

	/* this has the side-effect of clearing the current mask
	   therefore, we call it twice here to set it back to what it was,
 	   allowing us to exit the program freely. Should really be
	   unbreakable around these two lines zzx */

	create_mask = umask( 0 ); /* get (and wipe) my own */
	i = umask( create_mask ); 	/* i is dummy - throwaway result */

	if (argc-optind) {	/* we are in set umask mode */
		mode_str = argv[1];

	 	if (*mode_str>='0' && *mode_str<'8') {
			create_mode = parse_octal_mode( mode_str );
			if (create_mode>0777) {
				fprintf(stderr,"Octal number must be in the range 0-777\n");
				exit(EXIT_FAILURE);
			}
		 } else {
 			/* complement create_mask, (current mask) */
			create_mask = ~create_mask;
			/* we want file permission bits portion only */
			create_mask &= 0777;

			create_mode = parse_mode( mode_str, create_mask, create_mask );

			/* invert again before saving */
			create_mode = ~create_mode;
			create_mode &= 0777;
		}

#ifdef __QNXNTO__
		{
			pid_t dadspid;

			dadspid=getppid();

			i = _umask(dadspid, create_mode);
		}
#else
		{
			pid_t dadspid;
			struct _psinfo psdata;

			dadspid=getppid();

			qnx_psinfo(PROC_PID,dadspid,&psdata,0,0);
	
			if (psdata.pid!=dadspid || psdata.pid==PROC_PID || psdata.flags & _PPF_VID) {
				fprintf(stderr,"umask failed: parent does not exist or is running on another node.\n");
			} else {
				i = qnx_umask(dadspid, create_mode);
			}
		}
#endif
	} else {	/* we are supposed to just display the current umask */
		if(flag&DISP_OCTAL)
			fprintf(stdout,"0%o\n", create_mask);
		else {
			/* one's complement */
			create_mask = ~create_mask;
			/* file permission bits portion only */
			create_mask &= 0777;

			fprintf(stdout,"u=");
     		if (create_mask & 0700)	print_mode(create_mask>>6);
			fprintf(stdout,",g=");
			if ( create_mask & 070) print_mode(create_mask>>3);
			fprintf(stdout,",o=");
			if ( create_mask & 07)  print_mode(create_mask);
			fprintf(stdout,"\n");
		}	
	}

	return EXIT_SUCCESS;
}
