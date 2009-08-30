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



#ifdef __MINGW32__
#include <lib/compat.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SKIP_TO_SLASH(p)	{for(;*p && *p!='/';p++);}
#define SKIP_BACK_TO_SLASH(p,e)	{for(;p!=e && *p!='/';p--);}

/*
	qnx_create_path_to(filename)

	Will create directories in the path as neccessary to make it possible to
	subsequently create <filename>.

	This call does not guarantee that the file could actually be
	written - just that the path does exist. (i.e. no checking of
	permissions is done 

	returns 0 on success. Returns -1 on failure.
*/

int qnx_create_path_to(fullpath)
char *fullpath;
{
	char *baseptr, *ptr, *stopptr, *endptr, was;
	struct stat buf;

	#ifdef DIAG
		fprintf(stderr,"In qnx_create_path_to(%s)\n",fullpath);
	#endif

	/* scan backwards until I have a path component which does exist */

	endptr  = fullpath+strlen(fullpath);
	stopptr = endptr;
	baseptr = fullpath;
	ptr		= fullpath;

	/* if starts with //, move ptr up to what follows the node */
	if (ptr[0]=='/' && ptr[1]=='/') {
		ptr+=2;
		SKIP_TO_SLASH(ptr);
	}
	#ifdef DIAG
		fprintf(stderr,"endptr -> '%s'\n",endptr);
		fprintf(stderr,"stopptr -> '%s'\n",stopptr);
		fprintf(stderr,"baseptr -> '%s'\n",baseptr);
		fprintf(stderr,"ptr -> '%s'\n",ptr);
	#endif

	while (stopptr!=ptr) {
		was = *stopptr;
		*stopptr = 0;
		#ifdef DIAG
			fprintf(stderr,"checking baseptr -> '%s'\n",baseptr);
			getchar();
		#endif

		if (stat(baseptr,&buf)!=-1) {
			*stopptr = was;
			#ifdef DIAG
				fprintf(stderr,"and it exists!\n");
			#endif
			break;
		}

    	*stopptr=was;
		#ifdef DIAG
			fprintf(stderr,"stopptr->'%s'\n",stopptr);
		#endif
		if (*stopptr=='/') stopptr--;
		SKIP_BACK_TO_SLASH(stopptr,ptr);
		#ifdef DIAG
			fprintf(stderr,"stopptr->'%s'\n",stopptr);
		#endif
	}
		
	/* baseptr points to whole path */	
	/* endptr points to end of string */		
	/* ptr points to path past node number, or path if no node # supplied */
	/* >>>> stopptr points to first non-existent part <<<< */

	if (*stopptr=='/') stopptr++;
	SKIP_TO_SLASH(stopptr);

	for (;*stopptr;)
	{
		was = *stopptr;
		*stopptr = (char)0x00;
		#ifdef DIAG
			fprintf(stderr,"#2 create_path_to - a - mkdir(%s)\n",baseptr);
			getchar();
		#endif
		if (mkdir(baseptr,S_IRWXU|S_IRWXG|S_IRWXO)==-1)
		{
			if (errno!=EEXIST) {	// such as for . or ..
				perror(baseptr);
				#ifdef DIAG
					getchar();
				#endif
				*stopptr = was;
				return(-1);
			}
		}
		*stopptr = was;
		stopptr++;
		SKIP_TO_SLASH(stopptr);
	}
	return(0);
}

#ifdef TEST_PROGRAM

main()
{
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("//0/usr/fred/haha/boo"));
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("/usr/fred/haha/boo/hoo"));
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("/usr/fred/haha2/boo"));
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("/usr/fred/haha/boo2/"));
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("/usr/fred/haha3/"));
	fprintf(stderr,"qnx_create_path_to  rc=%d\n",qnx_create_path_to("/us/fred/haha3/"));
}

#endif
