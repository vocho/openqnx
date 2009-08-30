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




#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/conf.h>

void _conf_destroy(long *list) {
	long				*head;
	long				item;

	if((head = list)) {
		while((item = *list++) != _CONF_END) {
			switch(item & _CONF_CMD_MASK) {
			case _CONF_VALUE:
				if((item & _CONF_NAME_MASK) == _CONF_NAME_LONG) {
					list++;
				}
				if(item & _CONF_STR) {
					free((void *)*list);
				}
				/* Fall through */
			case _CONF_CALL:
			case _CONF_LINK:
				list++;
				break;
			default:
				break;
			}
		}
	}
	free(head);
}
                                   

__SRCVERSION("_conf_destroy.c $Rev: 153052 $");
