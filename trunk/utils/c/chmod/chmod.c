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




/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.15  2005/06/03 01:37:42  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.14  2003/08/21 20:09:56  martin
	Update QSSL Copyright.
	
	Revision 1.13  2002/07/23 18:09:34  marcind
	Re: PR 11137 Fixed the error msg so it is more friendly.
	
	Revision 1.12  2002/02/22 17:14:41  kewarken
	fix for PR:10241 where chmod -R chokes on broken links.  Shouts out to the
	Big CB - he may be down under but he ain't down and out y'all.  Word to the.
	1337 h4X0r5 in Kiwi land yo.
	
	Revision 1.11  2001/07/13 13:48:57  kewarken
	fixed option parsing bug
	
	Revision 1.10  2000/09/08 14:39:02  jbaker
	fixed pr 2710
	now parses '-rwx' type arguments properly
	
	Revision 1.9  1999/09/07 22:36:01  dagibbs
	Fix pr 1590.
	  chmod -R a+rw dir/
	would get wrong perms for files in dir/, worked ok if used as
	  chmod -R a+rw dir
	
	Added some comments about odd style/variable choices and dead routines.
	
	Revision 1.8  1998/09/15 18:10:52  eric
	cvs checkin
	
	Revision 1.7  1997/04/02 19:02:53  eric
	added -v option

	Revision 1.6  1996/06/13 07:07:46  peterv
	Added sticky bit support
	Added setting perms to other perm
	Added Link support

	Revision 1.4  1996/01/26 19:36:03  dtdodge
	Removed stupid need_usage().

	Revision 1.3  1995/09/26 23:48:36  steve
	fix util header files

 * Revision 1.2  1992/07/30  19:25:58  eric
 * added usage message one-liner
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: rmansfield $
	
---------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>

#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

#define ERR   1
#define NOERR 0

#define isoctal(c)  (c>='0' && c<='7')

#define T_ISDIR     			"invalid option for directories\n"
#define T_CANT_CHANGE			"can't change\n "
#define T_CLOSEDIR_ERROR        "error closing directory\n "
#define T_CANT_READ_DIR_ENTRY   "error reading directory\n "
#define T_CANT_OPEN_DIRECTORY   "can't open directory\n "

mode_t old_mask;
char   main_path[_POSIX_PATH_MAX+1];
int    rflag,isdir;
mode_t sum;
int    verbose=0;
int    posix_strict;

/* name of the program */
char *prog; 

#define RLINK_NEVER_FOLLOW		0
#define RLINK_MAIN_FOLLOW		1
#define RLINK_ALWAYS_FOLLOW		2

/* prototypes */
int parse(char *mode_str, int cur_mode, int create_mask);
void buildpath( char *path, int mode ); /* this is never called -- so why is it here? - dagibbs */
int parse_mode( char *mode_string, int creat_mask, int initial_val );
void process_perm( int perm, int who, int op,
                   int *cur_mode, int creat_mask,int initial_val );
int octnum(char c);
int change_file (char *path,int mode,char *mode_str,int rtype);
int change_directory_recursive (char *path, int mode,char *mode_str,int rtype );
void report_error ( int estat, char *fmt );



