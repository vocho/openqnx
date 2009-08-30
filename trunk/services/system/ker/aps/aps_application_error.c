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



/* aps_application_error.c 
 *
 * In the event the user options to handle a running out of critical budget, with the APS scheduler, by
 * causeing a restart, this module provides the error messages 
 */ 

#include "externs.h"


void aps_bankruptcy_crash(int ap, int pid, char *debug_name){ 
	kprintf("APsched: partition %d bankrupt running pid %d: %s\n. Crashing on user request. ", ap, pid, debug_name);
	crash();
}


__SRCVERSION("aps_application_error.c $Rev: 153052 $");

