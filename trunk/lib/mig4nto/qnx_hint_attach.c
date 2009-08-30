/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * qnx_hint_attach.c - QNX 4 to QNX Neutrino migration functions
 *
 * QNX Neutrino interrupt handlers can return with an event that
 * contains a pulse.  QNX 4 interrupt handlers can return a proxy.
 *
 * In order for this to still work, we install our own QNX Neutrino
 * interrupt handler that calls the user's interrupt handler.  So when
 * the interrupt is genertated, it is this hidden handler which is
 * called.  The hidden handler calls the user's handler.  If the user
 * handler returns with a proxy (non-zero value) then the hidden
 * handler returns a pulse event.  The proxy value is stuffed into
 * the event.
 *
 * The Receive*() migration function watches for this pulse event.
 * When it receives it, it pulls the proxy value from the pulse
 * message and returns with it.
 */

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <sys/neutrino.h>
#include <malloc.h>
#include <sys/netmgr.h>
#include <mig4nto.h>
#include <mig4nto_table.h>

typedef struct {
	int   int_id;				/* Interrupt ID                         */  
	pid_t (* handler)(void);	/* Pointer to QNX4.2X interrupt handler */
	struct sigevent event;		/* Event structure for proxy            */
} handler_info_t;

TABLE_T handler_info_table;		/* Table used for storing handlers      */

extern magic_t	magic;			/* defined in mig4nto.h                 */

static const struct sigevent *neutrino_handler(void *data, int id);

/*
 *  qnx_hint_attach
 *
 *  ds - The ds parameter is ignored.
 */
int
qnx_hint_attach(unsigned intnum, pid_t (* handler)(void), unsigned ds)
{
	handler_info_t	*int_info;
		
	if (ThreadCtl(_NTO_TCTL_IO, 0) == -1)
		return -1;

	if ((int_info = (handler_info_t *) calloc(1, sizeof(handler_info_t))) == NULL)
		return -1;

	int_info->handler = handler;
		
	SIGEV_PULSE_INIT(&int_info->event,
		ConnectAttach(ND_LOCAL_NODE, 0, magic.ipc_chid, _NTO_SIDE_CHANNEL, 0),
		getprio(0), PROXY_CODE, 0);

	if (!add_record(&handler_info_table, (void*) int_info)) {
		ConnectDetach(int_info->event.sigev_coid);
		free(int_info);
		return -1;
	}
	
	if ((int_info->int_id = InterruptAttach(intnum, neutrino_handler, 
							 	int_info, sizeof(handler_info_t *), 
							 	_NTO_INTR_FLAGS_TRK_MSK)) == -1) {
		delete_record(&handler_info_table, int_info, 0L);
		ConnectDetach(int_info->event.sigev_coid);
		free(int_info);
		return -1;
	}

	return int_info->int_id;
}

static const struct sigevent *
neutrino_handler(void *data, int id)
{
	pid_t			proxy;
	handler_info_t	*int_info;
	struct sigevent	*event = NULL;

	if (data) {
		int_info = (handler_info_t *) data;

		/* 
		 * now call the original QNX 4 interrupt handler and analyze it's
		 * return code to determine if we need to notify the process with
		 * a proxy
		 */
		proxy = int_info->handler();

		if (proxy > 0) {
			/* 
			 * the QNX 4 interrupt handler is returned a proxy, so we need
			 * to setup an event to be returned to the process/thread
			 */
			int_info->event.sigev_value.sival_int = proxy; 
			event = &int_info->event;
		}
	}
	return(event);
}

int
qnx_hint_detach(int id)
{
	handler_info_t	*int_info = NULL;
	
	if (InterruptDetach(id) == -1)
		return -1;

	if ((int_info = (handler_info_t *) calloc(1, sizeof(handler_info_t))) == NULL)
		return -1;

	/* Stuff the interrupt ID in so we can search the table for it*/
	int_info->int_id = id;

	if (!delete_record(&handler_info_table, int_info, 0L)) {
		free(int_info);
		errno = EINVAL;
		return -1;
	}    

	return EOK;
}

int
compare(const void *p1, const void *p2)
{                       
	handler_info_t	**ptr2 = (handler_info_t **) p2;  
	handler_info_t	*pp2;      
	
	handler_info_t	**ptr1 = (handler_info_t **) p1; 
	handler_info_t	*pp1;        
	
	pp2 = *ptr2;
	pp1 = *ptr1;
	
	if (pp1->int_id < pp2->int_id)
		return -1;
	if (pp1->int_id == pp2->int_id)  
		return 0;                                         

	return 1;
}

void
qnx_hint_table_init(void)
{
	init_table(&handler_info_table, sizeof(handler_info_t), compare, 0);
}
