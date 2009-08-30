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





#ifdef __USAGE		/* mkdir.c */
%C - make directories (POSIX)

%C	[-p] [-m mode] dir...
Options:
 -m mode  Use to specify permissions for the directory being created.
 -p       Create any missing intermediate path components.
Where:
  dir    A pathname at which a directory is to be created.

  mode   =    [who]op[perm]  |  octal

  who    =    u   user
              g   group
              o   other
              a   all

  op     =    +   add
              -   remove
              =   explicit set

  perm   =    r   read
              w   write
              x   execute
              s   set ID on execution
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.26  2007/04/17 17:08:26  aristovski
	Fixed trailing slash issue

	PR: 46248
	CI: rmansfield

	Revision 1.25  2007/04/17 14:38:21  aristovski
	PR: 46523
	CI: rmansfield
	
	Added mkdir define. in windows (mingw), second orgument (mode) does not exist.
	
	Revision 1.24  2007/04/03 19:49:53  jbaker
	
	PR: 46248
	CI: rmansfield
	
	port our mkdir to cygwin.
	
	Revision 1.23  2006/11/02 16:53:34  seanb
	
	- 'mkdir -p' failing without error.
	- Also fixed some vararg and buffer overrun issues.
	PR:42551
	CI:jgarvey
	
	Revision 1.22  2006/07/06 16:13:47  jgarvey
	PR/39300
	Ignore final EEXIST error.  The form_path() routine can go one
	directory too deep doing simple string-based pathname tokenisation
	(SKIP_PATH()), and can end up performing duplicate mkdir() call.
	Rather than add filesystem logic here (to skip trailing "/" or "/."),
	just let the duplicate mkdir() calls be made and silently ignore any
	EEXIST (we know the EEXIST is not real/original, because earlier mkdir
	and lstat probes did not identify such a situation, so it must be due
	to this utility having alerady created the directory in form_path()).
	CI: sboucher
	
	Revision 1.21  2005/06/03 01:37:50  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.20  2005/04/07 15:16:23  kewarken
	Final fix for PR:19533.  Remove -s option
	
	Revision 1.19  2005/02/03 21:45:18  mshane
	Removed somewhat offensive comments at the request of dkeefe.
	CR:Marcind
	
	Revision 1.18  2005/01/31 20:07:33  kewarken
	Removed '-s' option from use message.  Also removed QNX4isms since they are
	preserved in the QNX4 cvs tree.
	
	Revision 1.17  2003/08/26 18:56:25  martin
	Add QSSL Copyright.
	
	Revision 1.16  2001/12/13 19:47:15  kewarken
	fix for PR:9688 - mask for permissions was wrong
	
	Revision 1.15  2001/03/16 14:47:38  kewarken
	fixed excess verbosity with -p option
	
	Revision 1.14  2000/07/31 18:24:59  thomasf
	Partially addresses PR 2313, but in a very round about kind of way.
	
	Revision 1.13  1998/09/18 13:58:48  builder
	gcc cleanup
	
	Revision 1.12  1998/09/15 18:44:03  eric
	cvs

	Revision 1.11  1997/02/27 16:57:11  eric
	added -s size option for QNX 424++

	Revision 1.10  1996/11/01 15:59:54  eric
	changed // processing for mkdir -p so that it would not fail to
	create the next dir level after //nid. (Used to fail for
	mkdir -p //61/a/b/c by only commencing the attempt at //61/a/b)

	Revision 1.9  1996/10/30 18:47:41  eric
	removed reference to stdutil.h

	Revision 1.8  1996/10/02 15:36:09  eric
	nto port

	Revision 1.7  1996/09/16 18:14:07  eric
	added prototype which appears to be necessary when compiling
	nto version (parse())

 * Revision 1.6  1996/07/02  15:41:46  eric
 * now treats mkdir -p existing_non-directory_file as an error;
 * mkdir -p existing_directory_file is still not an error
 *
	Revision 1.5  1996/07/02 14:46:20  steve
	changed include headers so it would compile

 * Revision 1.4  1992/10/27  17:12:00  eric
 * added usage one-liner
 *
 * Revision 1.3  1992/03/19  14:33:38  eric
 * put -m before -p (alphabetical) in the usage message
 * (matches the printed docs)
 *
 * Revision 1.2  1991/11/18  16:29:06  eric
 * Fixed problem with not proceeding to next dir on cmd line when an
 * error is encountered creating the current one.
 *
 * Fixed problems with permissions for created dirs not being quite
 * right.
 *
 * Cleaned up the code a bit, eliminated redundant umask() calls.
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/

#include <lib/compat.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#if defined(__MINGW32__)
# define mkdir(path, mode) mkdir(path) // mingw does not have second argument.
#endif

#define FALSE 0
#define TRUE  !(FALSE) 

#define SKIP_TO_SLASH(p)	{for(;*p && *p!='/';p++);}
#define isoctal(c)           (c>='0' && c<='7')

int parse(char *mode_str, int cur_mode, int create_mask);
void buildpath(char *path, int mode );
int parse_mode(char *mode_string, int creat_mask, int initial_val );
void process_perm(int perm, int who, int op, int *cur_mode, int creat_mask );
void report_error (int estat, char *fmt);
void prerror(const char *format, ...);
int form_path(char *fullpath);
int octnum(char c);

