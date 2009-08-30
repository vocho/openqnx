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




#include <sys/neutrino.h>

int MsgSendAsync(int coid) {

	return MsgSend(coid, NULL, 0, NULL, 0);

}

int MsgSendAsyncGbl(int coid, const void *smsg, size_t sbytes, unsigned msg_prio) {

	return MsgSend(coid, smsg, sbytes, NULL, msg_prio);

}


__SRCVERSION("msgsendasync.c $Rev: 153052 $");
