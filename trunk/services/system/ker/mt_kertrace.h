/*
 * services/system/public/sys/mt_kertrace.h

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/

#ifndef __mt_kertrace_h__
#define __mt_kertrace_h__

/* _mt_LTT_TRACES_ enables probes when defined, every probe deactivated without it
 * (only non-probe traces like task_periodicity and dummy_check are available without that definition)
 * it should be renamed to _mt_PROBES_ENABLE, it is not so yet for historic reasons */
#define _mt_LTT_TRACES_
/* _mt_TRACE_DEBUG_ enables debug traces when defined. Put a debug probe right before a real probe.
 * The debug probe should have as parameter the name of the function
 * so that you can see which function emitted the following trace. */
#define _mt_TRACE_DEBUG_

/* mt_TRACE_DEBUG (without '_') allows easier debug tracing.
 * When _mt_TRACE_DEBUG_ is not defined it does nothing. */
#ifdef _mt_TRACE_DEBUG_
#define mt_TRACE_DEBUG(txt) \
	mt_trace_debug(__func__, __LINE__, txt)
#else
#define mt_TRACE_DEBUG(txt)
#endif


#include <externs.h>
#include <process.h>
#include <stdint.h>	/* for types like uint32_t */
#include <sys/types.h>
#include <sys/mt_trace.h>

/* mt_TRACESET_SIZE is (obviously) the size of ONE traceset.
 * There are as many tracesets as mt_TRACESETS_PER_CPU * mt_CPUS.
 * A traceset contains a "ltt_subbuffer_header" and traces (first one with id 29).
 * We allocate for each traceset a data_ctrl structure to manage data inside the traceset.
 *
 * A ltt_subbuffer_header is 68 B.
 * A trace is 4 + trace_size (0 to 16 till now) B
 * Header26 adds 16 B
 * Header31 adds  4 B
 *
 * num_traces = ( mt_TRACESET_SIZE - (ltt_subbuffer_header = 68) - (header16 = 16) ) / trace_average
 * mt_TRACESET_SIZE = (ltt_subbuffer_header = 68) + (header16 = 16) + num_traces * trace_average
 *
 * The trace buffer containes first all the data _MT_CPUScontrol structures, and next the tracesets.
 * Tracesets are in the same order as their corresponding data control structures.
 */

#define _MT_ALIGNMENT 		0 /* static value for alignment in traces (all of them) */

typedef struct mt_ker_info
{
	mt_data_ctrl_t *in_use;
	//mt_data_ctrl_t *next;
	mt_data_ctrl_t *first;
	mt_data_ctrl_t *last;
} mt_ker_info_t;

typedef struct mt_trace_filters
{
	char	task;
	char	sem;
	char	irq;
	char	debug;
} mt_trace_filters_t;

/*	function prototypes	*/
/* init and format functions */
int mt_buffer_init		(mt_data_ctrl_t *ptdc);
int mt_new_buffer_init	(mt_data_ctrl_t *ptdc, int remaining);
int mt_buffer_init		(mt_data_ctrl_t *ptdc);
void * mt_write_id		(void *pt0, unsigned short id, uint64_t clk_cycles);
void * mt_write_id29	(void *pt0, unsigned short id, unsigned size, uint64_t clk_cycles);
void * mt_write_id31	(void *pt0, unsigned short id, uint64_t clk_cycles);
void * mt_write_header	(void *pt0);

void mt_filter_tracing_debug(char val);
void mt_list_task_info();
void mt_send_flush_pulse		();

/* tracing functions */
void mt_trace_dummy_check		();
void mt_trace_task_create		(unsigned pid, unsigned tid, unsigned char priority);
void mt_trace_task_suspend		(unsigned pid, unsigned tid);
void mt_trace_task_resume		(unsigned pid, unsigned tid);
void mt_trace_task_delete		(unsigned pid, unsigned tid, int status);
void mt_trace_task_periodicity	(unsigned pid, unsigned tid, unsigned long long period);
void mt_trace_task_priority		(unsigned pid, unsigned tid, unsigned char priority);
void mt_trace_task_info			(unsigned pid, unsigned tid, unsigned char state, unsigned char priority);
void mt_trace_task_sig_return	(unsigned pid, unsigned tid);
void mt_trace_sem_init			(void *sem, int value, unsigned pid, unsigned tid);
void mt_trace_sem_P				(void *sem, int value, unsigned pid, unsigned tid);
void mt_trace_sem_V				(void *sem, int value, unsigned pid, unsigned tid);
void mt_trace_mutex_init		(void *mutex, unsigned pid, unsigned tid);
void mt_trace_mutex_lock		(void *mutex, unsigned pid, unsigned tid, unsigned owner);
void mt_trace_mutex_unlock		(void *mutex, unsigned pid, unsigned tid, unsigned owner);
void mt_trace_sync_destroy		(void *sem, int value, unsigned pid, unsigned tid);

