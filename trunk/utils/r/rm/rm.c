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
	Revision 1.17  2007/04/20 20:24:20  aristovski
	PR: 46523
	CI: rmansfield

	Added WIN32_ENVIRON=mingw
	added __MINGW32__

	Revision 1.16  2005/06/03 01:37:58  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.15  2003/08/28 20:42:27  martin
	Add QSSL Copyright.
	
	Revision 1.14  1998/12/02 15:34:11  bstecher
	added Win32 variant
	
	Revision 1.13  1998/09/26 16:15:15  bstecher
	all uses of <util/cpdeps.h> now should use <lib/compat.h>

	Revision 1.12  1998/09/16 14:51:12  builder
	gcc cleanup

	Revision 1.11  1998/09/15 18:26:18  eric
	cvs

	Revision 1.10  1997/04/23 21:41:48  bstecher
	Port to Win32

	Revision 1.9  1997/03/14 18:42:19  eric
	changed to use lstat_optimize() util lib fn

	Revision 1.8  1996/09/13 20:43:34  eric
	conditional compilation for nto

	Revision 1.7  1994/12/05 21:28:31  eric
	Now checks POSIX_STRICT envar and if set, does 1003.2 behaviour
	wrt -f -i, else does historical QNX 4 behaviour.

 * Revision 1.6  1994/11/17  20:33:50  eric
 * Changed -i, -f to override each other if both are
 * specified. Last one wins! Changed usage to reflect this.
 *
 * Revision 1.5  1992/10/27  17:48:32  eric
 * added usage one-liner
 *
 * Revision 1.4  1992/04/24  14:03:11  eric
 * rm (without -f) was not exiting failure when operands did not
 *    exist
 *
 * rm with -f was not exiting failure when it failed to remove a
 *    file for some reason OTHER than that it did not exist.
 *
 * Revision 1.3  1992/01/09  18:51:50  eric
 * Now does lstat instead of stat if lstat call is available - so
 * it will work to remove a symlink.
 *
 * Revision 1.2  1991/08/23  14:12:41  eric
 * code clean-ups, no actual code changes (i.e. just made prettier)
 *
 * Revision 1.1  1991/08/23  13:59:47  eric
 * Initial revision
 *
	
	Revision 1.4 Wed Apr 11 09:16:35 1990 ejohnson
	added support for Mr Bill'
	s readdir/stat optimization
	
	Revision 1.3 Wed Mar 21 09:02:47 1990 opr
	(ej) added check for '..'
	
	Revision 1.2 Fri Nov 24 11:57:10 1989 ejohnson
	changed c var from char to int for storing return from getc in
	prompt routine
	
	Revision 1.1 Fri Nov 24 11:51:04 1989 ejohnson
	 *** QRCS - Initial Revision ***
	
---------------------------------------------------------------------*/

/*
-----------------------------------------------------------------------
POSIX UTILITY: RM

1003.2 -- Draft 9 (Shell and Utilities working group)

-----------------------------------------------------------------------
*/

/*
----------------------------------------------------------- includes ------
*/

#include <lib/compat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef __NT__
	#undef NAME_MAX
	#include <direct.h>
#else
	#include <dirent.h>
#endif
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

typedef int		bool;
#define TRUE	1
#define FALSE	0

#if defined(__QNX__) && !defined(__QNXNTO__)
#define DO_STAT_OPT
#endif

#ifdef DO_STAT_OPT
#include <util/stat_optimiz.h>
#endif

#ifdef __STDC__
extern short prompt(char *);
extern short remove_directory_recursive(char *);
extern short remove_guy(char *);
#endif

/*
----------------------------------------------------------- defines  --------
*/

/* ----------- text messages ------------ */

