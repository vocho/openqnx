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





/*
#ifdef __USAGE
%C - move files (POSIX)

%C	[-f|-i] [-V|-v] source_file target_file
	[-f|-i] [-V|-v] source_file... target_dir
Options:
 -f       Force existing destinations to be removed before moving
          without prompting for confirmation.
 -i       Interactive. Write a prompt to stderr requesting confirmation
          for each move which would overwrite an existing file.
 -v | -V  Verbose. -V is extra verbose.
#endif
*/

/*
-----------------------------------------------------------------------
POSIX UTILITY: MV

1003.2 -- Draft 9 (Shell and Utilities working group)

-----------------------------------------------------------------------
*/

/*
------------------------------------------------------------- includes -----
*/

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <utime.h>
#include <stdlib.h>
#include <string.h>
#ifndef __MINGW32__
#include <sys/disk.h>
#else
#include <lib/compat.h>
#endif
#include <process.h>

/* my extensions */
#include <util/defns.h>
#include <util/stdutil.h>
/*
------------------------------------------------------------- defines ------
*/

#define TXT(s)			  s

#define T_MISSING_OPERAND		"mv: Missing operand (source or destination file(s))\n"
#define T_TARGET_MUST_BE_DIR	"mv: Target (%s) must be a directory in order to move\nmv: directories or multiple files to it.\n"
#define T_SOURCE_AINT_THERE		"mv: Source file not found. "
#define T_BLKCHARSPECIAL		"mv: Block & character special files may not be moved. "
#define T_STAT_FAILED 			"mv: stat() failed. "
#define T_FROM_TO				"mv: %s --> %s\n"
#define T_OVERWRITE 			"mv: Destination exists! Overwrite? (y/N) "
#define T_NOWRITEFORCE			"mv: No write permission for destination. Unlink first? (y/N) "
#define T_CANT_UNLINK			"mv: Can't unlink file %s : %s\n"
#define T_SKIPPING				"mv: Skipping %s\n"
#define T_VERBOSE_MSG			"mv: Moving %s to %s\n"

/*
--------------------------------------------------------------- GLOBALS -----
*/

int strict=0;	/* will set when POSIX_STRICT */
int num_files;
int force_del, interactive, verbose, super_verbose, draft11;
int directory_target, target_index,target_exists, dst_exists;

char *target;			/* pointer to string of target file/directory name	*/

char source[_POSIX_PATH_MAX+1], dest[_POSIX_PATH_MAX+1];	/* src and dest pathnames					*/
char destpathfull[_POSIX_PATH_MAX+1];	/* full destination path if a directory		*/
char temppathfull[_POSIX_PATH_MAX+1];	/* buffer for obtaining full paths			*/
char tmp2pathfull[_POSIX_PATH_MAX+1];	/* 2nd buffer for obtaining full paths		*/

/* next three for spawning stuff */
char arg1[32];					/* options */
char arg2[_POSIX_PATH_MAX+1];	/* filename */
char arg3[_POSIX_PATH_MAX+1];   /* filename */

struct stat statbufsrc;			/* used for getting file statuses			*/
struct stat statbufdst;			/* used for getting file statuses			*/

int in, out;					/* input and output file descriptors		*/

int errs = 0;					/* # errors encountered in copy operation	*/

/*
----------------------------------------------------------- prototypes -----
*/

#ifdef __STDC__
bool ask_user(char *src_filename,char *dst_filename,char *format);
void copy_directory_recursive(char *src,char *dst);
#endif

/*
--------------------------------------------------------- interrogation code -
*/

bool ask_user(src_filename,dst_filename,format)
char *src_filename;
char *dst_filename;
char *format;
{
	char answer[80];

	fprintf(stderr,TXT(T_FROM_TO),src_filename,dst_filename);
	fprintf(stderr,format,src_filename,dst_filename);
	fflush(stdout);
	fflush(stderr);
	fgets(answer,sizeof(answer),stdin);
	if (strlen(answer)>1) {
		if (!strncmp(answer,"yes",strlen(answer)-1)) return(TRUE);
		if (!strncmp(answer,"YES",strlen(answer)-1)) return(TRUE);
	}
	fprintf(stderr,TXT(T_SKIPPING),src_filename);
	fflush(stderr);
	return(FALSE);
}

