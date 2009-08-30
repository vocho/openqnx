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





#ifdef __USAGE
%-ln
%C	- create links to (aliases for) files (POSIX)

%C	[-f|-i] [-Psv] source_file target_file
%C	[-f|-i] [-Psv] source_file... target_directory

Options:
 -f       Force existing destinations to be removed before linking
          without prompting for confirmation.
 -F       When creating process manager symlinks (-P), ignore existence
          of target files/directories.
 -i       Interactive. Write a prompt to stderr requesting confirmation
          for each link that would overwrite an existing file.
 -P       Create link in process manager prefix tree.
 -s       Create a symbolic link
 -v       Verbose. Write actions performed to standard output.

Where:
 source_file is an already existing file, and
 target_file is the filename for the new link.

Note:
 If neither -f nor -i is specified, ln will not unlink an existing file. (i.e.
 The specified target file must not exist.)
%-link
%C     - Creates a symbolic link

Usage:
%C src_file dest_file

#endif

#ifdef __USAGENTO
%-ln
%C	- create links to (aliases for) files (POSIX)

%C	[-f|-i] [-Psv] source_file target_file
%C	[-f|-i] [-Psv] source_file... target_directory

Options:
 -f       Force existing destinations to be removed before linking
          without prompting for confirmation.
 -i       Interactive. Write a prompt to stderr requesting confirmation
          for each link that would overwrite an existing file.
 -P       Create link in process manager prefix tree.
 -s       Create a symbolic link
 -v       Verbose. Write actions performed to standard output.

Where:
 source_file is an already existing file, and
 target_file is the filename for the new link.

Note:
 If neither -f nor -i is specified, ln will not unlink an existing file. (i.e.
 The specified target file must not exist.)
%-link
%C     - Creates a symbolic link

Usage:
%C src_file dest_file

#endif

/*
-----------------------------------------------------------------------
POSIX UTILITY: LN
1003.2 -- Draft 12 (Shell and Utilities working group)
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
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#define NO_EXT_KEYS
#include <stdlib.h>
#include <util/stdutil.h>

#ifdef __QNXNTO__
#include <sys/pathmgr.h>
#endif
#include <libgen.h>

/* my extensions */
#include <util/defns.h>

#define EXTRA_OPTS		"PF"

char *purty (char *string);

/*
------------------------------------------------------------- defines ------
*/

#define TXT(s)			  s

#define T_MISSING_OPERANDS		"ln: Missing operand(s) (source or destination file).\n"
#define T_NODASHFDASHI			"ln: -f and -i may not be used at the same time.\n"
#define T_STAT_FAILED 			"ln: stat() failed. "
#define T_LSTAT_FAILED 			"ln: lstat() failed. "
#define T_ERRMSG_PREFIX 		"ln: "
#define T_LINK_ERROR			"ln: unable to link. "
#define T_TARGET_MUST_BE_DIR	"ln: target must be a directory to link multiple files.\nln: %s is not a directory.\n"
#define T_CANT_LINK_DIRECTORIES "ln: QNX does not support hard links to directories.\nln: %s is a directory.\n"
#define T_THISORTHAT			"ln: unable to link. (%s or %s)"
#define T_ON_DIFF_FILSYS		"ln: %s and %s are on different file systems -\nCan't create hard link. Use ln -s or make a copy.\n"
#define T_PROMPT				"ln: Must unlink destination (%s). Do it? "
#define T_DEST_EXISTS			"ln: Target file '%s' already exists. Use ln -f or -i to overwrite.\n"
#define T_CANT_UNLINK			"ln: Couldn't unlink destination file.\n"
#define T_DEST_IS_DIR			"ln: Destination path is an existing directory."
#define T_LINK_WRONG_NUM_OF_ARGS	"link: Wrong number of arguments.\n"

/*
------------------------------------------------------------- global vars ---------
*/

#define BIG_FAT_BUFFER 4096

int	prefix=0;
int force_del=0, interactive=0, symbolic=0,ignore_exist=0;
int directory_target=0;

char *target;					/* pointer to string of target name   */
char *source, dest[BIG_FAT_BUFFER+1];		/* src and dest pathnames. dest must be a
								   buffer since in the case of creating a link
								   under an existing target directory, we cat the
								   name of the link to the destination directory
								   name. */
struct stat statbuf;			/* used for getting file statuses     */
struct stat statbufdst;			/* used for getting file statuses     */
int error=0;					/* count of errors encountered        */
int	verbose;					/* verbose flag						  */