void rdecl mt_trace_irq_entry			(unsigned irq);
void rdecl mt_trace_irq_exit			(unsigned irq);
void rdecl mt_trace_irq_handler_entry	(unsigned irq);
void rdecl mt_trace_irq_handler_exit	(unsigned irq);
void rdecl mt_trace_irq_timer_entry	();
void rdecl mt_trace_irq_timer_exit	();
#if 1
void mt_trace_var_debug			(unsigned v1, unsigned v2, unsigned *p);
#endif
void mt_trace_debug				(const char *func, unsigned line, const char *txt);


/* this is the new 2-buffers version
 * This macro contains the top part of a probe/tracing function.
 * All non-specific code is provided by the mt_TRACE_FUNK_top and mt_TRACE_FUNK_bottom.
 * In-between are the probe-specific values assignment.
 */
#define mt_TRACE_FUNK_top(id, size) \
	if(mt_tracebuf_addr == NULL) \
		return; \
	\
	uint64_t clk_cycles = ClockCycles(); \
	\
	void					*pt0; \
	mt_data_ctrl_t		*ptdc; \
	\
	ptdc = mt_buf_info.in_use; \
	/*lock_kernel();*/ \
	if (ptdc->status == 0)	/* likely, filling */ \
		/*ptdc->data_current = mt_write_id(ptdc->data_current, id);*/ \
		ptdc->data_current = ((clk_cycles - last_clk_cycles < 0x07FFFFFF) ? \
				mt_write_id(ptdc->data_current, id, clk_cycles) : \
				mt_write_id29(ptdc->data_current, id, size, clk_cycles)); \
	else { \
		if (ptdc->status == -1) { \
			/* traceset is empty (subbuffer header required) */ \
			ptdc->data_current = mt_write_header(ptdc->buf_begin); \
			/* traceset is empty (header 29 required) */ \
			ptdc->data_current = mt_write_id29(ptdc->data_current, id, size, clk_cycles); \
			ptdc->status = 0; \
		} else { /* not filling and not empty, guess what, it must be full */ \
			kprintf("SubBuffer full: Tracing event lost\n"); \
			/* not tested ((ltt_subbuffer_header_t *) ptdc->buf_begin)->events_lost++;*/ \
			return; /* could switch to next traceset for next trace...*/ /* should add lost events counter */ \
		} \
	} \
	last_clk_cycles = clk_cycles; \
	pt0 = ptdc->data_current

/* here in between are the trace-specific affectations
 * the tracing function must set the pt0 void pointer to next writable byte
 */

#define mt_TRACE_FUNK_bottom \
	/* update current pointer position*/ \
	ptdc->data_current = pt0; \
	 \
	/* check fullness */ \
	if (pt0 >= ptdc->buf_lim) { \
		/* traceset is full */ \
		ptdc->status = 1; \
		/* switch to "next" traceset */ \
		if (mt_buf_info.in_use == mt_buf_info.last) \
			mt_buf_info.in_use = mt_buf_info.first; \
		else \
			mt_buf_info.in_use = mt_buf_info.last; /* TODO: change to +1 */ \
		\
		/* send signal... */ \
		mt_send_flush_pulse(); \
 	} \
	/*unlock_kernel();*/ \
	return


struct mt_event_header {
	unsigned 	id			: 5;
	unsigned 	timestamp	: 27;
};

struct mt_event_id29 {
	uint16_t	id;
	uint16_t	smallsize;
	uint32_t	fullsize;
	uint64_t	timestamp;
};

#endif /* __mt_kertrace_h__ */
