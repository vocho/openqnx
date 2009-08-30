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
 * tputs - put the string cp out onto the terminal, using the function
 * outc. This should do padding for the terminal, but I can't find a
 * terminal that needs padding at the moment...
 */
int tputs(char *cp, int affcnt, int (*outc)(int)) {
    affcnt = affcnt;
    if (cp) {
	/* do any padding interpretation - left null for now */
	while ('0' <= *cp && *cp <= '9') cp++;
	if (*cp == '.') cp++;
	while ('0' <= *cp && *cp <= '9') cp++;
	if (*cp == '*') cp++;

	while (*cp) (*outc)(*cp++);
    }
    return 1;
}