static void rmtrailslash(char *filename)
{
	int fnameend = strlen(filename) - 1;

	if (filename[fnameend] == '/' || filename[fnameend] == '\\')
	    filename[fnameend] = '\0';
}

mode_t mode;
mode_t dir_mode;
char   mode_num[5];
mode_t sum;
char   *mode_str;
int    create_mask;

int main(int argc, char **argv)
{
	int		i;
	char	*filename;
	static int pathflag=0,modeflag=0,error=0,dir=0;

	create_mask = umask(0);
	mode |= ( S_IRWXU | S_IRWXG | S_IRWXO ) & ~create_mask;  /* default a=rwx (777) */
	dir_mode = mode | (S_IWUSR | S_IXUSR );

	while(( i= getopt( argc, argv, "pm:")) != -1)
	{	switch (i) 	
		{	case 'p': pathflag++;                 
					  break;                   

			case 'm': modeflag++;
					  mode_str = optarg;
					  break;

			case '?': error++;
					  break;
		}
	}

	if (optind >= argc) {     
		fprintf(stderr,"mkdir: no directory specified\n");
		error++;
	}

	if (error) exit(EXIT_FAILURE);

	if (modeflag)
		mode = parse( mode_str, mode, create_mask);

	for( ; optind < argc; optind++ ) {
		filename = argv[optind];
		
		rmtrailslash(filename);
		
		if ((dir=mkdir(filename, mode))==-1) {
			if (!pathflag) {
				prerror("mkdir: %s",filename); 
				error++;
				continue;
			} else if ( errno == EEXIST || errno == EPERM || errno == EACCES ) {
				struct stat buf;
				int errno_saved;

				errno_saved = errno;

				/* have to stat it to determine if it is an existing
				directory (not an error), or some other filetype (an
				error) */
				if (lstat(filename, &buf) == -1) {
					/*
					 * We really don't have perms, not a read only
					 * filesystem with an existing dir.
					 */
					errno = errno_saved;
				}
				else if (!S_ISDIR(buf.st_mode)) {
					/* not an existing directory - this is an error */
					errno=EEXIST;
				}		
				else {
					/* else the already existing file is a directory;
					in -p mode this is not an error */
					continue;
				}
				prerror("mkdir: %s",filename);
				error++;
				continue;
			} else {
				/* -p was specified - restore umask and create in-between dirs */
				if (form_path(filename)) {
					if ( mkdir(filename, mode) == -1 && errno != EEXIST) {
						prerror("mkdir: %s",filename);
						error++;
					}
				} else error++;	/* printed err msg was displayed in form_path() */
			}
		}
	}                    

	exit(error?EXIT_FAILURE:EXIT_SUCCESS);
}

int parse(char *mode_str, int cur_mode, int create_mask)
{
	int len,i;
	int mode;
	
	len = i = 0;
	if (isoctal(*mode_str) )        /* parse mode */
	{
	 	len = strlen(mode_str);
		for(sum=0,i=0; (mode_str[i] != 0); i++)
		{	if ( octnum( mode_str[i] ) == FALSE )
			{	fprintf(stdout, "mkdir: Invalid Mode Specification ('%s' )\n",mode_str);
				exit(1);
			}
		}
		if ( sum & ~07777 )
		{	fprintf(stdout, "mkdir: Invalid Octal Mode (%o)\n",sum);
			exit(1);
		}
			mode = sum;
			return(mode);
	}
	else
	{	mode = parse_mode(mode_str, create_mask, cur_mode);
		return(mode);
	}
}
	

void buildpath(char *path, int mode )
{
	char	*step, *end;

	end = strrchr( path, '/');
	step = strchr( path, '/');
	if (step==path)
		step = strchr( step+1, '/');
	if (end==step)
		return;
	while (step<=end && step!=NULL){
		*step=0;
		*end = 0;
		if (mkdir(path, mode)==-1)
			if (errno != EEXIST){
				report_error(1,"Error creating path components");
				exit( 1 );
				}
        *step = '/';
		*end  = '/';
		step = strchr( step+1, '/');
		}
	*end = '/';
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

int parse_mode(char *mode_string, int creat_mask, int initial_val )
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
				default:		report_error(1,"Invalid Mode Specification\n");	break;
				}
			}
	
		if (!exit_loop) report_error(1,"Invalid Mode Specification\n");


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
				case 's':	process_perm( MP_SETUID, who, op, &cur_mode, creat_mask );
							i++;
							break;
				case ',':	i++;
							exit_loop = TRUE;
							break;
				default:	report_error(1, "Invalid permission in mode argument\n" );
							break;
				}
			}
		}
	
	/*
	 * Mode Spec Parsing Completed.  Return final mode integer value
	 */
	return( cur_mode );
}

