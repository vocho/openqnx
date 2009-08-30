/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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


#include <search.h>
#include <stdlib.h>

struct qelem {
	struct qelem  *q_forw;
	struct qelem  *q_back;
};


void
insque(void *element, void *pred)
{
	struct qelem	*e, *p;

	e = element;
	p = pred;

	if ((e->q_back = p) == NULL) {
		e->q_forw = NULL;
	}
	else {
		if ((e->q_forw = p->q_forw) != NULL) {
			e->q_forw->q_back = e;
		}
		p->q_forw = e;
	}
}

void
remque(void *element)
{
	struct qelem	*e;

	e = element;

	if (e->q_back != NULL) {
		e->q_back->q_forw = e->q_forw;
	}

	if (e->q_forw != NULL) {
		e->q_forw->q_back = e->q_back;
	}

}