#define TXT(s)			  s
#define T_MUST_SPEC_FILE		"rm: Must specify file[s] for removal.\n"
#define T_INTERNAL_ERROR        "rm: Internal Error\n"
#define T_NOWRITEWARNINGFMT     "rm: %s is unwriteable. "
#define T_NOWRITESKIP			"rm: Skipping %s -- no write permission\n"
#define T_CANTRECURSE           "rm: %s is a directory. Use -R to recurse.\n"
#define T_CANT_OPEN_DIRECTORY   "rm: Can't open directory "
#define T_CANT_READ_DIR_ENTRY   "rm: Error reading directory "
#define T_CANT_RM_DOTDOT		"rm: Not allowed to rm '..'\n"
#define T_CANT_RM_DOT			"rm: Not allowed to rm '.'\n"
#define T_CLOSEDIR_ERROR        "rm: Error closing directory "
#define T_READDIR_ERROR         "rm: Error reading directory "
#define T_RMDIR_ERROR			"rm: Can't remove directory "
#define T_CANT_UNLINK			"rm: Can't unlink "
#define T_DELETEANYWAY          "rm: Remove"
#define T_REMOVING				"rm: Removing file '%s'\n"
#define T_SKIPPING				"rm: Skipping file '%s'\n"
#define T_DIR_SKIPPING			"rm: Skipping removal of directory '%s'\n"
#define T_REMOVING_DIR			"rm: Removing directory '%s'\n"
#define T_VERIFY				"rm: remove "
#define T_DIR_VERIFY			"rm: remove directory "
#define T_TOO_MANY_LEVELS		"rm: levels limited to %d. Use -l## option to set higher.\n"
#define T_LONG_FILENAME			"rm: Skipping %s -- Filename too long\n"

/* ----------- return codes ------------- */

#define FAILURE_RC		   -1
#define SUCCESS_RC          0
#define INVALID_USAGE_RC    1

#define MAX_LEVELS			50

/*
------------------------------------------------------------- GLOBALS -------
*/

bool  Strict, Forced, Interactive, Recursive, Attended, Verbose;
int16_t Max_Levels;
int16_t Levels;
bool  Dont_rmdir;

char Main_path[PATH_MAX];

struct stat Statbuf;
bool statbuf_valid;

int save_errno=0;

/*
------------------------------------------------------------- main ----------

See POSIX 1003.2 draft 9 for functional description of utility.

rm [-f | -i] [-Rr] file...

The rm utility removes the directory entry specified by each file arg.

By default, rm shall refuse to remove any directory entry that
names a directory.

The rm utility shall refuse to remove a directory entry specified
by a file operand of dot-dot ("..")


-f = force each entry to be removed without prompting for confirmation,
	 regardless of the permissions of the file to which it refers.
	 Supress diagnostic messages regarding non-existent operands.

-i = Write a prompt to the standard error output requesting confirmation
	 before removing each existing directory entry, regardless of the
	 permissions of the file to which it refers.

-R = -r = Recursive. See 1003.2 for details.
	 
-----------------------------------------------------------------------------
*/

int main (int argc, char * argv[])
{
int16_t error;
int16_t first_index, last_index, num_files, index;
int opt;

#ifdef DIAG
fprintf(stderr,"Main_path[%d]\n",sizeof(Main_path));
#endif

Attended = isatty(fileno(stdin));

Max_Levels = MAX_LEVELS;
Levels = 0;

/* parse cmd line arguments */
Strict = Dont_rmdir = Forced = Interactive = Recursive = Verbose = FALSE;
error = 0;

if (getenv("POSIX_STRICT")!=NULL) Strict=TRUE;

while ((opt = getopt(argc,argv,"fiRr**l:dvV")) != -1) {
	switch(opt) {
		case 'f':	Forced		=   TRUE;
					if (Strict) Interactive = FALSE;
					break;
		case 'i':	Interactive	=	TRUE;
					if (Strict) Forced = FALSE;
					break;
		case 'r':
		case 'R':	Recursive   =   TRUE;			break;
		case 'l':	Max_Levels  =	atoi(optarg);	break;
		case 'd':	Dont_rmdir	=	TRUE;			break;			
		case 'v':
		case 'V':	Verbose 	= 	TRUE;			break;

		default:	error++; break;
	}
}


#ifdef DIAG
	printf("After getopt parsing: optind = %d, argc = %d\n",optind,argc);
#endif

first_index = optind;
last_index  = argc-1;   

num_files = last_index - first_index + 1;

if (error) exit(EXIT_FAILURE);

if (num_files<1) {
	fprintf(stderr,TXT(T_MUST_SPEC_FILE));
	exit(EXIT_FAILURE);
}

#ifdef DIAG
	fprintf(stderr,"%s %s %s\n",Forced?"Forced":"",Interactive?"Interactive":"",
			Recursive?"Recursive":"");
#endif

for (index = first_index; index <=last_index ; index++) {
	/* for each file/directory */	
       if (strlen(argv[index]) <= PATH_MAX) {
               strcpy(Main_path,argv[index]);
               statbuf_valid = FALSE;
               if (remove_guy(Main_path)) error++;
       } else {
               fprintf(stderr,TXT(T_LONG_FILENAME),argv[index]);
               error++;
       }

}			
				
return(error?EXIT_FAILURE:EXIT_SUCCESS);
}

