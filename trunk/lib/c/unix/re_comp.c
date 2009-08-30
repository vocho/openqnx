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




#include <regex.h>
#include <string.h>

static regex_t re;
static char rs[40];
static int rc;

char *re_comp(char *s) {
    if ((rc = regcomp(&re, s, REG_EXTENDED | REG_NEWLINE | REG_NOSUB)) == REG_OK)
	return 0;
    (void)regerror(rc, &re, rs, sizeof rs);
    return rs;
}

int re_exec(char *s) {
    if (rc != REG_OK)
	return -1;
    return regexec(&re, s, 0, 0, 0) == REG_OK ? 1 : 0;
}

__SRCVERSION("re_comp.c $Rev: 153052 $");
