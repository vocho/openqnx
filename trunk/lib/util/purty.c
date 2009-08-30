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
#include <limits.h>
#include <util/stdutil.h>

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
	