/*
-------------------------------------------------------- prompt(string) -----
*/


int16_t prompt (string)
char *string;
{
int answer;
int c;
char buffer[80];

fprintf(stderr,"%s? (y/N) ",string);

errno = 0;
answer = 'N';

fflush(stderr); fflush(stdout);
if (fgets(buffer,sizeof(buffer),stdin)) c=buffer[0];
else c=0;

switch(c) {
	case  'Y':
	case  'y':
	case  'N':
	case  'n':	answer=toupper(c); c=0;	break;
	case '\n':
	case  EOF:	break;

	default:	c=0; break;
}

return((answer=='Y')?SUCCESS_RC:FAILURE_RC);
}

static void remove_filename( char *path )
{
char	*last;

last = NULL;
for( ;; ) {
	if( *path == '\0' ) break;
	if( IS_DIRSEP( *path ) ) last = path;
	++path;
}
if( last != NULL ) *last = '\0';
}

/*
--------------------------------------- remove_directory_recursive(path) ----
*/

int16_t remove_directory_recursive(path)
register char *path;
{
DIR *dirp;				
struct dirent *entry;
int16_t rc = 0;
char c;

#ifdef DIAG
	fprintf(stderr,"Levels = %d, Max_Levels = %d\n",Levels, Max_Levels);
#endif

if (Levels >= Max_Levels) {
	fprintf(stderr,TXT(T_TOO_MANY_LEVELS),Max_Levels);
	return(SUCCESS_RC);			/* not a failure, its
									what we were axed to do*/
}

Levels++;

#ifdef DIAG
	fprintf(stderr,"rem_dir_recursive(%s)\n",path);
#endif

if (!(dirp=opendir(path))) {
	fprintf(stderr,TXT(T_CANT_OPEN_DIRECTORY));
	perror(path);
	Levels--;
	return(FAILURE_RC);
}

#ifdef DIAG
	fprintf(stderr,"opendir(%s) succeeded.\n",path);
#endif

while (errno=0, entry=readdir(dirp) ) {
	#ifdef DIAG
		fprintf(stderr,"got '%s'\n",entry->d_name);
	#endif

	if (entry->d_name[0] == '.') {
		if (!entry->d_name[1]) continue;
		if ((entry->d_name[1]=='.') && (!entry->d_name[2])) continue;
	}

	if (errno) {
		fprintf(stderr,TXT(T_CANT_READ_DIR_ENTRY));
		perror(path);
		break;
	}

	statbuf_valid=FALSE;
	#ifdef DO_STAT_OPT
		if (lstat_optimize(entry, &Statbuf)!=-1) statbuf_valid=TRUE;
	#endif

	#ifdef DIAG
		fprintf(stderr,"statbuf_valid = %s\n",statbuf_valid?"TRUE":"FALSE");
	#endif

	/* add filename to end of path */
	c = path[strlen(path)-1];
	if ( !IS_DIRSEP( c ) ) {
		#ifdef DIAG
			fprintf(stderr,"path = '%s', filename = '%s'\n",path,entry->d_name);
		#endif

		strcat(path,"/");
		strcat(path,entry->d_name);

		#ifdef DIAG
			fprintf(stderr,"path = '%s'\n",path);
		#endif

		/* remove that file  - rc end up non-zero if an error occurred removing
		   something in this directory */
		rc |= remove_guy(path);

		/* remove filename from path */
		remove_filename( path );
		#ifdef DIAG
			fprintf(stderr,"after remove_guy() path = '%s'\n",path);
		#endif
	} else {
		#ifdef DIAG
			fprintf(stderr,"SPL CSE (.../): path = '%s', filename = '%s'\n",path,entry->d_name);
		#endif

		strcat(path,entry->d_name);

		#ifdef DIAG
			fprintf(stderr,"SPL CSE (.../): path = '%s'\n",path);
		#endif

		/* remove that file  - rc end up non-zero if an error occurred removing
		   something in this directory */
		rc |= remove_guy(path);
		remove_filename( path );
		#ifdef DIAG
			fprintf(stderr,"SPL CSE (.../): after remove_guy() path = '%s'\n",path);
		#endif
	}
}

if (errno) {
	save_errno=errno;
	fprintf(stderr,TXT(T_READDIR_ERROR));
	errno=save_errno;
	perror(path);
}

#ifdef DIAG
	fprintf(stderr,"out of readdir loop\n");
#endif

if (closedir(dirp)==-1) {
	save_errno=errno;
	fprintf(stderr,TXT(T_CLOSEDIR_ERROR));
	errno=save_errno;
	perror(path);
}

Levels--;

if (!errno && !rc) {
	if (Dont_rmdir) return(SUCCESS_RC);

	if (Interactive) {
		fprintf(stderr,TXT(T_DIR_VERIFY));
		if (prompt(path)) {
			if (Verbose) fprintf(stdout,TXT(T_DIR_SKIPPING),path);
			return(FAILURE_RC);
		}
	}

	if (Verbose)
		fprintf(stdout,TXT(T_REMOVING_DIR),path);

	if(rmdir(path)==-1) {
		fprintf(stderr,TXT(T_RMDIR_ERROR));
		perror(path);
		return(FAILURE_RC);
	} else return(SUCCESS_RC);
} else	return(FAILURE_RC);
}	