int main(int argc, char *argv[])
{
	static struct   stat statbuf;
	int             i, error;
	int             create_mask;
	int				break_out;
	int				mode;
	char		    *mode_str;
	int				rtype = RLINK_NEVER_FOLLOW;

	prog = basename(argv[0]);

	error=i=0;
	create_mask = umask(0);
	posix_strict = getenv( "POSIX_STRICT" ) != NULL;

	break_out = 0;
	opterr = 0;

	while((!break_out) && ( i= getopt( argc, argv, "R**HLPv")) != -1)
	{ 	switch (i) 	
		{	case 'R': rflag++;                 
					  break;                   
			case 'v': verbose++;                 
					  break;                   
			case 'H': rtype = RLINK_MAIN_FOLLOW;
					  break;
			case 'L': rtype = RLINK_ALWAYS_FOLLOW;
					  break;
			case 'P': rtype = RLINK_NEVER_FOLLOW;
					  break;
			case '?': switch (optopt)
					  {
						case 'r':
						case 'w':
						case 'x': 
						case 's': 
							optind--;
							break_out = 1; 
							break;
						case 't': 
						case 'X': 
						case 'u': 
						case 'g': 
						case 'o': 
							if(!posix_strict)
							{
								optind--;
								break_out = 1; 
								break;
							}
						default: error++;
								 fprintf(stdout,"Invalid option '%c'\n",optopt);
								 break;
					  }
					  break;
		}
	}

    if ((argc-optind) < 2) {
		fprintf(stderr,"%s: Must indicate a mode and at least one file\n", prog);
		error++;
	}

	if ( error )	exit(EXIT_FAILURE);

	if ( !rflag )	rtype = RLINK_NEVER_FOLLOW;

	mode_str = argv[optind++];   /* mode value */

	for( ; optind < argc; optind++ ) 
	{	
	 	if (strlen(argv[optind]) > _POSIX_PATH_MAX)
		{
			error++;
			fprintf(stderr, "%s: %s\n", argv[optind], strerror(ENAMETOOLONG));
			continue;
		}
		strncpy(main_path, argv[optind], _POSIX_PATH_MAX); 
		if ( ( ( rtype ? stat(main_path, &statbuf) : lstat(main_path, &statbuf ) ) == -1 ) )  	
		{ 		
			perror(main_path);
			error++;
			continue;
		}

		if(!S_ISLNK(statbuf.st_mode))
		{
			mode = parse(mode_str, (int)statbuf.st_mode, create_mask);
			if (change_file(main_path,mode,mode_str,rtype)==ERR)
				error++;
		}
	}				
	if (error)
		exit(ERR);
	else
		exit(NOERR);
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
			{	fprintf(stdout, "Invalid Mode Specification ('%s' )\n",mode_str);
				exit(1);
			}
		}
		if ( sum & (posix_strict ? ~06777 : ~07777 ) )
		{	fprintf(stdout, "Invalid Octal Mode (%o)\n",sum);
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
	

/* this is never called -- so why is it here? - dagibbs */
void buildpath( char *path, int mode )
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
		if (mkdir( path, mode ) == -1)
			if (errno != EEXIST){
				perror( "CHMOD: Error creating path components");
				exit( ERR );
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
 *		who		::= [u|g|o]...|a
 *		op		::= +|-|=
 *		perm	::= [r|w|x|s|t|X|u|g|o]...
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
#define MP_SETVTX	16
#define MP_CONDEXECUTE	32
#define MP_SETUSR	64
#define MP_SETGRP	128
#define MP_SETOTH	256

int parse_mode( char * mode_string,int creat_mask, int initial_val )
{
int		i, exit_loop, len;
int		cur_mode, who, op, perm;

	cur_mode = initial_val & (S_IPERMS|S_ISUID|S_ISGID|S_ISVTX);
	len = strlen( mode_string );
	i = 0;

	who = 0;
	while (i < len ){
		/*
	 	 * Figure out who to affect.  It is optional and there
		 * may be multiple who(s) per clause
		 */
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
								if (who&MP_OTHER) cur_mode&=~(S_IRWXO|S_ISVTX);
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
				case 'r':	process_perm( MP_READ, who, op, &cur_mode, creat_mask, initial_val );
							i++;
							break;
				case 'w':	process_perm( MP_WRITE, who, op, &cur_mode, creat_mask, initial_val );
							i++;
							break;
				case 'x':	process_perm( MP_EXECUTE, who, op, &cur_mode, creat_mask, initial_val );
							i++;
							break;
				case 's':	process_perm( MP_SETUID, who, op, &cur_mode, creat_mask, initial_val );
							i++;
							break;
				case ',':	i++;
							who = 0;
							exit_loop = TRUE;
							break;
				case 'X':
				case 't':
				case 'u':
				case 'g':
				case 'o':	if(!posix_strict){
					switch( mode_string[ i ] ){
					case 'X':	process_perm( MP_CONDEXECUTE, who, op, &cur_mode, creat_mask, initial_val );
								i++;
								break;
					case 't':	process_perm( MP_SETVTX, who, op, &cur_mode, creat_mask, initial_val );
								i++;
								break;
					case 'u':	process_perm( MP_SETUSR, who, op, &cur_mode, creat_mask, initial_val );
								i++;
								exit_loop = TRUE;
								break;
					case 'g':	process_perm( MP_SETGRP, who, op, &cur_mode, creat_mask, initial_val );
								i++;
								exit_loop = TRUE;
								break;
					case 'o':	process_perm( MP_SETOTH, who, op, &cur_mode, creat_mask, initial_val );
								i++;
								exit_loop = TRUE;
								break;
						}
					break;
					}
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



void process_perm( int perm, int who, int op,int *cur_mode, int creat_mask, 
                   int initial_val )
{
int	A, B, C;

	switch( perm ){
		case MP_READ:	A = S_IRUSR; B = S_IRGRP; C = S_IROTH;	break;
		case MP_WRITE:	A = S_IWUSR; B = S_IWGRP; C = S_IWOTH;	break;
		case MP_EXECUTE:A = S_IXUSR; B = S_IXGRP; C = S_IXOTH;	break;

		case MP_SETUSR:	A = B = C = (initial_val & S_IRWXU); B >>= 3; C >>= 6;	break;
		case MP_SETGRP:	A = B = C = (initial_val & S_IRWXG); A <<= 3; C >>= 3;	break;
		case MP_SETOTH:	A = B = C = (initial_val & S_IRWXO); A <<= 6; B <<= 3;	break;

		case MP_SETUID:	A = S_ISUID; B = S_ISGID; C = 0;		break;
		case MP_SETVTX:
			C = 0;
			if(!who || (who & (MP_USER | MP_GROUP))) {
				who = MP_OTHER; C = S_ISVTX;
			}
			A = 0; B = 0;
			break;

		case MP_CONDEXECUTE:
			if(op == MP_SET && (S_ISDIR(initial_val) || (initial_val & (S_IXUSR|S_IXGRP|S_IXOTH)))) {
				A = S_IXUSR; B = S_IXGRP; C = S_IXOTH;	break;
			}

		default:		A = 0;       B = 0;       C = 0;		break;
		}
	
	if ( op == MP_SET || op == MP_ASSIGN){
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


int octnum(char c)
{
	if (sum>=512) return(FALSE);   /* bounds checking, highest 7777 octal */

	if( isoctal(c) )    
	{	sum *= 8;                  
		sum += ( c - '0');
	}
	else
	{	fprintf(stdout, "Invalid octal number ('%c')\n", c);
		return(FALSE);
	}
	return(TRUE);
}       

/*       
 *	Change the mode of the file specified by path.
 *  If must recurse ( -R option on command line ) and file is a directory,
 *	recurse thru the tree
 */
int change_file (char *path,int mode,char *mode_str,int rtype)
{
	int		status;
	static struct   stat statbuf;

	if (verbose) {
		printf("%s: Changing %s to mode 0%o\n", prog, path, mode);
	}

	status = chmod(path, mode);


	if (status==-1) {
		fprintf(stderr,"%s: changing permission of %s : %s\n", prog, path, strerror(errno));
		return( ERR );
	}

    /* why mix path & main_path -- can the be different? -dagibbs */
	if  ( ( rtype ? stat(main_path, &statbuf) : lstat(main_path, &statbuf ) ) == -1 )  	
	{ 	
		perror(main_path);
		return( ERR );
	}

	/*
	 * if path is a directory and -R specified
	 */
	if(rtype == RLINK_MAIN_FOLLOW) rtype = RLINK_NEVER_FOLLOW;
	if ( S_ISDIR(statbuf.st_mode) && rflag )	
		return( change_directory_recursive(path,mode,mode_str,rtype));


	if ( status == -1 )	
	{ 	report_error(NOERR, T_CANT_CHANGE);
		perror(path);
		return( ERR );
	} 
	return( NOERR );
}
/*
 *   If the -R option is specified  it will recursively change the file mode bits.
 * 	 For each file operand that names a directory, this function changes the file 
 *   mode bits of the directory and all the files in the file hierarchy below it.
 */
int change_directory_recursive ( char *path,int mode,char *mode_str,int rtype )
{
	DIR    *dirp;				
	struct dirent *entry;
	static struct stat statbuf;
	int    rc = 0;
	int    create_mask;

	if ( !( dirp=opendir( path ) ) )	{
		report_error( NOERR, T_CANT_OPEN_DIRECTORY );
        perror( path );
		return( ERR );
	}

	errno = 0;
	create_mask = umask(0);

	while (errno=0, entry = readdir( dirp ) )	
	{	if (!entry->d_name[0]) {
			continue;
		}
		if ( entry->d_name[0] == '.' )	{
			if ( !entry->d_name[1] ) 
				continue;
			if ( ( entry->d_name[1] == '.' ) && ( !entry->d_name[2] ) ) 
				continue;
		}

		if ( errno )	{
			report_error( NOERR, T_CANT_READ_DIR_ENTRY );
            perror( path );
			rc=ERR;
			continue;
		}

		/* add filename to end of path */
		if ( path[strlen( path )-1] != '/' )	{

			strcat( path, "/" );
			strcat( path, entry->d_name );

            /* main_path and path are the same thing, so why change names? */
			if ( ( stat(main_path, &statbuf) == -1 ) )  	
			{ 	
				perror(main_path);
				/* remove filename from path */
				*(strrchr(path, '/')) = '\0';
				continue;
			}

			mode = parse(mode_str, statbuf.st_mode, create_mask);

			/* 
			 * 	change that file  - rc end up non-zero if an error occurred 
			 *	changing something in this directory
			 */
			rc |= change_file( path,mode,mode_str,rtype );
	
			/* remove filename from path */
			*( strrchr( path, '/' ) ) = (char) NULL;
		} 
		else {

			strcat( path,entry->d_name );

            /* main_path and path are the same thing, so why change names? */
			if ( ( stat(main_path, &statbuf) == -1 ) )  	
			{ 	
				perror(main_path);
				continue;
			}

			mode = parse(mode_str, statbuf.st_mode, create_mask);

			/*
			 * change that file  - rc end up non-zero if an error occurred 
			 * changing something in this directory
			 */
			rc |= change_file( path,mode,mode_str,rtype );

			*( strrchr( path,'/' )+1 ) = (char) NULL;

		}
	}

	if (errno) {
		perror(path);
	}

	if ( closedir( dirp ) == -1 )	{
		report_error( NOERR, T_CLOSEDIR_ERROR );
		perror( path );
		rc=ERR;
	}

	if ( !errno && !rc )
		return( NOERR );
	else 
		return( ERR );
}	

void report_error ( int estat, char *fmt )
{
	fprintf( stderr, "%s", fmt );
	if ( estat )
		exit( estat );
}

