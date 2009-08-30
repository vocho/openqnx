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
 interact.c -- a few routines for value checked input
*/

#include <libc.h>
#include <stdarg.h>

#include "login.h"

int
inputval_range(char *prompt, long low, long hi, long def, long *res)
{
	long            value;
	char            buffer[100];

	while (1) {
		char           *p;
		value = def;
		sprintf(buffer, "%ld", value);
		if (get_string(prompt, buffer) == -1) {
			return -1;
		}
		value = strtol(buffer, &p, 0);
		if (*p != '\0') {
			printf("illegal character near %s\n", p);
			continue;
		}
		if (value >= low && value < hi) {
			break;
		}
		printf("error, input [%ld] not in range [%ld-%ld]\n",
			value, low, hi);
	}
	*res = value;
	return 0;
}


int
get_string(char *prompt, char *bufp)
{
	int	i;
	int	c=c;
	printf("%s (%s) ", prompt, bufp);
	for (i=0; i < 80 && ((c=getchar()) != EOF && c != '\n'); bufp[i++] = c)
		;
	if (c == EOF)
		return -1;
	if (i == 0)
		return strlen(bufp);
	bufp[i] = '\0';
	return i;
}

int 
getyn(char *prompt,...)
{
	va_list         vargs;
	int             c;
	va_start(vargs, prompt);
	vfprintf(stdout, prompt, vargs);
	fflush(stdin);
	while ((c = getchar()) != EOF) {
		if (toupper(c) == 'Y')
			return 1;
		if (toupper(c) == 'N')
			return 0;
	}
	return 0;
}