/*
------------------------------------------------------------- prototypes ---
*/

void link_guy (char *, char *);
int proc_link(char *src, char *dst);

/*
------------------------------------------------------------- main ---------
*/

int main (int argc,char **argv)
{
	int opt;
	int file;
	int num_files=0;
	int first_index, last_index; 	/* index in argv of first and last files to
	                                   be copied */

	if(strcmp(basename(argv[0]), "link") == 0) {
		if (argc != 3) {
			fprintf(stderr, TXT(T_LINK_WRONG_NUM_OF_ARGS));
			exit(1);
		}
		if (link(argv[1], argv[2]) != 0) {
			fprintf(stderr, "link: %s\n", strerror(errno));
			exit(1);
		}
		exit(0);
	}

	while ((opt = getopt(argc,argv,"fisv" EXTRA_OPTS)) != -1) {
		switch(opt) {
			case 'v':	verbose 	=	TRUE;	break;
			case 'f':	force_del   =   TRUE;	break;
			case 'F':   ignore_exist=   TRUE;   break;
			case 'i':	interactive =   TRUE;	break;
			case 's':	symbolic    =   TRUE;	break;
			case 'P':	prefix		=	TRUE;	break;
			default:	error++;				break;
		}
	}

	first_index = optind;
	last_index  = argc-2;   
	num_files	= last_index - first_index + 1;

	if (num_files<1) {
		fprintf(stderr,TXT(T_MISSING_OPERANDS));
		error++;
	}
	
	if (force_del && interactive) {
		fprintf(stderr,TXT(T_NODASHFDASHI));
		error++;
	}

	if (error) exit(EXIT_FAILURE);

	/* target is last file specified on cmd line. If it is a directory, we will be
	   setting the directory_target flag and creating links to all the other files on
       the cmd line _in_ that directory */

	target = argv[argc - 1];

	if (lstat(target,&statbuf)==-1) {
		/* target does not (yet) exist */
		if (errno!=ENOENT) {
			/* exit with failure if target cannot be lstat'ed for some reason other
			   than that it does not exist */
			prerror("%s %s",TXT(T_LSTAT_FAILED),target);
			exit(EXIT_FAILURE);
		}
	} else if (S_ISDIR(statbuf.st_mode))  directory_target = TRUE;

	/* check that if we are creating multiple links, the destination for these
       links is an existing directory */

	if ((num_files>1) && (!directory_target)) {
		fprintf(stderr,TXT(T_TARGET_MUST_BE_DIR),target);
		exit(EXIT_FAILURE);
	}

	for (file = first_index; file <=last_index ; file++) {
		source=argv[file];	/* this is safe, we never munge what this points to */

		/*
		*	get a fresh copy of dest each time, since if it is a directory we will
		*	have tacked on a /filename the previous iteration
		*/
		strcpy(dest,target);

		/*
		*	Symbolic links can link to _anything_, whether it exists or not. So
		*	perform these checks only for hard links
		*/				
		if (!symbolic && !prefix) {

			/*
			*	we _stat_ the src, as opposed to _lstat_, because when we link a
			*	symbolic link, we link the destination of the link, not the link
			*	itself. (This is what Sun does.)
			*/
			if (stat(source,&statbuf)==-1) {
				if (errno==ENOENT) prerror("%s %s",TXT(T_ERRMSG_PREFIX),source);
				else prerror("%s %s",TXT(T_STAT_FAILED),source);
				error++;	
				continue;
			}
		
#ifdef PROHIBIT_DIRECTORY_HARDLINKS
  			if (S_ISDIR(statbuf.st_mode)) {
				/* src is a directory - can't do it */
				fprintf(stderr,TXT(T_CANT_LINK_DIRECTORIES),source);
				error++;
				continue;
			}
#endif
		}

		if (directory_target && !prefix) {
			char *ptr;

			strcat(dest,"//");		

			/* catenate basename portion of source to dest */
			if ((ptr=strrchr(source,'/'))) strcat(dest,ptr+1);
			else strcat(dest,source);

			/* check if the dest as built is a directory. If it is,
			*  this is an error, whether we are going to create a hard
			*  or symbolic link of the same name. Why is obvious -
			*  we can't unlink() a directory
			*/

			if (lstat(dest,&statbufdst)==-1) {
				if (errno!=ENOENT) {
					prerror("%s %s",TXT(T_STAT_FAILED),dest);
					error++;
					continue;
				}
				/* else destination doesn't exist, which is cool */
			} else if (S_ISDIR(statbufdst.st_mode)) {
				fprintf(stderr,"%s (%s)\n",TXT(T_DEST_IS_DIR),dest);
				error++;
				continue;
			}
		} /* if a directory target, create real dest, check resulting filename */

		link_guy(source,dest);
	} /* for each file */
					
	exit(error?EXIT_FAILURE:EXIT_SUCCESS);
}

