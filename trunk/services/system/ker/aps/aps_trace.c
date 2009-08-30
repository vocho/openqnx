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



/* aps_trace.c 
 * 
 * Adaptive Partitioning scheduling 
 *
 * mostly just a placeholder for now. Eventual home of aps trace functions that are currently in 
 * nano_trace.c 
 * 
 */
#include "aps_data.h" 
#include "proto_aps.h"




/* trace out intial budgets and names */ 
void
sched_trace_initial_parms_ppg() {
	int i; 
	PPG	*ppg;
	for (i=0;i<num_ppg;i++){
		ppg = actives_ppg[i];
		trace_emit_sys_aps_name(ppg->dpp.id, ppg->name);
		trace_emit_sys_aps_budgets(ppg->dpp.id, ppg->budget_in_percent, ppg->critical_budget_in_cycles/cycles_per_ms); 

	}
}


__SRCVERSION("aps_trace.c $Rev: 153052 $"); 

