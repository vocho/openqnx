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





#include "filter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/neutrino.h>

int	termProcs = 0, allProcs = 0, sessionProcs = 0;
int fullListing = 0, longListing = 0;
int groupSession = 0, userEffective = 0;
char	*groupList, *procList, *termList, *userList;
char	*userName = 0;

int
is_valid_filter_list (char *s)
// this function does a quick and dirty validation of filter lists 
// given on the command line
{
	while (*s)
	{
		if (!isalnum (*s) && (*s != ' ') && (*s != ','))
			return 0;
		s++;
	}
	return 1;
}

int
filter_string (char *s, char *l)
// this function returns true iff the string 's' is in the list 'l'
{
	char *tok;
	char list[100];

	//todo:eliminate arbitrary length restriction
	strncpy (list, l, sizeof (list)); 

	// iter through each token in the list and see if it is 
	// equal to 's'
	tok = strtok (list, delimiters);
	while (tok)
	{
//		fprintf (stderr, "*%s*%s*\n", s, tok);
		if (strcmp (s, tok) == 0)
			return 1;
		tok = strtok (0, delimiters);
	}	
	return 0;	
}

int
filter_number (int i, char *l)
// this function returns true iff the integer 'i' is in the list 'l'
{
	char buffer[100];

	// get filter_string to do the dirty work
	itoa (i, buffer, 10);
//	fprintf (stderr, "!%s!%s!\n", buffer, l);
	return filter_string (buffer, l);
}

int 
filter_ps (struct _ps *psp)
// this function handles all of the filtering for all lists; it returns
// true if the given process passes at least 1 filter
{
	// check to see if there are any tests in the first place
	if (allProcs)
		return 1;
	
if(psp->sid == 1) psp->pflags |= _NTO_PF_NOCTTY;	// @@@ Temp until new procs are used
	if (termProcs && (psp->pflags & (_NTO_PF_NOCTTY | _NTO_PF_SLEADER)) == 0)
		return 1;

	if (sessionProcs && (psp->pflags & _NTO_PF_NOCTTY) == 0)
		return 1;

	if (userList)
	{
		// try to match by user name or uid 
		if (filter_number (userEffective ? psp->user : psp->ruser, userList))
			return 1;
	
		if (userName && filter_string (userName, userList))
			return 1;
	}

	// if groupSession != 0, must check session leader's group instead
	if (groupList && filter_number (psp->rgroup, groupList))
		return 1;
	if (procList && filter_number (psp->pid, procList))
		return 1;
	if (termList && filter_number (psp->tty, termList))
		return 1;

	if(!termProcs && !sessionProcs && !userList && !groupList && !procList && !termList) {
		if((psp->pflags & _NTO_PF_NOCTTY) == 0 && psp->user == getuid()) // And tty matches the invokers tty
			return 1;
	}

	return 0;
}

