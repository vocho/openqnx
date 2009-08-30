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
 * This program tests the ability to limit the size of a core 
 * dump file.  If the "-nocore" option is given on the command
 * line then the program calls setrlimit() to set the core
 * dump size to zero. After
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>



int
main(int argc, char **argv)
{
   struct rlimit lim;
   int x, i;

   if (argc > 1 && strcmp(argv[1], "-nocore") == 0) {
      lim.rlim_cur = lim.rlim_max = 0;
      x = setrlimit(RLIMIT_CORE, &lim);
      if (x != 0) {
	 printf("%s: setrlimit failed (%s)!\n", argv[0], strerror(errno));
	 errno = 0;
      }
   }

   x = getrlimit(RLIMIT_CORE, &lim);
   if (x == 0) {
      if (lim.rlim_cur == 0) {
	 printf("%s: Core dumps not allowed.\n", argv[0]);
      } else {
	 printf("%s: Core dumps are allowed.\n", argv[0]);
      }
   } else {
      printf("getrlimit failed (%s)!\n", strerror(errno));
      errno = 0;
   }
   fflush(stdout);   /* make sure it has all been printed */

   /* woohoo! let's crash this baby! */
   *(int *)0 = 0xc0debabe;

   return 0;
}

__SRCVERSION("crash.c $Rev: 153052 $");
