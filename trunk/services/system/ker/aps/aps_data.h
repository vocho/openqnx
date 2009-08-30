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



/* 
 * aps_data.h 
 * 
 * Adaptive Partitioning scheduling 
 *
 * Types and data defintions
 * 	- overall control
 * 	- partitions 
 *
 * Terminolgy note: partitions were originally called "purpose groups", 
 * hence the proponderance of variables like ppg. "group" and "partition" 
 * are interchangable terms. 
 * 
 */
#ifndef aps_data_included
#define aps_data_included 

/*
 * Want SMP versions of the RUNCPU/KERCPU macros. But not on SH because WANT_SMP_MACROS invokes a 
 * less effieicnt implementation of INTR_LOCK. (And SH doesnt support SMP anyway.) 
 * 
 */

#define WANT_SMP_MACROS

#include "externs.h"
#include <sys/sched_aps.h>
#include "aps_crit.h"



/*
 * PPG are arranged in an array from 0 to APS_MAX_PARTITIONS, from <sys/sched_aps.h>.
 * Each ppg maintains its own stats on history (microbilling),
 * its own dispatch queue and limits.
 */


/*
 * Accounting for time used is done with a sliding average of length windowsize 
 */ 
#define DEFAULT_WINDOWSIZE_MS 100 
#define MAX_WINDOWSIZE_MS     400 
#define MIN_WINDOWSIZE_MS       8


/* all timing calculations are done in ClockCyles() divided by a constant factor to control the number of signifcant
 * bits used. All variables with '_cycles' in their names nave been conditioned. Variables with _raw in their names
 * are not conditioned */

extern	uint32_t	windowsize_in_ms; 
extern	uint64_t	windowsize_in_cycles;
extern	int		bmp_support_factor; // initialized to 1; 

/* the averaging window is divided into buckets, each 1 tick long */ 
extern	int		ts_buckets;				/* number of buckets in the sliding average windows: usage_hist[],
							critical_usage_hist[], and idle_hist[].  */ 
#define MAX_TS_BUCKETS	MAX_WINDOWSIZE_MS / (_NTO_TICKSIZE_FAST / 1000000) /* note: ticksize is in nsec */ 
#define MIN_TS_BUCKETS	2	/* need at least two slots for a rotating window */ 
extern	uint64_t	cycles_per_ms;   
extern	uint64_t	cycles_per_bucket;


/*
 * Budgets are counted in the natural measurement units
 * for micro-billing, e.g. ClockCycles() that maybe scaled by a constant.
 */


/* ppg_entry defines the main partition data strucure. It is an extenstion of object.h:DISPATCH */ 
#define APS_MAX_PARTITIONS 8


struct ppg_entry {
	/* constant properties */
	DISPATCH	dpp; 	/* must be first field */ 
	int		max_prio;
	int		high_prio;
	int		budget_in_percent;
	int32_t		bmp_budget_in_cycles; 			/* is bmp_support_factor times actual budget. Same as std_budget
														   for non-SMP or SMP without BMP_SAFETY */ 
	int32_t		std_budget_in_cycles;			/* is NUM_PROCESSORS *budget_percent * windowize  */ 
	uint32_t	critical_budget_in_cycles;
	uint32_t	relative_fraction_used_factor; 		/* see set_factors() and RELATIVE_FRACTION_USED() */
	uint32_t	relative_fraction_used_offset;		/* see set_factors() and RELATIVE_FRACTION_USED() */ 
	bankruptcy_info bnkr_info;
	/* changes with scheduling */ 
	uint32_t	used_cycles;				/* may be over budget */
	uint32_t	used_cycles_lasttick;			/* as of last tick, for reporting purposes */ 
	uint32_t	critical_cycles_used; 
	uint32_t	usage_hist[MAX_TS_BUCKETS];		/* sliding averaging window for cpu time */
	uint32_t	critical_usage_hist[MAX_TS_BUCKETS];	/* same, but for time spent in "critical"mode */
	uint32_t	state_flags;				/* or-ing of PPG_STATE_* below */
	/* other constant properties */
	char		name[APS_PARTITION_NAME_LENGTH+1];
	struct ppg_entry	*parent; 
	struct sigevent		bankruptcy_notifier;
	struct sigevent		overload_notifier;
	int32_t			notify_pid;
	int32_t			notify_tid;
};