/*
----------------------------------------------------- remove_guy(path) ------
*/

int16_t remove_guy (char *path)
{
int lookat = S_IWOTH;

#ifdef DIAG
	fprintf(stdout,"remove_guy(%s)\n",path);
#endif


if (path[0] == '.' && path[1]=='.' && !path[2]) {
	fprintf(stderr,TXT(T_CANT_RM_DOTDOT));
	return(FAILURE_RC);
}

if (path[0] == '.' && !path[1]) {
	fprintf(stderr,TXT(T_CANT_RM_DOT));
	return(FAILURE_RC);
}

if (!statbuf_valid && (lstat(path,&Statbuf)==-1)) {
	/* if we are forced, only print an error if the errno is
	   something OTHER than ENOENT. (See 1003.2 description of
	   the -f option */
	if ((!Forced) || (errno!=ENOENT)) {
		perror(path); 
		return(FAILURE_RC);
	} else
		return(SUCCESS_RC);	/* forced, or error was other than ENOENT */
}

#ifdef DIAG
fprintf(stderr,"ok, now check for dir..\n");
#endif

if (S_ISDIR(Statbuf.st_mode)) {
	/* path is a directory */
	if (Recursive) {
		return(remove_directory_recursive(path));
	}

	fprintf(stderr,TXT(T_CANTRECURSE),path);
	return(FAILURE_RC);
}


/* if we don't have write access, and we aren't Forced, do the
   warning/prompt thing */
{

#if !defined(__NT__) && !defined(__MINGW32__)
	if (Statbuf.st_uid==geteuid()) lookat=S_IWUSR;
	else if (Statbuf.st_gid==getegid()) lookat=S_IWGRP;
#endif

	if (((Statbuf.st_mode&lookat)==0) && (!Forced)) {

		/* if we are Attended or Interactive we want to pause */
		if (Attended || Interactive) {
			fprintf(stderr,TXT(T_NOWRITEWARNINGFMT),path);
			/* If user says to skip it, return */
			if (prompt(TXT(T_DELETEANYWAY))==-1) {
				if (Verbose) fprintf(stdout,TXT(T_SKIPPING),path);
				return(FAILURE_RC);
			}
		} else {						
			fprintf(stderr,TXT(T_NOWRITESKIP),path);
			/* and skip this file */
			return(FAILURE_RC);
		}
	} else 	if (Interactive) {
		fprintf(stderr,TXT(T_VERIFY));
		if (prompt(path)) {
			if (Verbose) fprintf(stdout,TXT(T_SKIPPING),path);
			return(FAILURE_RC);
		}
	}
}

if (Verbose) fprintf(stdout,TXT(T_REMOVING),path);

#ifdef __NT__
if ((Statbuf.st_mode&lookat)==0) {
	/* have to chmod the file under Win32 */
	chmod( path, 0666 );
}
#endif
if (unlink(path)==-1) {
	fprintf(stderr,TXT(T_CANT_UNLINK));
	perror(path);
	return(FAILURE_RC);
} else return(SUCCESS_RC);
}

