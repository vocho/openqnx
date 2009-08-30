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
%C     - Unlinks (removes) a file

Usage:
%C file_to_unlink

#endif


#include <stdio.h>
#include <unistd.h>

void usage(char* name)
{
  fprintf(stderr, "Usage: %s file_to_unlink\n", name);
}

int main(int argc, char **argv)
{
  int retval, opt;

  while ((opt = getopt(argc, argv, "")) != -1) 
  {
    switch(opt)
    {
      case '?':
      default : usage(argv[0]);
                return 1;
    }
  }
  if (optind+1 <= argc && argc >= 2)
  {
    if ((retval = unlink(argv[optind])) == -1)
      perror(argv[0]);
  }
  else
  {
    usage(argv[0]);
    retval = 1;
  }

  return retval;
}
