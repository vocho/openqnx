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

#define	_QUERY_SOULS                0           // Query soul vector, index1 as follows
	#define _QUERY_SOULS_PROCESS        0
	#define _QUERY_SOULS_THREAD         1
	#define _QUERY_SOULS_CHANNEL        2
	#define _QUERY_SOULS_CONNECT        3
	#define _QUERY_SOULS_PULSE          4
	#define _QUERY_SOULS_INTERRUPT      5
	#define _QUERY_SOULS_SYNC           6
	#define _QUERY_SOULS_SIGTABLE       7
	#define _QUERY_SOULS_TIMER          8
	#define _QUERY_SOULS_INTREVENT      9
	#define _QUERY_SOULS_FPU            10
	#define _QUERY_SOULS_CLIENT         11
	#define _QUERY_SOULS_CREDENTIAL     12
	#define _QUERY_SOULS_LIMITS         13
	#define _QUERY_SOULS_VTHREAD        14
	#define _QUERY_SOULS_NUM            15
#define _QUERY_PROCESS              1           // Query a process, subtype follows
	#define _QUERY_PROCESS_VECTOR       0       // Just return process vector
	#define _QUERY_PROCESS_CHANCONS     1
	#define _QUERY_PROCESS_FDCONS       2
	#define _QUERY_PROCESS_THREADS      3
	#define _QUERY_PROCESS_TIMERS       4
	#define _QUERY_PROCESS_SYNCS        5
#define _QUERY_INTERRUPT            2
#define _QUERY_VTHREAD              3
/* FIX ME - this subtype stuff doesn't seem to work without nano_query.c changes
#define _QUERY_PARTITION			4
*/
#define _QUERY_MEMORY_PARTITION		4
#define _QUERY_SCHEDULER_PARTITION	5

/* __SRCVERSION("query.h $Rev: 168445 $"); */