/*
------------------------------------------------------------- main ---------

	See POSIX 1003.2 draft 9 for functional description of utility.

----------------------------------------------------------------------------
*/

int main (int argc, char *argv[])
{
	int error, opt;
	int file;
	int first_index, last_index; 	/* index in argv of first and last files to
	                                   be copied */
	char *tack_point;
	bool onatty;

	error = 0;
	force_del = interactive = draft11 = FALSE;


	while ((opt = getopt(argc,argv,"fi**Vv1")) != -1)
	{
		switch(opt)
		{
			case '1': 
				draft11 	=	TRUE;
				break;

			case 'f':
				force_del   =   TRUE;
				if (draft11) interactive = FALSE;
				break;

			case 'i':
				interactive =   TRUE;
				if (draft11) force_del = FALSE;
				break;

			case 'V':
				super_verbose = TRUE;
			case 'v':
				verbose		=	TRUE;
				break;

			default:
				error++;
				break;
		}
	}

	strict = POSIX_STRICT;

	onatty = isatty(fileno(stdin));

	first_index = optind;
	last_index  = argc-2;   

	num_files = last_index - first_index + 1;

	target = argv[argc - 1];

	/*
		there must be at least two cmd line parameters
	*/
	if (num_files<1) fprintf(stderr,TXT(T_MISSING_OPERAND));


	if ((num_files<1) || error) exit(EXIT_FAILURE);

	#ifdef DIAG
		fprintf(stderr,"%s %s\n",force_del?"force_del":"",interactive?"interactive":"");
	#endif

	/*
		does TARGET(destination) exist ?
	*/

	if (stat(target,&statbufdst)==-1) 
	{
		directory_target = FALSE;
		if (errno!=ENOENT)
		{
			fprintf(stderr,"%s (%s):%s\n",TXT(T_STAT_FAILED),target,strerror(errno));
			exit(EXIT_FAILURE);
		} else target_exists = FALSE;
	} else {	
		/*
			TARGET file exists -- is it a directory?
		*/

		target_exists = TRUE;

		if (S_ISDIR(statbufdst.st_mode)) directory_target = TRUE;
		else directory_target = FALSE;
	}

	if ((num_files > 1) && (!directory_target))
	{
		/*
			we can't do this - when copying multiple files,
			gotta have directory destination
		*/

		fprintf(stderr,TXT(T_TARGET_MUST_BE_DIR),target);
		exit(EXIT_FAILURE);
	}

	/* for each file/directory to be moved */

	for (file = first_index; file <=last_index ; file++) {
		strncpy(source,argv[file],_POSIX_PATH_MAX);
		strncpy(dest,target,_POSIX_PATH_MAX);
		source[_POSIX_PATH_MAX] = '\0';
		dest[_POSIX_PATH_MAX] = '\0';

		if (lstat(source,&statbufsrc)==-1) {
			if (errno!=ENOENT) {
				fprintf(stderr,"%s (%s):%s\n",TXT(T_STAT_FAILED),source,strerror(errno));
			} else {
				fprintf(stderr,"%s (%s)\n",TXT(T_SOURCE_AINT_THERE),source);
			}
			errs++;
			continue;
		}

		if (!strict) {
			if (S_ISCHR(statbufsrc.st_mode) || S_ISBLK(statbufsrc.st_mode)) {
				fprintf(stderr,"%s (%s)\n",TXT(T_BLKCHARSPECIAL),source);
				errs++;
				continue;
			}
		}

		/* If we are copying to a directory, build full destination path */ 

		if (directory_target) {
			char *ptr;
			tack_point = dest+strlen(dest);

			strcat(dest,"//");		
			while ((ptr=strrchr(source,'/')) != NULL) {
				if (*(ptr+1)) {
					strcat(dest,ptr+1);
					break;
				} else *ptr=0;
			}

			if (!ptr) strcat(dest,source);
		} else tack_point = NULL;

#ifdef DIAG
		fprintf(stderr,"DIAG: dest='%s', dir_target = %d \n",dest,directory_target);
#endif

		/* does the destination exist? */
		if (stat(dest,&statbufdst)!=-1) 
		{
			bool access_ok;

			dst_exists = TRUE;

			if (!strict) {
				if (S_ISCHR(statbufdst.st_mode) || S_ISBLK(statbufdst.st_mode)) {
					fprintf(stderr,"%s (%s)\n",TXT(T_BLKCHARSPECIAL),purty(dest));
					errs++;
					continue;
				}
			}
			
			access_ok = (access(dest,W_OK)!=-1);
			if ((interactive && (!draft11 || onatty)) || ((!access_ok)&&(!force_del)&&onatty) ) {
				if (ask_user(source,purty(dest),access_ok?TXT(T_OVERWRITE):TXT(T_NOWRITEFORCE))
					== FALSE) continue;
			} 
			if (!access_ok) {				
				if (unlink(dest)==-1)
				{
					fprintf(stderr,TXT(T_CANT_UNLINK),purty(dest),strerror(errno));
					continue;
				}
			}
		} else {
			dst_exists = FALSE;
		}

		if (verbose) fprintf(stdout,TXT(T_VERBOSE_MSG),source,purty(dest));
		
		if (rename(source,dest)==-1) {
			error = errno;
		} else {
			error = EOK;
		}

		if (error)
		{
			if (error==EXDEV)
			{
				if (dst_exists)
				{
					if (S_ISDIR(statbufsrc.st_mode) && !S_ISDIR(statbufdst.st_mode))
					{
						fprintf(stderr,"mv: cannot overwrite non-directory %s with directory\n",purty(dest));
						errs++;
						continue;
					}
					if (!S_ISDIR(statbufsrc.st_mode) && S_ISDIR(statbufdst.st_mode))
					{
						fprintf(stderr,"mv: cannot overwrite directory %s with non-directory\n",purty(dest));
						errs++;
						continue;
					}
				}

				if (super_verbose)
					fprintf(stdout,"mv: %s and %s are on different devices\n",source,purty(dest));

				if (unlink(dest)==-1)
				{
					if ((errno!=ENOENT) && (errno!=EPERM))
					{
						fprintf(stderr,"mv: unlink(%s) : %s\n",purty(dest),strerror(errno));
						errs++;
						continue;
					} 
				}
			
				/* if (tack_point!=NULL) *tack_point = 0; */

				sprintf(arg1,"-pLRD%s%s%s",force_del?"f":"",interactive?"i":"",super_verbose?"V":(verbose?"v":""));
				sprintf(arg2,"%s",source);
				sprintf(arg3,"%s",purty(dest));
				if (super_verbose)
					fprintf(stdout,"mv: spawning '/bin/cp %s %s %s'\n",arg1,arg2,arg3);
				errno=0;
				if (spawnl(P_WAIT,"/bin/cp","cp",arg1,arg2,arg3,NULL)==0) {
					sprintf(arg1,"-rf%s",verbose?"v":"");
					sprintf(arg2,"%s",source);

					if (super_verbose)
						fprintf(stdout,"mv: spawning '/bin/rm %s %s'\n",arg1,arg2);
					errno=0;
					if (spawnl(P_WAIT,"/bin/rm","rm",arg1,arg2,NULL)) {
						errs++;
						if (errno) 
							fprintf(stderr,"mv: error spawning /bin/rm (%s)\n",strerror(errno));
						else
							fprintf(stderr,"mv: /bin/rm failed.\n");
					}
				} else {
					if (errno) 
						fprintf(stderr,"mv: error spawning /bin/cp (%s)\n",strerror(errno));
					else
						fprintf(stderr,"mv: /bin/cp exited with non-zero status. Original will not be removed.\n");

					errs++;
				}
			} else {
				fprintf(stderr,"mv: rename(%s,%s) : %s\n",source,purty(dest),strerror(errno));
				errs++;
			}
		}
	}
	return errs?EXIT_FAILURE:EXIT_SUCCESS;
}


/*------------------------------------------------------------- purty -------*/
/* strips multiple '/'s */
char *purty (char *string)
{
	static char purtystr[_POSIX_PATH_MAX+1];
	char *ptr=string, *prty = purtystr;
		
	/* copy over valid leading // only when not '///' */
	if (ptr[0]=='/'&&ptr[1]=='/'&&ptr[2]!='/') {
		*prty++=*ptr++;
		*prty++=*ptr++;
	}

	for (;*ptr;ptr++)	if (*ptr!='/'||*(ptr+1)!='/') *prty++=*ptr;
	*prty=*ptr;	/* null terminate */
	return(purtystr);
}
	