/*
------------------------------------------------------------- link_guy -----
*/

void link_guy (char *src, char *dst)
{
	char answer[80];

	/* this is only called with two paths to real files */
	
	errno = 0;

	/* check for destination file existing and all that stuff */

	if (!ignore_exist) {
      if (lstat(dst,&statbufdst)!=-1) {
		if (!symbolic) {
			/* if src and dst on different filesystems, can't link */
			if (statbufdst.st_dev!=statbuf.st_dev) {
				errno = EXDEV;
				fprintf(stderr,TXT(T_ON_DIFF_FILSYS),src,purty(dst));
				error++;
				return;
			}

			/* if the files have the same ino, then they are the same file,
			   i.e. are already a link to each other! */
			if (statbufdst.st_ino == statbuf.st_ino) {
				if (verbose) 
					fprintf(stderr,"ln: '%s' is already a link to '%s' - skipping\n",
							purty(dst), src);
				return;
			}
		}

		/* Destination exists. Bummer. */
		if (!prefix  &&  !force_del) {
			if (interactive) {
				if (isatty(fileno(stdin))||interactive) {
					fprintf(stderr,TXT(T_PROMPT),purty(dst));
					fflush(stderr);
					fgets(answer,sizeof(answer),stdin);
					if ( (strncmp(answer,"yes",strlen(answer)-1)) &&
						 (strncmp(answer,"YES",strlen(answer)-1)) )  return;
				}
			} else {
				error++;
				fprintf(stderr,TXT(T_DEST_EXISTS),purty(dst));
				return;
			}
		}

		if (verbose) {
			if(prefix)
				printf("ln: '%s' replaced by new procmgr link\n",
						purty(dst));
			else
				printf("ln: unlinking '%s', to be replaced by new link\n",
						purty(dst));
		}

		/* unlink the destination file */
		if(!prefix){
			if (unlink(dst)==-1) {
				if (errno != ENOENT) {
					prerror("%s %s",TXT(T_CANT_UNLINK),purty(dst));
					error++;
					return;
				}
			}
		}
		else if (force_del){
			pathmgr_unlink(dst);
		}
	  } 
    } /* not ignore_exist */

	if (verbose) {
		if (symbolic) {
			printf("ln: creating symbolic link '%s' pointing to '%s'\n",
					purty(dst), src);
		} else {
			printf("ln: creating hard link '%s' to existing file '%s'\n",
					purty(dst), src);
		}
	}

	if ((prefix ?	(symbolic ? pathmgr_symlink(src, dst): proc_link(src, dst)) :
					(symbolic ? symlink(src,dst) : link(src,dst))) == -1) {
/*
	if ((symbolic ? symlink(src,dst) : link(src,dst))==-1) {
*/
		switch(errno) {
			case EEXIST:
			case ENOSPC:
			case EROFS:
				prerror("%s %s",TXT(T_LINK_ERROR),purty(dst));
				break;

			case EXDEV:
				fprintf(stderr,TXT(T_ON_DIFF_FILSYS),src,purty(dst));
				break;

			default:			
				prerror(TXT(T_THISORTHAT),src,purty(dst));
				break;
		}
		error++;
    } 
}


int proc_link(char *src, char *dst) {
	if(prefix  &&  !symbolic) {	// Temp code till we make this work...
		fprintf(stderr, "Only symbolic prefix links are currently supported.\n");
		fprintf(stderr, "Specify -s with -P.\n");
		++error;
	}

//	In the future we parse src into nid, pid, chid, handle.
//	flags |= PROCMGR_STICKY;
//	return procmgr_link(dst, nid, pid, chid, handle, file_type, flags);

	if (error) return -1;
	return 0;
}


/*------------------------------------------------------------- purty -------*/
/* strips multiple '/'s */
char *purty (char *string)
{
	static char purtystr[BIG_FAT_BUFFER+1];
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