#define PPG_STATE_WAS_BANKRUPT 		0x00000001		/* bankruptcy was detected */ 



/* each bucket in usage_hist[] is 1 tick wide */ 

typedef struct ppg_entry PPG;

extern	PPG	*actives_ppg[APS_MAX_PARTITIONS];

/* Default partition (partition 0) */
extern	PPG	*system_ppg;

extern	int	num_ppg;	    /* the current number of PPG structures allocated */	


/* idle time is accumulated in it's own sliding window. Also indexed by curr_hist_index */
extern	int		idle_hist[MAX_TS_BUCKETS];
extern	uint32_t	idle_cycles;
extern	uint32_t	idle_cycles_lasttick; /* as of the last tick for reporting purposes */ 
#define PERCENT_SYSTEM_IDLE ((uint64_t)idle_cycles*(uint64_t)100)/windowsize_in_cycles)
/* /idle time */ 



/* enhanced reporting 
 *
 * 10 window and 100 windows averages (nominally 1 and 10 seconds)
 *
 * not used for scheduling
 *
 */ 

// make window2 equal to 10 scheduling windows 
#define MAX_W2_BUCKETS 10
// make window3 equal to 100 scheduling windows 
#define MAX_W3_BUCKETS 10

extern	int	hist_index_w2; 	/* cyclical index into _w2 tables */
extern	int	hist_index_w3; 	/* cyclical index into _w3 tables */



typedef struct { 
		/* nominal 1 second reporting window  */ 
		uint64_t	usage_w2;	/* sum of usage_hist_w2, total cpu time spent in w2 window */ 
		uint64_t	critical_usage_w2;	/* sum of critical_hist_w2, total time spent critical in w2 window */ 
		uint32_t	usage_hist_w2[MAX_W2_BUCKETS]; 	 
		uint32_t	critical_hist_w2[MAX_W2_BUCKETS];
		/* nominal 10 second reporting window */ 
		uint64_t	usage_w3;	/* sum of usage_hist_w3, total cpu time spent in w2 window */ 
		uint64_t	critical_usage_w3;	/* sum of critical_hist_w3, total time spent critical in w3 window */ 
		uint64_t	usage_hist_w3[MAX_W3_BUCKETS]; 
		uint64_t	critical_hist_w3[MAX_W3_BUCKETS];
} long_reporting_window; 

/* Long reporting windows are in an array parallel actives_ppg. They're separate to avoid disturbing the caching
 * behavior of actives_ppg */ 
extern	long_reporting_window *actives_lrw[APS_MAX_PARTITIONS]; 


extern	uint32_t	idle_hist_w2[MAX_W2_BUCKETS];	/* time spend idle during window2, nominally last second */ 
extern	uint64_t	idle_cycles_w2;	/* total time spent idle in w2 window */ 
extern	uint64_t	idle_hist_w3[MAX_W3_BUCKETS]; /* time spent idle during window3, nominally last 10 seconds */ 
extern	uint64_t	idle_cycles_w3;	/* total time spent idle in w3 window */ 


extern	uint32_t	zerobudget_ap_set; /* a bitmap of which partitions have budgets ==0 */ 

/* overall controls */ 
extern	uint32_t	aps_security;	/* set of SCHED_APS_SEC_* flags from sys/sched_aps.h */ 


/* functions to init key bits of data: 
   (real definitions are in proto_aps.h) 

   
void zero_idle_cycles();  


* must zero the lrws whever the scheduling window changes size *
void zero_lrw(int ap);   

 * reinit_window_parms recalculates all dynamic scheduling parameters. It also clears all scheduling
 * history  
int reinit_window_parms(int new_windowsize_ms); 


 * bmp_support_factor configures data for BMP. Note call reinit_window_parms() after use * 
void set_bmp_support_factor(); 


 * create_default_dispatch creates the system parition. * 
DISPATCH *create_default_dispatch(void);
*/
#endif


__SRCVERSION("aps_data.h $Rev: 153052 $"); 