void process_perm(int perm, int who, int op, int *cur_mode, int creat_mask )
{
	int	A=0, B=0, C=0;

	switch( perm ){
		case MP_READ:	A = S_IRUSR; B = S_IRGRP; C = S_IROTH;	break;
		case MP_WRITE:	A = S_IWUSR; B = S_IWGRP; C = S_IWOTH;	break;
		case MP_EXECUTE:A = S_IXUSR; B = S_IXGRP; C = S_IXOTH;	break;
		case MP_SETUID:	A = S_ISUID; B = S_ISGID; C = 0;		break;
		default:												break;
		}
	
	if ( op == MP_SET || op ==MP_ASSIGN){
		if ( who & MP_USER )
			*cur_mode |= A;
		if ( who & MP_GROUP )
			*cur_mode |= B;
		if ( who & MP_OTHER )
			*cur_mode |= C;
		if ( !who ){
			*cur_mode |= A&~creat_mask;
			*cur_mode |= B&~creat_mask;
			*cur_mode |= C&~creat_mask;
		}
	}
	else{
		if ( who & MP_USER )
			*cur_mode &= ~A;
		if ( who & MP_GROUP )
			*cur_mode &= ~B;
		if ( who & MP_OTHER )
			*cur_mode &= ~C;
		if ( !who )
		{
			*cur_mode &= ~(A|B|C);
		}
	}
}

/*
process_perm2( perm, who, creat_mask )
int perm, who, creat_mask;
{
int	A, B, C;
mode_t cur_mode = 0;

	switch( perm ){
		case MP_READ:	A = S_IRUSR; B = S_IRGRP; C = S_IROTH;	break;
		case MP_WRITE:	A = S_IWUSR; B = S_IWGRP; C = S_IWOTH;	break;
		case MP_EXECUTE:A = S_IXUSR; B = S_IXGRP; C = S_IXOTH;	break;
		case MP_SETUID:	A = S_ISUID; B = S_ISGID; C = 0;		break;
		default:												break;
	}
	
	if ( who & MP_USER ) 		cur_mode |= A;
	if ( who & MP_GROUP ) 		cur_mode |= B;
	if ( who & MP_OTHER ) 		cur_mode |= C;
}
*/

int octnum(char c)
{
	if (sum>=450) return(FALSE);   /* bounds checking, highest 6777 octal */

	if( isoctal(c) )    
	{	sum *= 8;                  
		sum += ( c - '0');
	}
	else
	{	fprintf(stdout, "mkdir: Invalid octal number ('%c')\n", c);
		return(FALSE);
	}
	return(TRUE);
}       


/***************************************************************************
*	This function, given a pathname will create all the intermediate       *
*  	directories needed.  This does not include the final filename.  	   *
****************************************************************************/


int form_path(char *fullpath)
{
	char *ptr = fullpath;

	#ifdef DIAG
		fprintf(stderr,"In qnx_create_path_to(%s)\n",fullpath);
		getchar();
		printf("Form path on [%s] \n", fullpath);
	#endif
	if (fullpath[0] == '/' && fullpath[1] == '/') {
		/* starts with '//' */
		ptr += 2;
#ifdef WRONG
		SKIP_TO_SLASH(ptr);
		if (!*ptr || !*(ptr+1)) return(0);  /* zzx should check that path exists */
		ptr++;
		SKIP_TO_SLASH(ptr);
		if (!*ptr || !*(ptr+1)) return(0);
		ptr++;
#endif
	} else if (fullpath[0] == '/') ptr++;

	SKIP_TO_SLASH(ptr);
	
	for (;*ptr;) {
		*ptr = (char)0x00;
		#ifdef DIAG
			fprintf(stderr,"#2 create_path_to - a - mkdir(%s)\n",fullpath);
			getchar();
		#endif
		if (mkdir(fullpath, dir_mode)==-1) {
			if (errno!=EEXIST && errno != EPERM && errno != EACCES) {
				int  e = errno;

				if ( access( fullpath, F_OK ) ) {
					errno = e;
					prerror("mkdir: %s",fullpath);
					*ptr = '/';
					return(-1);
				}
			}
		}
		*ptr = '/';
		ptr++;
		SKIP_TO_SLASH(ptr);
	}
	return(TRUE);
}

void report_error (int estat, char *fmt)
{
	fprintf(stderr, "%s", fmt);
	if (estat) 	exit( estat );
}


/*
--------------------------------------------------------- prerror(str1,args)
*/

void prerror(const char *format, ...)
{
        va_list arglist;
        char buffer[512];
   
        va_start(arglist, format);
        vsnprintf(buffer, sizeof(buffer), format, arglist);
        va_end(arglist);
        perror(buffer);
}

void aberror(const char *format, ...)
{
        va_list arglist;
        char buffer[512];
   
	va_start(arglist, format);
	if (errno != EOK) {
		vsnprintf(buffer, sizeof(buffer), format, arglist);
		va_end(arglist);
		perror(buffer);
	}
	else {
		vfprintf(stderr, format, arglist);
		va_end(arglist);
	}
	exit(EXIT_FAILURE);
}

void abprt(const char *format, ...)
{
        va_list arglist;
   
        va_start(arglist, format);
        vfprintf(stderr, format, arglist);
        va_end(arglist);
		exit(EXIT_FAILURE);
}
