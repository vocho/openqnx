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



/*-
 displays a file on stdout



 $Log$
 Revision 1.3  2005/06/03 01:22:46  adanko
 Replace existing QNX copyright licence headers with macros as specified by
 the QNX Coding Standard. This is a change to source files in the head branch
 only.

 Note: only comments were changed.

 PR25328

 Revision 1.2  1999/04/02 20:15:18  bstecher
 Clean up some warnings

 Revision 1.1  1997/02/18 16:50:05  kirk
 Initial revision

*/
#include <unistd.h>
#include <fcntl.h>
#include "login.h"

int
cat_file(char *fname)
{
	int	fd;
	int	n;
	char	buf[1024];
	if ((fd=open(fname,O_RDONLY)) == -1)
		return 0;
	while ((n=read(fd,buf,sizeof buf)) > 0)
		write(1,buf,n);
	close(fd);
	return 1;
}
