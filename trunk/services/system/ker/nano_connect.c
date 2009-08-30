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

#include "externs.h"

void rdecl
connect_detach(CONNECT *cop, int prio) {
	CHANNEL	*chp;

	if((cop->flags & COF_VCONNECT) == 0) {
		// Inform the server that the connection has been dropped via a pulse.
		if((chp = cop->channel)) {
			if(chp->flags & _NTO_CHF_DISCONNECT) {
				// Tell the server.
				_TRACE_COMM_EMIT_SPULSE_DIS(cop, cop->scoid|_NTO_SIDE_CHANNEL, prio|_PULSE_PRIO_BOOST);
				pulse_deliver(chp, prio|_PULSE_PRIO_BOOST,
						_PULSE_CODE_DISCONNECT, 0, cop->scoid | _NTO_SIDE_CHANNEL, 0);
				// The client is no longer associated with the connection.
				// Setting process to NULL will allow lookup_rcvid()
				// to fail if further operations are attempted on the rcvid.
				cop->process = 0;
				return;
			} else {
				// Remove the connection from the servers channels vector.
				vector_rem(&chp->process->chancons, cop->scoid);
				if (chp->flags & _NTO_CHF_ASYNC) {
					LINK1_REM( *(CONNECT **)(&((CHANNELASYNC *)chp)->ch.reply_queue), cop, CONNECT);
				}
			}
		}
	} else {
		LINK1_REM(cop->un.lcl.cop->next, cop, CONNECT);
		if(--cop->un.lcl.cop->links == 0) {
			connect_detach(cop->un.lcl.cop, prio);
		}
	}

	// Release the connection back to the free pool.
	object_free(cop->process, &connect_souls, cop);
}



void rdecl connect_coid_disconnect(CONNECT *cop, CHANNEL *chp, int prio) {
	int		 i, flag = 0;
	CONNECT	*cop2;
	VECTOR	*vec;

	if((chp->flags & _NTO_CHF_COID_DISCONNECT) == 0)
		return;

	for(vec = &chp->process->fdcons;;) {
		for(i = 0 ; i < vec->nentries ; ++i) {
			if(VECP2(cop2, vec, i) == cop) {
				_TRACE_COMM_EMIT_SPULSE_DEA(cop, (-1), prio);
				pulse_deliver(chp, prio, _PULSE_CODE_COIDDEATH, i|flag, -1, 0 );
			}
		}

		if(vec == &chp->process->chancons) break;

		vec = &chp->process->chancons;
		flag = _NTO_SIDE_CHANNEL;
	}
}

__SRCVERSION("nano_connect.c $Rev: 169542 $");
