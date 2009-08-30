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



#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include "struct.h"


enum {
	ATTR_ARGV0,
	ATTR_CPU,
	ATTR_DEBUG,
	ATTR_EXTERNAL,
	ATTR_PRI,
	ATTR_SESSION,
	ATTR_EXTSCHED_APS,
};

static struct attr_types attr_table[] ={
	{ "argv0=",		ATTR_ARGV0 },
	{ "cpu=",		ATTR_CPU },
	{ "debug",		ATTR_DEBUG },
	{ "external",	ATTR_EXTERNAL },
	{ "pri=",		ATTR_PRI },
	{ "session",	ATTR_SESSION },
	{ "sched_aps=",	ATTR_EXTSCHED_APS },
	{ NULL }
};


void parse_script_attr(int tokenc, char *tokenv[], struct attr_script_entry *attrp) {
	int		 i;
	int		ival;
	char	*sval;

	for(i = 0 ; i < tokenc ; ++i) {
		switch(decode_attr(1, attr_table, tokenv[i], &ival, &sval)) {
		case ATTR_ARGV0:
			attrp->argv0 = strdup(sval);
			break;
		case ATTR_CPU:
			if(*sval == '*') {
				attrp->flags &= ~SCRIPT_FLAGS_CPU_SET;
			} else {
				attrp->flags |= SCRIPT_FLAGS_CPU_SET;
				attrp->cpu = ival;
			}
			break;
		case ATTR_DEBUG:
			if(ival) {
				attrp->flags |= SCRIPT_FLAGS_KDEBUG;
			} else {
				attrp->flags &= ~SCRIPT_FLAGS_KDEBUG;
			}
			break;
		case ATTR_EXTERNAL:
			attrp->external = ival;
			break;
		case ATTR_PRI:
			attrp->policy = SCRIPT_POLICY_NOCHANGE;
			if(*sval == '*') {
				attrp->flags &= ~SCRIPT_FLAGS_SCHED_SET;
			} else {
				attrp->flags |= SCRIPT_FLAGS_SCHED_SET;
				attrp->priority = ival;
				switch(sval[strcspn(sval, "fFrRoO")]) {
				case 'f':
				case 'F':
					attrp->policy = SCRIPT_POLICY_FIFO;
					break;
				case 'r':
				case 'R':
					attrp->policy = SCRIPT_POLICY_RR;
					break;
				case 'o':
				case 'O':
					attrp->policy = SCRIPT_POLICY_OTHER;
					break;
				}
			}
			break;
		case ATTR_SESSION:
			if(ival) {
				attrp->flags |= SCRIPT_FLAGS_SESSION;
			} else {
				attrp->flags &= ~SCRIPT_FLAGS_SESSION;
			}
			break;
		case ATTR_EXTSCHED_APS:
			ext_sched = SCRIPT_SCHED_EXT_APS;
			if((ival = aps_lookup(sval)) != -1) {
				attrp->flags |= SCRIPT_FLAGS_EXTSCHED;
				attrp->extsched.aps.id = ival;
			} else {
				attrp->flags &= ~SCRIPT_FLAGS_EXTSCHED;
				memset(&attrp->extsched, 0, sizeof(attrp->extsched));
			}
			break;
		}
	}
}



void parse_script_init(struct attr_script_entry *attrp) {

	attrp->policy = SCRIPT_POLICY_NOCHANGE;
	memset(&attrp->extsched, 0, sizeof(attrp->extsched));
}

__SRCVERSION("parse_script_attr.c $Rev: 153052 $");
